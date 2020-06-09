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

// This library provides functions for writing out process-wide informational
// and debug logging. The logging API effectively acts like a singleton
// interface where any code anywhere in the process can call the global logging
// functions to write out information.

#ifndef ECCLESIA_LIB_LOGGING_LOGGING_H_
#define ECCLESIA_LIB_LOGGING_LOGGING_H_

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/time/time.h"
#include "lib/time/clock.h"

namespace ecclesia {

// Generic logger interface for writing out different kinds of log files. It is
// called with logs which have already been decorated with metadata about the
// source of the call (e.g. timestamps, log lines).
class LoggerInterface {
 public:
  LoggerInterface() = default;
  virtual ~LoggerInterface() = default;

  // Take a log line and write it out to the underlying sink. The text does not
  // contain a trailing newline and so the writer should add one as necessary.
  //
  // NOTE: The text not having a trailing newline does not mean that it does not
  // contain any newlines.
  virtual void Write(absl::string_view text) = 0;
};

// Null logger implementation that discards all logs it is given.
class NullLogger : public LoggerInterface {
 public:
  NullLogger() : LoggerInterface() {}

 private:
  void Write(absl::string_view text) override {}
};

// Logger implementation that writes all logs out to standard error.
class StderrLogger : public LoggerInterface {
 public:
  StderrLogger() : LoggerInterface() {}

 private:
  void Write(absl::string_view text) override {
    std::cerr << text << std::endl;
  }
};

// Object that absorbs streamed text from users. When the various *Log()
// functions are called an instance of this object will be returned and the
// caller can write to it via the << operator.
//
// The logging will all be written out when the LogMessageStream object is
// destroyed. It is expected that callers will not hold onto the object outside
// of the logging statement, and so the lifetime should generally end along with
// the statement.
class LogMessageStream {
 public:
  // When the LogMessage is destroyed it will flush its contents out to the
  // underlying logger. If logger is null then it does nothing.
  ~LogMessageStream() {
    if (logger_) logger_->Write(stream_.str());
  }

  // You can't copy or assign to a message stream, but you can move it. This is
  // because message stream is basically intended to be used as a temporary
  // object that accumulates data as it is written to and then flushes it out to
  // the logger when it is destroyed. Thus, we delete all of these operators
  // except for the move, which allows values to be efficiently returned.
  LogMessageStream(const LogMessageStream &other) = delete;
  LogMessageStream &operator=(const LogMessageStream &other) = delete;
  LogMessageStream(LogMessageStream &&other)
      : logger_(other.logger_), stream_(std::move(other.stream_)) {
    // Explicitly clear out the logger from the moved-from object. This keeps it
    // from flushing anything to the logger in its destructor.
    other.logger_ = nullptr;
  }
  LogMessageStream &operator=(LogMessageStream &&other) = delete;

  // Implement the stream operator (<<) for the log message stream. Under the
  // covers this writes to the underlying ostringstream.
  template <typename T>
  friend LogMessageStream operator<<(LogMessageStream lms, T &&value) {
    lms.stream_ << std::forward<T>(value);
    return lms;
  }

 private:
  // Used to give LeveledLogger permission to construct these objects.
  friend class LeveledLogger;

  explicit LogMessageStream(LoggerInterface *logger) : logger_(logger) {}

  LoggerInterface *logger_;
  std::ostringstream stream_;
};

// Class representing a set of logging implementations, one for each supported
// log level plus a default one for levels with no explicit logger.
//
// Normally this class is used by constructing a new instance, using AddLogger
// to configure loggers for all the various levels you support, and then using
// SetGlobalLogger to install this globally.
class LeveledLogger {
 public:
  // Create a new levelled logger using the given logger as the default and the
  // given clock for capturing log timestamps.
  LeveledLogger(std::unique_ptr<LoggerInterface> default_logger, Clock *clock)
      : default_logger_(std::move(default_logger)), clock_(clock) {}

  // Add a new logger for handling the given log level.
  void AddLogger(int log_level, std::unique_ptr<LoggerInterface> logger) {
    // Make sure the loggers vector is large enough to hold this log level.
    if (log_level >= loggers_.size()) {
      loggers_.resize(log_level + 1);
    }
    // Add the logger into the logger vector.
    loggers_[log_level] = std::move(logger);
  }

  // Create a new message stream for logging at a particular level. The stream
  // will be prepended with log level and timestamp text.
  LogMessageStream MakeStream(int log_level) const {
    return LogMessageStream(GetLogger(log_level))
           << "L" << log_level
           << absl::FormatTime(" %Y-%m-%d %H:%M:%E6S] ", clock_->Now(),
                               absl::UTCTimeZone());
  }

 private:
  // Find the logger to use for a particular log level. Returns the default
  // logger if no logger was defined for that level.
  LoggerInterface *GetLogger(int log_level) const {
    if (log_level >= loggers_.size() || !loggers_[log_level]) {
      return default_logger_.get();
    }
    return loggers_[log_level].get();
  }

  // The default logger, used for log levels with no explicit level.
  std::unique_ptr<LoggerInterface> default_logger_;

  // A map of log levels to loggers. A vector is used because it is very low
  // overhead for both storage and lookups; since the in-use log levels are
  // usually just 0-4, or a subset of that, the amount of space wasted on
  // "blank" entries is generally small.
  std::vector<std::unique_ptr<LoggerInterface>> loggers_;

  // The clock to use for capturing log timestamps.
  Clock *clock_;
};

// Get the global logger.
const LeveledLogger &GetGlobalLogger();

// Install a new global set of loggers. Calling this while GetGlobalLogger is
// being used (or while other calls are being made to SetGlobalLogger) is a data
// race and so to call it safely you should call this as early as possible in
// your program startup.
void SetGlobalLogger(std::unique_ptr<LeveledLogger> logger);

// Generic function for logging to a numeric log level. Normally only needed in
// contexts where you need the log level to be parameterized.
template <int LogLevel>
LogMessageStream WriteLog() {
  static_assert(LogLevel >= 0, "log levels must be positive");
  return GetGlobalLogger().MakeStream(LogLevel);
}

// Alias functions with more meaningful names for log levels 0-4. Generally
// preferred in most code.
inline LogMessageStream FatalLog() { return WriteLog<0>(); }
inline LogMessageStream ErrorLog() { return WriteLog<1>(); }
inline LogMessageStream WarningLog() { return WriteLog<2>(); }
inline LogMessageStream InfoLog() { return WriteLog<3>(); }
inline LogMessageStream DebugLog() { return WriteLog<4>(); }

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_LOGGING_LOGGING_H_
