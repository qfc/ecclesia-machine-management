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

#ifndef ECCLESIA_MAGENT_LIB_IPMI_IPMITOOL_MOCK_H_
#define ECCLESIA_MAGENT_LIB_IPMI_IPMITOOL_MOCK_H_

#include <cstdint>

#include "gmock/gmock.h"

// Forward declarations
struct ipmi_sdr_iterator;
struct ipmi_intf;
struct sdr_get_rs;
struct ipmi_rq;
struct ipmi_rs;
struct sdr_record_full_sensor;
struct ipm_devid_rsp;
struct sel_info;
struct sel_event_record;

namespace ecclesia {

// This class allows us to mock calls to ipmitool's function.
class MockIpmitool {
 public:
  MockIpmitool(const MockIpmitool &) = delete;
  MockIpmitool &operator=(const MockIpmitool &) = delete;

  // Get a pointer to the global instance of this Mock.
  static MockIpmitool *Get() {
    if (instance == nullptr) {
      instance = new MockIpmitool();
    }
    return instance;
  }

  static void SetUp() { Get(); }

  static void TearDown() {
    delete instance;
    instance = nullptr;
  }

  MOCK_METHOD(struct ipmi_sdr_iterator *, ipmi_sdr_start,
              (struct ipmi_intf * intf, int use_builtin));

  MOCK_METHOD(struct sdr_get_rs *, ipmi_sdr_get_next_header,
              (struct ipmi_intf * intf, struct ipmi_sdr_iterator *itr));

  MOCK_METHOD(uint8 *, ipmi_sdr_get_record,
              (struct ipmi_intf * intf, struct sdr_get_rs *header,
               struct ipmi_sdr_iterator *itr));

  MOCK_METHOD(void, ipmi_sdr_end,
              (struct ipmi_intf * intf, struct ipmi_sdr_iterator *itr));

  MOCK_METHOD(struct ipmi_rs *, ipmi_sendrecv,
              (struct ipmi_intf * intf, struct ipmi_rq *rq));

  MOCK_METHOD(struct ipmi_intf *, ipmi_intf_load,
              (char *name));

  MockIpmitool() {}
  virtual ~MockIpmitool() {}

 private:
  static MockIpmitool *instance;
};

MockIpmitool *MockIpmitool::instance = nullptr;

extern "C" {
struct ipmi_sdr_iterator *ipmi_sdr_start(struct ipmi_intf *intf,
                                         int use_builtin) {
  return MockIpmitool::Get()->ipmi_sdr_start(intf, use_builtin);
}

struct sdr_get_rs *ipmi_sdr_get_next_header(struct ipmi_intf *intf,
                                            struct ipmi_sdr_iterator *itr) {
  return MockIpmitool::Get()->ipmi_sdr_get_next_header(intf, itr);
}

// Returns raw sdr record
uint8 *ipmi_sdr_get_record(struct ipmi_intf *intf,  // NOLINT
                           struct sdr_get_rs *header,
                           struct ipmi_sdr_iterator *itr) {
  return MockIpmitool::Get()->ipmi_sdr_get_record(intf, header, itr);
}

// Frees the memory allocated from ipmi_sdr_start
void ipmi_sdr_end(struct ipmi_intf *intf, struct ipmi_sdr_iterator *itr) {
  return MockIpmitool::Get()->ipmi_sdr_end(intf, itr);
}

struct ipmi_rs *ipmi_sendrecv(struct ipmi_intf *intf,  // NOLINT
                              struct ipmi_rq *rq) {
  return MockIpmitool::Get()->ipmi_sendrecv(intf, rq);
}

struct ipmi_intf * ipmi_intf_load(char *name) {
  return MockIpmitool::Get()->ipmi_intf_load(name);
}

void ipmi_intf_session_set_hostname(struct ipmi_intf * intf, char * hostname) {
  return;
}

void ipmi_intf_session_set_username(struct ipmi_intf * intf, char * username) {
  return;
}

void ipmi_intf_session_set_password(struct ipmi_intf * intf, char * password) {
  return;
}

void ipmi_intf_session_set_port(struct ipmi_intf * intf, int ) {
  return;
}

}  // extern "C"

}  // namespace ecclesia

// This is a convenient way to wrap around EXPECT_CALL
#define EXPECT_IPMITOOL_CALL(name, ...) \
  EXPECT_CALL(*MockIpmitool::Get(), name(__VA_ARGS__))

#endif  // ECCLESIA_MAGENT_LIB_IPMI_IPMITOOL_MOCK_H_
