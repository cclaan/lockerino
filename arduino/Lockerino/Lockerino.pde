#include "WiFly.h"
#include "Credentials.h"

//#include <Ethernet.h> // should work with Ethernet shield too

#include <Servo.h> 

#include <String.h>

#include "LockClient.h"

//#include "KnockDetector.h"
#include "SecretKnockDetector.h"

#include <avr/pgmspace.h>

#define SOFTWARE_RESET_PIN 5
#define MS_BETWEEN_REQUESTS 1200

#define MS_REQUEST_TIMEOUT 30000 // how long to wait for a reset.. 30 seconds for our router to reset

Lock lock; // Servo on PIN 7 , less than 230 pot reading = locked, greater than 450 = unlocked 
                        // you'll have to change this to suit your needs.. or if the servo is upside down, even more

//KnockDetector knockDetector;                        
SecretKnockDetector secretKnockDetector;

LockClient lockClient("lockitron.appspot.com", 80);



unsigned long lastCommandTimestamp = 0;
unsigned long lastRequestMadeTimestamp = 0;
unsigned long lastResponseReceivedTimestamp = 0;

boolean pendingLockConfirmation;

void setupWifi(); 
void stupidReset(); // not recommended reset of arduino + wifly if we dont get a response for a while
void beep(); // optional piezo buzzer for debugging
void stateCallback();
void timeoutCallback();
void knockDetectedCallback();

void setup() {
    
  digitalWrite(SOFTWARE_RESET_PIN, HIGH); //We need to set it HIGH immediately on boot
  pinMode(SOFTWARE_RESET_PIN,OUTPUT);     //We can declare it an output ONLY AFTER it's HIGH

  Serial.begin(38400);
  Serial.println("Init...");

  setupWifi();
  
  lockClient.stateReceivedCallback = &stateCallback;
  lockClient.requestTimeoutCallback = &timeoutCallback;
  
  //knockDetector.knockDetectedCallback = &knockDetectedCallback;
  
  secretKnockDetector.secretKnockDetectedCallback = &secretKnockDetectedCallback;
  secretKnockDetector.normalKnockDetectedCallback = &knockDetectedCallback;
  
  // keep tabs on how long it's been since we've gotten a server response
  lastResponseReceivedTimestamp = millis();
  
  delay(100);
  
}

void secretKnockDetectedCallback() {
  Serial.println("sec kn");
  
   lock.unlock();
}

void knockDetectedCallback() {
  
  Serial.println("\n Knck \n");
  lockClient.knockDetected = true;
  pendingLockConfirmation = true;
  
  
}

void timeoutCallback() {
  
  // if request times out or parser can't handle the response this happens
  // since our loop will just request again in a few seconds we dont do anything here
  Serial.println("t-out");
  //delayMicroseconds(10);
  
}

void stateCallback() {
  
  Serial.println("State callback");
  
  lastResponseReceivedTimestamp = millis();
  
  //if ( lockClient.hasKnockSentConfirmation ) {
    lockClient.knockDetected = false;
    pendingLockConfirmation = false;
  //}
  
    unsigned long diff = lockClient.serverNowTimestamp - lockClient.lastCommandTimestamp;
    //Serial.print("Diff: ");
    //Serial.println(diff);
    
    //if ( )
  if ( diff <= 20 && (lastCommandTimestamp != lockClient.lastCommandTimestamp)  ) {
    
    //Serial.print(" mine: ");
    //Serial.print(lastCommandTimestamp);
    //Serial.print(" client: ");
    //Serial.println(lockClient.lastCommandTimestamp);
    
    
    // this ts is how we know a new command was issued, since we're constantly polling
    lastCommandTimestamp = lockClient.lastCommandTimestamp;
    
    
    if ( lockClient.desiredState == LOCK_STATE_LOCKED ) {
      lock.lock();
      Serial.println("*lock");
    } else if ( lockClient.desiredState == LOCK_STATE_UNLOCKED ) {
      lock.unlock();
      Serial.println("*unlock");
    }
    
  }
  
}

void loop()
{
  
  // send status every second 
  
  if ( lockClient.connected() && !lockClient.isRequestPending() && ( (millis()-lastRequestMadeTimestamp) > MS_BETWEEN_REQUESTS) ) {

    Serial.println("reqst..");
        
    lockClient.observedState = lock.observedState();
    lockClient.lockAngle = lock.position();
    
    lockClient.postLockStateToServer();
    lastRequestMadeTimestamp = millis();
    
    //delay(1000);
    
  } else {
   
    // seems to always be connected even when no wifi.. have to check on these
    if ( !lockClient.connected() ) {
      Serial.println("n con'd");
    }
    if ( lockClient.isRequestPending() ) {
      //Serial.println("pend");
    }
    
    //stupidReset();
    delayMicroseconds(1);
    
  }
  
  //knockDetector.loop();
  
  secretKnockDetector.loop();
  
  lock.loop();
  
  lockClient.loop();
  
  
  if ( (millis()-lastResponseReceivedTimestamp) > MS_REQUEST_TIMEOUT ) {
    Serial.println("Req t-out! rst!");
    delay(300);
    stupidReset();
    
  }
    
}

//////////////



void checkServer() {
  
    
   
}


//////////////

void stupidReset() {
  
  //Serial.println("Doing Software Reset...");
  digitalWrite(SOFTWARE_RESET_PIN, LOW);
    
}

void setupWifi() {
  
  WiFly.begin();

  Serial.println("Init");

  // this fails sometimes and hangs everything, need a non-blocking one.. will look into
  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Assoc fail");
    delay(1000);
    stupidReset();
  }  

  Serial.println("..");

  WiFly.configure(WIFLY_BAUD, 38400);

  //Serial.println("..");

  //delay(1);

  if ( lockClient.connect() ) {

    Serial.println("..");
    beep();

  } else {

  Serial.println(".failed");

    delay(1000);
    stupidReset();

  }
   
  
}

void beep() {
    
  digitalWrite(6, HIGH);
  delay(90);
  digitalWrite(6, LOW);
    
}











