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

// The library provides support for interacting with an "API filesystem". What
// that means is interacting with systems or hardware where the API is reading
// and writing files in a directory in the filesystem (e.g in /proc or /sys).

#ifndef ECCLESIA_LIB_APIFS_APIFS_H_
#define ECCLESIA_LIB_APIFS_APIFS_H_

#include <sys/stat.h>   // IWYU pragma: keep
#include <sys/types.h>  // IWYU pragma: keep
#include <unistd.h>     // IWYU pragma: keep

#include <filesystem>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/types/span.h"

namespace ecclesia {

class ApifsDirectory {
 public:
  // Construct a default valued object that points at nothing. Using any of
  // the apifs methods will fail; a default object is only useful for being
  // assigned into.
  ApifsDirectory();

  // Construct an object for accessing the API filesystem. Expects a path to a
  // root directory within the filesystem. All "path" arguments to methods are
  // then interpreted as being relative to this root. The root can be specified
  // with either an absolute path, or by using a path relative to an
  // ApifsDirectory.
  explicit ApifsDirectory(std::string path);
  ApifsDirectory(const ApifsDirectory &directory, std::string path);

  // Apifs access objects can be copied or moved.
  ApifsDirectory(ApifsDirectory &&other) = default;
  ApifsDirectory(const ApifsDirectory &other) = default;
  ApifsDirectory &operator=(ApifsDirectory &&other) = default;
  ApifsDirectory &operator=(const ApifsDirectory &other) = default;

  // Return a boolean indicating if the object is "null", not set to any path.
  bool IsNull() const { return dir_path_.empty(); }

  // Return the filesystem path this object refers to.
  std::string GetPath() const { return dir_path_; }

  // Indicates if this object refers to a path that exists.
  bool Exists() const;

  // Indicates if the given path exists.
  bool Exists(std::string path) const;

  // Retreive the stat information for a given path.
  absl::Status Stat(std::string path, struct stat *st) const;

  // List all the entries in the apifs, or in a given directory in the apifs.
  absl::Status ListEntries(std::vector<std::string> *entries) const;
  absl::Status ListEntries(std::string path,
                           std::vector<std::string> *entries) const;

  // Read and write the entire contents of a file.
  absl::Status Read(std::string path, std::string *value) const;
  absl::Status Write(std::string path, const std::string &value) const;

  // Read a symlink value for a given path.
  absl::Status ReadLink(std::string path, std::string *link) const;

 private:
  friend class ApifsFile;  // For accessing the root.

  std::filesystem::path dir_path_;
};

class ApifsFile {
 public:
  // Construct a default valued object that points at nothing. Using any of
  // the apifs methods will fail; a default object is only useful for being
  // assigned into.
  ApifsFile();

  // Construct an object for accessing a file in an API filesystem. The file
  // can be specified by either using an absolute path, or by using a path
  // relative to an ApifsDirectory.
  explicit ApifsFile(std::string path);
  ApifsFile(const ApifsDirectory &directory, std::string path);

  // Apifs access objects can be copied or moved.
  ApifsFile(ApifsFile &&other) = default;
  ApifsFile(const ApifsFile &other) = default;
  ApifsFile &operator=(ApifsFile &&other) = default;
  ApifsFile &operator=(const ApifsFile &other) = default;

  // Return a boolean indicating if the object is "null", not set to any path.
  bool IsNull() const { return path_.empty(); }

  // Return the filesystem path this object refers to.
  std::string GetPath() const { return path_; }

  // Indicates if the given path exists.
  bool Exists() const;

  // Retrieve the stat information for a given path.
  absl::Status Stat(struct stat *st) const;

  // Read and write the entire contents of a file.
  absl::Status Read(std::string *value) const;
  absl::Status Write(const std::string &value) const;

  // Seek to offset then Read spcific number of bytes
  absl::Status SeekAndRead(uint64_t offset, absl::Span<char> value) const;
  // Seek to offset then Write spcific number of bytes
  absl::Status SeekAndWrite(uint64_t offset,
                            absl::Span<const char> value) const;

  // Read a symlink value for a given path.
  absl::Status ReadLink(std::string *link) const;

 private:
  std::filesystem::path path_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_APIFS_APIFS_H_
