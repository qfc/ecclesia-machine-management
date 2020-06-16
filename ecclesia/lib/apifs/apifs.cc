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

#include "ecclesia/lib/apifs/apifs.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>   // IWYU pragma: keep
#include <sys/types.h>  // IWYU pragma: keep
#include <unistd.h>     // IWYU pragma: keep

#include <cstddef>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "absl/types/span.h"
#include "ecclesia/lib/cleanup/cleanup.h"

namespace ecclesia {

namespace fs = ::std::filesystem;

ApifsDirectory::ApifsDirectory() {}

ApifsDirectory::ApifsDirectory(std::string path) : dir_path_(std::move(path)) {}

ApifsDirectory::ApifsDirectory(const ApifsDirectory &directory,
                               std::string path)
    : dir_path_(directory.dir_path_ / std::move(path)) {}

bool ApifsDirectory::Exists() const {
  std::error_code ec;  // Ignored on error, we return false.
  return fs::exists(dir_path_, ec);
}

bool ApifsDirectory::Exists(std::string path) const {
  ApifsDirectory d(*this, std::move(path));
  return d.Exists();
}

absl::Status ApifsDirectory::Stat(std::string path, struct stat *st) const {
  ApifsFile f(*this, std::move(path));
  return f.Stat(st);
}

absl::Status ApifsDirectory::ListEntries(
    std::vector<std::string> *entries) const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("directory not found at path: %s", dir_path_));
  }

  std::error_code ec;  // Used to track errors from directory iteration.

  // Try to construct a directory iterator.
  fs::directory_iterator end_iter;  // Default instance is an end iterator.
  auto iter = fs::directory_iterator(dir_path_, ec);
  if (ec) {
    return absl::InternalError(
        absl::StrFormat("error accessing directory at path: %s, error: %s",
                        dir_path_, ec.message()));
  }

  // If the directory iterator is empty then return after clearing the list.
  // Doing this check up front lets us use a simpler do-while loop.
  entries->clear();
  if (iter == end_iter) return absl::OkStatus();

  // Iterate over the entire directory, populating the vector. If we hit any
  // failures then stop immediately and return an error.
  do {
    entries->push_back(iter->path());
    iter.increment(ec);
  } while (!ec && iter != end_iter);
  if (ec) {
    return absl::InternalError(absl::StrFormat(
        "error accessing directory: %s, error: %s", dir_path_, ec.message()));
  }
  return absl::OkStatus();  // Iteration terminated without an error.
}

absl::Status ApifsDirectory::ListEntries(
    std::string path, std::vector<std::string> *entries) const {
  ApifsDirectory d(*this, std::move(path));
  return d.ListEntries(entries);
}

absl::Status ApifsDirectory::Read(std::string path, std::string *value) const {
  ApifsFile f(*this, std::move(path));
  return f.Read(value);
}

absl::Status ApifsDirectory::Write(std::string path,
                                   const std::string &value) const {
  ApifsFile f(*this, std::move(path));
  return f.Write(value);
}

absl::Status ApifsDirectory::ReadLink(std::string path,
                                      std::string *link) const {
  ApifsFile f(*this, std::move(path));
  return f.ReadLink(link);
}

ApifsFile::ApifsFile() {}

ApifsFile::ApifsFile(std::string path) : path_(std::move(path)) {}

ApifsFile::ApifsFile(const ApifsDirectory &directory, std::string path)
    : path_(directory.dir_path_ / std::move(path)) {}

bool ApifsFile::Exists() const {
  std::error_code ec;  // Ignored on error, we return false.
  return fs::exists(path_, ec);
}

absl::Status ApifsFile::Stat(struct stat *st) const {
  if (stat(path_.c_str(), st) < 0) {
    return absl::InternalError(absl::StrFormat(
        "failure while stat-ing file at path: %s, errno: %d", path_, errno));
  }
  return absl::OkStatus();
}

absl::Status ApifsFile::Read(std::string *value) const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("File not found at path: %s", path_));
  }

  const int fd = open(path_.c_str(), O_RDONLY);
  if (fd < 0) {
    return absl::NotFoundError(absl::StrFormat(
        "unable to open the file at path: %s, errno: %d", path_, errno));
  }
  auto fd_closer = FdCloser(fd);

  value->clear();
  while (true) {
    char buffer[4096];
    const ssize_t n = read(fd, buffer, sizeof(buffer));
    if (n < 0) {
      const auto read_errno = errno;
      if (read_errno == EINTR) {
        continue;  // Retry on EINTR.
      }
      return absl::InternalError(absl::StrFormat(
          "failure while reading from file at path: %s, errno: %d", path_,
          read_errno));
      break;
    } else if (n == 0) {
      break;  // Nothing left to read.
    } else {
      value->append(buffer, n);
    }
  }
  return absl::OkStatus();
}

absl::Status ApifsFile::Write(const std::string &value) const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("File not found at path: %s", path_));
  }
  const int fd = open(path_.c_str(), O_WRONLY);
  if (fd < 0) {
    return absl::NotFoundError(
        absl::StrFormat("unable to open the file at path: %s", path_));
  }
  auto fd_closer = FdCloser(fd);
  const char *data = value.data();
  size_t size = value.size();
  while (size > 0) {
    ssize_t result = write(fd, data, size);
    if (result <= 0) {
      const auto write_errno = errno;
      if (write_errno == EINTR) continue;  // Retry on EINTR.
      return absl::InternalError(absl::StrFormat(
          "failure while writing to file at path: %s, errno: %d", path_,
          errno));
      break;
    }
    // We successfully wrote out 'result' bytes, advance the data pointer.
    size -= result;
    data += result;
  }
  return absl::OkStatus();
}

absl::Status ApifsFile::SeekAndRead(uint64_t offset,
                                    absl::Span<char> value) const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("File not found at path: %s", path_));
  }

  int fd = open(path_.c_str(), O_RDONLY);
  if (fd < 0) {
    return absl::NotFoundError(absl::StrFormat(
        "Unable to open the file at path: %s, errno: %d", path_, errno));
  }
  auto fd_closer = FdCloser(fd);
  if (lseek64(fd, offset, SEEK_SET) == static_cast<off_t>(-1)) {
    return absl::NotFoundError(absl::StrFormat(
        "Unable to seek to offset: %#x in file %s", offset, path_));
  }

  // Read data.
  size_t size = value.size();
  int rlen = read(fd, value.data(), size);
  if (rlen != size) {
    return absl::InternalError(absl::StrFormat(
        "Fail to read %d bytes from offset %#x. rlen: %d", size, offset, rlen));
  }
  return absl::OkStatus();
}

absl::Status ApifsFile::SeekAndWrite(uint64_t offset,
                                     absl::Span<const char> value) const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("File not found at path: %s", path_));
  }
  const int fd = open(path_.c_str(), O_WRONLY);
  if (fd < 0) {
    return absl::NotFoundError(
        absl::StrFormat("Unable to open the file at path: %s", path_));
  }
  auto fd_closer = FdCloser(fd);
  if (lseek64(fd, offset, SEEK_SET) == static_cast<off_t>(-1)) {
    return absl::NotFoundError(absl::StrFormat(
        "Unable to seek to msr: %#x in file %s", offset, path_));
  }
  // Write data.
  size_t size = value.size();
  int wlen = write(fd, value.data(), size);
  if (wlen != size) {
    return absl::NotFoundError(
        absl::StrFormat("Failed to write %d bytes to msr %s", size, path_));
  }
  return absl::OkStatus();
}

absl::Status ApifsFile::ReadLink(std::string *link) const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("link not found at path: %s", path_));
  }
  if (!fs::is_symlink(path_)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("path: %s is not a symlink", path_));
  }
  *link = fs::read_symlink(path_).string();
  return absl::OkStatus();
}

}  // namespace ecclesia
