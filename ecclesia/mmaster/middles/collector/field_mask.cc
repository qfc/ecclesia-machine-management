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

#include "mmaster/middles/collector/field_mask.h"

#include "google/protobuf/field_mask.proto.h"
#include "net/proto2/public/message.h"
#include "net/proto2/util/public/field_mask_util.h"
#include "absl/strings/string_view.h"

namespace ecclesia {

void TrimResponseMessage(const ::google::protobuf::FieldMask &mask,
                         ::proto2::Message *response) {
  if (mask.paths_size() == 0) {
    response->Clear();
  } else {
    ::proto2::util::FieldMaskUtil::TrimMessage(mask, response);
  }
}

bool DoesFieldMaskHaveAnyOf(const ::google::protobuf::FieldMask &mask,
                            absl::string_view field) {
  return ::proto2::util::FieldMaskUtil::IsPathInFieldMask(field, mask);
}

bool DoesFieldMaskHaveAnyOf(const ::google::protobuf::FieldMask &mask,
                            const ::google::protobuf::FieldMask &requested) {
  ::google::protobuf::FieldMask overlap;
  ::proto2::util::FieldMaskUtil::Intersect(mask, requested, &overlap);
  return overlap.paths_size() > 0;
}

}  // namespace ecclesia
