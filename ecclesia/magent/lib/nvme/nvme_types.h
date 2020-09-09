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

// Structure format defined by NVM-Express 1.3

#ifndef ECCLESIA_MAGENT_LIB_NVME_NVME_TYPES_H_
#define ECCLESIA_MAGENT_LIB_NVME_NVME_TYPES_H_

#include <stddef.h>
#include <string.h>

#include <cstdint>

#include "absl/base/attributes.h"
#include "absl/numeric/int128.h"

namespace ecclesia {

struct PowerStateFormat {
  uint16_t max_power; /* centiwatts */
  uint8_t rsvd2;
  uint8_t flags;
  uint32_t entry_lat; /* microseconds */
  uint32_t exit_lat;  /* microseconds */
  uint8_t read_tput;
  uint8_t read_lat;
  uint8_t write_tput;
  uint8_t write_lat;
  uint16_t idle_power;
  uint8_t idle_scale;
  uint8_t rsvd19;
  uint16_t active_power;
  uint8_t active_work_scale;
  uint8_t rsvd23[9];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kPowerStateSize = 32;
static_assert(sizeof(PowerStateFormat) == kPowerStateSize,
              "Size of PowerStateFormat must be kPowerStateSize");

struct LBAFormat {
  uint16_t metadata_size;
  uint8_t data_size;  // in log2(number of bytes)
  uint8_t relative_performance : 2;
  uint8_t reserved : 6;
  bool operator!=(const LBAFormat &rhs) const {
    return memcmp(this, &rhs, sizeof(*this));
  }
  bool operator==(const LBAFormat &rhs) const { return !(*this != rhs); }
} ABSL_ATTRIBUTE_PACKED;
static_assert(sizeof(LBAFormat) == sizeof(uint32_t),
              "LBAFormat should be 4 bytes");

// Format of the NVM-Express data returned from Identify  Namespace request.
inline constexpr int kIdentifyNamespaceLbaFormatCapacity = 16;
struct IdentifyNamespaceFormat {
  uint64_t size;         // in number of logical blocks
  uint64_t capacity;     // in number of logical blocks
  uint64_t utilization;  // in number of logical blocks
  uint8_t features;
  uint8_t num_lba_formats;
  uint8_t formatted_lba_size;  // an index into the lba_format table below.
  uint8_t metadata_capabilities;
  uint8_t data_protection_capabilities;
  uint8_t data_protection_type;
  uint8_t multi_path;
  uint8_t reservation_capabilities;
  uint8_t format_progress_indicator;
  uint8_t deallocate_logical_block_features;
  uint16_t namespace_atomic_write_unit_normal;
  uint16_t namespace_atomic_write_unit_power_fail;
  uint16_t namespace_atomic_compare_and_write_unit;
  uint16_t namespace_atomic_boundary_size_normal;
  uint16_t namespace_atomic_boundary_offset;
  uint16_t namespace_atomic_boundary_size_power_fail;
  uint16_t namespace_optimal_io_boundary;
  uint64_t nvm_capacity;  // in number of bytes
  char reserved_one[48];
  uint64_t namespace_guid_low;
  uint64_t namespace_guid_high;
  uint64_t ieee_euid;
  LBAFormat lba_format[kIdentifyNamespaceLbaFormatCapacity];
  char reserved_two[192];
  // Vendor Specific
  char reserved_eight[3712];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kIdentifyNamespaceSize = 4096;
static_assert(sizeof(IdentifyNamespaceFormat) == kIdentifyNamespaceSize,
              "Size of IdentifyNamespaceFormat must be kIdentifyNamespaceSize");

// Format of the NVM-Express data returned from Identify Controller request.
// Section 5.15 'Identify command'
struct IdentifyControllerFormat {
  unsigned char vendor_id[2];
  unsigned char subsystem_vendor_id[2];
  char serial_number[20];
  char model_number[40];
  char firmware_revision[8];
  uint8_t rab;
  uint8_t ieee[3];
  uint8_t cmic;
  uint8_t mdts;
  unsigned char cntlid[2];
  uint32_t ver;
  uint32_t rtd3r;
  uint32_t rtd3e;
  uint32_t oaes;
  uint32_t ctratt;
  uint16_t rrls;
  uint8_t rsvd102[154];
  uint16_t oacs;
  uint8_t acl;
  uint8_t aerl;
  uint8_t frmw;
  uint8_t lpa;
  uint8_t elpe;
  uint8_t npss;
  uint8_t avscc;
  uint8_t apsta;
  unsigned char wctemp[2];
  unsigned char cctemp[2];
  uint16_t mtfa;
  uint32_t hmpre;
  uint32_t hmmin;
  uint8_t tnvmcap[16];
  uint8_t unvmcap[16];
  uint32_t rpmbs;
  uint16_t edstt;
  uint8_t dsto;
  uint8_t fwug;
  uint16_t kas;
  uint16_t hctma;
  uint16_t mntmt;
  uint16_t mxtmt;
  unsigned char sanicap[4];
  uint32_t hmminds;
  uint16_t hmmaxd;
  uint16_t nsetidmax;
  uint8_t rsvd340[2];
  uint8_t anatt;
  uint8_t anacap;
  uint32_t anagrpmax;
  uint32_t nanagrpid;
  uint8_t rsvd352[160];
  uint8_t sqes;
  uint8_t cqes;
  uint16_t maxcmd;
  unsigned char nn[4];
  uint16_t oncs;
  uint16_t fuses;
  uint8_t fna;
  uint8_t vwc;
  uint16_t awun;
  uint16_t awupf;
  uint8_t nvscc;
  uint8_t nwpc;
  uint16_t acwu;
  uint8_t rsvd534[2];
  uint32_t sgls;
  uint32_t mnan;
  uint8_t rsvd544[224];
  char subnqn[256];
  uint8_t rsvd1024[768];
  uint32_t ioccsz;
  uint32_t iorcsz;
  uint16_t icdoff;
  uint8_t ctrattr;
  uint8_t msdbd;
  uint8_t rsvd1804[244];
  struct PowerStateFormat psd[32];
  uint8_t vs[1024];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kIdentifyControllerSize = 4096;
static_assert(
    sizeof(IdentifyControllerFormat) == kIdentifyControllerSize,
    "Size of IdentifyControllerFormat must be kIdentifyControllerSize");

// Format of the NVM-Express data returned from Identify List Namespace request.
//
// The maximum number of entries which can fit in this struct.
// Equal to sizeof(IdentifyListNamespaceFormat) / sizeof(uint32_t).
inline constexpr uint32_t kIdentifyListNamespacesCapacity = 1024;
struct IdentifyListNamespaceFormat {
  // An ordered list of NSIDs.  Unused entries will be set to 0.
  uint32_t nsid[kIdentifyListNamespacesCapacity];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kIdentifyListNamespaceSize = 4096;
static_assert(
    sizeof(IdentifyListNamespaceFormat) == kIdentifyListNamespaceSize,
    "Size of IdentifyListNamespaceFormat must be kIdentifyListNamespaceSize");

inline constexpr size_t kNumThermalManagementEntries = 2;
inline constexpr size_t kNumTemperatureSensors = 8;
// Format of the NVM-Express data returned from Smart Log Page read request.
// Section 5.14.1.2 'Smart / Health Information (Log Identifier 02h)'
struct SmartLogPageFormat {
  uint8_t critical_warning;
  uint8_t temperature[2];
  uint8_t available_spare;
  uint8_t available_spare_threshold;
  uint8_t percent_used;
  uint8_t endurance_cw;
  uint8_t rsvd6[25];
  uint8_t data_units_read[16];
  uint8_t data_units_written[16];
  uint8_t host_reads[16];
  uint8_t host_writes[16];
  uint8_t ctrl_busy_time[16];
  uint8_t power_cycles[16];
  uint8_t power_on_hours[16];
  uint8_t unsafe_shutdowns[16];
  uint8_t media_errors[16];
  uint8_t num_err_log_entries[16];
  uint32_t warning_temp_time;
  uint32_t critical_comp_time;
  uint16_t temp_sensor[kNumTemperatureSensors];
  uint32_t thm_temp_trans_count[kNumThermalManagementEntries];
  uint32_t thm_temp_total_time[kNumThermalManagementEntries];
  uint8_t rsvd232[280];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kSmartLogPageSize = 512;
static_assert(sizeof(SmartLogPageFormat) == kSmartLogPageSize,
              "Size of SmartLogPageFormat must be kSmartLogPageSize");

struct SanitizeCDW10Format {
  uint8_t sanact : 3;
  uint8_t ause : 1;
  uint8_t owpass : 4;
  uint8_t oipbp : 1;
  uint8_t no_dealloc : 1;
  uint8_t rsvd1 : 6;
  uint8_t rsvd[2];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kSanitizeCDW10FormatSize = 4;
static_assert(sizeof(SanitizeCDW10Format) == kSanitizeCDW10FormatSize,
              "Size of SanitizeCDW10Format must be kSanitizeCDW10FormatSize");

// Format of the NVM-Express data returned from Sanitize Status Log Page read
// request. Section 5.14.1.9.2 'Sanitize Status (Log Identifier 81h)'
struct SanitizeLogPageFormat {
  uint16_t progress;
  uint8_t recent_sanitize_status : 3;
  uint8_t overwrite_passes : 5;
  uint8_t global_data_erased : 1;
  uint8_t rsvd1 : 7;
  SanitizeCDW10Format cdw10;
  uint32_t estimate_overwrite_time;
  uint32_t estimate_block_erase_time;
  uint32_t estimate_crypto_erase_time;
  uint8_t rsvd[492];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kSanitizeLogPageSize = 512;
static_assert(sizeof(SanitizeLogPageFormat) == kSanitizeLogPageSize,
              "Size of SanitizeLogPageFormat must be kSanitizeLogPageSize");

// A list of Controller IDs.  Used in Namespace Attachment/Detachment.
inline constexpr uint32_t kControllerListCapacity = 2047;
struct ControllerListFormat {
  uint16_t num_identifiers;
  uint16_t identifiers[kControllerListCapacity];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kControllerListSize = 4096;
static_assert(sizeof(ControllerListFormat) == kControllerListSize,
              "Size of ControllerListFormat must be kControllerListSize");

// Format of the Namespace Management command.
struct NamespaceManagementFormat {
  uint64_t size;      // in number of logical blocks
  uint64_t capacity;  // in number of logical blocks
  uint8_t reserved_one[10];
  uint8_t formatted_lba_size;  // an index into the lba_format table.
  uint8_t reserved_two[2];
  uint8_t data_protection_type;
  uint8_t multi_path;
  uint8_t reserved_three[993];
  uint8_t vendor_specific[3072];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kNamespaceManagementSize = 4096;
static_assert(
    sizeof(NamespaceManagementFormat) == kNamespaceManagementSize,
    "Size of NamespaceManagementFormat must be kNamespaceManagementSize");

// Format of the Error Information Log Entry
// Section 5.14.1.1 Error Information (Log Identifier 01h)
struct ErrorLogInfoEntry {
  uint64_t error_count;
  uint16_t submission_queue_id;
  uint16_t command_id;
  uint16_t status;
  uint16_t parameter_error_location;
  uint64_t lba;
  uint32_t namespace_id;
  uint8_t vendor_log_page_id;
  uint8_t tytype;
  uint8_t reserved1[2];
  uint64_t command_specific_info;
  uint16_t trtype_specific_info;
  uint8_t reserved2[22];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kErrorLogInfoEntrySize = 64;
static_assert(sizeof(ErrorLogInfoEntry) == kErrorLogInfoEntrySize,
              "Size of ErrorLogInfoEntry must be kErrorLogInfoEntrySize");

// Section 5.14.1.3 Firmware Slot Information (Log Identifier 03h)
inline constexpr size_t kFirmwareRevisionMaxLen = 8;
inline constexpr size_t kNumFirmwareSlots = 7;
struct FirmwareSlotInfoFormat {
  uint8_t afi;  // Active firmware info
  uint8_t reserved1[7];
  // Firmware Revision for Slots 1-7
  char frs[kNumFirmwareSlots][kFirmwareRevisionMaxLen];
  uint8_t reserved2[448];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kFirmwareSlotInfoFormatSize = 512;
static_assert(sizeof(FirmwareSlotInfoFormat) == kFirmwareSlotInfoFormatSize,
              "Size of FirmwareSlotInfo must be kFirmwareSlotInfoFormatSize");

// Section 5.14.1.6 Device Self-test (Log Identifier 06h)
struct DeviceSelfTestResultFormat {
  uint8_t status;
  uint8_t segment_number;
  uint8_t sc_valid : 1;
  uint8_t sct_valid : 1;
  uint8_t flba_valid : 1;
  uint8_t nsid_valid : 1;
  uint8_t reserved1 : 4;
  uint8_t reserved2;
  uint64_t power_on_hours;
  uint32_t nsid;
  uint64_t flba;
  uint8_t status_code_type;
  uint8_t status_code;
  uint16_t vendor_specific;
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kDeviceSelfTestResultFormatSize = 28;
static_assert(sizeof(DeviceSelfTestResultFormat) ==
                  kDeviceSelfTestResultFormatSize,
              "Size of DeviceSelfTestResultFormat must be "
              "kDeviceSelfTestResultFormatSize");

inline constexpr size_t kMaxNumDeviceSelfTests = 20;

struct DeviceSelfTestLogFormat {
  uint8_t current_self_test_operation;
  uint8_t current_self_test_completion;
  uint8_t reserved1[2];
  DeviceSelfTestResultFormat results[kMaxNumDeviceSelfTests];
} ABSL_ATTRIBUTE_PACKED;

inline constexpr size_t kDeviceSelfTestLogFormatSize = 564;
static_assert(
    sizeof(DeviceSelfTestLogFormat) == kDeviceSelfTestLogFormatSize,
    "Size of DeviceSelfTestLogFormat must be kDeviceSelfTestLogFormatSize");

// Section 5.24 Sanitize Action possible values.
enum SanitizeAction {
  kSanitizeExitFailureMode = 0x1,
  kSanitizeBlockErase = 0x2,
  kSanitizeOverwrite = 0x3,
  kSanitizeCryptoErase = 0x4
};

// Section 5.15 Identify Command Sanitize Capability mask.
enum SanitizeCapabilityMask {
  kCryptoEraseCapabilityMask = 0x1,
  kBlockEraseCapabilityMask = 0x2,
  kOverwriteCapabilityMask = 0x4
};

// Section 5.14.1.9.2 Get Log Page Sanitize Status(SSTAT).
enum SanitizeStatus {
  kSanitizeNeverBeenSanitized = 0x0,
  kSanitizeSuccess = 0x1,
  kSanitizeInProgress = 0x2,
  kSanitizeFailed = 0x3,
};

}  // namespace ecclesia

#endif  // ECCLESIA_MAGENT_LIB_NVME_NVME_TYPES_H_
