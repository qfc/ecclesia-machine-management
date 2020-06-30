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

#include "ecclesia/magent/lib/ipmi/ipmitool.h"

#include <time.h>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "ecclesia/lib/logging/logging.h"
#include "ecclesia/magent/lib/fru/fru.h"

extern "C" {
#include "include/ipmitool/helper.h"
#include "include/ipmitool/ipmi.h"
#include "include/ipmitool/ipmi_constants.h"
#include "include/ipmitool/ipmi_fru.h"
#include "include/ipmitool/ipmi_intf.h"
#include "include/ipmitool/ipmi_mc.h"
#include "include/ipmitool/ipmi_raw.h"
#include "include/ipmitool/ipmi_sdr.h"
#include "include/ipmitool/ipmi_sol.h"
#include "include/ipmitool/log.h"

struct ipmi_intf ipmi_open_intf;
extern const struct valstr completion_code_vals[];

extern int read_fru_area(struct ipmi_intf *intf, struct fru_info *fru,
                         uint8_t id, uint32_t offset, uint32_t length,
                         uint8_t *frubuf);
extern void fru_area_print_board(struct ipmi_intf *intf, struct fru_info *fru,
                                 uint8_t id, uint32_t offset);

extern char *get_fru_area_str(uint8_t *data, uint32_t *offset);

// These two global variables are defined in ipmitool/src/ipmitool.c or
// ipmitool/src/ipmievd.c, we are not including either one,
// So we will define them here.
int csv_output = 0;
int verbose = 0;

}  // extern "C"

namespace ecclesia {

// IPMI Completion Codes.
constexpr uint8_t IPMI_OK_CODE = 0x00;
constexpr uint8_t IPMI_INVALID_CMD_COMPLETION_CODE = 0xC1;
constexpr uint8_t IPMI_TIMEOUT_COMPLETION_CODE = 0xC3;
constexpr uint8_t IPMI_UNKNOWN_ERR_COMPLETION_CODE = 0xff;

namespace {

std::string IpmiResponseToString(uint8_t code) {
  const struct valstr *curr = &completion_code_vals[0];

  // completion_code_vals is a null-entry terminated array.
  while (curr->str != nullptr) {
    if (curr->val == code) return curr->str;

    curr++;
  }

  return "unknown response code";
}

void ConfigureLanPlusInterface(ipmi_intf *intf) {
  // Default is name-only lookup, from ipmitool's ipmi_main.c
  constexpr uint8_t kIpmiDefaultLookupBit = 0x10;

  // Default from table 22-19 of the IPMIv2 spec, from ipmitool's ipmi_main.c
  constexpr uint8_t kIpmiDefaultCipherSuiteId = 3;

  // Default is empty, from ipmitool's ipmi_main.c
  uint8_t kgkey[IPMI_KG_BUFFER_SIZE] = {0};

  // The following values are all defaults taken from the implementation in
  // google3/v1_8_18/lib/ipmi_main.c.
  ipmi_intf_session_set_kgkey(intf, kgkey);
  ipmi_intf_session_set_privlvl(intf, IPMI_SESSION_PRIV_ADMIN);
  ipmi_intf_session_set_lookupbit(intf, kIpmiDefaultLookupBit);
  ipmi_intf_session_set_sol_escape_char(intf, SOL_ESCAPE_CHARACTER_DEFAULT);
  ipmi_intf_session_set_cipher_suite_id(intf, kIpmiDefaultCipherSuiteId);
  intf->devnum = 0;
  intf->devfile = nullptr;
  intf->ai_family = AF_UNSPEC;
  intf->my_addr = IPMI_BMC_SLAVE_ADDR;
  intf->target_addr = IPMI_BMC_SLAVE_ADDR;
}

std::string ReadFruNameInternal(struct sdr_record_fru_locator *fru) {
  return std::string(reinterpret_cast<const char *>(fru->id_string),
                     fru->id_code & 0x1f);
}

}  // namespace

class IpmitoolImpl : public IpmiInterface {
 public:
  struct FreeDeleter {
    inline void operator()(void *ptr) const { free(ptr); }
  };

  template <typename T>
  using SdrRecordUniquePtr = std::unique_ptr<T, FreeDeleter>;

  explicit IpmitoolImpl(
      absl::optional<ecclesia::MagentConfig::IpmiCredential> cred)
      : cred_(std::move(cred)), intf_(GetIpmiIntf()) {}

  std::vector<BmcFruInterfaceInfo> GetAllFrus() override {
    if (frus_cache_.empty()) {
      if (!FindAllFrus().ok()) return {};
    }

    std::vector<BmcFruInterfaceInfo> frus;
    for (const auto &fru_pair : frus_cache_) {
      BmcFruInterfaceInfo fru;
      fru.record_id = fru_pair.first;
      auto entity = fru_pair.second->entity;
      EntityIdentifier fru_entity = {
          entity.id,
          static_cast<uint8_t>((entity.logical << 7) | entity.instance)};
      fru.entity = fru_entity;
      fru.name = ReadFruNameInternal(fru_pair.second.get());
      frus.push_back(fru);
    }
    return frus;
  }

  absl::Status ReadFru(uint16_t fru_id, size_t offset,
                       absl::Span<unsigned char> data) override {
    struct fru_info fru {};

    uint8_t access;
    absl::Status status;

    status = GetFruInfo(intf_, fru_id, &fru.size, &access);
    if (!status.ok()) {
      return status;
    }
    fru.access = access;

    // Maximum output message size for KCS/SMIC is 38 with 2 utility bytes,
    // a byte for completion code and 35 bytes of data.
    // Maximum output message size for BT is 40 with 4 utility bytes, a byte
    // for completion code and 35 bytes of data.
    fru.max_read_size = 35;

    if (read_fru_area(intf_, &fru, fru_id, offset, data.size(), data.data())) {
      return absl::UnknownError(absl::StrFormat(
          "Failed to read_fru_area for fru_id: %d, offset: %d, len: %d.\n",
          fru_id, offset, data.size()));
    }

    return absl::OkStatus();
  }

  absl::Status GetFruSize(uint16_t fru_id, uint16_t *size) override {
    return GetFruInfo(intf_, fru_id, size, nullptr);
  }

 private:
  // A map of FRU numbers to SDR records for them. This map is only modified
  // during construction and then serves as a cache of the read FRU
  // information.
  absl::flat_hash_map<uint16_t,
                      SdrRecordUniquePtr<struct sdr_record_fru_locator>>
      frus_cache_;
  absl::optional<ecclesia::MagentConfig::IpmiCredential> cred_;
  ipmi_intf *intf_;

  ipmi_intf *GetIpmiIntf() {
    if (cred_ == absl::nullopt) {
      ErrorLog() << "Fail to create ipmi interface due to invalid credential.";
      return nullptr;
    }

    ipmi_intf *intf = ipmi_intf_load(cred_->mutable_ipmi_interface()->data());

    if (intf == nullptr) {
      ErrorLog() << "Fail to create ipmi interface due to internal error.";
      return nullptr;
    }

    // Close any currently-active session.
    if (intf->close) {
      intf->close(intf);
    }

    ipmi_intf_session_set_retry(intf, 5);
    ipmi_intf_session_set_timeout(intf, 30);

    ipmi_intf_session_set_hostname(intf,
                                   cred_->mutable_ipmi_hostname()->data());
    ipmi_intf_session_set_port(intf, cred_->ipmi_port());
    ipmi_intf_session_set_username(intf,
                                   cred_->mutable_ipmi_username()->data());
    ipmi_intf_session_set_password(intf,
                                   cred_->mutable_ipmi_password()->data());

    ConfigureLanPlusInterface(intf);

    return intf;
  }

  absl::Status Raw(absl::Span<uint8_t> buffer, ipmi_rs **resp) {
    if (!intf_) {
      ErrorLog() << "Ipmi interface: intf_ is nullptr.";
    }

    const uint8_t *bytes = buffer.data();
    uint32_t len = buffer.size();

    if (len < kMinimumIpmiPacketLength) {
      return absl::InvalidArgumentError("Invalid number of bytes to raw call");
    }

    uint32_t data_len = len - kMinimumIpmiPacketLength;
    uint8_t data[kMaximumPipelineBandwidth]{};
    ipmi_rq request{};

    // Skip beyond netfn and command.
    if (data_len > 0) {
      std::memcpy(data, &bytes[kMinimumIpmiPacketLength], data_len);
    }

    ipmi_intf_session_set_timeout(intf_, 15);
    ipmi_intf_session_set_retry(intf_, 1);

    request.msg.netfn = bytes[0];
    request.msg.lun = 0x00;
    request.msg.cmd = bytes[1];
    request.msg.data = data;
    request.msg.data_len = data_len;

    ipmi_rs *response = intf_->sendrecv(intf_, &request);
    if (nullptr == response) {
      return absl::InternalError("response was NULL from intf->sendrecv");
    }

    if (resp) {
      *resp = response;
    }

    if (response->ccode > 0) {
      if (IPMI_TIMEOUT_COMPLETION_CODE == response->ccode)
        return absl::InternalError("Timeout from IPMI");
      else
        return absl::InternalError(absl::StrCat(
            "Unable to send code: ", IpmiResponseToString(response->ccode)));
    }

    return absl::OkStatus();
  }

  absl::Status SendWithRetry(const IpmiRequest &request, int retries,
                             IpmiResponse *response) {
    ipmi_rs *resp;
    int tries = retries + 1;
    std::vector<uint8_t> buffer(kMinimumIpmiPacketLength + request.data.size());
    buffer[0] = static_cast<uint8_t>(request.network_function);
    buffer[1] = static_cast<uint8_t>(request.command);
    if (!request.data.empty()) {
      std::memcpy(&buffer[kMinimumIpmiPacketLength], request.data.data(),
                  request.data.size());
    }

    int count = 0;
    absl::Status result;
    while (count < tries) {
      result = Raw(absl::MakeSpan(buffer), &resp);
      if (result.ok()) break;
      count++;
    }

    if (!result.ok()) {
      return absl::InternalError(
          absl::StrCat("Failed to send IPMI command after ", count, " tries."));
    }

    response->ccode = resp->ccode;
    response->data =
        std::vector<uint8_t>(resp->data, resp->data + resp->data_len);

    return absl::OkStatus();
  }

  absl::Status Send(const IpmiRequest &request, IpmiResponse *response) {
    return SendWithRetry(request, 0, response);
  }

  // read Fru size and access.
  absl::Status GetFruInfo(ipmi_intf *intf_, uint16_t fru_id, uint16_t *size,
                          uint8_t *access) {
    uint8_t buffer[4]{};
    IpmiResponse rsp;

    IpmiRequest req(kGetFruInfo, absl::MakeSpan(buffer, 4));

    absl::Status status;
    status = Send(req, &rsp);
    if (!status.ok()) {
      return status;
    }

    if (rsp.ccode > 0) {
      return absl::InternalError(
          absl::StrFormat(" Device not present (%s)\n",
                          val2str(rsp.ccode, completion_code_vals)));
    }

    if (size) {
      *size = (rsp.data[1] << 8) | rsp.data[0];
    }
    if (access) {
      *access = rsp.data[2] & 0x1;
    }

    return absl::OkStatus();
  }

  void printBoardInfo(uint16_t fru_id, uint8_t fru_id_string[16]) {
    std::vector<uint8_t> data(72);
    absl::Status status = ReadFru(fru_id, 0, absl::MakeSpan(data));
    if (!status.ok()) {
      ErrorLog() << "ERROR: " << status.message() << '\n';
    }

    VectorFruImageSource fru_image(absl::MakeSpan(data));
    BoardInfoArea bia;
    bia.FillFromImage(fru_image, 8);

    InfoLog() << "FRU Device Description: " << fru_id_string << " (ID "
              << (int)fru_id << ")";
    time_t t = bia.manufacture_date();
    InfoLog() << "Board Mfg Date        : " << asctime(localtime(&t));
    InfoLog() << "Board Mfg             : "
              << bia.manufacturer().GetDataAsString();
    InfoLog() << "Board Product         : "
              << bia.product_name().GetDataAsString();
    InfoLog() << "Board Serial          : "
              << bia.serial_number().GetDataAsString();
    InfoLog() << "Board Part Number     : "
              << bia.part_number().GetDataAsString();
  }

  absl::Status FindAllFrus() {
    if (!intf_) {
      return absl::InternalError("Ipmi interface: intf_ is nullptr.");
    }

    struct ipmi_sdr_iterator *itr = nullptr;
    if ((itr = ipmi_sdr_start(intf_, 0)) == nullptr) {
      return absl::InternalError("Unable to open SDR for reading.");
    }

    absl::Status status;
    struct sdr_get_rs *header;
    struct sdr_record_fru_locator *fru;
    while ((header = ipmi_sdr_get_next_header(intf_, itr)) != nullptr) {
      if (header->type == SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR) {
        fru = reinterpret_cast<struct sdr_record_fru_locator *>(
            ipmi_sdr_get_record(intf_, header, itr));
        if (fru == nullptr || !fru->logical) {
          if (fru) {
            free(fru);
            fru = nullptr;
          }
          InfoLog() << "Fail to get logical frus.";
          continue;
        }
        printBoardInfo(fru->device_id, fru->id_string);

        // We need this line because sdr_get_rs is packed.
        uint16_t id = header->id;
        frus_cache_.emplace(
            id, SdrRecordUniquePtr<struct sdr_record_fru_locator>(fru));
      }
    }
    // Frees the memory allocated by ipmi_sdr_start
    ipmi_sdr_end(intf_, itr);

    return status;
  }
};

Ipmitool::Ipmitool(absl::optional<ecclesia::MagentConfig::IpmiCredential> cred)
    : ipmi_impl_(absl::make_unique<IpmitoolImpl>(cred)) {}

}  // namespace ecclesia
