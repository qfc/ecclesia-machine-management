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

// This header provides constants for converting between different byte units.
#ifndef ECCLESIA_LIB_TYPES_BYTES_H_
#define ECCLESIA_LIB_TYPES_BYTES_H_

#include <cstdint>

namespace ecclesia {

inline constexpr uint64_t kBytesInKiB = 1 << 10;
inline constexpr uint64_t kBytesInMiB = 1 << 20;
inline constexpr uint64_t kBytesInGiB = 1 << 30;

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_TYPES_BYTES_H_
