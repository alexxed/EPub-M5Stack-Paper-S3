// Minimal InkPlatePlatform shim for ESP32 builds.
// For M5Paper, provides platform-specific initialization

#pragma once

#include "global.hpp"
#include "non_copyable.hpp"

#if defined(BOARD_TYPE_M5PAPER)

#include "driver/gpio.h"

class InkPlatePlatform : NonCopyable
{
private:
  static constexpr char const * TAG = "InkPlatePlatform";
  static InkPlatePlatform singleton;
  InkPlatePlatform() = default;

public:
  static inline InkPlatePlatform & get_singleton() noexcept { return singleton; }

  bool setup(bool sd_card_init = false);
  bool light_sleep(uint32_t minutes_to_sleep, gpio_num_t gpio_num = (gpio_num_t)0, int level = 1);
  void deep_sleep(gpio_num_t gpio_num = (gpio_num_t)0, int level = 1);
};

extern InkPlatePlatform & inkplate_platform;

#endif
