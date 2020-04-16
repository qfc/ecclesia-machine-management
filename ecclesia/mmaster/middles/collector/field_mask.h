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

// Utilities for interacting with field masks. These are intended to help with
// implementing ResourceCollector::Query functions.
//
// Common Query pattern 1:
//   Querying the backend sends you multiple fields worth of data, but the field
//   mask may only request some of them.
//
//   In this case the easiest pattern is to simply populate any fields that you
//   need to have in the response message and then use TrimResponseMessage() to
//   remove any fields that were not requested.
//
//   This can waste some resources setting fields you will not be sending, but
//   it can save a lot of code complexity by not doing field-by-field checks.
//   Assuming that fetching the fields from the backend (which you cannot avoid)
//   is much more expensive than adding-then-trimming the fields from the
//   response then the overall resource impact is small.
//
//   Note that message trimming should always be done BEFORE attaching id or
//   status information to a query response, since these should always be
//   returned regardless of the mask specified.
//
//
// Common Query pattern 2:
//   You have a query that you will send to the backend that will fetch one or
//   more fields and you need to determine if the query is necessary.
//
//   In this case you want to check if the field mask overlaps with at least one
//   of the fields you will be fetching from the backend. In the single field
//   case this is fairly easy but in the multi-field case checking the field
//   mask by hand is a lot more work. To simplify this pattern the
//   DoesFieldMaskHaveAnyOf() function takes a list of one or more fields and
//   returns true if there is any overlap between the mask and the given fields.
//
//   This enables code that looks like:
//
//     if (DoesFieldMaskHaveAnyOf(field_mask, "x", "y", "z")) {
//       ... do backend call to fetch x, y and z ...
//       ... populate response with x, y, and z ...
//     }
//     if (DoesFieldMaskHaveAnyOf(field_mask, "a", "b")) {
//       ... do backend call to fetch a and b ...
//       ... populate response with a and b ...
//     }
//     TrimResponseMessage(field_mask, &response);
//
//   Most of the code can then be focused on doing collection from the backend
//   and translating it into the response, instead of spending a lot of lines of
//   code fiddling with field mask checks.

#ifndef ECCLESIA_MMASTER_MIDDLES_COLLECTOR_FIELD_MASK_H_
#define ECCLESIA_MMASTER_MIDDLES_COLLECTOR_FIELD_MASK_H_

#include <iterator>

#include "google/protobuf/field_mask.proto.h"
#include "net/proto2/public/message.h"
#include "absl/strings/string_view.h"

namespace ecclesia {

// Given a field mask an a response proto, trim the contents of the response
// down to only the fields specified by the mask.
void TrimResponseMessage(const ::google::protobuf::FieldMask &mask,
                         ::proto2::Message *response);

// Given a field mask and one or more paths, return true if one or more of the
// paths are matched by the mask, and false otherwise.
//
// There are two ways of specifying the paths:
//   - via another FieldMask, which is implicitly a set of paths
//   - via a variadic list of string views, each of which is a path
// When you have a fixed, known set of paths you want to check you usually want
// to use the literal string version.
bool DoesFieldMaskHaveAnyOf(const ::google::protobuf::FieldMask &mask,
                            const ::google::protobuf::FieldMask &requested);
bool DoesFieldMaskHaveAnyOf(const ::google::protobuf::FieldMask &mask,
                            absl::string_view field);
template <typename... Args>
bool DoesFieldMaskHaveAnyOf(const ::google::protobuf::FieldMask &mask,
                            absl::string_view field1, absl::string_view field2,
                            Args... args) {
  // This template has two fixed string_view arguments so that it is only
  // used if two or more strings are passed as parameters. This allows a simpler
  // and more efficient implementation in the single-param case. However, this
  // should be considered an implementation detail, not part of the API.
  //
  // It also jams all of the arguments into an absl::string_view array to
  // effectively "force" all of the args to be string_views.
  absl::string_view fields[] = {field1, field2, args...};
  ::google::protobuf::FieldMask requested;
  requested.mutable_paths()->Reserve(std::size(fields));
  for (absl::string_view field : fields) {
    requested.add_paths(field);
  }
  return DoesFieldMaskHaveAnyOf(mask, requested);
}

}  // namespace ecclesia

#endif  // ECCLESIA_MMASTER_MIDDLES_COLLECTOR_FIELD_MASK_H_
