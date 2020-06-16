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

// Helper functions for logging information from POSIX errors. These will use
// errno and strerror to produce generic error messages that can be included in
// logging when a failure occurs.

#ifndef ECCLESIA_LIB_LOGGING_POSIX_H_
#define ECCLESIA_LIB_LOGGING_POSIX_H_

#include <string>

#include "lib/logging/globals.h"
#include "lib/logging/interfaces.h"

namespace ecclesia {

// Given an errno value, produce a generic error message containing both the
// error number as well as a standard string message describing it.
std::string PosixErrorMessage(int errno_value);

// Logging function that can be used to wrap an standard log sink operation but
// which will prepend the log message with a generic POSIX error, using the
// message from PosixErrorMessage. Expected to be used as in the form of:
//
//   PosixLog(ErrorLog) << "my message";
//
// Which will write out an error log that starts with the generic POSIX error.
LogMessageStream PosixLog(LogMessageStream (*log_func)(SourceLocation),
                          SourceLocation loc = SourceLocation::current());

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_LOGGING_POSIX_H_
