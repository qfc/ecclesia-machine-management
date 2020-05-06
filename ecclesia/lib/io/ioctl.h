// Provides a virtual interface for execution ioctl() operations.
//
// This doesn't provide any actual functionality, it simply acts as a trivial
// wrapper around the system call so that users of it can take it as an input to
// support dependency injection for testing.

#ifndef ECCLESIA_LIB_IO_IOCTL_H_
#define ECCLESIA_LIB_IO_IOCTL_H_

#include <cstdint>

namespace ecclesia {

class IoctlInterface {
 public:
  IoctlInterface() {}
  virtual ~IoctlInterface() = default;

  // Call ioctl with the given arguments. Note that while technically ioctl is
  // a variadic function, you can only actually provide a single variadic
  // parameter and so we support only void* (for pointer valued arguments) and
  // intptr_t (for scalar integer arguments).
  virtual int Call(int fd, unsigned long request, intptr_t argi) = 0;
  virtual int Call(int fd, unsigned long request, void *argp) = 0;
};

// Implementation of the interface that uses the system call.
class SysIoctl final : public IoctlInterface {
 public:
  int Call(int fd, unsigned long request, intptr_t argi) override;
  int Call(int fd, unsigned long request, void *argp) override;
};

}  // namespace ecclesia

#endif  // ECCLESIA_LIB_IO_IOCTL_H_
