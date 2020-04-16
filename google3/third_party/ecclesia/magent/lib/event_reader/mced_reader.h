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

#ifndef ECCLESIA_MAGENT_LIB_EVENT_READER_MCED_READER_H_
#define ECCLESIA_MAGENT_LIB_EVENT_READER_MCED_READER_H_

#include <queue>
#include <string>
#include <thread>  // NOLINT(build/c++11)
#include <utility>

#include "absl/base/thread_annotations.h"
#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "absl/types/optional.h"
#include "absl/types/variant.h"
#include "magent/lib/event_reader/event_reader.h"

namespace ecclesia {

// A reader class to reading machine check exceptions from the mcedaemon
// (https://github.com/thockin/mcedaemon)
class McedaemonReader : public SystemEventReader {
 public:
  // Input is the path to the unix domain socket to talk to the mcedaemon
  explicit McedaemonReader(std::string mced_socket_path);

  absl::optional<SystemEventRecord> ReadEvent() override {
    absl::MutexLock l(&mces_lock_);
    if (mces_.empty()) return absl::nullopt;
    auto event = std::move(mces_.front());
    mces_.pop();
    return event;
  }

  ~McedaemonReader() {
    // Signal the reader loop to exit
    exit_loop_.Notify();
    reader_loop_.join();
  }

 private:
  // The reader loop. Polls for mces from the mcedaemon and logs them in mces_.
  // Runs as a seperate thread.
  void Loop();

  std::string mced_socket_path_;
  absl::Mutex mces_lock_;
  std::queue<SystemEventRecord> mces_ ABSL_GUARDED_BY(mces_lock_);
  absl::Notification exit_loop_;
  std::thread reader_loop_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_EVENT_READER_MCED_READER_H_
