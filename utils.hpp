// NAME: Rob Royce, Tyler Hackett
// EMAIL: robroyce1@ucla.edu, tjhackett@ucla.edu
// ID: 705357270

#include <iostream>
#include <stdarg.h>
#include <getopt.h>
#include <string>

#define MAX_ARG_LEN 128


int args_init(int argc, char **argv, struct option *long_options, ...) {
  while (1) {
    int optind;
    int c = getopt_long(argc, argv, "", long_options, &optind);

    if (c == -1)
      break;

    switch (c) {
    case 0:
      break;
    case '?':
    default:
      return -1;
      break;
    }
  }

  return optind;
}

