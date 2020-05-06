#include "magent/lib/event_logger/indus_cpu_topology.h"

#include <iostream>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

namespace ecclesia {

namespace {
const IndusCpuTopology::Options &GetDefaultOptions() {
  static const IndusCpuTopology::Options *const options =
      new IndusCpuTopology::Options();
  return *options;
}

}  // namespace

IndusCpuTopology::IndusCpuTopology()
    : apifs_(GetDefaultOptions().apifs_path),
      lpu_to_package_id_(GetCpuTopology()) {}

IndusCpuTopology::IndusCpuTopology(const Options &options)
    : apifs_(options.apifs_path),
      lpu_to_package_id_(GetCpuTopology()) {}

// Since CPU topology is static, we will read it when the object being
// constructed, cache it in vector lpu_to_package_id_.
std::vector<int> IndusCpuTopology::GetCpuTopology() {
  std::string online;
  std::vector<int> ret;

  if (!ReadApifsFile<std::string>("online", &online).ok()) {
    std::cerr << "Failed to find number of CPUs.";
    return ret;
  }
  std::vector<absl::string_view> range = absl::StrSplit(online, '-');

  if (range.size() != 2) {
    std::cerr << "Invalid CPU id range: " << online << '.';
    return ret;
  }

  int from, to;
  if (!absl::SimpleAtoi(range[0], &from) || from != 0) {
    std::cerr << "Invalid CPU id range, from: ", range[0];
    return ret;
  }

  if (!absl::SimpleAtoi(range[1], &to) || to <= 0) {
    std::cerr << "Invalid CPU id range, to: ", range[1];
    return ret;
  }

  // "from" is always 0
  ret.resize(to - from + 1, -1);
  for (int i = from; i <= to; i++) {
    ReadApifsFile<int>(absl::StrCat("cpu", i, "/topology/physical_package_id"),
                       &ret[i])
        .IgnoreError();
  }

  return ret;
}

int IndusCpuTopology::GetSocketIdForLpu(int lpu) const {
  if (lpu >= (lpu_to_package_id_).size()) {
    std::cerr << "LPU index out of range.";
    return -1;
  }
  return lpu_to_package_id_[lpu];
}

std::vector<int> IndusCpuTopology::GetLpusForSocketId(int socket_id) const {
  std::vector<int> lpus;
  for (int i = 0; i < lpu_to_package_id_.size(); i++) {
    if (lpu_to_package_id_[i] == socket_id) {
      lpus.push_back(i);
    }
  }
  return lpus;
}

}  // namespace ecclesia
