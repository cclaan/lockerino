#include "CrappyParser.h"


// a very limited parser for some json-like data coming from google app engine
// doesn't have error handling or anything like that
// supports strings and integer values

bool isNumeric(char c);
int digitValue(char c );

void CrappyParser::reset() {
  pstate = WAIT_START_CONTENT;
  memset(tbuff,0,BUFF_LEN);
  //key = "";
  //value = "";
  intValue = 0;
  
  
  this->clearValBuff();
  this->clearKeyBuff();
  
}

boolean CrappyParser::isDoneParsing() {
  return ( pstate == DONE_PARSING );
}

boolean CrappyParser::hasNewKeyValue() {
  return _hasNewKeyValue;
}


// this does nothing.. 
boolean CrappyParser::hadError() {
  return _hadError;
}

/*
void CrappyParser::setDoneCallback(void (*callback)()) {
  _doneCallback = callback; 
}
*/
///

void CrappyParser::parseChar(char c) {
 
 _hasNewKeyValue = false;
 
 if ( pstate == DONE_PARSING ) {
   return;
 }  
 
 if ( pstate == WAIT_START_CONTENT ) {
   
   if ( waitForStart(c) ) {
     pstate = WAIT_START_OPEN;
   }
   
 } else if ( pstate == WAIT_START_OPEN ) {

   if ( c == '{' ) {
     pstate = WAIT_KEY_START;
   }
   
 } else if ( pstate == WAIT_KEY_START ) {
   if ( c == '\"' ) {
     pstate = WAIT_KEY;
     //key = "";
     this->clearKeyBuff();
   }
 } else if ( pstate == WAIT_KEY ) {
   
   if ( c == '\"' ) {
     pstate = WAIT_COLON;
   } else {
     //key += c;
     this->addCharToKey(c);
     //Serial.print(c);
   }
   
 } else if ( pstate == WAIT_COLON ) {
   
   if ( c == ':' ) {
     pstate = WAIT_VALUE_START;
   }
   
 } else if ( pstate == WAIT_VALUE_START ) {
   
   if ( c == '\"' ) {
     pstate = WAIT_VALUE;
     //value = "";
     this->clearValBuff();
     valueIsInteger = false;
   } else if ( isNumeric(c) ) {
     
     pstate = WAIT_VALUE;
     valueIsInteger = true;
     //value = "";
     this->clearValBuff();
     
     //value += c;
     intValue = 0;
     intValue = digitValue(c);
     //Serial.println(c);
     //Serial.println(intValue);
   }
   
 } else if ( pstate == WAIT_VALUE ) {
   
   if ( c == '\"' ) {
     
     pstate = WAIT_COMMA_OR_END;
     parsedKeyValue();
     
   } else if ( valueIsInteger ) {
     
     if ( c == ' ') {
       
       pstate = WAIT_COMMA_OR_END;
       parsedKeyValue();
       
     } else if (c == '}') {
       
       pstate = DONE_PARSING;
       parsedKeyValue();
       doneParsing();
       
     } else if (c == ',') {
       
       parsedKeyValue();
       pstate = WAIT_KEY_START;
       
     } else if ( isNumeric(c) ) {
       intValue *= 10;
       intValue += digitValue(c);
       //Serial.println(c);
       //Serial.println(intValue);
     }
     
   } else {
     //value += c;
     this->addCharToVal(c);
   }
   
 } else if ( pstate == WAIT_COMMA_OR_END ) {
   
   if ( c == ',' ) {
     pstate = WAIT_KEY_START;
   } else if ( c == '}' ) {
     pstate = DONE_PARSING;
     doneParsing();
   }
   
 }
  
}

void CrappyParser::parsedKeyValue() {

  _hasNewKeyValue = true;
  
  //Serial.println("\nGOT KEYVAL");
  
  /*
 Serial.println("\nGOT KEYVAL");
 Serial.print(key);
 Serial.print(" : " );
 
 if ( valueIsInteger ) {
   Serial.println(intValue);
 } else {
   Serial.println(value);
 }
 */
 
}
void CrappyParser::doneParsing() {
  
  //Serial.println("DONE");
  
}

boolean CrappyParser::waitForStart(char c) {
  
  if ( tcnt == BUFF_LEN-1 ) {
   for(int i=0; i<BUFF_LEN-1; i++){
     tbuff[i] = tbuff[i+1];
   }
 }
 
 tbuff[tcnt] = c;
 
 if ( tcnt < BUFF_LEN-1 ) {
   tcnt ++;
 } 

 if ( tbuff[0] == '\r' && tbuff[1] == '\n' ) {
   return true;
 } else {
   return false;
 }
 
}

void CrappyParser::clearKeyBuff(){
  memset(key_buff,0,K_BUFF_LEN);
  key_buff_index=0;
}

void CrappyParser::clearValBuff(){
  memset(val_buff,0,K_BUFF_LEN);
  val_buff_index=0;
}

void CrappyParser::addCharToKey(char c) {
   key_buff[key_buff_index++] = c;
}

void CrappyParser::addCharToVal(char c) {
   val_buff[val_buff_index++] = c;
}

//////////////

int digitValue(char c ) {
  
  int num = int(c) - 48;
  if ( c >= 48 && c <= 57 ) {
     return num; 
  }
  
  return -1;
  
}

bool isNumeric(char c) {
  // no decimal for now 
  //if ( c == '.' ) {
  //  return true;
  //}
  int num = int(c) - 48;
  if ( c >= 48 && c <= 57 ) {
     return true; 
  }
  
  return false;
  
}


