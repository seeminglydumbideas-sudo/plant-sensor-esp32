#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#define POWER_BAT 0
#define POWER_USB 1

class PowerManager {

public:
  void  init();
  float getLevel();
  int   checkSource();

  int  getBootCount();

  // Called by Sensor after each reading is pushed to BT.
  // calibrated=false means moisture % is not yet valid.
  void notifyDataReady(bool calibrated);

  // Sleep management — call from main loop().
  // Stays awake while not calibrated; sleeps once a valid reading is ready.
  void loop();

private:
  bool          _dataReady;
  bool          _calibrated;
  unsigned long _dataReadyAt;

  void _deepSleep(unsigned long seconds);
  void _incBootCount();
  void _printWakeupReason();
};

#endif // POWERMANAGER_H
