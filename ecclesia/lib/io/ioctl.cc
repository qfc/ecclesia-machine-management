#include "lib/io/ioctl.h"

#include <sys/ioctl.h>

#include <cstdint>

namespace ecclesia {

int SysIoctl::Call(int fd, unsigned long request, intptr_t argi) {
  return ioctl(fd, request, argi);
}

int SysIoctl::Call(int fd, unsigned long request, void *argp) {
  return ioctl(fd, request, argp);
}

}  // namespace ecclesia
