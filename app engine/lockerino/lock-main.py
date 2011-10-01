import os
import cgi
import random
import time
import uuid

from datetime import timedelta
from datetime import datetime

from django.utils import simplejson as json

from google.appengine.ext import webapp
from google.appengine.ext.webapp.util import run_wsgi_app
from google.appengine.ext import db
from google.appengine.ext.webapp import template
from google.appengine.api import users

import google.appengine.ext.db

from twilio.rest    import TwilioRestClient


class UserPrefs(db.Model):
  
    user = db.UserProperty()
    lock = db.ReferenceProperty()
    lock_id = db.StringProperty()
    #counter = db.IntegerProperty(0)
    #locks = db.ListProperty(db.Key)
    #is_admin = db.BooleanProperty()
    
    @classmethod
    def find_or_create_by_user(klass, n):
      q = klass.gql("WHERE user = :1 LIMIT 1" , n)
      entity = q.get()
      if entity is None:
        entity = UserPrefs(user=n)
        entity.put()
      return entity

    #return db.run_in_transaction(txn)



class DoorLock(db.Model):
    ipaddress = db.StringProperty()
    lock_name = db.StringProperty()
    lock_status = db.StringProperty()
    lock_id = db.StringProperty()
    
    observed_status = db.StringProperty()
    date_observed_status_updated = db.DateTimeProperty(auto_now_add=True)
    date_updated = db.DateTimeProperty(auto_now_add=True)
    
    is_locked = db.BooleanProperty()
    date_created = db.DateTimeProperty(auto_now_add=True)
    date_used = db.DateTimeProperty(auto_now_add=True)

    lock_angle = db.IntegerProperty()
    
    IS_LOCKED="islocked"
    UNLOCKED="unlocked"
    UNKNOWN="unknown"
                
                    
    def get_basic_json_response(self):
      dt = datetime.now()
      ts2 = time.mktime(self.date_observed_status_updated.timetuple())

      json_resp = { 
          "observed_status": self.observed_status, 
          "last_updated": str(self.date_observed_status_updated),
          "lock_angle": str(self.lock_angle),
          "last_updated_ts": ts2,
          "date_now": str(datetime.now()),    
      }
      return json_resp
    
    # go back to using this since we want unique keys
    @classmethod
    def find_or_create_by_name(klass, n):
    	q = klass.gql("WHERE lock_name = :1 LIMIT 1" , n)
    	dl = q.get()
    	if not dl:
    		dl = DoorLock(lock_name=n, lock_status=klass.IS_LOCKED)
    		dl.put()
    	return dl
    	
    @classmethod
    def get_lock_by_id(klass, lock_id):
      q = klass.gql("WHERE lock_id = :1 LIMIT 1" , lock_id)
      return q.get()



class HowToWeb(webapp.RequestHandler):
  def get(self):
      
      lock_id = ""
      logout_url = ""
      
      user = users.get_current_user()
      
      if user:
          logout_url = users.create_logout_url("/")
          userprefs = UserPrefs.find_or_create_by_user(user)
          
          if not userprefs.lock:
            lock_id = uuid.uuid4()
            dl = DoorLock(lock_name="front_door", lock_status=DoorLock.IS_LOCKED)
            dl.lock_id = str(lock_id)
            dl.put()
            #self.response.out.write("no lock")
            userprefs.lock = dl
            userprefs.lock_id = str(lock_id)
            userprefs.put()
            
          lock_id = userprefs.lock.lock_id
                              
      
      lurl = users.create_login_url(dest_url="/howto")
      
      path = os.path.join(os.path.dirname(__file__), 'welcome.html')
      template_values = {
      'g_user': user,
      'lock_id':lock_id,
      'login_url': lurl,
      'logout_url' : logout_url
      }
      content = template.render(path, template_values)
      
      template_values = {
          'g_user': user,
          'page_content': content,
          'page_title': 'Welcome!',
          'login_url': lurl,
          'logout_url' : logout_url,
      }

      path = os.path.join(os.path.dirname(__file__), 'basic-page.html')
      self.response.out.write(template.render(path, template_values))
      
      
      
     
class MainPage(webapp.RequestHandler):
    def get(self):    
        
        lock_id = ""
        lout = ""
        logout_url = ""
        user = users.get_current_user()

        if user:
            logout_url = users.create_logout_url("/")
            userprefs = UserPrefs.find_or_create_by_user(user)
            
            if not userprefs.lock:
              self.redirect("/howto")
              return
            
            lock_id = userprefs.lock.lock_id
            

        login_url = users.create_login_url(dest_url="/")
        
        template_values = {
        'g_user': user,
        'lock': lock_id,
        'login_url' : login_url,
        'logout_url': logout_url,
        }
        
        path = os.path.join(os.path.dirname(__file__), 'index.html')
        self.response.out.write(template.render(path, template_values))
        
        
        
def set_lock_status_trans(dl, st):

  if st in [DoorLock.UNLOCKED, DoorLock.IS_LOCKED]:
    lock_set = 1
    try:
      db.run_in_transaction(set_lock_status, dl.key(), st)
    except Rollback:
      lock_set = 0

  return lock_set
  
def set_lock_status(key, status):
  dl = db.get(key)
  dl.lock_status = status
  dl.date_updated = datetime.now();
  dl.put()

# API set_observed_status
# for arduino 
class SetObservedStatus(webapp.RequestHandler):
  def post(self):
    
    observed = self.request.get('observed_status')
    lock_id = self.request.get('unsecure_lock_id')
    
    knock_detected = self.request.get('knock_detected')

    if ( knock_detected ):
     send_knock_message()

    dl = DoorLock.get_lock_by_id(lock_id)
    
    if not dl:
      self.response.out.write("no lock for: ")
      self.response.out.write(lock_id)
      return

    if observed in [DoorLock.UNLOCKED, DoorLock.IS_LOCKED, DoorLock.UNKNOWN]:

       dl.date_observed_status_updated = datetime.now();
       dl.observed_status = observed

       if self.request.get('lock_angle'):
         dl.lock_angle = int(self.request.get('lock_angle'));

       dl.put()


    ts_updated = time.mktime(dl.date_updated.timetuple())
    ts_now = time.mktime(datetime.utcnow().timetuple())
    
    json_resp = { 
       "lock_status" : dl.lock_status,
       "ts" : int(ts_updated),
       "tsnow" : int(ts_now),
    }

    if (knock_detected):
     json_resp["knock_msg_sent"] = 1

    self.response.out.write(json.dumps(json_resp))
    
    
# used by iphone app and ajax call from webpage 
class GetObservedStatus(webapp.RequestHandler):
  def get(self):
    
    lock_id = self.request.get('unsecure_lock_id')
    
    dl = ""
    user = users.get_current_user()
    
    if lock_id:
      q = DoorLock.gql("WHERE lock_id = :1 LIMIT 1" , lock_id)
      dl = q.get()
    elif user:
      userprefs = UserPrefs.find_or_create_by_user(user)
      dl = userprefs.lock
    else:
      json_resp = { 
          "observed_status": "not_logged_in"}
      self.response.headers["Content-Type"] = "text/json"
      self.response.out.write(json.dumps(json_resp))
      return


    json_resp = dl.get_basic_json_response()
    
    self.response.headers["Content-Type"] = "text/json"
    self.response.out.write(json.dumps(json_resp))


# /api/set_lock      
class SetLockAPI(webapp.RequestHandler):
  def post(self):
    st = self.request.get('status')
  
    lock_id = self.request.get('unsecure_lock_id')
    
    dl = DoorLock.get_lock_by_id(lock_id)
    
    lock_set = set_lock_status_trans(dl,st)
    
    json_resp = dl.get_basic_json_response()
    json_resp["lock_set"] = lock_set
    
    self.response.headers["Content-Type"] = "text/json"
    self.response.out.write(json.dumps(json_resp))
    
    
    
class SetLockWeb(webapp.RequestHandler):
  
    def post(self):
      
      user = users.get_current_user()
      
      if user:

        userprefs = UserPrefs.find_or_create_by_user(user)
        dl = userprefs.lock
        
        st = self.request.get('status')
        
        lock_set = set_lock_status_trans(dl,st)
        
        json_resp = dl.get_basic_json_response()
        json_resp["lock_set"] = lock_set
            
        self.response.headers["Content-Type"] = "text/json"
        self.response.out.write(json.dumps(json_resp))
        

application = webapp.WSGIApplication([
('/api/set_observed_status', SetObservedStatus), # used by arduino, returns lock_status
('/api/get_observed_status', GetObservedStatus), # used by iphone
('/api/set_lock', SetLockAPI ), # used by iphone app
('/set_lock', SetLockWeb ), # used by web page
('/howto', HowToWeb ), # 
('/.*', MainPage) ],
debug=True)



def send_knock_message():
  
    TWILIO_ACCOUNT_SID = "YOURACCOUNTSID"
    TWILIO_AUTH_TOKEN = "YOUR AUTH TOKEN"
    
    sms_recipient   = "15551234567"         # Your phone number
    sms_sender      = "14155992671"         # Twilio trial accounts must use this sender
    
    dt = datetime.utcnow()
    mintime = timedelta(hours=4) # edt time ... need better way to do this
    dt3 = dt - mintime
    fdate = dt3.strftime("%a %I:%M:%S %p ")
    
    desired_message = "Knock: " + fdate

    client          = TwilioRestClient( TWILIO_ACCOUNT_SID, TWILIO_AUTH_TOKEN)

    message         = client.sms.messages.create(
            to      = sms_recipient,
            from_   = sms_sender,
            body    = desired_message)


def main():
    run_wsgi_app(application)


if __name__ == "__main__":
    main()
    
    
    