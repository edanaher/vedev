#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>

#include <libevdev-1.0/libevdev/libevdev.h>

struct libevdev *open_capture_dev() {
  int fd = open("/dev/input/event0", O_RDONLY);
  struct libevdev *dev;

  if(libevdev_new_from_fd(fd, &dev) < 0) {
    printf("Failed to init libevdev (%m)\n");
    exit(1);
  }

  //libevdev_grab(dev, LIBEVDEV_GRAB);

  return dev;
}

int get_event(struct libevdev *dev, struct input_event *ev) {
  return libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, ev);
}
