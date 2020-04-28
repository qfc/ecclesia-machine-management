#include "magent/lib/io/smbus_kernel_dev.h"

#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdarg.h>

#include "platforms/util/testing/fake_filesystem.h"
#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "absl/base/integral_types.h"

using ::testing::_;
using ::testing::DoAll;
using ::testing::ElementsAre;
using ::testing::Return;
using ::testing::status::CanonicalStatusIs;

namespace ecclesia {
namespace {

// This allows us to mock calls to ioctl().
class MockIoctl {
 public:
  // Get a pointer to the global instance of the Mock.
  // Not thread safe.
  static MockIoctl *Get() {
    if (instance_ == nullptr) {
      instance_ = new ::testing::StrictMock<MockIoctl>();
    }
    return instance_;
  }

  // Reset the Mock.
  static void Reset() {
    delete instance_;
    instance_ = nullptr;
  }

  // ioctl(fd, I2C_SMBUS, struct i2c_smbus_ioctl_data *);
  MOCK_METHOD(int, Smbus,
              (int fd, char read_write, uint8_t command, int size,
               union i2c_smbus_data *data));

  // ioctl(fd, I2C_SLAVE, int);
  MOCK_METHOD(int, Slave, (int fd, int));

  // ioctl(fd, I2C_RDWR, i2c_rdwr_ioctl_data)
  MOCK_METHOD(int, I2CReadWrite, (int fd, struct i2c_msg *msgs, int nmsgs));

  // ioctl(fd, I2C_FUNCS, uint32_t *);
  MOCK_METHOD(int, I2CFuncs, (int fd, uint32_t *));

 protected:
  MockIoctl() {}
  virtual ~MockIoctl() {}

  static ::testing::StrictMock<MockIoctl> *instance_;
};
::testing::StrictMock<MockIoctl> *MockIoctl::instance_ = nullptr;

// handle I2C ioctl()s
int DoMockIoctl_I2C(int fd, uint64_t request, va_list args) {
  // see if it is a request code that we understand
  if (request == I2C_SMBUS) {
    struct i2c_smbus_ioctl_data *arg =
        va_arg(args, struct i2c_smbus_ioctl_data *);
    return MockIoctl::Get()->Smbus(fd, arg->read_write, arg->command, arg->size,
                                   arg->data);
  } else if (request == I2C_SLAVE) {
    int arg = va_arg(args, int);
    return MockIoctl::Get()->Slave(fd, arg);
  } else if (request == I2C_RDWR) {
    struct i2c_rdwr_ioctl_data *arg =
        va_arg(args, struct i2c_rdwr_ioctl_data *);
    return MockIoctl::Get()->I2CReadWrite(fd, arg->msgs, arg->nmsgs);
  } else if (request == I2C_FUNCS) {
    uint32_t *funcs = va_arg(args, uint32_t *);
    return MockIoctl::Get()->I2CFuncs(fd, funcs);
  }

  // unknown request code
  abort();
  return 0;
}

// This is the fake ioctl, which takes advantage of weak symbols in glibc.
extern "C" int ioctl(int fd, unsigned long request, ...) {
  va_list args;
  va_start(args, request);
  int ret = DoMockIoctl_I2C(fd, request, args);
  va_end(args);
  return ret;
}

// I2C_SMBUS: Respond to I2C_SMBUS_READ with a fixed value.  Return 0.
ACTION_P(ReadSmbusData, value) {
  int size = arg3;
  union i2c_smbus_data *sm_data = arg4;
  if (sm_data) {
    if (size == I2C_SMBUS_BYTE_DATA || size == I2C_SMBUS_BYTE) {
      sm_data->byte = value;
    } else if (size == I2C_SMBUS_WORD_DATA) {
      sm_data->word = value;
    }
  }

  return 0;
}

// I2C_SMBUS: Verify an I2C_SMBUS_WRITE against a fixed value.  Return 0.
ACTION_P(WriteSmbusData, value) {
  int size = arg3;
  union i2c_smbus_data *sm_data = arg4;
  if (sm_data) {
    if (size == I2C_SMBUS_BYTE_DATA || size == I2C_SMBUS_BYTE ||
        size == I2C_SMBUS_QUICK) {
      EXPECT_EQ(static_cast<int>(value), static_cast<int>(sm_data->byte));
    } else if (size == I2C_SMBUS_WORD_DATA) {
      EXPECT_EQ(static_cast<int>(value), static_cast<int>(sm_data->word));
    }
  }

  return 0;
}

// Respond to a block read request with the given buffer and size.
ACTION_P2(ReadBlockData, value, size) {
  union i2c_smbus_data *sm_data = arg4;
  if (sm_data) {
    EXPECT_LE(size, 32);
    sm_data->block[0] = size;
    for (int i = 1; i < size + 1; ++i) sm_data->block[i] = value[i - 1];
  }

  return 0;
}

// Verify the data of a block write request.
ACTION_P(WriteBlockData, value) {
  union i2c_smbus_data *sm_data = arg4;
  if (sm_data) {
    EXPECT_LE(sm_data->block[0], 32);
    for (int i = 1; i < sm_data->block[0]; ++i)
      EXPECT_EQ(sm_data->block[i], value[i - 1]);
  }

  return 0;
}

class KernelSmbusAccessTest : public testing::Test {
 public:
  KernelSmbusAccessTest() : fs(FLAGS_test_tmpdir) {}

 protected:
  ::platforms_util_testing::FakeFilesystem fs;
};

TEST_F(KernelSmbusAccessTest, TestDevFilesNotPresent) {
  std::string test_data("test_data");
  fs.CreateDir("/dev");

  SmbusLocation loc({{1}, {0}});
  KernelSmbusAccess access(FLAGS_test_tmpdir + "/dev");

  EXPECT_THAT(access.SendByte(loc, 0),
              CanonicalStatusIs(absl::StatusCode::kInternal));
}

class KernelSmbusAccessTestBus1 : public testing::Test {
 protected:
  KernelSmbusAccessTestBus1() : fs(FLAGS_test_tmpdir) {
    mock_ioctl = MockIoctl::Get();

    fs.CreateDir("/dev");

    dev_dir_ = FLAGS_test_tmpdir + "/dev";

    std::string test_data("dont_care");
    fs.CreateAndWriteFile("/dev/i2c-1", test_data.c_str(), test_data.length());
  }

  ~KernelSmbusAccessTestBus1() { MockIoctl::Reset(); }

  ::platforms_util_testing::FakeFilesystem fs;
  MockIoctl *mock_ioctl;
  std::string dev_dir_;
};

// Try to read from bus 2 (nonexistent).
TEST_F(KernelSmbusAccessTestBus1, TestNonExistentBusNumber) {
  SmbusLocation loc({{2}, {0}});
  KernelSmbusAccess access(dev_dir_);

  uint8_t data;
  EXPECT_THAT(access.ReceiveByte(loc, &data),
              CanonicalStatusIs(absl::StatusCode::kInternal));
}

TEST_F(KernelSmbusAccessTestBus1, SetSlaveFailure) {
  SmbusLocation loc({{1}, {0}});
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(-1));
  EXPECT_THAT(access.SendByte(loc, 0),
              CanonicalStatusIs(absl::StatusCode::kInternal));
}

TEST_F(KernelSmbusAccessTestBus1, ProbeDevice) {
  SmbusLocation loc({{1}, {0}});
  KernelSmbusAccess access(dev_dir_);

  // Device found via probe.
  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  // Note: smbus quick transaction data bit is read/write parameter
  EXPECT_CALL(*mock_ioctl, Smbus(_, 0, _, I2C_SMBUS_QUICK, _))
      .WillOnce(Return(0));
  EXPECT_OK(access.ProbeDevice(loc));

  // Device not found (error during SMBUS_QUICK).
  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  EXPECT_CALL(*mock_ioctl, Smbus(_, 0, _, I2C_SMBUS_QUICK, _))
      .WillOnce(Return(-1));
  EXPECT_THAT(access.ProbeDevice(loc),
              CanonicalStatusIs(absl::StatusCode::kNotFound));
}

TEST_F(KernelSmbusAccessTestBus1, WriteQuickSuccess) {
  SmbusLocation loc({{1}, {0}});
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint8_t expected_data = 1;
  // Note: smbus quick transaction data bit is read/write parameter
  EXPECT_CALL(*mock_ioctl, Smbus(_, expected_data, _, I2C_SMBUS_QUICK, _))
      .WillOnce(Return(0));
  EXPECT_OK(access.WriteQuick(loc, expected_data));
}

TEST_F(KernelSmbusAccessTestBus1, SendByteSuccess) {
  SmbusLocation loc({{1}, {0}});
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint8_t expected_data = 0xea;
  EXPECT_CALL(*mock_ioctl, Smbus(_, I2C_SMBUS_WRITE, _, I2C_SMBUS_BYTE, _))
      .WillOnce(WriteSmbusData(expected_data));
  EXPECT_OK(access.SendByte(loc, expected_data));
}

TEST_F(KernelSmbusAccessTestBus1, ReceiveByteSuccess) {
  SmbusLocation loc({{1}, {0}});
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint8_t expected_data = 0xea;
  EXPECT_CALL(*mock_ioctl, Smbus(_, I2C_SMBUS_READ, _, I2C_SMBUS_BYTE, _))
      .WillOnce(ReadSmbusData(expected_data));

  uint8_t data = 0;
  EXPECT_OK(access.ReceiveByte(loc, &data));
  EXPECT_EQ(expected_data, data);
}

TEST_F(KernelSmbusAccessTestBus1, Write8Success) {
  SmbusLocation loc({{1}, {0}});
  int reg = 0xab;
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint8_t expected_data = 0xea;
  EXPECT_CALL(*mock_ioctl,
              Smbus(_, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BYTE_DATA, _))
      .WillOnce(WriteSmbusData(expected_data));

  EXPECT_OK(access.Write8(loc, reg, expected_data));
}

TEST_F(KernelSmbusAccessTestBus1, Read8Success) {
  SmbusLocation loc({{1}, {0}});
  int reg = 0xab;
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint8_t expected_data = 0xea;
  EXPECT_CALL(*mock_ioctl,
              Smbus(_, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, _))
      .WillOnce(ReadSmbusData(expected_data));

  uint8_t data = 0;
  EXPECT_OK(access.Read8(loc, reg, &data));
  EXPECT_EQ(expected_data, data);
}

TEST_F(KernelSmbusAccessTestBus1, Write16Success) {
  SmbusLocation loc({{1}, {0}});
  int reg = 0xab;
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint16_t expected_data = 0xea;
  EXPECT_CALL(*mock_ioctl,
              Smbus(_, I2C_SMBUS_WRITE, reg, I2C_SMBUS_WORD_DATA, _))
      .WillOnce(WriteSmbusData(expected_data));

  EXPECT_OK(access.Write16(loc, reg, expected_data));
}

TEST_F(KernelSmbusAccessTestBus1, Read16Success) {
  SmbusLocation loc({{1}, {0}});
  int reg = 0xab;
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint16_t expected_data = 0xabcd;
  EXPECT_CALL(*mock_ioctl,
              Smbus(_, I2C_SMBUS_READ, reg, I2C_SMBUS_WORD_DATA, _))
      .WillOnce(ReadSmbusData(expected_data));

  uint16_t data = 0;
  EXPECT_OK(access.Read16(loc, reg, &data));
  EXPECT_EQ(expected_data, data);
}

TEST_F(KernelSmbusAccessTestBus1, WriteBlockSuccess) {
  SmbusLocation loc({{1}, {0}});
  int reg = 0xab;
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  unsigned char expected_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
  EXPECT_CALL(*mock_ioctl,
              Smbus(_, I2C_SMBUS_WRITE, reg, I2C_SMBUS_BLOCK_DATA, _))
      .WillOnce(WriteBlockData(expected_data));

  EXPECT_OK(access.WriteBlock(
      loc, reg, absl::MakeConstSpan(expected_data, sizeof(expected_data))));
}

TEST_F(KernelSmbusAccessTestBus1, ReadBlockSuccess) {
  SmbusLocation loc({{1}, {0}});
  int reg = 0xab;
  KernelSmbusAccess access(dev_dir_);

  EXPECT_CALL(*mock_ioctl, Slave(_, loc.address.addr)).WillOnce(Return(0));
  uint8_t expected_data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
  EXPECT_CALL(*mock_ioctl,
              Smbus(_, I2C_SMBUS_READ, reg, I2C_SMBUS_BLOCK_DATA, _))
      .WillOnce(ReadBlockData(expected_data, sizeof(expected_data)));

  unsigned char data[sizeof(expected_data)] = {0};
  size_t len;
  EXPECT_OK(
      access.ReadBlock(loc, reg, absl::MakeSpan(data, sizeof(data)), &len));
  EXPECT_EQ(sizeof(data), len);
  EXPECT_THAT(data, ElementsAre(0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06));
}

}  // namespace
}  // namespace ecclesia
