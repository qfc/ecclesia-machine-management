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

#ifndef ECCLESIA_MAGENT_LIB_IO_PCI_CONSTS_H_
#define ECCLESIA_MAGENT_LIB_IO_PCI_CONSTS_H_

#include "magent/lib/io/pci_regs.h"

namespace ecclesia {

// Type of address used in a BAR.
enum class BarType { kMem = 0, kIo = 1 };

// Possible types of the PCI configuration header.
enum class ConfigHeaderType {
  kUnknown = -1,
  kEndpoint = kPciHdrTypeEndpoint,
  kBridge = kPciHdrTypePciBridge,
  kCardbus = kPciHdrTypeCardbus
};

// The type of physical transport a device is accessed through.
enum class TransportType { kPci, kPciX, kPciExpress };

// PCI device class code values.
enum class ClassType {
  kUnknown = 0x00,
  kStorage = 0x01,
  kNetwork = 0x02,
  kDisplay = 0x03,
  kMultimedia = 0x04,
  kMemory = 0x05,
  kBridge = 0x06,
  kComm = 0x07,
  kSystem = 0x08,
  kInput = 0x09,
  kDocking = 0x0a,
  kProcessor = 0x0b,
  kBus = 0x0c,
  kWireless = 0x0d,
  kIo = 0x0e,
  kSatellite = 0x0f,
  kCrypto = 0x10,
  kSignal = 0x11,
};

// The extended device type for PCI Express devices.
enum class kPciExpressDeviceType {
  kEndpoint = 0,
  kLegacyEndpoint = 1,
  kRootComplex = 4,
  kUpstreamSwitch = 5,
  kDownstreamSwitch = 6,
  kPciEtoPciBridge = 7,
  kPcitoPciEBridge = 8,
  kRootComplexIntegratedEnpoint = 9,
  kRootComplexEventCollector = 10
};

enum class PciExpressLinkSpeed {
  kSpeed2_5G = 1,
  kSpeed5_0G = 2,
  kSpeed8_0G = 3,
  kSpeed16_0G = 4
};

enum class PciExpressLinkWidth {
  kX1 = 1,
  kX2 = 2,
  kX4 = 4,
  kX8 = 8,
  kX12 = 12,
  kX16 = 16,
  kX32 = 32
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_CONSTS_H_
