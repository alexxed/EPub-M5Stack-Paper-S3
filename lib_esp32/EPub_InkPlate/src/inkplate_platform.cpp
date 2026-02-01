#include "inkplate_platform.hpp"

#if defined(BOARD_TYPE_M5PAPER)

#include <cstdio>

#include "logging.hpp"

#include <M5EPD.h>

InkPlatePlatform InkPlatePlatform::singleton;
InkPlatePlatform & inkplate_platform = InkPlatePlatform::get_singleton();

bool InkPlatePlatform::setup(bool sd_card_init)
{
  LOG_I("M5Paper InkPlatePlatform setup (sd_card_init=%d)", sd_card_init ? 1 : 0);

  if (sd_card_init) {
    // M5EPD library handles SD card initialization
    if (!SD.begin()) {
      LOG_E("M5Paper: Failed to mount SD card");
      return false;
    }
    LOG_I("M5Paper: SD card mounted successfully");
  }

  return true;
}

bool InkPlatePlatform::light_sleep(uint32_t minutes_to_sleep, gpio_num_t gpio_num, int level)
{
  LOG_I("M5Paper light_sleep for %u minutes", minutes_to_sleep);
  
  // Configure wake-up sources
  if (gpio_num != 0) {
    esp_sleep_enable_ext0_wakeup(gpio_num, level);
  }
  
  // Set timer wake-up if minutes specified
  if (minutes_to_sleep > 0) {
    esp_sleep_enable_timer_wakeup(minutes_to_sleep * 60 * 1000000ULL);
  }
  
  // Enter light sleep
  esp_light_sleep_start();
  
  return true;
}

void InkPlatePlatform::deep_sleep(gpio_num_t gpio_num, int level)
{
  LOG_I("M5Paper deep_sleep (gpio=%d, level=%d)", (int)gpio_num, level);
  
  // Configure wake-up source
  if (gpio_num != 0) {
    esp_sleep_enable_ext0_wakeup(gpio_num, level);
  }
  
  // Shutdown M5Paper properly
  M5.shutdown();
  
  // Enter deep sleep
  esp_deep_sleep_start();
}

#endif // BOARD_TYPE_M5PAPER
