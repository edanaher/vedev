#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/input.h>
#include <libevdev-1.0/libevdev/libevdev.h>

#include "vedve.h"

int clone_input_dev(struct libevdev *capture);
void send_key(int dev, int k, int state);
void send_event(int dev, int k, int code, int state);
void destroy_input_dev(int dev);

struct libevdev *open_capture_dev();
int get_event(struct libevdev *dev, struct input_event *ev);

void load_config(char *filename, struct config *config);

int state = 0;

#define NCHORDS 5
int chords[NCHORDS][10] = {
  { KEY_RIGHTCTRL, KEY_LEFTCTRL, KEY_LEFTALT, KEY_RIGHTALT, -1 },
  { KEY_D, KEY_F, KEY_J, -1},
  { KEY_D, KEY_F, KEY_K, -1},
  { KEY_D, KEY_F, KEY_H, -1},
  { KEY_D, KEY_F, KEY_L, -1}
};

int chordInProgress = -1;
int chordLenInProgress = 0;
int chordFired = 0;

// -1 - not part of a chord
// -2 - suppress the key as part of a chord
// >= 0 - fire this chord
int process_chords(int dev, struct input_event *ev) {
  if(ev->type != EV_KEY)
    return -1;
  if(ev->value == 2) {
    if(chordFired && chords[chordInProgress][chordLenInProgress] == -1) {
      return chordInProgress;
    }
    if(chordInProgress != -1)
      return -2;
    return -1;
  }
  if(ev->value == 0) {
    if(chordFired) {
      chordLenInProgress--;
      if(chordLenInProgress == 0) {
        chordFired = 0;
        chordInProgress = -1;
      }
      return -2;
    }
    // This is insufficient; e.g., (k1, k2, k2 release, k2) should chord.
    int k;
    for(k = 0; k < chordLenInProgress; k++)
      send_key(dev, chords[chordInProgress][k], 1);
    chordInProgress = -1;
    chordLenInProgress = 0;
    return -1;
  }
  if(chordInProgress != -1) {
    if(chords[chordInProgress][chordLenInProgress] == ev->code) {
      chordLenInProgress++;
      if(chords[chordInProgress][chordLenInProgress] == -1) {
        chordFired = 1;
        return chordInProgress;
      }
      return -2;
    } else { // Search for another matching chord
      int ch, k;
      for(ch = 0; ch < NCHORDS; ch++) {
        for(k = 0; k < chordLenInProgress; k++)
          if(chords[ch][k] != chords[chordInProgress][k])
            break;
        if(k != chordLenInProgress || chords[ch][chordLenInProgress] != ev->code)
          continue;
        chordInProgress = ch;
        chordLenInProgress++;
        if(chords[chordInProgress][chordLenInProgress] == -1) {
          chordFired = 1;
          return chordInProgress;
        }
        return -2;
      }
      // Fail the chord; (belatedly) send what we have so far
      for(k = 0; k < chordLenInProgress; k++)
        send_key(dev, chords[chordInProgress][k], 1);
      return -1;
    }
  }
  // No chord in progress, key pressed.  Check if it starts.
  int ch;
  for(ch = 0; ch < NCHORDS; ch++)
    if(chords[ch][0] == ev->code) {
      chordInProgress = ch;
      chordLenInProgress = 1;
      return -2;
    }
  return -1;
}

int process_event(int dev, struct input_event *ev) {
  //printf("%d %d %d %d\n", ev->type, EV_KEY, ev->code, KEY_Q);
  if(ev->type == EV_KEY) {
    int ch = process_chords(dev, ev);
    if(ch == 0)
      return 1;
    if(ch == 1) {
      send_key(dev, KEY_DOWN, 1);
      send_key(dev, KEY_DOWN, 0);
    }
    if(ch == 2) {
      send_key(dev, KEY_UP, 1);
      send_key(dev, KEY_UP, 0);
    }
    if(ch == 3) {
      send_key(dev, KEY_LEFT, 1);
      send_key(dev, KEY_LEFT, 0);
    }
    if(ch == 4) {
      send_key(dev, KEY_RIGHT, 1);
      send_key(dev, KEY_RIGHT, 0);
    }
    if(ch != -1)
      return 0;
  }
  if(ev->type == EV_KEY && ev->code == KEY_F8) {
    if(ev->value != 2)
      send_key(dev, BTN_LEFT, ev->value);
  /*} else if(ev->type == EV_KEY && ev->code == KEY_Q) {
    return 1;*/
  } else if(ev->type == EV_KEY && ev->code == KEY_F9 && ev->value > 0) {
    send_event(dev, EV_REL, REL_X, 5);
    send_event(dev, EV_REL, REL_Y, 5);
    send_event(dev, EV_SYN, SYN_REPORT, 0);
  /*} else if(ev->type == EV_KEY && ev->code == KEY_RIGHTCTRL) {
    send_event(dev, ev->type, KEY_RIGHTSHIFT, ev->value);*/
  } else {
    send_event(dev, ev->type, ev->code, ev->value);
  }

  return 0;
}

int main() {
  struct config config;
  load_config("vedve.conf", &config);
  printf("name is %s\n", config.name);
  usleep(100000);
  struct libevdev *capture = open_capture_dev();
  int dev = clone_input_dev(capture);
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
