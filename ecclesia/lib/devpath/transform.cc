// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lib/devpath/transform.h"

#include <stddef.h>

#include <string>
#include <utility>
#include <vector>

#include "google/protobuf/field_mask.pb.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "google/protobuf/util/field_mask_util.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace ecclesia {
namespace {

// A slightly modified version of
// ::google::protobuf::util::FieldMaskUtil::GetFieldDescriptors() to allow diving into
// repeated sub-messages.
bool GetFieldDescriptors(
    const ::google::protobuf::Descriptor *descriptor, absl::string_view path,
    std::vector<const ::google::protobuf::FieldDescriptor *> *field_descriptors) {
  std::vector<std::string> parts = absl::StrSplit(path, '.');
  for (const std::string &field_name : parts) {
    if (descriptor == nullptr) {
      return false;
    }
    const ::google::protobuf::FieldDescriptor *field =
        descriptor->FindFieldByName(field_name);
    if (field == nullptr) {
      return false;
    }
    if (field_descriptors != nullptr) {
      field_descriptors->push_back(field);
    }
    if (field->cpp_type() == ::google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE) {
      descriptor = field->message_type();
    } else {
      descriptor = nullptr;
    }
  }
  return true;
}

// Represent a "path" of a field in a nested message. Consider the message type
//
// message A {
//   message B {
//     message C {
//       string c = 1;
//     }
//     repeated string b = 1;
//     repeated C cc = 2;
//   }
//   B bb = 1;
//   string a = 3;
// }
//
// Given a message of type A, the path to field b in B is denoted with ".bb.b",
// and the path to field c in C is denoted with ".bb.cc.c". The path to field a
// is simply ".a". There are two types of path "segments" in this scheme. The
// field at the end, which is not of message type, and the intermediate fields
// before it, which are all of message type (called "MsgPath" in this class).
// The former corresponds to the class member "field_", and the latter
// "msg_path_".
class FieldPath {
 public:
  // In a message of type `msg_type`, find all fields that is masked by
  // `path_mask` and construct a FieldPath object for them. If the path is not
  // a valid one this will return null.
  static absl::optional<FieldPath> FindPath(
      const ::google::protobuf::Descriptor *msg_type, const absl::string_view path_mask);

  bool HasField() const { return field_ != nullptr; }

  const ::google::protobuf::FieldDescriptor *field() const { return field_; }

  // Given a message, find the sub-messages that are denoted by the
  // msg path. Suppose the field path is ".bb.cc.c", calling its
  // GetMutableLeafMessages with a message of type A as root gives all
  // the message of type C that are nested in it.
  void GetMutableLeafMessages(::google::protobuf::Message *root,
                              std::vector<::google::protobuf::Message *> *result) const;
  // Return the string representation of the field path. Example: .bb.cc.c
  std::string AsString() const;

 private:
  // path: Msg Path prefix.
  //
  // desc_vec: Given a vector of field descriptors, set the field to its last
  // element, and append all elements before last to the msg path (after
  // `path`). This is used to construct a FieldPath from result of
  // google::protobuf::util::GetFieldDescriptors(), which does not distinguish between
  // field and msg path.
  //
  // Note that GetFieldDescriptors() only returns a nested path (with multiple
  // segments) if it is given a correct nested field mask. For example (using
  // the message type mentioned in the beginning) if the mask is simply "c,b",
  // GetFieldDescriptors() would not be able to find any fields given a message
  // of type A. However if the mask is "bb.b,bb.cc.c", it would be able to find
  // both fields.
  explicit FieldPath(std::vector<const ::google::protobuf::FieldDescriptor *> desc_vec)
      : msg_path_(std::move(desc_vec)), field_(msg_path_.back()) {
    msg_path_.pop_back();  // The last element is stored in field_.
  }

  void GetMutableLeafMessagesRecurse(
      ::google::protobuf::Message *root, size_t level,
      std::vector<::google::protobuf::Message *> *result) const;

  std::vector<const ::google::protobuf::FieldDescriptor *> msg_path_;
  const ::google::protobuf::FieldDescriptor *field_;
};

absl::optional<FieldPath> FieldPath::FindPath(
    const ::google::protobuf::Descriptor *msg_type, const absl::string_view path_mask) {
  std::vector<const ::google::protobuf::FieldDescriptor *> path;
  if (GetFieldDescriptors(msg_type, path_mask, &path)) {
    return FieldPath(std::move(path));
  }
  return absl::nullopt;
}

void FieldPath::GetMutableLeafMessages(
    ::google::protobuf::Message *root, std::vector<::google::protobuf::Message *> *result) const {
  return GetMutableLeafMessagesRecurse(root, 0, result);
}

void FieldPath::GetMutableLeafMessagesRecurse(
    ::google::protobuf::Message *root, size_t level,
    std::vector<::google::protobuf::Message *> *result) const {
  if (level == msg_path_.size()) {
    result->push_back(root);
    return;
  }

  const ::google::protobuf::Reflection *reflection = root->GetReflection();
  const auto *field = msg_path_[level];

  if (field->is_optional() && !(reflection->HasField(*root, field))) {
    return;
  }

  if (field->is_repeated()) {
    int repeat_count = reflection->FieldSize(*root, field);  // May be 0.
    for (int field_idx = 0; field_idx < repeat_count; field_idx++) {
      GetMutableLeafMessagesRecurse(
          reflection->MutableRepeatedMessage(root, field, field_idx), level + 1,
          result);
    }
  } else {
    GetMutableLeafMessagesRecurse(reflection->MutableMessage(root, field),
                                  level + 1, result);
  }
}

std::string FieldPath::AsString() const {
  std::string result;
  absl::StrAppend(&result, ".");
  for (const auto *seg : msg_path_) {
    absl::StrAppend(&result, seg->name(), ".");
  }
  if (HasField()) {
    absl::StrAppend(&result, field()->name());
  }
  return result;
}

bool TransformLeafDevpaths(const TransformDevpathFunction &transform,
                           const ::google::protobuf::FieldDescriptor *devpath_field,
                           const std::vector<::google::protobuf::Message *> &leaf_msgs) {
  for (auto *msg : leaf_msgs) {
    // msg contains the devpath field. Read the current value of the field,
    // apply the transformation function to it, and if that was successful then
    // write the new value into it. If the transform function fails then we give
    // up the whole process.
    const ::google::protobuf::Reflection *reflection = msg->GetReflection();
    if (devpath_field->label() == ::google::protobuf::FieldDescriptor::LABEL_REPEATED) {
      int repeat_count = reflection->FieldSize(*msg, devpath_field);

      for (int field_idx = 0; field_idx < repeat_count; field_idx++) {
        std::string old_devpath =
            reflection->GetRepeatedString(*msg, devpath_field, field_idx);
        absl::optional<std::string> new_devpath = transform(old_devpath);

        if (!new_devpath) return false;
        if (*new_devpath != old_devpath) {
          reflection->SetRepeatedString(msg, devpath_field, field_idx,
                                        std::move(*new_devpath));
        }
      }
    } else {
      std::string old_devpath = reflection->GetString(*msg, devpath_field);
      absl::optional<std::string> new_devpath = transform(old_devpath);
      if (!new_devpath) return false;
      if (*new_devpath != old_devpath) {
        reflection->SetString(msg, devpath_field, std::move(*new_devpath));
      }
    }
  }
  return true;
}

// Different versions of the protobuf library unfortunately use different types
// of string_view type objects (e.g. std vs absl vs custom) and not all of them
// are convertable between each other; in particular the custom ones cannot be
// implicitly constructed from our string_view type.
//
// In particular this is an issue for our use of FieldMaskUtil::FromString. To
// work around this we have a helper template here which will infer the type of
// its first parameter, and then we manually construct an object of that type
// from the string view and call it.
//
// If all versions of protobuf converge on std::string_view or absl::string_view
// then this workaround can be removed.
template <typename>
struct ArgExtractor;
template <typename T>
struct ArgExtractor<void(T, ::google::protobuf::FieldMask *)> {
  using type = T;
};
void CallFieldMaskUtilFromString(absl::string_view str,
                                 ::google::protobuf::FieldMask *out) {
  using StringViewType = typename ArgExtractor<decltype(
      ::google::protobuf::util::FieldMaskUtil::FromString)>::type;
  StringViewType converted_str(str.data(), str.size());
  ::google::protobuf::util::FieldMaskUtil::FromString(converted_str, out);
}

}  // namespace

bool TransformProtobufDevpaths(const TransformDevpathFunction &transform,
                               absl::string_view field_mask,
                               ::google::protobuf::Message *message) {
  ::google::protobuf::FieldMask mask_proto;
  CallFieldMaskUtilFromString(field_mask, &mask_proto);

  for (absl::string_view path : mask_proto.paths()) {
    // Extract the field descriptors for traversing the message. These will be
    // needed to read and write the devpath field itself. If the path is invalid
    // this will fail and we give up the transformation.
    absl::optional<FieldPath> field_path =
        FieldPath::FindPath(message->GetDescriptor(), path);
    if (!field_path) return false;

    if (field_path->field()->type() != ::google::protobuf::FieldDescriptor::TYPE_STRING) {
      return false;
    }

    std::vector<::google::protobuf::Message *> leaf_msgs;
    field_path->GetMutableLeafMessages(message, &leaf_msgs);
    if (!TransformLeafDevpaths(transform, field_path->field(), leaf_msgs)) {
      return false;
    }
  }

  // If we get here it means we successfully transformed every single field at
  // the specified paths. Return success.
  return true;
}

}  // namespace ecclesia
