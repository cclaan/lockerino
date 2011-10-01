#include "LockClient.h"

#include "Constants.h"

#include <avr/pgmspace.h>

//extern char * MY_LOCK_ID;


void LockClient::loop() {

  if ( this->available() ) {

    char c = this->read();
    //Serial.print(c);
    
    if ( !parser.isDoneParsing() ) {
       parser.parseChar(c); 
       
       if ( parser.hasNewKeyValue() ) {
         // save it
         this->_keyValueRecieved();
       }
       
       if ( parser.isDoneParsing() ) {
         this->_parserFinished();
       }
    }
    
  }
  
  // TODO: fix this , gets called million times if actually times out
  if ( (millis() - lastRequestMadeTimestamp) > LOCK_CLIENT_TIMEOUT_MS ) {
    
    this->reset();
    //requestTimeoutCallback();
    
  }
  

}

void LockClient::_keyValueRecieved() {
  
  //Serial.println("Got keyval");
  //Serial.println(parser.key_buff);
  
  //if ( parser.key == "lock_status" ){
  if ( strcmp ( parser.key_buff, "lock_status" ) == 0 ){
     
     //Serial.print("gt sts: ");
     //Serial.println(parser.val_buff);
     
     //if ( parser.value == "unlocked" ){
     if ( strcmp ( parser.val_buff, "unlocked" ) == 0 ){
       this->desiredState = LOCK_STATE_UNLOCKED;
     //} else if ( parser.value == "islocked" ){
     } else if ( strcmp ( parser.val_buff, "islocked" ) == 0 ) {
       this->desiredState = LOCK_STATE_LOCKED;
     } else {
       this->desiredState = LOCK_STATE_UNKNOWN;
     }
    
  } else if ( strcmp ( parser.key_buff, "tsnow" ) == 0 ){

       Serial.print("gt TS NOW: ");
       Serial.println(parser.intValue);

       serverNowTimestamp = parser.intValue;
       
  } else if ( strcmp ( parser.key_buff, "ts" ) == 0 ){
    
    //Serial.print("gt TS: ");
    //Serial.println(parser.intValue);
    
    lastCommandTimestamp = parser.intValue;
    
  //} else if ( parser.key == "knock_msg_sent" && parser.intValue == 1 ) {
  } else if ( (strcmp ( parser.key_buff, "knock_msg_sent" ) == 0) && parser.intValue == 1 ) {
    hasKnockSentConfirmation = true;
  }
  
}

void LockClient::_parserFinished() {
  
  if ( !parser.hadError() ) {
    stateReceivedCallback();
  } else {
  
  }
  
  this->flush();
  pendingRequest = false;
  parser.reset();
  
}

void LockClient::reset()
{
  this->flush();
  pendingRequest = false;
  parser.reset();
}

boolean LockClient::isRequestPending() {
  return pendingRequest;
}

/*
int appendString(char * buff, const char * str2 ) {
  strcpy(buff+strlen(buff), str2);
  return strlen(buff);
}*/

void LockClient::postLockStateToServer() {
  
  if ( !this->connected() ) {
    Serial.println("no conn!");
    return;
  }
  
  lastRequestMadeTimestamp = millis();
  hasKnockSentConfirmation = false;
  pendingRequest = true;
  parser.reset();
  
  char * tempBuff = (char*)malloc(160); // guess of how big this might be
  memset(tempBuff,0,160);
      
  //String content = "observed_status=";
  strcat(tempBuff, "observed_status=" );
  
  if ( this->observedState == LOCK_STATE_UNLOCKED ) {
    //content += "unlocked";
    strcat(tempBuff, "unlocked" );
  } else if ( this->observedState == LOCK_STATE_LOCKED ) {
    //content += "islocked";
    strcat(tempBuff, "islocked" );
  } else if ( this->observedState == LOCK_STATE_UNKNOWN ) {
    //content += "unknown";
    strcat(tempBuff, "unknown" );
  }

  
  if ( this->knockDetected ) {
    //content += "&knock_detected=1";
    strcat(tempBuff, "&knock_detected=1" );
  }
  
  strcat(tempBuff, "&lock_angle=" );
  sprintf (tempBuff+strlen(tempBuff), "%i", lockAngle);
  
  //strcat(tempBuff, "418" );
  
  //content += "&lock_angle=";
  //content += lockAngle;
  
  //content += "&unsecure_lock_id=";
  //content += "b9c9746c-d9ac-4205-9d9c-840d7307b7d7";
  strcat(tempBuff, "&unsecure_lock_id=" );
  //strcat(tempBuff, "b9c9746c-d9ac-4205-9d9c-840d7307b7d7" );
  strcat(tempBuff, MY_LOCK_ID );
  
  //content += '\r';
  //content += '\n';
  strcat(tempBuff, "\r" );
  strcat(tempBuff, "\n" );
  
  //int content_len = content.length()-2;
  int content_len = strlen(tempBuff)-2; // ?
  
  this->print("POST /");
  this->print("api/set_observed_status");
  this->print(" HTTP/1.1\nHost: ");
  this->println(myDomain);
  this->print("Content-Length: ");
  this->println(content_len);
  this->println("Content-Type:application/x-www-form-urlencoded");
  this->println();
  
  //char * buff = (char*)malloc(content.length());
  //content.toCharArray(buff, content.length());
  //this->println(buff);
  this->println(tempBuff);
  //Serial.println(tempBuff);
  
  this->println();
  
  //content = "";
  
  //free(buff);
  free(tempBuff);
  
}
/*
void LockClient::postLockStateToServer() {
  
  if ( !this->connected() ) {
    Serial.println("not connected");
    return;
  }
  
  lastRequestMadeTimestamp = millis();
  hasKnockSentConfirmation = false;
  pendingRequest = true;
  parser.reset();
  
  char * tempBuff = malloc(160); // guess of how big this might be

  String content = "observed_status=";
  
  
  if ( this->observedState == LOCK_STATE_UNLOCKED ) {
    content += "unlocked";
  } else if ( this->observedState == LOCK_STATE_LOCKED ) {
    content += "islocked";
  } else if ( this->observedState == LOCK_STATE_UNKNOWN ) {
    content += "unknown";
  }
  //} else if ( this->observedState == LOCK_STATE_TRANSITIONING ) {
   // content += "transition";
  //}
  
  if ( this->knockDetected ) {
    content += "&knock_detected=1";
  }
  
  content += "&lock_angle=";
  content += lockAngle;
  
  content += "&unsecure_lock_id=";
  content += "b9c9746c-d9ac-4205-9d9c-840d7307b7d7";
  //content += "b9c9746c";
  
  content += '\r';
  content += '\n';
  
  int content_len = content.length()-2;
  
  
  this->print("POST /");
  this->print("api/set_observed_status");
  this->print(" HTTP/1.1\nHost: ");
  this->println(myDomain);
  this->print("Content-Length: ");
  this->println(content_len);
  this->println("Content-Type:application/x-www-form-urlencoded");
  this->println();
  
  char * buff = (char*)malloc(content.length());
  
  content.toCharArray(buff, content.length());
  
  this->println(buff);
  this->println();
  
  content = "";
  
  free(buff);
  
}
*/

/*
void LockClient::post(const char* dest, const char* content) {
  
  if ( !this->connected() ) {
    Serial.println("not connected");
    return;
  }
  
  this->print("POST /");
  this->print(dest);
  this->print(" HTTP/1.1\nHost: ");
  this->println(myDomain);
  this->print("Content-Length: ");
  //this->println("24");
  this->println(strlen(content));
  this->println("Content-Type:application/x-www-form-urlencoded");
  //this->println("Accept-Language: en-US,en;q=0.8");
  
  this->println();
  //this->println("observed_status=unlocked");
  this->println(content);
  this->println();

}

void LockClient::get(const char* dest) {
  
  if ( !this->connected() ) {
    Serial.println("not connected");
    return;
  }
  
  this->print("GET /");
  this->print(dest);
  this->print(" HTTP/1.1\nHost: ");
  this->println(myDomain);
  
  this->println("Cache-Control: max-age=0");
  this->println("User-Agent: Arduino");
  
  //this->println("Accept-Language: en-US,en;q=0.8");
  
  //this->println("Accept-Charset: US-ASCII");
  this->print("\r\n");
  
  
}
*/

/*
void LockClient::get2(const char* dest) {
  
  if ( !this->connected() ) {
    Serial.println("not connected");
    return;
  }
  
  this->print("GET /");
  this->print(dest);
  this->print(" HTTP/1.1\nHost: ");
  this->println(myDomain);
  
  strcpy_P(tempBuffer, requestHeaders ); 
  this->println(requestHeaders);
  
  //this->println();

}
*/





