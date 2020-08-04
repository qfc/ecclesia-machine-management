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

#ifndef ECCLESIA_LIB_FILE_DIR_H_
#define ECCLESIA_LIB_FILE_DIR_H_

#include <dirent.h>

#include <cstdlib>
#include <string>

#include "absl/strings/string_view.h"

namespace ecclesia {

// Iterates over a list of all filenames in directory, invoking output_func with
// each filename. The filename will be passed as a string_view whose underlying
// buffer will be released at the end of the WithEachFileInDirectory call.
//
// The paths passed to output_func will be directory entries, not full paths
// (e.g. a file "/tmp/myfile.txt" in dirname "/tmp" will be passed as
// "myfile.txt"). If no files were found or there are any errors, output_func
// will not be called.
template <typename F>
void WithEachFileInDirectory(absl::string_view dirname, F output_func) {
  // Constants for the names of the pseudo directory entries.
  static constexpr absl::string_view kCurrentDir = ".";
  static constexpr absl::string_view kParentDir = "..";

  class ScandirCloser {
   public:
    ScandirCloser(struct dirent **namelist, int n)
        : namelist_(namelist), n_(n) {}
    ~ScandirCloser() {
      int i = n_;
      while (i--) free(namelist_[i]);
      free(namelist_);
    }

   private:
    struct dirent **namelist_;
    int n_;
  };

  struct dirent **namelist;
  if (int n = scandir(dirname.data(), &namelist, nullptr, nullptr); n >= 0) {
    ScandirCloser closer(namelist, n);
    while (n--) {
      absl::string_view directory_entry = namelist[n]->d_name;
      // Skip the entries which don't correspond to real entries.
      if (directory_entry == kCurrentDir || directory_entry == kParentDir) {
        continue;
      }
      // Call the provided output function with the name.
      output_func(directory_entry);
    }
  }
}

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_FILE_DIR_H_
