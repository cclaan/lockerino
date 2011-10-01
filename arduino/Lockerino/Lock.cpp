#include "Lock.h"

#define LOCK_TRANSITION_TIME_MS 800


Lock::Lock() {
  
  // you'll have to adjust this stuff for your lock...
  
  
  // these are the PWM values that are written to servo
  _servoLockedPosition = 114;
  _servoUnlockedPosition  = 10;
  
  // which pin is the lock servo on.. 
  _servoPin = 7;
    
  // if you modified your servo to watch the potentiometer, which analog pin are you using...
  _potentiometerPin = A0;
  
  
  // these are the values that are read in from the modified servo pot on the analog pin
  // you'll probably have to log out what values work for you then modify these...
  _potLockedValue = 295; // greater than this is locked
  _potUnlockedValue = 220; // less than this is unlocked 
  
  
  /////////////////////
  /*
  float pl = (float)_potLockedValue;
  float pu = (float)_potUnlockedValue;
  
  l_a = (100.0 - 50.0) / (pl - pu);
  l_b = 50.0 - l_a * pu; 
  
  // b = y1 * a - x1 
  // y = ax + b
  
  float test_x = _potLockedValue;
  int scaled_val = l_a * test_x + l_b;
  
  Serial.print("Should be >= 100.0: ");
  Serial.println(scaled_val);
  
  test_x = _potUnlockedValue;
  scaled_val = l_a * test_x + l_b;
  
  Serial.print("Should be <= 50.0: ");
  Serial.println(scaled_val);
  */

  
}



void Lock::loop() {
  
  if ( this->isTransitioning() ) { 
    
    if ( (millis() - _timeStartTransition) > LOCK_TRANSITION_TIME_MS ) {
      
      if ( _desiredState == LOCK_STATE_LOCKING && this->observedState() == LOCK_STATE_LOCKED ) {
        
        _desiredState = LOCK_STATE_LOCKED;
        lockServo.detach();
        
      } else if ( _desiredState == LOCK_STATE_UNLOCKING && this->observedState() == LOCK_STATE_UNLOCKED ) {

        _desiredState = LOCK_STATE_UNLOCKED;
        lockServo.detach();

      } else {
        // still waiting for desired state to match observed state
      }
      
    }
    
  }
  
}

void Lock::lock() {
  
  _timeStartTransition = millis();
  _desiredState = LOCK_STATE_LOCKING;
  
  lockServo.attach(_servoPin);
  
  delayMicroseconds(5);
  
  lockServo.write(_servoLockedPosition); 
  
}

void Lock::unlock() {
  
  _timeStartTransition = millis();
  _desiredState = LOCK_STATE_UNLOCKING;
  
  lockServo.attach(_servoPin);
  
  delayMicroseconds(5);
  
  lockServo.write(_servoUnlockedPosition);
  
}

int Lock::position() {

  delayMicroseconds(450);
  
  int potentiometerValue = analogRead(_potentiometerPin);  
  
  delayMicroseconds(450);
  
  
  //float _x = potentiometerValue;

  // scaled values to 50 - 100.. where less than 50 is unlocked and greater than 100 is locked
  
  //return int(l_a * _x + l_b);
  
 return potentiometerValue;
  
}

//bool Lock::isLocking();
//bool Lock::isUnlocking();

bool Lock::isTransitioning() {
  return ( _desiredState == LOCK_STATE_UNLOCKING || _desiredState == LOCK_STATE_LOCKING );
}

LockState Lock::observedState() {
  
  LockState _obs = LOCK_STATE_UNKNOWN;
  
  delayMicroseconds(450);
  
  int potentiometerValue = analogRead(_potentiometerPin);  
  
  delayMicroseconds(450);
  
  if ( potentiometerValue >= _potLockedValue ) {
    _obs = LOCK_STATE_LOCKED;
  } else if ( potentiometerValue <= _potUnlockedValue ){
    _obs = LOCK_STATE_UNLOCKED;
  } else {
    _obs = LOCK_STATE_UNKNOWN;
  }
  
  return _obs;
  
}









