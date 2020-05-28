/*
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This header provides the snapshot aspect of a generic shared data store that
// can be updated with Read-Copy-Update (RCU) style semantics.
//
// The class template provided by this header is the RcuSnapshot class. This
// represents a snapshot of data coming from an underlying RCU store. At a basic
// level, the snapshot acts much like a shared_ptr:
//   * it points at the underlying data, and does not store it internally
//   * it is reference counted, so that the data is still available and usable
//     even after the underlying store has switched to fresher data
//
// This also means that users of a snapshot should not hold onto it for an
// extended period of time, as holding onto the snapshot will keep the data
// alive.
//
// However, in addition to just being a shared_ptr-like object, the snapshot
// also provides facilities for checking if the data is still fresh and
// registering a notification object that will get set when the data becomes
// stale. These features are provided via the RcuNotification class.
//
// Given an existing RcuSnapshot, you can move or copy it just like you can with
// a shared_ptr. However, to construct a RcuSnapshot<T> from scratch is a bit
// more complicated as you must:
//   * use the RcuSnapshot<T>::Create factory
//   * pass all of the arguments that the T constructor would take
// The factory will construct both a snapshot and an invalidator for doing the
// snapshot invalidation.
//
// Note that this means that you cannot "rewrite" or modify the contents of the
// snapshot. The stored value is created an construction time, and can only
// be invalidated (never "validated"). When the data is updated, the creator
// of the snapshot should invalidate the existing one and then create a new
// snapshot instance.
//
//
// Notes on thread safety:
// In general, all of the (const) accessor functions provided by RcuSnapshot and
// RcuNotification are threadsafe. However, in general the mutating functions
// are not. Specifically:
//   * Calling RcuNotifciation::Reset and/or RcuSnapshot::RegisterNotification
//     from multiple threads is undefined behavior. The expectation is that a
//     single thread will create a notification, and be responsible for doing
//     all registration and resetting of it.
//   * Calling Notify from multiple threads is safe, but only if it's being
//     called from the invalidation code of an RcuSnapshot that it is registerd
//     with. Outside of those contexts it has the same limitations as Reset and
//     RegisterNotification.

#ifndef ECCLESIA_LIB_CACHE_RCU_SNAPSHOT_H_
#define ECCLESIA_LIB_CACHE_RCU_SNAPSHOT_H_

#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"

namespace ecclesia {

class RcuNotification {
 public:
  // Construct a notification. By default a new notification is untriggered
  // but you can also explicitly specify the triggered state.
  RcuNotification() : triggered_(false) {}
  explicit RcuNotification(bool triggered) : triggered_(triggered) {}

  // Snapshots keep pointers to notifications, so it isn't safe to copy them
  // or move them around.
  RcuNotification(const RcuNotification &other) = delete;
  RcuNotification &operator=(const RcuNotification &other) = delete;

  // Reset the notification on destruction to ensure that there are no
  // lingering references to it.
  ~RcuNotification() { Reset(); }

  // Check if a notification has been triggered.
  bool HasTriggered() const { return triggered_; }

  // Trigger the actual notification. The can be called more than once if a
  // notification is registered with multiple snapshots.
  void Notify() { triggered_ = true; }

  // Unregister the notification with any snapshots it is associated with and
  // reset it to an unfired state. This can be safely called on a notification
  // that has not been triggered.
  void Reset() {
    for (const auto &remove_from_snapshot : remove_from_snapshot_funcs_) {
      remove_from_snapshot();
    }
    remove_from_snapshot_funcs_.clear();
    triggered_ = false;
  }

 private:
  // The state of the trigger.
  std::atomic<bool> triggered_;

  // Functions for removing this notification from all of the snapshots it is
  // registered with. When called the function will remove the notification from
  // the snapshot. These functions work even if the snapshot is no longer live.
  //
  // The RcuSnapshot class is responsible for adding a function to this vector
  // when the notification is registered. In practice this is done by capturing
  // a weak_ptr to its internal data structure and promoting that weak_ptr to a
  // shared_ptr when called to do the removal. If the promotion fails then the
  // function is still successful since there was nothing to remove from.
  std::vector<std::function<void()>> remove_from_snapshot_funcs_;

  // When a notification is registered with an RcuSnapshot it will add itself
  // to the snapshots vector.
  template <typename T>
  friend class RcuSnapshot;
};

// Helper class used to expose the Invalidate function to the creator of the
// snapshot. When a snapshot is constructed a reference to this type must be
// provided and it will be populated with a pointer to the snapshot.
//
// This layer of indirection is used so that users of the snapshot are not
// able to invalidate it themselves. Otherwise we could just make Invalidate
// a part of the RcuSnapshot public interface.
class RcuInvalidator {
 public:
  // This type has default construction, copy and move semantics. The semantics
  // of passing around this object is equivalent to that of passing around a
  // function object.
  RcuInvalidator() {}
  RcuInvalidator(const RcuInvalidator &other) = default;
  RcuInvalidator &operator=(const RcuInvalidator &other) = default;

  // Invalidate the referenced snapshot.
  void InvalidateSnapshot() {
    if (invalidate_func_) invalidate_func_();
  }

 private:
  // The underlying function call to invalidate the snapshot. The RcuSnapshot
  // is responsible for populating this with something that will do the
  // invalidation.
  std::function<void()> invalidate_func_;

  // When an Invalidator is provided to an RcuSnapshot it will assign a copy
  // of its own internal pointer to data_ptr_.
  template <typename T>
  friend class RcuSnapshot;
};

template <typename T>
class RcuSnapshot {
 public:
  // Simple struct that combines a snapshot and invalidator.
  struct WithInvalidator {
    RcuSnapshot<T> snapshot;
    RcuInvalidator invalidator;
  };

  // Creates a snapshot plus an invalidator that can invalidate it.
  //
  // The arguments provided to the factory will be used to construct a value of
  // type T using perfect forwarding.
  template <typename... Args>
  static WithInvalidator Create(Args &&... args) {
    // Create the initial snapshot and an empty invalidator.
    WithInvalidator object = {
        .snapshot = RcuSnapshot<T>(std::forward<Args>(args)...),
    };
    // Populate the invalidator with a function that can invalidate the newly
    // created snapshot object. The function will be bound to a weak_ptr to the
    // snapshot's underlying data block.
    object.invalidator.invalidate_func_ = [weak_data_ptr = std::weak_ptr<Data>(
                                               object.snapshot.data_ptr_)]() {
      if (auto shared_data_ptr = weak_data_ptr.lock()) {
        absl::MutexLock ml(&shared_data_ptr->mutex);
        shared_data_ptr->fresh = false;
        // Trigger all of the notifications. After this we can clear the
        // notifications since they'll never be re-triggered.
        for (RcuNotification *notification : shared_data_ptr->notifications) {
          notification->Notify();
        }
        shared_data_ptr->notifications.clear();
      }
    };
    return object;
  }

  // The type is copyable and movable, just like a shared_ptr would be.
  RcuSnapshot(const RcuSnapshot &other) = default;
  RcuSnapshot &operator=(const RcuSnapshot &other) = default;
  RcuSnapshot(RcuSnapshot &&other) = default;
  RcuSnapshot &operator=(RcuSnapshot &&other) = default;

  // Implementation of the dereference operators. These dereference against the
  // underlying T object, not the Data object that wraps it.
  const T &operator*() const noexcept { return data_ptr_->value; }
  const T *operator->() const noexcept { return &data_ptr_->value; }

  // Implementation of comparison operators. This does comparisons on the
  // underlying stored pointers. Note that unlike shared_ptr we do not support
  // comparisons to nullptr or conversions to bool, as snapshots cannot be
  // created without being populated and so the snapshot is never "null".
  friend bool operator==(const RcuSnapshot &lhs, const RcuSnapshot &rhs) {
    return lhs.data_ptr_ == rhs.data_ptr_;
  }
  friend bool operator!=(const RcuSnapshot &lhs, const RcuSnapshot &rhs) {
    return lhs.data_ptr_ != rhs.data_ptr_;
  }

  // Indicate if the underlying data is still fresh. Note that the data being
  // stale doesn't necessarily indicate that it can't be used, just that the
  // underlying data store now has new data.
  bool IsFresh() const {
    absl::MutexLock ml(&data_ptr_->mutex);
    return data_ptr_->fresh;
  }

  // Register a notification to be triggered. Note that if the snapshot is
  // already invalid this may immediately trigger it.
  void RegisterNotification(RcuNotification &notification) {
    absl::MutexLock ml(&data_ptr_->mutex);
    if (data_ptr_->fresh) {
      // The data is still fresh, so actually register the notification. This
      // requires provding the notification with a lambda that can remove itself
      // from this snapshot.
      data_ptr_->notifications.push_back(&notification);
      notification.remove_from_snapshot_funcs_.push_back(
          [weak_data_ptr = std::weak_ptr<Data>(data_ptr_),
           notification = &notification]() {
            if (auto shared_data_ptr = weak_data_ptr.lock()) {
              absl::MutexLock ml(&shared_data_ptr->mutex);
              auto &v = shared_data_ptr->notifications;
              v.erase(std::remove(v.begin(), v.end(), notification), v.end());
            }
          });
    } else {
      // The data is already stale so just notify right away.
      notification.Notify();
    }
  }

 private:
  // Create the underlying snapshot object.
  template <typename... Args>
  explicit RcuSnapshot(Args &&... args)
      : data_ptr_(std::make_shared<Data>(std::forward<Args>(args)...)) {}

  // The underlying data stored in the snapshot. The core of this is the value
  // of type T, but there are also additional control structures used to track
  // the freshness of the data.
  struct Data {
    // Construct the data object. We need to keep forwarding the constructor
    // arguments that originally came from the RcuSnapshot constructor.
    template <typename... Args>
    Data(Args &&... args) : value(std::forward<Args>(args)...), fresh(true) {}

    // The underlying data value stored in the snapshot.
    T value;

    // A control block of additional metadata needed to handle notifications.
    absl::Mutex mutex;
    bool fresh ABSL_GUARDED_BY(mutex);
    std::vector<RcuNotification *> notifications ABSL_GUARDED_BY(mutex);
  };
  std::shared_ptr<Data> data_ptr_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_CACHE_RCU_SNAPSHOT_H_
