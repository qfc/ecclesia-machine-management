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
#include <string>
#include <utility>

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "lib/logging/location.h"
#include "lib/time/clock.h"

namespace ecclesia {

// Generic logger interface for writing logging out to different kinds of sinks.
class LoggerInterface {
 public:
  // Parameters that will be passed to the Write call.
  struct WriteParameters {
    // The log level of the call. This should be used to control what sinks the
    // logs will be sent to. Different LoggerInterface implementations will have
    // different behaviors for this.
    int log_level;
    // The source location where the logging call originated from.
    SourceLocation source_location;
    // The raw text to written out. This does not contain any metadata derived
    // from the above fields, so if the sink needs metadata attached the logger
    // must add it itself. This normally would be done with MakeMetadataPrefix.
    absl::string_view text;
  };

  LoggerInterface() = default;
  virtual ~LoggerInterface() = default;

  // Standard helper function for generating a standard string with metadata
  // information that can be prefixed to log lines. Captures log level,
  // timestamp and source location.
  static std::string MakeMetadataPrefix(
      int log_level, absl::Time timestamp,
      SourceLocation loc) {  // Find the base name of the source file logging.
                             // Logging the full path is
    // a bit too noisy and depends on too many details of the source tree.
    absl::string_view loc_file_name = loc.file_name();
    auto slash_pos = loc_file_name.find_last_of('/');
    if (slash_pos != loc_file_name.npos) {
      loc_file_name.remove_prefix(slash_pos + 1);
    }
    // Use a UTC timestamp with microsecond precision.
    std::string timestamp_str =
        absl::FormatTime("%Y-%m-%d %H:%M:%E6S", timestamp, absl::UTCTimeZone());
    // Combine the log level, timestamp and location into a prefix.
    return absl::StrFormat("L%d %s %s:%d] ", log_level,
                           std::move(timestamp_str), loc_file_name, loc.line());
  }

  // Take a log line and write it out to the appropriate sinks for the given log
  // level. The text does not contain a trailing newline and so the writer
  // should add one if necessary.
  //
  // NOTE: The text not having a trailing newline does not mean that it does not
  // contain any newlines.
  //
  // This function is expected to be thread-safe. In particular:
  //   * log text from concurrent writes should _never_ be interleaved
  //   * log metadata (timestamps) should be sequenced in the same way that
  //     writes are sequenced, i.e. timestamps in logs should be ordered in the
  //     same way as the log lines themselves
  // The MakeMetadataPrefix function provides a standard format for prefixing
  // log metadata to text-based logs. However, it is not required if the
  // underlying sink has its own ways to represent such metadata.
  virtual void Write(WriteParameters params) = 0;
};

// The default logger implementation. Its behavior is:
//   * L0 logs go to stderr and then terminate the program
//   * L1 logs go to stderr
//   * L2+ logs are discarded
// This is the logger that will be added to the global LogMessageStream on
// startup. Programs desiring different behavior should install their own logger
// implementation to replace it.
class DefaultLogger : public LoggerInterface {
 public:
  explicit DefaultLogger(Clock *clock);

  void Write(WriteParameters params) override;

 private:
  // Used to synchronize logging.
  absl::Mutex mutex_;
  // Used to capture timestamps for log lines.
  Clock *clock_;
};

// Object that absorbs streamed text from users. When the various *Log()
// functions are called an instance of this object will be returned and the
// caller can write to it via the << operator.
//
// The logging will all be written out when the LogMessageStream object is
// destroyed. Callers should not hold onto the object outside of the logging
// statement, and so the lifetime should generally end when the logging
// statement does.
class LogMessageStream {
 public:
  // When the LogMessage is destroyed it will flush its contents out to the
  // underlying logger. If logger is null then it does nothing.
  ~LogMessageStream() {
    if (logger_) {
      logger_->Write({.log_level = log_level_,
                      .source_location = source_location_,
                      .text = stream_.str()});
    }
  }

  // You can't copy or assign to a message stream, but you can move it. This is
  // because message stream is basically intended to be used as a temporary
  // object that accumulates data as it is written to and then flushes it out to
  // the logger when it is destroyed. Thus, we delete all of these operators
  // except for the move, which allows values to be efficiently returned.
  LogMessageStream(const LogMessageStream &other) = delete;
  LogMessageStream &operator=(const LogMessageStream &other) = delete;
  LogMessageStream(LogMessageStream &&other)
      : log_level_(other.log_level_),
        source_location_(other.source_location_),
        logger_(other.logger_),
        stream_(std::move(other.stream_)) {
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
  // Used to give LoggerStreamFactory permission to construct these objects.
  friend class LoggerStreamFactory;

  LogMessageStream(int log_level, SourceLocation source_location,
                   LoggerInterface *logger)
      : log_level_(log_level),
        source_location_(source_location),
        logger_(logger) {}

  // The information and objects need to actually write out the log.
  int log_level_;
  SourceLocation source_location_;
  LoggerInterface *logger_;

  // A string stream used to accumulate the log text.
  std::ostringstream stream_;
};

// Class that wraps a LoggerInterface implementation that can be used to
// construct logging streams. Used as a global singleton.
class LoggerStreamFactory {
 public:
  // Construct a new factory using the given logger.
  LoggerStreamFactory(std::unique_ptr<LoggerInterface> logger);

  // Replace the current logger implementation with an new one. This code is not
  // thread-safe, and is not safe to call if there are any outstanding
  // LogMessageStream objects.
  void SetLogger(std::unique_ptr<LoggerInterface> logger);

  // Create a new message stream for logging at a particular level. The stream
  // will be prepended with log level and timestamp text.
  LogMessageStream MakeStream(int log_level, SourceLocation loc) const {
    return LogMessageStream(log_level, loc, logger_.get());
  }

 private:
  // The underlying log implementation stored in this object.
  std::unique_ptr<LoggerInterface> logger_;
};

// Get the global logger. Note that this returns a reference to the stream
// factory, and not the underlying LoggerInterface.
const LoggerStreamFactory &GetGlobalLogger();

// Install a new global logger implementation. Calling this while
// GetGlobalLogger is being used (or while other calls are being made to
// SetGlobalLogger) is a data race and so to call it safely you should call this
// as early as possible in your program startup.
void SetGlobalLogger(std::unique_ptr<LoggerInterface> logger);

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
