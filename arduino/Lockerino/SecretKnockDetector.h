#ifndef __SECRET_KNOCK_DETECTOR_H__
#define __SECRET_KNOCK_DETECTOR_H__

#include "WProgram.h"

class SecretKnockDetector {

public:
  
  SecretKnockDetector();
  
  void loop();
  void (*secretKnockDetectedCallback)();
  void (*normalKnockDetectedCallback)();
  
private:
  
  //int piezoPin;    // piezo for knock detection
  
  boolean listeningToKnocks;
  int currentKnockNumber;         			// Incrementer for the array.
  int startTime;           			// Reference for when this knock started.
  
  void listenToSecretKnock();
  boolean validateSecretKnock();
  boolean validateNormalKnock();
  
};

#endif
