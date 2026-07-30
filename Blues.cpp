#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEAdvertising.h>
#include "Blues.h"
#include "Secrets.h"

// Company ID 0xFFFF is reserved for internal / testing use (Bluetooth SIG).
#define BLE_COMPANY_ID_LO 0xFF
#define BLE_COMPANY_ID_HI 0xFF

// Custom 128-bit service UUID identifying this soil-moisture sensor service.
// BLE clients can filter advertisements by this UUID.
#define BLE_SERVICE_UUID "a1b2c3d4-0001-4c9a-8f2e-5a7b9e3d6f80"

BlueS::BlueS() : _pAdvertising(nullptr), _hasData(false), _data({-1, 0.0f, 0, 0, 0, 0, 0}) {
}

void BlueS::_updateScanResponse() {
  // Scan response manufacturer data layout:
  //   Byte 0-1   : company ID (little-endian, 0xFFFF = testing)
  //   Byte 2     : moisture percentage (0-100), 0xFF = not yet calibrated
  //   Byte 3     : battery voltage * 10 (e.g. 38 = 3.8 V)
  //   Byte 4     : power source (0 = battery, 1 = USB)
  //   Byte 5-6   : rawValue (uint16_t little-endian)
  //   Byte 7-8   : rawMin   (uint16_t little-endian)
  //   Byte 9-10  : rawMax   (uint16_t little-endian)
  //   Byte 11    : bootCount (wraps at 255)
  uint16_t rawValue = (uint16_t)_data.rawValue;
  uint16_t rawMin   = (uint16_t)_data.rawMin;
  uint16_t rawMax   = (uint16_t)_data.rawMax;

  // Use a binary-safe char array instead of an Arduino String.
  // Arduino String to std::string conversion uses c_str() which truncates at the first 0x00 (null) byte.
  // On battery power, byte 4 (powerSource) is 0x00, truncating the payload to 4 bytes and causing the server to ignore it.
  char buf[12];
  buf[0]  = (char)BLE_COMPANY_ID_LO;
  buf[1]  = (char)BLE_COMPANY_ID_HI;
  buf[2]  = (char)(_data.moisture < 0 ? 0xFF : (uint8_t)_data.moisture);
  buf[3]  = (char)((uint8_t)(_data.battery * 10));
  buf[4]  = (char)((uint8_t)_data.powerSource);
  buf[5]  = (char)(rawValue & 0xFF);
  buf[6]  = (char)(rawValue >> 8);
  buf[7]  = (char)(rawMin   & 0xFF);
  buf[8]  = (char)(rawMin   >> 8);
  buf[9]  = (char)(rawMax   & 0xFF);
  buf[10] = (char)(rawMax   >> 8);
  buf[11] = (char)((uint8_t)_data.bootCount);

  String payload = String(buf, 12);

  BLEAdvertisementData scanResponse;
  scanResponse.setName(SENSOR_ID);
  scanResponse.setManufacturerData(payload);
  _pAdvertising->setScanResponseData(scanResponse);
}

void BlueS::setData(const SensorData& data) {
  _data = data;
  _hasData = true;
  _updateScanResponse(); // Update advertising hardware immediately with fresh telemetry
  Serial.printf("[DEBUG] BLE data updated [%s]: moisture=%d%%, battery=%.1fV, power=%d\n",
                SENSOR_ID, data.moisture, data.battery, data.powerSource);
}

void BlueS::init() {
  BLEDevice::init(SENSOR_ID); // this SENSOR_ID will not be sent in broadcast.
  _pAdvertising = BLEDevice::getAdvertising();
  _pAdvertising->setScanResponse(true);
  _pAdvertising->setMinPreferred(0x00); // disable connection hints

  // Advertisement: UUID only. Static — set once and never changed.
  // Clients use this to filter and identify the sensor without connecting.
  BLEAdvertisementData advData;
  advData.setCompleteServices(BLEUUID(BLE_SERVICE_UUID));
  _pAdvertising->setAdvertisementData(advData);
  _pAdvertising->start();
  Serial.println("[INFO ] BLE advertising started");
}

void BlueS::loop() {
  // BLE advertising updates are driven by setData() events to eliminate redundant register writes
}
