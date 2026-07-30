#include <Arduino.h>
#include <esp_sleep.h>
#include <BLEDevice.h>
#include "PowerManager.h"
#include "Config.h"

/*
Use 2x220k resistor on an analog port to read voltage value (battery or USB)
ref: https://wiki.seeedstudio.com/check_battery_voltage/
*/

// bootCount stored in RTC RAM: survives deep sleep, resets on power cycle.
RTC_DATA_ATTR int bootCount = 0;

float PowerManager::getLevel() {
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++) {
    Vbatt += analogReadMilliVolts(D2); // ADC with correction
  }
  return 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V
}

int PowerManager::checkSource() {
  return getLevel() > 4.1 ? POWER_USB : POWER_BAT;
}

int PowerManager::getBootCount() {
  return bootCount;
}

void PowerManager::_incBootCount() {
  bootCount++;
}

void PowerManager::init() {
  Serial.println("[INFO ] Boot number: " + String(getBootCount()));
  pinMode(A2, INPUT); // ADC on port D2
  _dataReady  = false;
  _calibrated = false;
  _printWakeupReason();
  _incBootCount();
}

void PowerManager::notifyDataReady(bool calibrated) {
  _dataReady   = true;
  _dataReadyAt = millis();
  _calibrated  = calibrated;
}

#define BROADCAST_WINDOW_MS (10 * 1000UL) // stay awake for clients to scan

void PowerManager::loop() {
  if (!_calibrated) return; // sensor not yet calibrated — stay awake
  if (!_dataReady)  return; // no reading ready yet
  if (millis() - _dataReadyAt < BROADCAST_WINDOW_MS) return; // broadcast window
  _dataReady = false;

  if (getBootCount() <= LONG_SLEEP_BOOT_THRESHOLD) {
    Serial.printf("[INFO ] Deep sleep 2min (boot %d/%d)\n", getBootCount(), LONG_SLEEP_BOOT_THRESHOLD);
    _deepSleep(120);
  } else {
    Serial.printf("[INFO ] Deep sleep %ldmin\n", (long)(LONG_SLEEP_DURATION_SEC / 60));
    _deepSleep(LONG_SLEEP_DURATION_SEC);
  }
}

#define FIRST_BOOT_SAFETY_MS (120 * 1000UL) // 1-minute reflash window on first boot

void PowerManager::_deepSleep(unsigned long seconds) {
  // On the very first boot (cold power-on), stay awake for some minute
  // so the firmware can be reflashed without fighting deep sleep.
  // Deep sleep wakeups have bootCount > 1 and are not affected.
  if (getBootCount() == 1 && millis() < FIRST_BOOT_SAFETY_MS) {
    unsigned long remaining = (FIRST_BOOT_SAFETY_MS - millis()) / 1000;
    Serial.printf("[INFO ] First boot safety window — delay %lus (reflash window: %lus left)\n", seconds, remaining);
    
    // Stop BLE advertising to simulate deep sleep during this delay
    BLEDevice::getAdvertising()->stop();

    delay(seconds * 1000);
    return;
  }
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
  delay(100); // flush serial output before sleeping
  esp_deep_sleep_start();
}

void PowerManager::_printWakeupReason() {
  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();
  switch (reason) {
    case ESP_SLEEP_WAKEUP_EXT0:     Serial.println("[DEBUG] Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1:     Serial.println("[DEBUG] Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER:    Serial.println("[DEBUG] Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("[DEBUG] Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP:      Serial.println("[DEBUG] Wakeup caused by ULP program"); break;
    default: Serial.printf("[DEBUG] Wakeup was not caused by deep sleep: %d\n", reason); break;
  }
}
