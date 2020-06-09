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

#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "absl/synchronization/mutex.h"
#include "absl/time/time.h"
#include "lib/time/clock.h"

namespace ecclesia {

// Class used to serialize logging operations.
class LogSequencer {
 public:
  explicit LogSequencer(Clock *clock) : clock_(clock) {}

  // These objects are used for synchronization and so should not be copied.
  LogSequencer(const LogSequencer &other) = delete;
  LogSequencer &operator=(const LogSequencer &other) = delete;

  // Execute the given function under the sequencers mutex.
  template <typename F>
  void InSequence(F func) const {
    absl::MutexLock ml(&mutex_);
    func();
  }

  // Creates a string containing log metadata that can be prefixed to log lines.
  // In order to ensure that logs have consistent stamps this should normally
  // only be called from inside InSequence.
  std::string MakeMetadataPrefix(int log_level) const {
    return absl::StrFormat("L%d %s] ", log_level, MakeTimestamp());
  }

 private:
  // Creates a timestamp string using a standard format. Timestamps are always
  // rendered in UTC.
  std::string MakeTimestamp() const {
    return absl::FormatTime("%Y-%m-%d %H:%M:%E6S", clock_->Now(),
                            absl::UTCTimeZone());
  }

  // The underlying mutex object.
  mutable absl::Mutex mutex_;

  Clock *clock_;
};

// Generic logger interface for writing out different kinds of log files.
class LoggerInterface {
 public:
  // Parameters that will be passed to the Write call.
  struct WriteParameters {
    int log_level;
    absl::string_view text;
    const LogSequencer *sequencer;

    // Helper function to generate a metadata prefix using the sequencer.
    std::string MakeMetadataPrefix() const {
      return sequencer->MakeMetadataPrefix(log_level);
    }
  };

  LoggerInterface() = default;
  virtual ~LoggerInterface() = default;

  // Take a log line and write it out to the underlying sink. The text does not
  // contain a trailing newline and so the writer should add one if necessary.
  //
  // NOTE: The text not having a trailing newline does not mean that it does not
  // contain any newlines.
  //
  // This function is expected to be thread-safe. In particular:
  //   * log text from concurrent writes should _never_ be interleaved
  //   * log metadata (timestamps) should be sequenced in the same way that
  //     writes are sequenced, i.e. timestamps in logs should be ordered in the
  //     same way as the log lines themselves
  // The LogSequencer can be used as a simple way to provide a global order on
  // logs. However, if the underlying sink has its own metadata and timestamping
  // then implementations can use it instead.
  virtual void Write(WriteParameters params) = 0;
};

// Null logger implementation that discards all logs it is given.
class NullLogger : public LoggerInterface {
 public:
  NullLogger() : LoggerInterface() {}

 private:
  void Write(WriteParameters params) override {}
};

// Logger implementation that writes all logs out to standard error.
class StderrLogger : public LoggerInterface {
 public:
  StderrLogger() : LoggerInterface() {}

 private:
  void Write(WriteParameters params) override {
    params.sequencer->InSequence([&]() {
      std::cerr << params.MakeMetadataPrefix() << params.text << std::endl;
    });
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
    if (logger_) {
      logger_->Write({.log_level = log_level_,
                      .text = stream_.str(),
                      .sequencer = sequencer_});
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
        logger_(other.logger_),
        sequencer_(other.sequencer_),
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
  // Used to give LeveledLogger permission to construct these objects.
  friend class LeveledLogger;

  LogMessageStream(int log_level, LoggerInterface *logger,
                   const LogSequencer *sequencer)
      : log_level_(log_level), logger_(logger), sequencer_(sequencer) {}

  // The information and objects need to actually write out the log.
  int log_level_;
  LoggerInterface *logger_;
  const LogSequencer *sequencer_;
  // A string stream used to accumulate the log text.
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
      : default_logger_(std::move(default_logger)), sequencer_(clock) {}

  // Add a new logger for handling the given log level.
  void AddLogger(int log_level, std::unique_ptr<LoggerInterface> logger) {
    // Make sure the loggers vector is large enough to hold this log level.
    if (log_level >= loggers_.size()) {
      loggers_.resize(log_level + 1);
    }
    // Add the logger into the logger vector.
    loggers_[log_level] = std::move(logger);
  }

  // Find the logger to use for a particular log level. Returns the default
  // logger if no logger was defined for that level.
  LoggerInterface *GetLogger(int log_level) const {
    if (log_level >= loggers_.size() || !loggers_[log_level]) {
      return default_logger_.get();
    }
    return loggers_[log_level].get();
  }

  // Create a new message stream for logging at a particular level. The stream
  // will be prepended with log level and timestamp text.
  LogMessageStream MakeStream(int log_level) const {
    return LogMessageStream(log_level, GetLogger(log_level), &sequencer_);
  }

 private:
  // The default logger, used for log levels with no explicit level.
  std::unique_ptr<LoggerInterface> default_logger_;

  // A map of log levels to loggers. A vector is used because it is very low
  // overhead for both storage and lookups; since the in-use log levels are
  // usually just 0-4, or a subset of that, the amount of space wasted on
  // "blank" entries is generally small.
  std::vector<std::unique_ptr<LoggerInterface>> loggers_;

  // Sequencer used to sequence all log writes.
  LogSequencer sequencer_;
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
