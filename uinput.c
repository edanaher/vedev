#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <libevdev-1.0/libevdev/libevdev.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define checked_ioctl(...) if(ioctl(__VA_ARGS__)) printf("Error from ioctl: %m"), exit(1)
#define checked_write(fd, buffer) if(write(fd, &buffer, sizeof(buffer)) != sizeof(buffer)) \
  printf("Error writing " # buffer), exit(1)

int clone_input_dev(struct libevdev *capture) {
  int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
  if(fd < 0) {
    printf("Unable to open /dev/uinput\n");
    exit(1);
  }

  checked_ioctl(fd, UI_SET_EVBIT, EV_KEY);
  checked_ioctl(fd, UI_SET_EVBIT, EV_REL);
  checked_ioctl(fd, UI_SET_EVBIT, EV_SYN);

  int k;
  for(k = 0; k < KEY_CNT; k++)
    if(libevdev_has_event_code(capture, EV_KEY, k))
      checked_ioctl(fd, UI_SET_KEYBIT, k);
  checked_ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);

  checked_ioctl(fd, UI_SET_RELBIT, REL_X);
  checked_ioctl(fd, UI_SET_RELBIT, REL_Y);

  struct uinput_user_dev uidev;
  memset(&uidev, 0, sizeof(uidev));
  snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "vedve");
  uidev.id.bustype = BUS_USB;
  uidev.id.vendor = 0x1234;
  uidev.id.product = 0xfedc;
  uidev.id.version = 1;

  checked_write(fd, uidev);
  checked_ioctl(fd, UI_DEV_CREATE);

  return fd;
}

void destroy_input_dev(int dev) {
  checked_ioctl(dev, UI_DEV_DESTROY);
}

void send_event(int dev, int type, int code, int value) {
  struct input_event ev;
  memset(&ev, 0, sizeof(ev));
  gettimeofday(&ev.time, 0);
  ev.type = type;
  ev.code = code;
  ev.value = value;
  checked_write(dev, ev);
}

void send_key(int dev, int code, int state) {
  //printf("Sending key %d %d\n", code, state);
  send_event(dev, EV_KEY, code, state);
  send_event(dev, EV_SYN, SYN_REPORT, 0);
}
