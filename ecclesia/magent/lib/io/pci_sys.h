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

// A class for access PCI devices through sysfs

#ifndef ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_
#define ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_

#include "absl/status/status.h"
#include "absl/strings/str_format.h"
#include "lib/apifs/apifs.h"
#include "magent/lib/io/pci.h"

namespace ecclesia {

class SysPciRegion : public PciRegion {
 public:
  explicit SysPciRegion(PciLocation pci_loc);

  absl::Status Read8(size_t offset, uint8_t *data) override;
  absl::Status Write8(size_t offset, uint8_t data) override;

  absl::Status Read16(size_t offset, uint16_t *data) override;
  absl::Status Write16(size_t offset, uint16_t data) override;

  absl::Status Read32(size_t offset, uint32_t *data) override;
  absl::Status Write32(size_t offset, uint32_t data) override;

 private:
  PciLocation loc_;
  ApifsFile apifs_;
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_SYS_H_
