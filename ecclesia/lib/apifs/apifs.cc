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
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "ecclesia/lib/cleanup/cleanup.h"
#include "ecclesia/lib/file/dir.h"
#include "ecclesia/lib/file/path.h"

namespace ecclesia {

ApifsDirectory::ApifsDirectory() {}

ApifsDirectory::ApifsDirectory(std::string path) : dir_path_(std::move(path)) {}

ApifsDirectory::ApifsDirectory(const ApifsDirectory &directory,
                               std::string path)
    : dir_path_(JoinFilePaths(directory.dir_path_, path)) {}

bool ApifsDirectory::Exists() const {
  return access(dir_path_.c_str(), F_OK) == 0;
}

bool ApifsDirectory::Exists(std::string path) const {
  ApifsDirectory d(*this, std::move(path));
  return d.Exists();
}

absl::StatusOr<struct stat> ApifsDirectory::Stat(std::string path) const {
  ApifsFile f(*this, std::move(path));
  return f.Stat();
}

absl::StatusOr<std::vector<std::string>> ApifsDirectory::ListEntries() const {
  if (!Exists()) {
    return absl::NotFoundError(
        absl::StrFormat("directory not found at path: %s", dir_path_));
  }
  std::vector<std::string> entries;
  absl::Status status = WithEachFileInDirectory(
      dir_path_, [this, &entries](absl::string_view entry) {
        entries.push_back(JoinFilePaths(dir_path_, entry));
      });
  if (!status.ok()) {
    return status;
  }
  return entries;
}

absl::StatusOr<std::vector<std::string>> ApifsDirectory::ListEntries(
    std::string path) const {
  ApifsDirectory d(*this, std::move(path));
  return d.ListEntries();
}

absl::StatusOr<std::string> ApifsDirectory::Read(std::string path) const {
  ApifsFile f(*this, std::move(path));
  return f.Read();
}

absl::Status ApifsDirectory::Write(std::string path,
                                   absl::string_view value) const {
  ApifsFile f(*this, std::move(path));
  return f.Write(value);
}

absl::StatusOr<std::string> ApifsDirectory::ReadLink(std::string path) const {
  ApifsFile f(*this, std::move(path));
  return f.ReadLink();
}

ApifsFile::ApifsFile() {}

ApifsFile::ApifsFile(std::string path) : path_(std::move(path)) {}

ApifsFile::ApifsFile(const ApifsDirectory &directory, std::string path)
    : path_(JoinFilePaths(directory.dir_path_, path)) {}

bool ApifsFile::Exists() const { return access(path_.c_str(), F_OK) == 0; }

absl::StatusOr<struct stat> ApifsFile::Stat() const {
  struct stat st;
  if (stat(path_.c_str(), &st) < 0) {
    return absl::InternalError(absl::StrFormat(
        "failure while stat-ing file at path: %s, errno: %d", path_, errno));
  }
  return st;
}

absl::StatusOr<std::string> ApifsFile::Read() const {
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

  std::string value;
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
      value.append(buffer, n);
    }
  }
  return value;
}

absl::Status ApifsFile::Write(absl::string_view value) const {
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

absl::StatusOr<std::string> ApifsFile::ReadLink() const {
  // Do an lstat of the path to determine the link size and verify the file is
  // in fact a symlink.
  struct stat st;
  if (lstat(path_.c_str(), &st) < 0) {
    if (errno == ENOENT) {
      return absl::NotFoundError(
          absl::StrFormat("file not found at path: %s", path_));
    } else {
      return absl::InternalError(absl::StrFormat(
          "failure while lstat-ing file at path: %s, errno: %d", path_, errno));
    }
  }
  if (!S_ISLNK(st.st_mode)) {
    return absl::InvalidArgumentError(
        absl::StrFormat("path: %s is not a symlink", path_));
  }
  // Read the symlink using a std::string buffer size from the lstat.
  std::string link(st.st_size + 1, '\0');
  ssize_t rc = readlink(path_.c_str(), &link[0], link.size());
  if (rc == -1) {
    return absl::InternalError(absl::StrFormat(
        "unable to read the link at path: %s, errno: %d", path_, errno));
  }
  if (rc > st.st_size) {
    // If this happens it means someone changed (and enlarged) the link in
    // between the lstat and readlink. Just consider that an error.
    return absl::InternalError(absl::StrFormat(
        "the link at: %s was changed while it was being read", path_));
  }
  // The first "rc" characters in the string were populated. Return that.
  return link.substr(0, rc);
}

}  // namespace ecclesia
