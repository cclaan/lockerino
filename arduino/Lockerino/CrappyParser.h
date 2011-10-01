
#ifndef __CRAPPY_PARSER_H__
#define __CRAPPY_PARSER_H__

#include "WProgram.h"
//#include <String.h>

//const char * json = "HTTP 1.1/ IUHUHSD\nrewergwe\r\n {  \"lock status\": \"unlocked\" ,  \"ts\" : 1234567} ";
#define BUFF_LEN 2

#define K_BUFF_LEN 50

typedef enum ParseState {
   WAIT_START_CONTENT,
   WAIT_START_OPEN,
   WAIT_KEY_START,
   WAIT_KEY,
   IN_KEY,
   WAIT_COLON,
   WAIT_VALUE_START,
   WAIT_VALUE,
   WAIT_NUMERIC_VALUE,
   IN_VALUE,
   WAIT_COMMA_OR_END,
   WAIT_END,
   DONE_PARSING,
} ParseState;

class CrappyParser {
    
 public:
   
  CrappyParser()  { tcnt=0; pstate = WAIT_START_CONTENT; startContent=false; intValue=0; _hasNewKeyValue=false; } ;
  
  void parseChar(char c);  
  void reset();
  
  boolean isDoneParsing();
  boolean hasNewKeyValue();
  boolean hadError();
  
  //void setKeyValCallback();
  //void setDoneCallback(void (*callback)());
  
  //String key, value;
  
  char key_buff[K_BUFF_LEN];
  char val_buff[K_BUFF_LEN];
  int key_buff_index;
  int val_buff_index;
  
  bool valueIsInteger;
  unsigned long intValue;
  
 private:
    
   char tbuff[BUFF_LEN];
   int tcnt;
  
   boolean _hadError;
   
   ParseState pstate;
   bool startContent;
   bool _hasNewKeyValue;
   
   //void (*_doneCallback)() ;
   
   
   // make more types obviously

  boolean waitForStart(char c);
  void doneParsing();
  void parsedKeyValue();
  
  void clearKeyBuff();
  void clearValBuff();
  void addCharToKey(char c);
  void addCharToVal(char c);
  
     

};

#endif
