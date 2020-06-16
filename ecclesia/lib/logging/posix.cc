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

#include "ecclesia/lib/logging/posix.h"

#include <cerrno>
#include <string>
#include <system_error>

#include "absl/strings/str_format.h"
#include "ecclesia/lib/logging/globals.h"
#include "ecclesia/lib/logging/interfaces.h"

namespace ecclesia {

std::string PosixErrorMessage(int errno_value) {
  auto ec = std::make_error_code(static_cast<std::errc>(errno_value));
  return absl::StrFormat("POSIX error %d [%s]", errno_value, ec.message());
}

LogMessageStream PosixLog(LogMessageStream (*log_func)(SourceLocation),
                          SourceLocation loc) {
  int captured_errno = errno;
  return log_func(loc) << PosixErrorMessage(captured_errno) << " ";
}

}  // namespace ecclesia
