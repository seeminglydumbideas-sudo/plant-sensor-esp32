#ifndef BLUES_H
#define BLUES_H

#include <BLEAdvertising.h>
#include "Sensor.h" // for SensorData

class BlueS {

public:
  BlueS();
  void init();
  void loop();

  // Called by Sensor to push fresh readings into the next scan response.
  void setData(const SensorData& data);

private:
  BLEAdvertising* _pAdvertising;
  bool       _hasData;
  SensorData _data;

  void _updateScanResponse();
};

#endif // BLUES_H
