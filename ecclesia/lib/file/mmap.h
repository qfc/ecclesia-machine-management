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

// Provides a standard convenient implementation for opening a file for memory
// mapped file access, particularly for file-backed memory mapped I/O.

#ifndef ECCLESIA_LIB_FILE_MMAP_H_
#define ECCLESIA_LIB_FILE_MMAP_H_

#include <cstddef>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"

namespace ecclesia {

class MappedMemory {
 public:
  // Given a filename, attempt to open it and create a memory mapping of a range
  // of the file. The exposed byte range will be [offset, offset+size). Returns
  // a null object if opening the file or creating the mapping fails.
  static absl::optional<MappedMemory> OpenReadOnly(const std::string &path,
                                                   size_t offset, size_t size);

  // This object cannot be shared so copying is not allowed. It can be moved
  // with ownership of the underlying mapping moving along with the file.
  //
  // Using a moved-from mapping will return empty spans and views.
  MappedMemory(const MappedMemory &) = delete;
  MappedMemory &operator=(const MappedMemory &) = delete;
  MappedMemory(MappedMemory &&other) : mapping_(other.mapping_) {
    other.mapping_ = {nullptr, 0, 0, 0, false};
  }
  MappedMemory &operator=(MappedMemory &&other) {
    mapping_ = other.mapping_;
    other.mapping_ = {nullptr, 0, 0, 0, false};
    return *this;
  }

  // Destorying the object will release the underlying mapping.
  ~MappedMemory();

  // Expose the mapped memory as a string view.
  absl::string_view MemoryAsStringView() const;

  // Exposed the mapped memory as a span. You can request either a read-only
  // span or a read-write span, but if the underlying mapping is not writable
  // then the read-write span will be empty.
  absl::Span<const char> MemoryAsReadOnlySpan() const;
  absl::Span<char> MemoryAsReadWriteSpan() const;

 private:
  // Construct a mapping object. This constructor is only used by the factory
  // functions for this object, which do all the validation of parameters.
  struct MmapInfo {
    // The address and size of the mapping. Set to null in moved-from mappings.
    void *addr;
    size_t size;
    // The offset and size of the subset of the mapping exposed to the user. The
    // actually underlying mapping will generally be larger (due to page
    // alignment) than the mapping requested by the user.
    size_t user_offset;
    size_t user_size;
    // Indicates if the mapping is writable or not.
    bool writable;
  };
  explicit MappedMemory(MmapInfo mapping);

  // All the information about the stored mapping.
  MmapInfo mapping_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_FILE_MMAP_H_
