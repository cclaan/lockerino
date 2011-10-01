/* Detects patterns of knocks and triggers a motor to unlock
   it if the pattern is correct.
   
   By Steve Hoefer http://grathio.com
   Version 0.1.10.20.10
   Licensed under Creative Commons Attribution-Noncommercial-Share Alike 3.0
   http://creativecommons.org/licenses/by-nc-sa/3.0/us/
   (In short: Do what you want, just be sure to include this line and the four above it, and don't sell it or use it in anything you sell without contacting me.)
   
   Analog Pin 0: Piezo speaker (connected to ground with 1M pulldown resistor)
   
   Sep 25, 2011 - CC Laan Modified this program to not block with delay() so the program can do other stuff in the meantime
   -- also took out program mode for simplification... needs to be added back
   -- also added a normal knock detection with callback for the SMS alert
   
 */
 
#include "SecretKnockDetector.h"
 
// Pin definitions
const int knockSensor = A1;         // Piezo sensor on pin 0.
const int programSwitch = 2;       // If this is high we program a new code.
const int lockMotor = 3;           // Gear motor used to turn the lock.
const int redLED = 4;              // Status LED
const int greenLED = 5;            // Status LED
 
// Tuning constants.  Could be made vars and hoooked to potentiometers for soft configuration, etc.
const int threshold = 18;           // Minimum signal from the piezo to register as a knock... you'll need to adjust this 
const int rejectValue = 25;        // If an individual knock is off by this percentage of a knock we don't unlock..
const int averageRejectValue = 15; // If the average timing of the knocks is off by this percent we don't unlock.
const int knockFadeTime = 200;     // milliseconds we allow a knock to fade before we listen for another one. (Debounce timer.)
const int lockTurnTime = 650;      // milliseconds that we run the motor to get it to go a half turn.

const int maximumKnocks = 14;       // Maximum number of knocks to listen for.
const int knockComplete = 1200;     // Longest time to wait for a knock before we assume that it's finished.

// Variables.
int secretCode[maximumKnocks] = {50, 25, 25, 50, 100, 50, 0, 0, 0, 0, 0, 0, 0};  // Initial setup: "Shave and a Hair Cut, two bits."
int knockReadings[maximumKnocks];   // When someone knocks this array fills with delays between knocks.
int knockStrengthReadings[maximumKnocks];   // When someone knocks this array fills with delays between knocks.

int knockSensorValue = 0;           // Last reading of the knock sensor.
int programButtonPressed = false;   // Flag so we remember the programming button setting at the end of the cycle.

unsigned long lastKnockTime = 0;

SecretKnockDetector::SecretKnockDetector() {
  
  
  
  
  
}

void SecretKnockDetector::loop() {
  
  delayMicroseconds(450);
  
  // Listen for any knock at all.
  knockSensorValue = analogRead(knockSensor);
  delayMicroseconds(450);
  
  /*
  if (digitalRead(programSwitch)==HIGH){  // is the program button pressed?
    programButtonPressed = true;          // Yes, so lets save that state
    digitalWrite(redLED, HIGH);           // and turn on the red light too so we know we're programming.
  } else {
    programButtonPressed = false;
    digitalWrite(redLED, LOW);
  }
  */
  
  this->listenToSecretKnock();
  
  
  
}


// Records the timing of knocks.
void SecretKnockDetector::listenToSecretKnock(){
  
  int now=millis();
  
  /*
  digitalWrite(greenLED, LOW);      			// we blink the LED for a bit as a visual indicator of the knock.
  if (programButtonPressed==true){
     digitalWrite(redLED, LOW);                         // and the red one too if we're programming a new knock.
  }
  
  delay(knockFadeTime);                       	        // wait for this peak to fade before we listen to the next one.
  
  digitalWrite(greenLED, HIGH);  
  if (programButtonPressed==true){
     digitalWrite(redLED, HIGH);                        
  }
  */
  
  if ( knockSensorValue >= threshold && !listeningToKnocks ){
    
    listeningToKnocks = true;
    
    Serial.println("knock startng");   

    int i = 0;
    // First lets reset the listening array.
    for (i=0;i<maximumKnocks;i++){
      knockReadings[i]=0;
      knockStrengthReadings[i]=0;
    }

    currentKnockNumber=0;         			// Incrementer for the array.
    startTime=millis();           			// Reference for when this knock started.
    
    lastKnockTime = startTime;  
    
    knockStrengthReadings[currentKnockNumber] = knockSensorValue;
   
  }
  

    now=millis();
    
    if ( listeningToKnocks && knockSensorValue >=threshold && ((now - lastKnockTime) > knockFadeTime )){                   //got another knock...
      
      lastKnockTime = now;
      
      //record the delay time.
      Serial.print("knk: ");
      Serial.println(knockSensorValue);
      
      knockReadings[currentKnockNumber] = now-startTime;
      knockStrengthReadings[currentKnockNumber+1] = knockSensorValue;
      
      currentKnockNumber ++;                             //increment the counter
      startTime=now; 
      
      /*
      // and reset our timer for the next knock
      digitalWrite(greenLED, LOW);  
      if (programButtonPressed==true){
        digitalWrite(redLED, LOW);                       // and the red one too if we're programming a new knock.
      }
      delay(knockFadeTime);                              // again, a little delay to let the knock decay.
      digitalWrite(greenLED, HIGH);
      if (programButtonPressed==true){
        digitalWrite(redLED, HIGH);                         
      }
      */
    }

  
  if (listeningToKnocks && ((now-startTime >= knockComplete) || (currentKnockNumber >= maximumKnocks))) {
    
    listeningToKnocks = false;
    
    //we've got our knock recorded, lets see if it's valid
    if (programButtonPressed==false){             // only if we're not in progrmaing mode.
      
      Serial.println("Knk dn");
      
      if (validateSecretKnock() == true){
        
        Serial.println("vld");
        //triggerDoorUnlock(); 
        secretKnockDetectedCallback();
        
      } else if ( validateNormalKnock() ) {
        
        Serial.println("Scrt fail, nrml ok");
        normalKnockDetectedCallback();
        
      } else {
        
        // failed
        Serial.println("Scrt fail");
        /*
        digitalWrite(greenLED, LOW);  		// We didn't unlock, so blink the red LED as visual feedback.
        for (int i=0;i<4;i++){					
          digitalWrite(redLED, HIGH);
          delay(100);
          digitalWrite(redLED, LOW);
          delay(100);
        }
        digitalWrite(greenLED, HIGH);
        */
        
      }
      
    } else { // if we're in programming mode we still validate the lock, we just don't do anything with the lock
      validateSecretKnock();
      // and we blink the green and red alternately to show that program is complete.
      //Serial.println("New lock stored.");
      
      /*digitalWrite(redLED, LOW);
      digitalWrite(greenLED, HIGH);
      for (int i=0;i<3;i++){
        delay(100);
        digitalWrite(redLED, HIGH);
        digitalWrite(greenLED, LOW);
        delay(100);
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED, HIGH);      
      }*/
      
    }
    
    
  }
  
  
}



boolean SecretKnockDetector::validateNormalKnock() {
  
  int totalStrength = 0;
  int totalKnocks = currentKnockNumber+1;
  int maxKnockInterval = 0;
  
  for ( int i=0; i<totalKnocks; i++ ) {
    
    totalStrength += knockStrengthReadings[i];
    
    /*
    Serial.print("Strength: ");
    Serial.print(knockStrengthReadings[i]);
    Serial.print(", Interval: ");
    Serial.println(knockReadings[i]);
    */
    
    if ( knockReadings[i] > maxKnockInterval ) {
      maxKnockInterval = knockReadings[i];
    }
    
  }
  
  
  /*
  // you'll want to uncomment this to see what average values you are getting from the piezo... then set the 'threshold' variable accordingly
  Serial.println(" ");
  Serial.print("Total strength: ");
  Serial.println(totalStrength);
  
  Serial.print(" Count: ");
  Serial.println(totalKnocks);
  
  Serial.print(" Max Interval: ");
  Serial.println(maxKnockInterval);
  */
  if ( totalKnocks > 4 ) {
    return true;
  } else if ( totalKnocks >= 3 && totalStrength > threshold*3 ) {
    return true;
  }
  
  return false;
  
  
}

// Sees if our knock matches the secret.
// returns true if it's a good knock, false if it's not.
// todo: break it into smaller functions for readability.
boolean SecretKnockDetector::validateSecretKnock(){
  
  int i=0;
  
  // simplest check first: Did we get the right number of knocks?
  int currentKnockCount = 0;
  int secretKnockCount = 0;
  int maxKnockInterval = 0;          			// We use this later to normalize the times.
  
  for (i=0;i<maximumKnocks;i++){
    if (knockReadings[i] > 0){
      currentKnockCount++;
    }
    if (secretCode[i] > 0){  					//todo: precalculate this.
      secretKnockCount++;
    }
    
    if (knockReadings[i] > maxKnockInterval){ 	// collect normalization data while we're looping.
      maxKnockInterval = knockReadings[i];
    }
  }
  
  /*
  // If we're recording a new knock, save the info and get out of here.
  if (programButtonPressed==true){
      
    for (i=0;i<maximumKnocks;i++){ // normalize the times
        secretCode[i]= map(knockReadings[i],0, maxKnockInterval, 0, 100); 
      }
      
      // And flash the lights in the recorded pattern to let us know it's been programmed.
      digitalWrite(greenLED, LOW);
      digitalWrite(redLED, LOW);
      delay(1000);
      digitalWrite(greenLED, HIGH);
      digitalWrite(redLED, HIGH);
      
      delay(50);
      for (i = 0; i < maximumKnocks ; i++){
        digitalWrite(greenLED, LOW);
        digitalWrite(redLED, LOW);  
        // only turn it on if there's a delay
        if (secretCode[i] > 0){                                   
          delay( map(secretCode[i],0, 100, 0, maxKnockInterval)); // Expand the time back out to what it was.  Roughly. 
          digitalWrite(greenLED, HIGH);
          digitalWrite(redLED, HIGH);
        }
        delay(50);
      }
	  return false; 	// We don't unlock the door when we are recording a new knock.
  }
  */
  
  if (currentKnockCount != secretKnockCount){
    return false; 
  }
  
  /*  Now we compare the relative intervals of our knocks, not the absolute time between them.
      (ie: if you do the same pattern slow or fast it should still open the door.)
      This makes it less picky, which while making it less secure can also make it
      less of a pain to use if you're tempo is a little slow or fast. 
  */
  int totaltimeDifferences=0;
  int timeDiff=0;
  for (i=0;i<maximumKnocks;i++){ // Normalize the times
    knockReadings[i]= map(knockReadings[i],0, maxKnockInterval, 0, 100);      
    timeDiff = abs(knockReadings[i]-secretCode[i]);
    if (timeDiff > rejectValue){ // Individual value too far out of whack
      return false;
    }
    totaltimeDifferences += timeDiff;
  }
  // It can also fail if the whole thing is too inaccurate.
  if (totaltimeDifferences/secretKnockCount>averageRejectValue){
    return false; 
  }
  
  return true;
  
}










