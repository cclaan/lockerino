#ifndef __LOCK_H__
#define __LOCK_H__

#include "WProgram.h"

#include <Servo.h>

#define USE_POTENTIOMETER_FEEDBACK 1

typedef enum LockState {
  
  LOCK_STATE_UNKNOWN,
  LOCK_STATE_UNLOCKED,
  LOCK_STATE_LOCKED,
  LOCK_STATE_UNLOCKING,
  LOCK_STATE_LOCKING,
  
} LockState;


class Lock {
  
public:
  
  Lock();
  
  void loop();
  
  void lock();
  void unlock();
  
  int position();
  
  bool isTransitioning();
  
  LockState observedState();
  LockState desiredState();
  
private:
  
  int _position;
  //LockState _observedState;
  LockState _desiredState;
  
  Servo lockServo;  
  
  int _potUnlockedValue;
  int _potLockedValue;
  
  int _servoLockedPosition;
  int _servoUnlockedPosition;
  
  int _servoPin;
  int _potentiometerPin;
  
  // line equation for scaling potentiometer vals.. maybe change to ints  
  //float l_a;
  //float l_b;
  
  unsigned long _timeStartTransition;
  
};

#endif
