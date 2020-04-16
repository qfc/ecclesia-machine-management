// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lib/devpath/devpath.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "re2/re2.h"

namespace ecclesia {
namespace {

// Given a string, split it into 1-3 pieces using ":" as a divider. If the
// string is a devpath then this we break it into path and suffix parts.
std::vector<absl::string_view> SplitDevpathCandidateIntoParts(
    absl::string_view str) {
  return absl::StrSplit(str, absl::MaxSplits(':', 2));
}

};  // namespace

bool IsValidDevpath(absl::string_view path) {
  auto parts = SplitDevpathCandidateIntoParts(path);

  // Check that there are no empty parts. There can be missing parts, but if a
  // devpath has a suffix namespace or text it cannot be empty.
  if ((parts.size() > 1 && parts[1].empty()) ||
      (parts.size() > 2 && parts[2].empty())) {
    return false;
  }

  // Check that the path component is valid.
  // TODO(b/148420839): Remove '@' from permitted path regexp once connectors
  // are no longer permitted to have '@'.
  static const LazyRE2 kPathRegex = {"/phys(?:/[0-9a-zA-Z-_@]+)*"};
  if (!RE2::FullMatch(parts[0], *kPathRegex)) return false;

  // Check that the namespace is valid.
  if (parts.size() == 1) {
    return true;  // No suffix and the path is valid.
  } else if (parts[1] == "connector" && parts.size() == 3) {
    // TODO(b/148420839): Remove '@' from permitted connector regex.
    static const LazyRE2 kConnectorRegex = {"[0-9a-zA-Z-_@]+"};
    return RE2::FullMatch(parts[2], *kConnectorRegex);
  } else if (parts[1] == "device" && parts.size() == 3) {
    static const LazyRE2 kDeviceRegex = {
        "[0-9a-zA-Z-_@.]+(?::[0-9a-zA-Z-_@.]+)*"};
    return RE2::FullMatch(parts[2], *kDeviceRegex);
  }
  return false;
}

DevpathComponents GetDevpathComponents(absl::string_view path) {
  auto parts = SplitDevpathCandidateIntoParts(path);
  DevpathComponents components;
  components.path = parts[0];
  if (parts.size() > 1) components.suffix_namespace = parts[1];
  if (parts.size() > 2) components.suffix_text = parts[2];
  return components;
}

absl::string_view GetDevpathPlugin(absl::string_view path) {
  return GetDevpathComponents(path).path;
}

std::string MakeDevpathFromComponents(const DevpathComponents &components) {
  std::string path = std::string(components.path);
  if (!components.suffix_namespace.empty()) {
    absl::StrAppend(&path, ":", components.suffix_namespace);
    if (!components.suffix_text.empty()) {
      absl::StrAppend(&path, ":", components.suffix_text);
    }
  }
  return path;
}

}  // namespace ecclesia
