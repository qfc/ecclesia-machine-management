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

// Functions for logging errors, warnings and information, as well as for doing
// log-and-abort operations.
//
// This operation follows a familiar pattern for C++: to write out logs you use
// the << operator to stream objects to the logs you want. This allows you to
// write to logs with code like:
//
//   ErrorLog() << "something bad happened: " << status_object;
//
// Any object that you can stream to a std::ostream should be able to stream to
// the logging. In addition to logging the text of your log message the logging
// system can also capture timestamps and source code locations.
//
// This logging system rates severity via log levels, starting at 0 (the most
// severe) and increasing from there. The general idea is:
//   * 0 = unrecoverable error, abort program after logging
//   * 1 = serious error occurred, generally unexpected
//   * 2 = warning, something undesirable but not necessarily unexpected
//   * 3 = info, general information
//   * 4 = debug, verbose and noisy information
// These are generally called "Fatal", "Error", "Warning", "Info" and "Debug".
//
// Note that unlike some logging systems this one does not discard or compile
// out code just because a particular log level is not being recorded. Keep this
// in mind when writing expensive logging code: just because the logs aren't
// being recorded, doesn't mean the code isn't executing.
//
// On the flip side, just because we aren't optimizing away the logging code
// that doesn't make it a good idea to have side effects from your logging. We
// do not promise that we will never optimize away any logging in the future.
//
// While the object returned by the Log functions for streaming obvious has an
// actual type, users should treat it as being unspecified by the API. The
// object can be streamed to by anything that supports operator<< to a
// std::ostream but it is not itself a std::ostream. The objects should also not
// be saved or passed around, as this will result in the logs having incorrect
// source location information.
//
// If you wish to build additional log wrapper functions to add more features or
// options they should be constributed to this library rather than trying to
// build on top of the implementation details of these functions.

#ifndef ECCLESIA_LIB_LOGGING_LOGGING_H_
#define ECCLESIA_LIB_LOGGING_LOGGING_H_

#include "lib/logging/globals.h"
#include "lib/logging/interfaces.h"

namespace ecclesia {

// Generic function for logging to a numeric log level. Normally only needed in
// contexts where you need the log level to be parameterized.
template <int LogLevel>
LogMessageStream WriteLog(SourceLocation loc = SourceLocation::current()) {
  static_assert(LogLevel >= 0, "log levels must be positive");
  return GetGlobalLogger().MakeStream(LogLevel, loc);
}

// Alias functions with more meaningful names for log levels 0-4. Generally
// preferred in most code.
inline LogMessageStream FatalLog(
    SourceLocation loc = SourceLocation::current()) {
  return WriteLog<0>(loc);
}
inline LogMessageStream ErrorLog(
    SourceLocation loc = SourceLocation::current()) {
  return WriteLog<1>(loc);
}
inline LogMessageStream WarningLog(
    SourceLocation loc = SourceLocation::current()) {
  return WriteLog<2>(loc);
}
inline LogMessageStream InfoLog(
    SourceLocation loc = SourceLocation::current()) {
  return WriteLog<3>(loc);
}
inline LogMessageStream DebugLog(
    SourceLocation loc = SourceLocation::current()) {
  return WriteLog<4>(loc);
}

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_LOGGING_LOGGING_H_
