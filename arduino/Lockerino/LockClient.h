
#ifndef __LOCK_CLIENT_H__
#define __LOCK_CLIENT_H__

#include "WiFly.h"
#include "CrappyParser.h"

#include "Lock.h"

#define LOCK_CLIENT_TIMEOUT_MS 3000 

class LockClient : public Client {
    
 public:
     
  LockClient(uint8_t *ip, uint16_t port) : Client(ip, port){};
  LockClient(const char* _domain, uint16_t port) : Client(_domain, port){myDomain = _domain;};
  
  void loop();
  void get(const char* dest);
  void post(const char* dest, const char* content);
  
  LockState desiredState;
  LockState observedState;
  int lockAngle;
  unsigned long lastCommandTimestamp;
  unsigned long serverNowTimestamp;
  
  boolean knockDetected;
  boolean hasKnockSentConfirmation;
  
  unsigned long lastRequestMadeTimestamp;
  
  void postLockStateToServer();
  void reset();
  void (*stateReceivedCallback)();
  void (*requestTimeoutCallback)();
  
  boolean isRequestPending();
  //void makeRequest();
  //bool hasRequest();
  const char *myDomain;
  
  void sendFullRequestToClient();
  void makeRequest();
  
 private:
    
    void _parserFinished();
    void _keyValueRecieved();
    
     unsigned int timeout;
     CrappyParser parser;
     boolean pendingRequest;
     CrappyParser * parser2;

};

#endif
