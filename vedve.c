#include <stdio.h>
#include <stdlib.h>

int create_input_dev();
void send_key(int dev, int k);
void destroy_input_dev(int dev);

int main() {
  int dev = create_input_dev();

  char buf[100];
  char *ret = fgets(buf, sizeof(buf) - 1, stdin);
  if(fgets(buf, sizeof(buf) - 1, stdin) == NULL)
    printf("fgets failed\n");
  send_key(dev, 0);
  if(fgets(buf, sizeof(buf) - 1, stdin) == NULL)
    printf("fgets failed\n");
  ret = fgets(buf, sizeof(buf) - 1, stdin);
  printf("ret is %s\n", ret);
  destroy_input_dev(dev);

}
