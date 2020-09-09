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

#ifndef ECCLESIA_MAGENT_LIB_IO_PCI_REGS_H_
#define ECCLESIA_MAGENT_LIB_IO_PCI_REGS_H_

#include <cstdint>

namespace ecclesia {

// https://en.wikipedia.org/wiki/File:Pci-config-space.svg
// Common Configuration Space Registers Between Type 0 And Type 1 Devices.
constexpr int8_t kPciVidReg = 0x00;
constexpr int8_t kPciDidReg = 0x02;
constexpr int8_t kPciCommandReg = 0x04;
constexpr int8_t kPciStatusReg = 0x06;
constexpr int8_t kPciRevisionIdReg = 0x08;
constexpr int8_t kPciProgIfReg = 0x09;
// In PCI config space, the "class code" spans three registers with offsets
// 0x09-0x0b, where upper byte (0x0b) is the base class code; middle byte (0x0a)
// is the sub-class code; lower byte (0x09) is the Prog IF.
constexpr int8_t kPciClassCodeReg = 0x09;
constexpr int8_t kPciCacheLineSizeReg = 0x0c;
constexpr int8_t kPciLatencyTimerReg = 0x0d;
constexpr int8_t kPciHeaderTypeReg = 0x0e;
constexpr int8_t kPciHdrTypeEndpoint = 0x00;
constexpr int8_t kPciHdrTypePciBridge = 0x01;
constexpr int8_t kPciHdrTypeCardbus = 0x02;
constexpr int8_t kPciBistReg = 0x0f;
constexpr int8_t kPciCapPointerReg = 0x34;
constexpr int8_t kPciInterruptLineReg = 0x3c;
constexpr int8_t kPciInterruptPinReg = 0x3d;

// Type 0 Configuration Space.
constexpr uint8_t kPciType0Bar0Reg = 0x10;
constexpr uint8_t kPciType0Bar1Reg = 0x14;
constexpr uint8_t kPciType0Bar2Reg = 0x18;
constexpr uint8_t kPciType0Bar3Reg = 0x1c;
constexpr uint8_t kPciType0Bar4Reg = 0x20;
constexpr uint8_t kPciType0Bar5Reg = 0x24;
constexpr uint8_t kPciType0CardbusCisReg = 0x28;
constexpr uint8_t kPciType0SubsysVendorIdReg = 0x2c;
constexpr uint8_t kPciType0SubsysIdReg = 0x2e;
constexpr uint8_t kPciType0ExpRomBarReg = 0x30;
constexpr uint8_t kPciType0MinGntReg = 0x3e;
constexpr uint8_t kPciType0MaxLatReg = 0x3f;

// Type 1 Configuration Space.
constexpr uint8_t kPciType1Bar0Reg = 0x10;
constexpr uint8_t kPciType1Bar1Reg = 0x14;
constexpr uint8_t kPciType1PriBusNumReg = 0x18;
constexpr uint8_t kPciType1SecBusNumReg = 0x19;
constexpr uint8_t kPciType1SubBusNumReg = 0x1a;
constexpr uint8_t kPciType1SecLatTmrReg = 0x1b;
constexpr uint8_t kPciType1IoBaseReg = 0x1c;
constexpr uint8_t kPciType1IoLimitReg = 0x1d;
constexpr uint8_t kPciType1SecStatusReg = 0x1e;
constexpr uint8_t kPciType1MemBaseReg = 0x20;
constexpr uint8_t kPciType1MemLimitReg = 0x22;
constexpr uint8_t kPciType1PrefetchMemBaseReg = 0x24;
constexpr uint8_t kPciType1PrefetchMemLimitReg = 0x26;
constexpr uint8_t kPciType1PrefetchMemBaseUpperReg = 0x28;
constexpr uint8_t kPciType1PrefetchMemLimitUpperReg = 0x2c;
constexpr uint8_t kPciType1IoBaseUpperReg = 0x30;
constexpr uint8_t kPciType1IoLimitUpperReg = 0x32;
constexpr uint8_t kPciType1ExpRomBarReg = 0x38;
constexpr uint8_t kPciType1BridgeControlReg = 0x3e;

// Pci Capability Definitions.
constexpr uint8_t kPciCapListIdOffset = 0x00;
constexpr uint8_t kPciCapListNextOffset = 0x01;

// Power Management Capability.
constexpr uint8_t kPciCapIdPowerManagement = 0x01;
constexpr uint8_t kPciCapPowerManagementPmc = 0x02;
constexpr uint8_t kPciCapPowerManagementPmcsr = 0x04;
constexpr uint8_t kPciCapPowerManagementData = 0x07;

// Bridge Subsystem Capability.
constexpr uint8_t kPciCapIdBridgeSubsystem = 0x0d;
constexpr uint8_t kPciCapBridgeSubsystemVidOffset = 0x04;
constexpr uint8_t kPciCapBridgeSubsystemIdOffset = 0x06;

// Pci-x Capability.
constexpr uint8_t kPciCapIdPcix = 0x07;
constexpr uint8_t kPciCapPcixCmdOffset = 0x02;
constexpr uint8_t kPciCapPcixStatusOffset = 0x04;

// Pci Express Capability.
constexpr uint8_t kPciCapIdPcie = 0x10;
constexpr uint8_t kPciCapPcieCapsOffset = 0x02;
constexpr uint8_t kPciCapPcieDevCapsOffset = 0x04;
constexpr uint8_t kPciCapPcieDevControlOffset = 0x08;
constexpr uint8_t kPciCapPcieDevStatusOffset = 0x0a;
constexpr uint8_t kPciCapPcieLinkCapsOffset = 0x0c;
constexpr uint8_t kPciCapPcieLinkControlOffset = 0x10;
constexpr uint8_t kPciCapPcieLinkStatusOffset = 0x12;
constexpr uint8_t kPciCapPcieSlotCapsOffset = 0x14;
constexpr uint8_t kPciCapPcieSlotControlOffset = 0x18;
constexpr uint8_t kPciCapPcieSlotStatusOffset = 0x1a;
constexpr uint8_t kPciCapPcieRootControlOffset = 0x1c;
constexpr uint8_t kPciCapPcieRootCapsOffset = 0x1e;
constexpr uint8_t kPciCapPcieRootStatusOffset = 0x20;

// Pci Msi-x Capability
constexpr uint8_t kPciCapIdMsix = 0x11;
constexpr uint8_t kPciCapIdMsixMcOffset = 0x02;
constexpr uint8_t kPciCapIdMsixTableOffset = 0x04;
constexpr uint8_t kPciCapIdMsixPbaOffset = 0x08;

// Pci Express Extended Capabilities.
constexpr uint16_t kPcieExtCapStartReg = 0x100;

// Pci Express Advanced Error Reporting Capability.
constexpr uint16_t kPcieExtCapIdAer = 0x0001;
constexpr uint16_t kPcieExtCapVersionAer = 0x1;
constexpr uint16_t kPcieExtCapAerNoncomplexSize = 44;
constexpr uint16_t kPcieExtCapAerComplexSize = 56;

// Pci Express Device Serial Number Capability.
constexpr uint16_t kPcieExtCapIdDsn = 0x0003;
constexpr uint16_t kPcieExtCapVersionDsn = 0x1;
constexpr uint16_t kPcieExtCapDsnLower = 0x4;
constexpr uint16_t kPcieExtCapDsnUpper = 0x8;

// Pci Express Access Control Services Capability
constexpr uint16_t kPcieExtCapIdAcs = 0x00;
constexpr uint16_t kPcieExtCapVersionAcs = 0x1;
constexpr uint16_t kPcieExtCapAcsComplexSize = 8;

// Secondary Pci Express Extended Capability.
constexpr uint16_t kSecondaryPcieExtCapId = 0x0019;
constexpr uint16_t kSecondaryPcieExtCapVersion = 0x1;
constexpr uint16_t kSecondaryPcieExtCapComplexSize = 140;

// Pci Express Link Status Accessors.
constexpr uint32_t kPciCapPcieLinkStatusSpeedMask = (0x7 << 0);
constexpr uint32_t kPciCapPcieLinkStatusWidthMask = (0x3f << 4);
constexpr uint32_t kPciCapPcieLinkStatusTrainingMask = (1 << 11);
constexpr uint32_t kPciCapPcieLinkStatusSlotClockConfigMask = (1 << 12);
constexpr uint32_t kPciCapPcieLinkStatusDataLinkLayerLinkActiveMask = (1 << 13);
constexpr uint32_t kPciCapPcieLinkStatusBandwidthManamgementMask = (1 << 14);
constexpr uint32_t kPciCapPcieLinkStatusAutonomousBandwidthMask = (1 << 15);

constexpr uint32_t kPciCapPcieLinkSpeed25 = 0x1;
constexpr uint32_t kPciCapPcieLinkSpeed50 = 0x2;

constexpr uint16_t kPciCapPcieLinkWidthX1 = 0x010;
constexpr uint16_t kPciCapPcieLinkWidthX2 = 0x020;
constexpr uint16_t kPciCapPcieLinkWidthX4 = 0x040;
constexpr uint16_t kPciCapPcieLinkWidthX8 = 0x080;
constexpr uint16_t kPciCapPcieLinkWidthX16 = 0x100;
constexpr uint16_t kPciCapPcieLinkWidthX32 = 0x200;

// Hypertransport Capability.
constexpr uint32_t kPciCapIdHypertransport = 0x08;
constexpr uint32_t kPciCapHypertransportCmdOffset = 0x02;

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_IO_PCI_REGS_H_
