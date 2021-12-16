#pragma once

#ifdef ENABLE_LOGGING

#include <cstdio>
#define LOG(...) printf(__VA_ARGS__)

#else

#define LOG(...)

#endif
