#ifndef MOISTURE_H
#define MOISTURE_H

class Moisture {

public:
  struct Reading {
    int percentage; // 0-100, or -1 when not yet calibrated
    int rawValue;
    int rawMin;
    int rawMax;
  };

  Moisture();
  void init();
  void startRead();       // begin an async read cycle
  void loop();            // advance the state machine (non-blocking)
  bool isReady() const;   // true when a fresh reading is available
  bool isIdle()  const;   // true when not currently reading
  Reading getReading();   // returns last reading and resets ready/idle state
  void resetCalibration(); // clear rawMin/rawMax from memory and flash

private:
  enum State { IDLE, STABILIZING, SAMPLING, COMPLETE };

  State         _state;
  unsigned long _stateTimer;
  int           _sampleCount;
  int           _rawValue;
  int           _rawMin;
  int           _rawMax;
  bool          _ready;
  Reading       _lastReading;

  void _finalize();
  void _saveCalibration();
};

#endif // MOISTURE_H
