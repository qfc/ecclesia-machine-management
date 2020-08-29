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

#include "ecclesia/lib/file/mmap.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstddef>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "ecclesia/lib/cleanup/cleanup.h"
#include "ecclesia/lib/logging/globals.h"
#include "ecclesia/lib/logging/posix.h"

namespace ecclesia {

absl::optional<MappedMemory> MappedMemory::MappedMemory::OpenReadOnly(
    const std::string &path, size_t offset, size_t size) {
  // Open the file for reading.
  const int fd = open(path.c_str(), O_RDONLY);
  if (fd == -1) {
    return absl::nullopt;
  }
  auto fd_closer = FdCloser(fd);
  // Determine the offset and length to use for the memory mapping, rounding
  // to an appropriate page size.
  size_t true_offset = offset - (offset % sysconf(_SC_PAGE_SIZE));
  size_t user_offset = offset - true_offset;
  size_t true_size = size + user_offset;
  // Create the memory mapping.
  void *addr =
      mmap(nullptr, true_size, PROT_READ, MAP_PRIVATE, fd, true_offset);
  if (addr == MAP_FAILED) {
    return absl::nullopt;
  }
  // We have a good mapping. Note that we no longer need to keep the fd open.
  return MappedMemory({addr, true_size, user_offset, size, false});
}

MappedMemory::~MappedMemory() {
  if (mapping_.addr) {
    if (munmap(mapping_.addr, mapping_.size) == -1) {
      PosixErrorLog() << "munmap() of " << mapping_.addr << ", "
                      << mapping_.size << " failed";
    }
  }
}

absl::string_view MappedMemory::MemoryAsStringView() const {
  return absl::string_view(
      static_cast<char *>(mapping_.addr) + mapping_.user_offset,
      mapping_.user_size);
}

absl::Span<const char> MappedMemory::MemoryAsReadOnlySpan() const {
  return absl::MakeConstSpan(
      static_cast<char *>(mapping_.addr) + mapping_.user_offset,
      mapping_.user_size);
}

absl::Span<char> MappedMemory::MemoryAsReadWriteSpan() const {
  if (mapping_.writable) {
    return absl::MakeSpan(
        static_cast<char *>(mapping_.addr) + mapping_.user_offset,
        mapping_.user_size);
  } else {
    return absl::Span<char>();
  }
}

MappedMemory::MappedMemory(MmapInfo mapping) : mapping_(mapping) {}

}  // namespace ecclesia
