#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/select.h>

#include <libevdev-1.0/libevdev/libevdev.h>

struct libevdev *open_capture_dev(int *fd) {
  *fd = open("/dev/input/event0", O_RDONLY);
  struct libevdev *dev;

  if(libevdev_new_from_fd(*fd, &dev) < 0) {
    printf("Failed to init libevdev (%m)\n");
    exit(1);
  }

  libevdev_grab(dev, LIBEVDEV_GRAB);

  return dev;
}

int get_event(struct libevdev *dev, int capture_fd, long long timeout, struct input_event *ev) {
  // No events queued; just block until an event shows up.
  if(!timeout)
    return libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, ev);

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(capture_fd, &fds);
  struct timeval endtime;
  endtime.tv_sec = timeout / 1000000;
  endtime.tv_usec = timeout % 1000000;

  select(capture_fd + 1, &fds, NULL, NULL, &endtime);
  if(FD_ISSET(capture_fd, &fds))
    return libevdev_next_event(dev, LIBEVDEV_READ_FLAG_BLOCKING, ev);

  return -1;
}
