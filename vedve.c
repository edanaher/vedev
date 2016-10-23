#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>
#include <libevdev-1.0/libevdev/libevdev.h>

int create_input_dev();
void send_key(int dev, int k, int state);
void send_event(int dev, int k, int code, int state);
void destroy_input_dev(int dev);

struct libevdev *open_capture_dev();
int get_event(struct libevdev *dev, struct input_event *ev);

int process_event(int dev, struct input_event *ev) {
  //printf("%d %d %d %d\n", ev->type, EV_KEY, ev->code, KEY_Q);
  if(ev->type == EV_KEY && ev->code == KEY_Q)
    return 1;
  if(ev->type == EV_KEY && ev->code == KEY_F8 && ev->value != 2) {
    printf("Mouse?\n");
    send_key(dev, BTN_LEFT, ev->value);
  }
  if(ev->type == EV_KEY && ev->code == KEY_F9 && ev->value > 0) {
    printf("Mouse rel?\n");
    send_event(dev, EV_REL, REL_X, 5);
    send_event(dev, EV_REL, REL_Y, 5);
    send_event(dev, EV_SYN, SYN_REPORT, 0);
  }

  return 0;
}

int main() {
  usleep(100000);
  int dev = create_input_dev();
  struct libevdev *capture = open_capture_dev();
  struct input_event ev;
  while(1) {
    int rc = get_event(capture, &ev);
    //printf("status: %d\n", rc);
    if(rc == 0) {
      /*printf("Event: %s %s %d %d\n", libevdev_event_type_get_name(ev.type),
                                  libevdev_event_code_get_name(ev.code, ev.value), ev.code, ev.value);*/
      if(process_event(dev, &ev) == 1)
        break;
    }
  }


  /*char buf[100];
  char *ret = fgets(buf, sizeof(buf) - 1, stdin);
  if(fgets(buf, sizeof(buf) - 1, stdin) == NULL)
    printf("fgets failed\n");
  send_key(dev, 0);
  if(fgets(buf, sizeof(buf) - 1, stdin) == NULL)
    printf("fgets failed\n");
  ret = fgets(buf, sizeof(buf) - 1, stdin);
  printf("ret is %s\n", ret); */
  destroy_input_dev(dev);

  return 0;
}
