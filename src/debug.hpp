#pragma once
#include <stdio.h>

#ifdef WORKQ_DEBUG
#define DEBUG(fmt, args...)                                                    \
  printf("%s:%s:%d: " fmt "\n", __FILE__, __FUNCTION__, __LINE__, args)
#else
#define DEBUG(fmt, args...)
#endif