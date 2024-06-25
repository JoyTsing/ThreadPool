#include <unistd.h>

#include <cstdlib>

short *dummy(int i) {
  short *ret = new short;
  return ret;
}

int main(int argc, char *argv[]) {
  dummy(3);
  int *p = new int;
  delete p;
  void *q = malloc(sizeof(int));
  // int *q;
  // delete[] p;
  return 0;
}
