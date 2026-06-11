#pragma once

#include "esphome/core/component.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/automation.h"

#ifdef USE_ESP32
#include <sys/time.h>
#include <ctime>
#include "esp_system.h"
#include "esp_log.h"
#endif

namespace esphome {
namespace rtc_p4 {

static const char *const TAG = "rtc_p4";

class RTCP4Component : public Component {
 public:
  void setup() override {
    ESP_LOGI(TAG, "ESP32-P4 RTC Component initialized");
    // VBAT domain je automaticky aktívny ak je pripojená batéria
  }
  
  void dump_config() override {
    ESP_LOGCONFIG(TAG, "ESP32-P4 RTC with VBAT backup");
  }
  
  // Naèítanie èasu z RTC do systémových hodín ESPHome
  bool read_rtc_time() {
    #ifdef USE_ESP32
    time_t now;
    struct tm timeinfo;
    ::time(&now);
    localtime_r(&now, &timeinfo);
    
    // Validácia - ak je rok < 2024, RTC nie je inicializovaný
    if (timeinfo.tm_year + 1900 < 2024) {
      ESP_LOGW(TAG, "RTC time invalid (year=%d), waiting for network sync", 
               timeinfo.tm_year + 1900);
      return false;
    }
    
    ESP_LOGI(TAG, "RTC time read: %04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    return true;
    #else
    return false;
    #endif
  }
  
  // Zápis aktuálneho systémového èasu do RTC
  void write_rtc_time() {
    #ifdef USE_ESP32
    // ESP-IDF automaticky zapisuje èas do RTC pri settimeofday()
    // ESPHome interné hodiny už majú èas synchronizovaný
    time_t now = ::time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    ESP_LOGI(TAG, "RTC time written: %04d-%02d-%02d %02d:%02d:%02d",
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    #endif
  }
};

class RTCP4Time : public time::RealTimeClock {
 public:
  void setup() override {
    // Naèítanie èasu z RTC pri bootovaní
    this->read_time_from_rtc();
  }
  
  void update() override {
    // update() sa volá len ak je nastavený update_interval
    // Pre RTC s batériou to nie je potrebné (update_interval: never)
  }
  
  void read_time_from_rtc() {
    #ifdef USE_ESP32
    time_t now;
    struct tm timeinfo;
    ::time(&now);
    localtime_r(&now, &timeinfo);
    
    if (timeinfo.tm_year + 1900 >= 2024) {
      ESPTime esp_time = ESPTime::from_c_tm(&timeinfo, now);
      this->synchronize_epoch_(now);
      ESP_LOGI(TAG, "Time loaded from RTC");
    }
    #endif
  }
  
  void write_time_to_rtc() {
    // Keï ESPHome synchronizuje èas (napr. cez SNTP/HomeAssistant),
    // automaticky sa zapíše do RTC cez settimeofday()
    auto now = this->now();
    if (now.is_valid()) {
      ESP_LOGI(TAG, "RTC synchronized with system time");
    }
  }
};

// Actions pre použitie v automatizáciách
template<typename... Ts> class ReadTimeAction : public Action<Ts...> {
 public:
  ReadTimeAction(RTCP4Time *parent) : parent_(parent) {}
  
  void play(Ts... x) override {
    if (this->parent_ != nullptr) {
      this->parent_->read_time_from_rtc();
    }
  }
  
 protected:
  RTCP4Time *parent_;
};

template<typename... Ts> class WriteTimeAction : public Action<Ts...> {
 public:
  WriteTimeAction(RTCP4Time *parent) : parent_(parent) {}
  
  void play(Ts... x) override {
    if (this->parent_ != nullptr) {
      this->parent_->write_time_to_rtc();
    }
  }
  
 protected:
  RTCP4Time *parent_;
};

}  // namespace rtc_p4
}  // namespace esphome