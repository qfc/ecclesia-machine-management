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

#include "lib/smbios/reader.h"

#include <memory>
#include <string>
#include <vector>

#include "devtools/build/runtime/get_runfiles_dir.h"
#include "testing/base/public/gunit.h"
#include "absl/memory/memory.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "lib/smbios/bios.h"
#include "lib/smbios/processor_information.h"
#include "lib/smbios/structures.emb.h"
#include "lib/smbios/system_event_log.h"
#include "runtime/cpp/emboss_cpp_util.h"
#include "runtime/cpp/emboss_prelude.h"

namespace ecclesia {
namespace {

// The following set of tests are provided with smbios tables exported from an
// Indus server. The tests verify if the individual smbios structures are parsed
// correctly. The expectation for the tests is derived from a dump from
// dmidecode which is stored in test_data/decoded_smbios_tables.text

constexpr absl::string_view kTestDataDir =
    "google3/lib/smbios/test_data/";

class SmbiosReaderTest : public ::testing::Test {
 protected:
  SmbiosReaderTest() {
    std::string entry_point_path = devtools_build::GetDataDependencyFilepath(
        absl::StrCat(kTestDataDir, "smbios_entry_point"));
    std::string table_path = devtools_build::GetDataDependencyFilepath(
        absl::StrCat(kTestDataDir, "DMI"));
    reader_ = absl::make_unique<SmbiosReader>(entry_point_path, table_path);
  }

  std::unique_ptr<SmbiosReader> reader_;
};

TEST_F(SmbiosReaderTest, VerifyBiosInformationStructure) {
  auto bios_info = reader_->GetBiosInformation();
  auto bios_message_view = bios_info->GetMessageView();
  ASSERT_TRUE(bios_info.get() != nullptr);
  EXPECT_EQ(bios_info->GetString(bios_message_view.vendor_snum().Read()),
            "Google, Inc.");
  EXPECT_EQ(bios_info->GetString(bios_message_view.version_snum().Read()),
            "20.18.2");
  EXPECT_EQ(bios_info->GetString(bios_message_view.release_date_snum().Read()),
            "11/26/2019");
}

TEST_F(SmbiosReaderTest, VerifyMemoryDeviceStructure) {
  auto memory_devices = reader_->GetAllMemoryDevices();

  for (int i = 0; i < memory_devices.size() - 1; ++i) {
    // Verify that the memory devices are sorted on the device locator string
    EXPECT_LE(
        memory_devices[i].GetString(
            memory_devices[i].GetMessageView().device_locator_snum().Read()),
        memory_devices[i + 1].GetString(memory_devices[i + 1]
                                            .GetMessageView()
                                            .device_locator_snum()
                                            .Read()));
  }
}

TEST_F(SmbiosReaderTest, VerifySystemEventLogStructure) {
  auto system_event_log = reader_->GetSystemEventLog();
  ASSERT_TRUE(system_event_log.get());
  auto view = system_event_log->GetMessageView();

  EXPECT_EQ(view.log_area_length().Read(), 65535);
  EXPECT_EQ(view.log_header_start_offset().Read(), 0);
  EXPECT_EQ(view.log_data_start_offset().Read(), 12);
  EXPECT_EQ(view.access_method().Read(), AccessMethod::MEMORY_MAPPED_IO);
  EXPECT_TRUE(view.log_area_valid().Read());
  EXPECT_FALSE(view.log_area_full().Read());
  EXPECT_EQ(view.access_method_address().Read(), 0x745F7018);
  // OEM format
  EXPECT_EQ(view.log_header_format().Read(), 0x81);
}

TEST_F(SmbiosReaderTest, VerifyProcessorInformationStructure) {
  auto processors = reader_->GetAllProcessors();

  {  // CPU0
    auto &processor = processors[0];
    auto processor_view = processors[0].GetMessageView();
    EXPECT_EQ(
        processor.GetString(processor_view.socket_designation_snum().Read()),
        "CPU0");
    EXPECT_EQ(processor_view.processor_type().Read(),
              ProcessorType::CENTRAL_PROCESSOR);

    EXPECT_EQ(processor.GetString(processor_view.manufacturer_snum().Read()),
              "Intel(R) Corporation");
    EXPECT_EQ(processor_view.external_clk_freq_mhz().Read(), 100);

    EXPECT_EQ(processor_view.max_speed_mhz().Read(), 4000);
    EXPECT_EQ(processor_view.current_speed_mhz().Read(), 3100);
    EXPECT_TRUE(processor.IsProcessorEnabled());
    EXPECT_EQ(processor.GetCoreCount(), 18);
    EXPECT_EQ(processor.GetCoreEnabled(), 18);
    EXPECT_EQ(processor.GetThreadCount(), 36);
    EXPECT_EQ(processor_view.processor_family().Read(), 0xB3);  // Xeon
  }

  {  // CPU1
    auto &processor = processors[1];
    auto processor_view = processors[1].GetMessageView();
    EXPECT_EQ(
        processor.GetString(processor_view.socket_designation_snum().Read()),
        "CPU1");
    EXPECT_EQ(processor_view.processor_type().Read(),
              ProcessorType::CENTRAL_PROCESSOR);

    EXPECT_EQ(processor.GetString(processor_view.manufacturer_snum().Read()),
              "Intel(R) Corporation");
    EXPECT_EQ(processor_view.external_clk_freq_mhz().Read(), 100);

    EXPECT_EQ(processor_view.max_speed_mhz().Read(), 4000);
    EXPECT_EQ(processor_view.current_speed_mhz().Read(), 3100);
    EXPECT_TRUE(processor.IsProcessorEnabled());
    EXPECT_EQ(processor.GetCoreCount(), 18);
    EXPECT_EQ(processor.GetCoreEnabled(), 18);
    EXPECT_EQ(processor.GetThreadCount(), 36);
    EXPECT_EQ(processor_view.processor_family().Read(), 0xB3);  // Xeon
  }
}

// TODO: Add tests with corrupted SMBIOS tables

}  // namespace

}  // namespace ecclesia
