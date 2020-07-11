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

#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "ecclesia/lib/file/test_filesystem.h"
#include "ecclesia/magent/lib/ipmi/ipmitool_mock.h"
#include "ecclesia/lib/logging/logging.h"

extern "C" {
#include "include/ipmitool/ipmi.h"  // IWYU pragma: keep
#include "include/ipmitool/ipmi_google.h"  // IWYU pragma: keep
#include "include/ipmitool/ipmi_intf.h"  // IWYU pragma: keep
#include "include/ipmitool/ipmi_mc.h"  // IWYU pragma: keep
#include "include/ipmitool/ipmi_sdr.h"  // IWYU pragma: keep
#include "include/ipmitool/ipmi_sel.h"  // IWYU pragma: keep
#include "include/ipmitool/ipmi_sensor.h"  // IWYU pragma: keep
}  // extern "C"

namespace ecclesia {

namespace {

using ::testing::_;
using ::testing::A;
using ::testing::Return;
using ::testing::ReturnNull;

class IpmitoolTest : public testing::Test {
 public:
  IpmitoolTest() : fs_(GetTestTempdirPath()) {}

 protected:
  TestFilesystem fs_;
};

class IpmiTest : public ::testing::Test {
 public:
  IpmiTest() : fs_(GetTestTempdirPath()) {
    MockIpmitool::SetUp();
    // By default the Ipmitool ctor and dtor will call these respective funcs.
    EXPECT_IPMITOOL_CALL(ipmi_sdr_start, A<struct ipmi_intf *>(), 0)
        .Times(testing::AnyNumber())
        .WillRepeatedly(ReturnNull());
    EXPECT_IPMITOOL_CALL(ipmi_sdr_end, A<struct ipmi_intf *>(), _)
        .Times(testing::AnyNumber());

    fs_.CreateDir("/etc/google/magent");
    fs_.CreateFile("/etc/google/magent/config.pb", "123");
  }

  ~IpmiTest() override { MockIpmitool::TearDown(); }

 protected:
  TestFilesystem fs_;
};

// Builder class for constructing SdrRecords for sensors. Default values are
// zero for convenience.
class SdrSensorRecordBuilder {
 public:
  SdrSensorRecordBuilder() {}

  SdrSensorRecordBuilder &SetId(uint8 sensor_id) {
    sensor_id_ = sensor_id;
    return *this;
  }

  SdrSensorRecordBuilder &SetLun(uint8 lun) {
    lun_ = lun;
    return *this;
  }

  SdrSensorRecordBuilder &SetName(const std::string &name) {
    sensor_name_ = name;
    return *this;
  }

  SdrSensorRecordBuilder &SetRawSensorUnits(uint8 units) {
    raw_sensor_units_ = units;
    return *this;
  }

  SdrSensorRecordBuilder &SetSettable(bool settable) {
    settable_ = settable;
    return *this;
  }

  SdrSensorRecordBuilder &SetEntityId(uint8 id, uint8 instance) {
    entity_id_ = IpmiInterface::EntityIdentifier{id, instance};
    return *this;
  }

  // Allocates memory for a full sensor record with the given builder. The
  // caller is responsible for calling free() on the returned memory.
  struct sdr_record_full_sensor *Build() {
    auto *sensor = static_cast<struct sdr_record_full_sensor *>(
        malloc(sizeof(struct sdr_record_full_sensor)));
    memset(sensor, 0, sizeof(struct sdr_record_full_sensor));
    sensor->cmn.keys.sensor_num = sensor_id_;
    sensor->cmn.keys.lun = lun_;
    sensor->cmn.unit.type.base = raw_sensor_units_;
    sensor->id_code = sensor_name_.size() <= sizeof(sensor->id_string)
                          ? sensor_name_.size()
                          : sizeof(sensor->id_string);
    memset(sensor->id_string, 0, sizeof(sensor->id_string));
    memcpy(sensor->id_string, sensor_name_.c_str(), sensor->id_code);
    sensor->cmn.sensor.init.settable = settable_;
    return sensor;
  }

 private:
  uint8 sensor_id_ = 0;
  uint8 lun_ = 0;
  std::string sensor_name_ = "";
  uint8 raw_sensor_units_ = 0;
  bool settable_ = false;
  IpmiInterface::EntityIdentifier entity_id_ = {0, 0};
};

// Class for constructing SdrRecords for FRUs. Default values are mostly zero,
// expecting a logical FRU device.
class SdrFruRecordBuilder {
 public:
  SdrFruRecordBuilder &SetDevSlaveAddr(uint8 slave_addr) {
    fru_slave_addr_ = slave_addr;
    return *this;
  }

  SdrFruRecordBuilder &SetDevId(uint8 dev_id) {
    fru_device_id_ = dev_id;
    return *this;
  }

  SdrFruRecordBuilder &SetLogicalLunAndBus(bool logical, uint8 lun, uint8 bus) {
    logical_ = logical;
    lun_ = lun;
    bus_id_ = bus;
    return *this;
  }

  SdrFruRecordBuilder &SetChannel(uint8 channel_num) {
    channel_num_ = channel_num;
    return *this;
  }

  SdrFruRecordBuilder &SetDevTypeAndMod(uint8 dev_type, uint8 dev_mod) {
    device_type_ = dev_type;
    device_type_mod_ = dev_mod;
    return *this;
  }

  SdrFruRecordBuilder &SetEntity(uint8 id, uint8 instance) {
    entity_ = {id, instance};
    return *this;
  }

  SdrFruRecordBuilder &SetOem(uint8 oem) {
    oem_ = oem;
    return *this;
  }

  SdrFruRecordBuilder &SetFruName(std::string fru_name) {
    fru_name_ = fru_name;
    return *this;
  }

  struct sdr_record_fru_locator *Build() {
    auto *fru = static_cast<struct sdr_record_fru_locator *>(
        malloc(sizeof(struct sdr_record_fru_locator)));
    // 0-out the record.
    memset(fru, 0, sizeof(struct sdr_record_fru_locator));
    fru->dev_slave_addr = fru_slave_addr_;
    fru->device_id = fru_device_id_;
    fru->logical = logical_;
    fru->lun = lun_;
    fru->bus = bus_id_;
    fru->dev_type = device_type_;
    fru->dev_type_modifier = device_type_mod_;
    fru->oem = oem_;
    fru->entity = {entity_.entity_id, entity_.entity_instance};
    // Fru name must be at most 16 bytes.
    CheckCondition(sizeof(fru_name_.c_str()) <= 16);
    size_t str_len = fru_name_.size() <= sizeof(fru->id_string)
                         ? fru_name_.size()
                         : sizeof(fru->id_string);
    memset(fru->id_string, 0, sizeof(fru->id_string));
    memcpy(fru->id_string, fru_name_.c_str(), str_len);
    return fru;
  }

 private:
  uint8 fru_device_id_ = 0;
  uint8 fru_slave_addr_ = 0;
  // If true, device is a logical FRU device.
  bool logical_ = true;
  // 2-bit LUN, must be 00b if device is directly on IPMB
  uint8 lun_ = 0b00;
  // Private bus ID, 3 bits. 000b if device directly on IPMB or device is a
  // logical FRU device.
  uint8 bus_id_ = 0b000;
  // 4-bit channel number, in the actual record, the 4th bit here is stored in
  // the next byte.
  uint8 channel_num_ = 0b0000;
  uint8 device_type_ = 0;
  uint8 device_type_mod_ = 0;
  IpmiInterface::EntityIdentifier entity_ = {0, 0};
  uint8 oem_;
  std::string fru_name_ = "";
};

// Mock the sendrecv call for ipmi_intf, return nullptr.
ipmi_rs *sendrecv(struct ipmi_intf *intf, struct ipmi_rq *req) {
  return nullptr;
}

TEST_F(IpmiTest, GetAllFrus) {
  struct ipmi_sdr_iterator itr = {};
  struct sdr_get_rs fru_header_one;
  fru_header_one.type = SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR;
  fru_header_one.id = 0x06;
  struct sdr_get_rs fru_header_two;
  fru_header_two.type = SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR;
  fru_header_two.id = 0x11;

  struct ipmi_intf intf = {
      .name = "lanplus",
      .desc = "IPMI Interface Over Lan+",
      .setup = nullptr,
      .open = nullptr,
      .close = nullptr,
      .sendrecv = sendrecv,
  };

  // EXPECT_IPMITOOL_CALL(ipmi_sdr_start, A<struct ipmi_intf *>(), 0)
  EXPECT_IPMITOOL_CALL(ipmi_sdr_start, _, 0).WillOnce(Return(&itr));

  EXPECT_IPMITOOL_CALL(ipmi_intf_load, _).WillOnce(Return(&intf));

  EXPECT_IPMITOOL_CALL(ipmi_sdr_get_next_header, A<struct ipmi_intf *>(), _)
      .WillOnce(Return(&fru_header_one))
      .WillOnce(Return(&fru_header_two))
      .WillOnce(ReturnNull());

  EXPECT_IPMITOOL_CALL(ipmi_sdr_get_record, A<struct ipmi_intf *>(),
                       A<struct sdr_get_rs *>(),
                       A<struct ipmi_sdr_iterator *>())
      .WillOnce(
          Return(reinterpret_cast<uint8 *>(SdrFruRecordBuilder()
                                               .SetDevSlaveAddr(0)
                                               .SetDevId(0x13)
                                               .SetLogicalLunAndBus(true, 0, 0)
                                               .SetDevTypeAndMod(0x10, 0)
                                               .SetEntity(0x08, 0x01)
                                               .Build())))
      .WillOnce(
          Return(reinterpret_cast<uint8 *>(SdrFruRecordBuilder()
                                               .SetDevSlaveAddr(0)
                                               .SetDevId(0x16)
                                               .SetLogicalLunAndBus(true, 0, 0)
                                               .SetDevTypeAndMod(0x10, 0)
                                               .SetEntity(0x08, 0x02)
                                               .Build())));

  ecclesia::MagentConfig::IpmiCredential cred{};
  cred.set_ipmi_interface("lanplus");
  cred.set_ipmi_username("user");
  cred.set_ipmi_password("pass");
  cred.set_ipmi_hostname("host");
  cred.set_ipmi_port(123);
  Ipmitool ipmi(cred);
  auto frus = ipmi.GetAllFrus();
  EXPECT_EQ(frus.size(), 2);
}

TEST_F(IpmiTest, GetAllFrusInvalidCredential) {
  struct ipmi_sdr_iterator itr = {};
  struct sdr_get_rs fru_header_one;
  fru_header_one.type = SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR;
  fru_header_one.id = 0x06;
  struct sdr_get_rs fru_header_two;
  fru_header_two.type = SDR_RECORD_TYPE_FRU_DEVICE_LOCATOR;
  fru_header_two.id = 0x11;

  struct ipmi_intf intf = {
      .name = "lanplus",
      .desc = "IPMI Interface Over Lan+",
      .setup = nullptr,
      .open = nullptr,
      .close = nullptr,
      .sendrecv = sendrecv,
  };

  // EXPECT_IPMITOOL_CALL(ipmi_sdr_start, A<struct ipmi_intf *>(), 0)
  EXPECT_IPMITOOL_CALL(ipmi_sdr_start, _, 0).WillOnce(Return(&itr));

  EXPECT_IPMITOOL_CALL(ipmi_intf_load, _).WillOnce(Return(&intf));

  EXPECT_IPMITOOL_CALL(ipmi_sdr_get_next_header, A<struct ipmi_intf *>(), _)
      .WillOnce(Return(&fru_header_one))
      .WillOnce(Return(&fru_header_two))
      .WillOnce(ReturnNull());

  EXPECT_IPMITOOL_CALL(ipmi_sdr_get_record, A<struct ipmi_intf *>(),
                       A<struct sdr_get_rs *>(),
                       A<struct ipmi_sdr_iterator *>())
      .WillOnce(
          Return(reinterpret_cast<uint8 *>(SdrFruRecordBuilder()
                                               .SetDevSlaveAddr(0)
                                               .SetDevId(0x13)
                                               .SetLogicalLunAndBus(true, 0, 0)
                                               .SetDevTypeAndMod(0x10, 0)
                                               .SetEntity(0x08, 0x01)
                                               .Build())))
      .WillOnce(
          Return(reinterpret_cast<uint8 *>(SdrFruRecordBuilder()
                                               .SetDevSlaveAddr(0)
                                               .SetDevId(0x16)
                                               .SetLogicalLunAndBus(true, 0, 0)
                                               .SetDevTypeAndMod(0x10, 0)
                                               .SetEntity(0x08, 0x02)
                                               .Build())));

  ecclesia::MagentConfig::IpmiCredential cred{};
  cred.set_ipmi_interface("lanplus");
  cred.set_ipmi_username("user");
  cred.set_ipmi_password("pass");
  cred.set_ipmi_hostname("host");
  cred.set_ipmi_port(123);
  Ipmitool ipmi(cred);
  auto frus = ipmi.GetAllFrus();
  EXPECT_EQ(frus.size(), 2);
}

}  // namespace

}  // namespace ecclesia
