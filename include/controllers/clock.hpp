#pragma once

#include "global.hpp"

#if DATE_TIME_RTC

#if EPUB_INKPLATE_BUILD
  #include "inkplate_platform.hpp"
#endif

#include "logging.hpp"

#include <sys/time.h>

class Clock
{
  private:
    static constexpr char const * TAG = "Clock";

  public:
    static void set_date_time(const time_t & tm) {
      // On M5Paper (and non-Inkplate builds), just set system time.
      timeval tv;
      tv.tv_sec = tm;
      tv.tv_usec = 0;
      settimeofday(&tv, nullptr);
    }

    static void get_date_time(time_t & t) {
      // On M5Paper (and non-Inkplate builds), just use system time.
      time(&t);
    }
};

#endif