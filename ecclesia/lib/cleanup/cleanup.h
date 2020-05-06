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

// Helper RAII objects for doing automatic cleanup when scopes are exited.

#ifndef ECCLESIA_LIB_UTIL_FD_CLEANUP_H_
#define ECCLESIA_LIB_UTIL_FD_CLEANUP_H_

#include <unistd.h>

namespace ecclesia {
// A simple RAII class to close a file.
class FdCloser {
 public:
  explicit FdCloser(int fd) : fd_(fd) {}
  FdCloser(const FdCloser &) = delete;
  FdCloser &operator=(const FdCloser &) = delete;
  ~FdCloser() { close(fd_); }

 private:
  int fd_;
};

}  // namespace ecclesia
#endif  // ECCLESIA_LIB_UTIL_FD_CLEANUP_H_
