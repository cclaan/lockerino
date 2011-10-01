#include "WProgram.h"
void setup();
void loop();
#include "WiFly.h"
#include "Credentials.h"

//#include <Ethernet.h> // should work with Ethernet shield too

#include <Servo.h> 

#include <String.h>

#include "LockClient.h"

#include <avr/pgmspace.h>

#define SOFTWARE_RESET_PIN 5



LockClient lockClient("lockitron.appspot.com", 80);

void setupWifi(); 
void stupidReset(); // not recommended reset of arduino + wifly if we dont get a response for a while
void beep(); // optional piezo buzzer for debugging
void stateCallback();

void setup() {
    
  digitalWrite(SOFTWARE_RESET_PIN, HIGH); //We need to set it HIGH immediately on boot
  pinMode(SOFTWARE_RESET_PIN,OUTPUT);     //We can declare it an output ONLY AFTER it's HIGH

  Serial.begin(38400);
  Serial.println("Lockerino Initialization...");

  setupWifi();
  
  lockClient.stateReceivedCallback = &stateCallback;
  
  delay(100);
  
}

void stateCallback() {
  
  Serial.println("State callback!!!");
  
  if ( lockClient.desiredState == LOCK_STATE_LOCKED ) {
     Serial.println("LOCK_STATE_LOCKED");
  } else if ( lockClient.desiredState == LOCK_STATE_UNLOCKED ) {
     Serial.println("LOCK_STATE_UNLOCKED");
  } 
  
}

void loop()
{
  /*

  Lock lock(7, 230, 450); // PIN 7 , 230 min, 450 max locked 
  lock.run();
  lock.lock();
  lock.unlock();
  lock.status();
  lock.position(); // 0 = unlocked -> 100 = locked

  */

  // send status every second 

  if ( lockClient.connected() && !lockClient.isRequestPending() ) {

    Serial.println("Connected.. making request..");
    //lockClient.post("lock_status" , "observered=yes&blah");
    
    lockClient.observedState = LOCK_STATE_LOCKED; // lock.status()
    lockClient.lockAngle = 223; // lock.position()
    lockClient.postLockStateToServer();
    delay(1000);
    
  } else {

    //Serial.println("NOT connected");
    //stupidReset();
    delayMicroseconds(1);
    
  }
  

  lockClient.loop();
	
    
}

//////////////



void checkServer() {
  
    
   
}


//////////////

void stupidReset() {
  
  Serial.println("Doing Software Reset...");
  digitalWrite(SOFTWARE_RESET_PIN, LOW);
    
}

void setupWifi() {
  
  WiFly.begin();

  Serial.println(" WiFly Initialized... ");

  if (!WiFly.join(ssid, passphrase)) {
    Serial.println("Association failed.");
    delay(1000);
    stupidReset();
  }  

  Serial.println("Joined WiFi Network.. ");

  WiFly.configure(WIFLY_BAUD, 38400);

  Serial.println("Client Connecting...");

  delay(1);

  if ( lockClient.connect() ) {

    Serial.println("connected");
    beep();

  } else {

  Serial.println("connection failed");

    delay(1000);
    stupidReset();

  }
   
  
}

void beep() {
    
  digitalWrite(6, HIGH);
  delay(90);
  digitalWrite(6, LOW);
    
}











