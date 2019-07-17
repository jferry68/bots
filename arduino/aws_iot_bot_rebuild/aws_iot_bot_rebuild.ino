#include <AWS_IOT.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <Servo.h>

// https://github.com/aws-samples/aws-iot-workshop
// Make sure you install the certs/keys where specified!

AWS_IOT AWS_CLIENT;
WiFiMulti wifiMulti;

int POLLING_DELAY = 1000;

int ME = -1;
int HIM = -1;

// Configure the MAC Addresses here...  Jeremy's 3C:71:BF:84:B8:0C  Joe's spare 3C:71:BF:88:8D:6C
char* MAC[2] = {
  "3C:71:BF:88:92:78",
  "3C:71:BF:87:00:08"
};

char* BOT_NAME[2] = {
  "BotA",
  "BotB"
};

char HOST_ADDRESS[] = "ac0pct10qk7h9-ats.iot.us-west-2.amazonaws.com";
// char CLIENT_ID[] = "amebaClient";

char* UPDATE_TOPIC[2] = {
  "$aws/things/BotA/shadow/update",
  "$aws/things/BotB/shadow/update"
};

char* GET_TOPIC[2] = {
  "$aws/things/BotA/shadow/get",
  "$aws/things/BotB/shadow/get"
};

//char* SUBSCRIBE_TOPICS_TMPL[] = {
//  "$aws/things/%s/shadow/update/accepted",
//  "$aws/things/%s/shadow/update/rejected",
//  "$aws/things/%s/shadow/update/delta",
//  "$aws/things/%s/shadow/get/accepted",
//  "$aws/things/%s/shadow/get/rejected"
//};

char* SUBSCRIBE_TOPICS[4][4] =
{
  {
    "$aws/things/BotA/shadow/update/accepted",
    "$aws/things/BotA/shadow/update/rejected",
    "$aws/things/BotA/shadow/get/accepted",            // from merge
    "$aws/things/BotA/shadow/get/rejected"            // from merge
  },
  {
    "$aws/things/BotB/shadow/update/accepted",
    "$aws/things/BotB/shadow/update/rejected",
    "$aws/things/BotB/shadow/get/accepted",            // from merge
    "$aws/things/BotB/shadow/get/rejected"            // from merge
  }
};

int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];
char *payloadTopic;
const char desiredStateFmt[] = "{\"state\":{\"desired\":{\"kick\":%i,\"arm\":%i,\"alive\":%i}}}";
char getMsg[] = "{}";              // from merge

char kickMsgOn[] = "{\"state\":{\"desired\":{\"kick\": 10}}}";
char kickMsgOff[] = "{\"state\":{\"desired\":{\"kick\": 9}}}";
char armMsgOn[] = "{\"state\":{\"reported\":{\"arm\": 10}}}";
char armMsgOff[] = "{\"state\":{\"reported\":{\"arm\": 9}}}";
char armErrorOn[] = "{\"state\":{\"reported\":{\"error\": 10}}}";
char armErrorOff[] = "{\"state\":{\"reported\":{\"error\": 9}}}";
char kickReportOn[] = "{\"state\":{\"reported\":{\"kick\": 10}}}";
char kickReportOff[] = "{\"state\":{\"reported\":{\"kick\": 9}}}";
char ImAlive[] = "{\"state\":{\"reported\":{\"alive\": 10}}}";

const int led = 2;
const int REDLIGHT_PIN = 18;
const int GRLIGHT_PIN = 17;
const int MOTION_PIN = 16;
const int KICKBTN_PIN = 34;
const int ARMBTN_PIN = 35;
Servo servo_14;
                          // Establish Global Variables
int sendTry = 0;
int bootUpCheckInState = 9;
int myLastReportedArm = 9;
int myLastReportedArmError = 9;
int myLastDesiredKick = 9;
int myArmSwitch = 9;
int hisLastReportedArm = 9;
int hisLastReportedArmError = 9;
int hisLastDesiredKick = 9;
int myServoQueue = 9;
int myAliveTimer = 1;
int hisAliveTimer = 1;
int hesDeadError=9;
int hisLastAliveMessage=9;
int wifiTimer=1;

void callBackHandler(char *topicName, int payloadLen, char *payLoad) {
Serial.println("Method: callBackHandler");
  Serial.println("Callback received:");
  Serial.println(topicName);
  Serial.println(payLoad);
  payloadTopic = topicName;
  strncpy(rcvdPayload, payLoad, payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1;
}

void whoAmI() {
  String mac = WiFi.macAddress();
  String s = String(MAC[0]);

  Serial.print("MAC Address: ");
  Serial.println(mac);

  if (s.equals(mac)) {
    ME = 0;
    HIM = 1;
  } else {
    ME = 1;
    HIM = 0;
  }

  Serial.print("Hello, I am ");
  Serial.println(BOT_NAME[ME]);
}

boolean connectToWiFi() {
  Serial.println("Method: connectToWiFi");
    wifiMulti.addAP("NETGEAR37", "gentleraven032");
    wifiMulti.addAP("JKFERRY2", "f3rryl1nk");
    wifiMulti.addAP("bighoops", "123babe123");
    wifiMulti.addAP("Engenius", "tinker18");
        
    Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected to:");
        Serial.println(WiFi.SSID());
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
        digitalWrite(led, HIGH);
    }
}

void subscribeToTopics(int who) {
      Serial.print("who = ");
      Serial.println(who);
  for (int i = 0; i < 4; i++) {                  //for merge, changed i<2 to i<4
      Serial.print("count = ");
      Serial.println(i);
    Serial.print("Subscribing to topic: ");
    Serial.println(SUBSCRIBE_TOPICS[who][i]);

    if (0 == AWS_CLIENT.subscribe(SUBSCRIBE_TOPICS[who][i], callBackHandler)) {
      Serial.println("Subscribe Successfull");
    } else {
      Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
      while (1);
    }
  }
      // Blink 3 times to indicate subscriptions successful
      digitalWrite(led, LOW);
      delay(50);
      digitalWrite(led, HIGH);
      delay(50);
      digitalWrite(led, LOW);
      delay(50);
      digitalWrite(led, HIGH);
      delay(50); 
      digitalWrite(led, LOW);
      delay(50);
      digitalWrite(led, HIGH);
      delay(1000);
}

boolean connectToAWS() {
  if (0 == AWS_CLIENT.connect(HOST_ADDRESS, BOT_NAME[ME])) {
    Serial.println("Connected to AWS");
    
    // Blink 2 times to indicate Connection to AWS
      digitalWrite(led, LOW);
      delay(200);
      digitalWrite(led, HIGH);
      delay(200);
      digitalWrite(led, LOW);
      delay(200);
      digitalWrite(led, HIGH);
      delay(1000);

    subscribeToTopics(ME);
    subscribeToTopics(HIM);

  } else {
    Serial.println("AWS connection failed, Check the HOST Address");
    digitalWrite(led, LOW);
    while (1);
  }
}

void setup() {
Serial.println("Method: setup");
  pinMode(led, OUTPUT);
  pinMode(ARMBTN_PIN, INPUT);
  pinMode(KICKBTN_PIN, INPUT);
  pinMode(MOTION_PIN, INPUT);
  pinMode(REDLIGHT_PIN, OUTPUT);
  pinMode(GRLIGHT_PIN, OUTPUT);

  WiFi.disconnect(true);
  Serial.begin(115200);

  connectToWiFi();
  whoAmI();
  connectToAWS();

  servo_14.attach(14);
  servo_14.write(5);    //sets servo to 5 degrees
}


void handleMessageForMe(JsonObject& root) {
Serial.println("Method: handleMessageForMe");

  Serial.print("  My desired kick message: ");
  int kickMeMessage = root["state"]["desired"]["kick"].as<int>();
    Serial.println(kickMeMessage);
  if (kickMeMessage == 10) {
     myLastDesiredKick = 10;
  }
  if (kickMeMessage == 9) {
     myLastDesiredKick = 9;
  }
}

void handleMessageFromHim(JsonObject& root) {
Serial.println("Method: handleMessageFromHim");

  Serial.print("  His reported arm message: ");
  int reportedArm = root["state"]["reported"]["arm"].as<int>();
    Serial.println(reportedArm);
  if (reportedArm == 10) {
    hisLastReportedArm = 10;
    digitalWrite(GRLIGHT_PIN, HIGH);
  }
  if (reportedArm == 9) {
    hisLastReportedArm = 9;
    digitalWrite(GRLIGHT_PIN, LOW);
  }

  Serial.print("  His reported error message: ");
  int reportedError = root["state"]["reported"]["error"].as<int>();
    Serial.println(reportedError);
  if (reportedError == 10) {
     hisLastReportedArmError = 10;
  }
  if (reportedError == 9) {
     hisLastReportedArmError = 9;
  }

  Serial.print("  His desired kick message: ");
  int desiredKick = root["state"]["desired"]["kick"].as<int>();
    Serial.println(desiredKick);
    if (desiredKick == 10) {
    hisLastDesiredKick = 10;
     digitalWrite(REDLIGHT_PIN, HIGH);
  }
  if (desiredKick == 9) {
     hisLastDesiredKick = 9;
     digitalWrite(REDLIGHT_PIN, LOW);
  }

  Serial.print("  His StillAlive message: ");
  int hisLastAliveMessage = root["state"]["reported"]["alive"].as<int>();
    Serial.println(hisLastAliveMessage);
    if (hisAliveTimer>60) {
      hisAliveTimer=1;
      Serial.println("reseting hisAliveTimer to 1");
        if (hesDeadError==10) {
          hesDeadError=9;
        }
    }
}

void handleMessage(JsonObject& root) {
  Serial.println("Method: handleMessage for each Bot");
  Serial.print("Handling event for topic: ");
  String topic = String(payloadTopic);
  Serial.println(topic);

  if (topic.startsWith(SUBSCRIBE_TOPICS[ME][0])) {
    Serial.println("update to me accepted");
    handleMessageForMe(root);
  } else if (topic.startsWith(SUBSCRIBE_TOPICS[ME][1])) {
    Serial.println("update to me rejected");
      publishMessageError();
  } else if (topic.startsWith(SUBSCRIBE_TOPICS[HIM][0])) {
    handleMessageFromHim(root);
    Serial.println("update to him accepted");
  } else if (topic.startsWith(SUBSCRIBE_TOPICS[HIM][1])) {
    Serial.println("update to him rejected");
      publishMessageError();
  } else {
    Serial.println("unknown topic");
  }
}

void sendGet() {                               // from merge
  Serial.println("Sending get...");

  // publish the message
  if (AWS_CLIENT.publish(GET_TOPIC[HIM], getMsg) == 0) {
    Serial.print("Published Message:");
  } else {
    Serial.print("Publish failed:");
  }
  Serial.println(getMsg);
  delay(5000);
}

void publishMessageError() {
    Serial.println("Method: publishMessageError");
    // Blink 20 times to indicate publish error after 3 tries
    for (int count = 0; count < 20; count++) {
      digitalWrite(led, LOW);
      delay(50);
      digitalWrite(led, HIGH);
      delay(50);
    }
      delay(1000);
}

//--Message Sending-----------------------------------------------------------------------

void sendReportArmUp() {
  Serial.println("Method: sendReportArmUp");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armMsgOn) == 0) {
    Serial.print("Published Arm Up Message:");
    Serial.println(armMsgOn);
    sendTry = 3;
        delay(1000);
  } else {
    Serial.println("Arm Up Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError(); 
      } 
  }
 }
}

void sendReportArmDown() {
  Serial.println("Method: sendReportArmDown");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armMsgOff) == 0) {
    Serial.print("Published Arm Down Message:");
    Serial.println(armMsgOff);
    sendTry = 3;
        delay(1000);
  } else {
    Serial.println("Arm Down Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError();
      }
  }
 }
}

void sendReportArmErrorOn() {
  Serial.println("Method: sendReportArmErrorOn");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armErrorOn) == 0) {
    Serial.print("Published Arm Error On Message:");
    Serial.println(armErrorOn);
    sendTry = 3;
        delay(1000);
  } else {
    Serial.println("Arm Error On Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError();
   }
  }
 }
}

void sendReportArmErrorOff() {
  Serial.println("Method: sendReportArmErrorOff");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armErrorOff) == 0) {
    Serial.print("Published Arm Error Off Message:");
    Serial.println(armErrorOff);
    sendTry = 3;
        delay(1000);
  } else {
    Serial.println("Arm Error Off Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError();
   }
  }
 }
}

void sendKickHimOn() {
  Serial.println("Method: sendKickHimOn");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
   // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], kickMsgOn) == 0) {
    Serial.print("Published desiredKickHimOn Message:");
    Serial.println(kickMsgOn);
    sendTry = 3;
        delay(1000);
  } else {
    Serial.println("desiredKickHimOn Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError();
   }
  }
 }
}

void sendKickMeOff() {
  Serial.println("Method: sendKickMeOff");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
   // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], kickMsgOff) == 0) {
    Serial.print("Published desiredKickMeOff Message:");
    Serial.println(kickMsgOff);
        delay(1000);
    sendTry = 3;
  } else {
    Serial.println("desiredKickMeOff Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError();
   }
  }
 }
}

void sendImAlive() {
  Serial.println("Method: sendImAlive");
  for (int sendTry = 1; sendTry < 4; sendTry++) {
    Serial.print("Try number ");
    Serial.println(sendTry);
   // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], ImAlive) == 0) {
    Serial.print("Published ImAlive Message:");
    Serial.println(ImAlive);
        delay(1000);
    sendTry = 3;
  } else {
    Serial.println("ImAlive Publish failed - Trying again ******************************************");
    delay(1000);
      if (sendTry==3) {
      publishMessageError();
   }
  }
 }
}

//-------------------------------------------------------------------------

void bootUpCheckIn() {
  Serial.println("Method: bootUpCheckIn");
  
  Serial.println("  Part1: Arm Switch Report");
    if (analogRead(ARMBTN_PIN) > 4000) {
      myArmSwitch = 10;
      sendReportArmUp();
      myLastReportedArm = 10;
      }
    if (analogRead(ARMBTN_PIN) < 1000) {
      myArmSwitch = 9;
      sendReportArmDown();
      myLastReportedArm = 9;
      }
      
  Serial.println("  Part2: Publish other resets");
    sendReportArmErrorOff();
      myLastReportedArmError = 9;
    sendImAlive();
    sendGet();                          // from merge
  bootUpCheckInState = 10;  
}

void checkArmSwitch() {
  Serial.println("Method: checkArmSwitch");
  
    if (analogRead(ARMBTN_PIN) > 4000) {
      myArmSwitch = 10;
      if (myLastReportedArm == 9) {
        sendReportArmUp();
        myLastReportedArm =10;
      }
    }
    if (analogRead(ARMBTN_PIN) < 1000) {
      myArmSwitch = 9;
      if (myLastReportedArm == 10) {
        sendReportArmDown();
        myLastReportedArm =9;
          if (myLastReportedArmError == 10) {
            Serial.println("reporting my arm error off");
            delay(1000);
            sendReportArmErrorOff();
            myLastReportedArmError = 9;
              Serial.print("  myLastReportedArmError = ");
              Serial.println(myLastReportedArmError);
          }
      }
    }
}

void checkKickButton() {
    Serial.println("Method: checkKickButton");
      if (digitalRead(KICKBTN_PIN) == HIGH) {
        Serial.println("  Kick Button is being pushed");
        digitalWrite(REDLIGHT_PIN, HIGH);
        delay(1000);
        digitalWrite(REDLIGHT_PIN, LOW);
          if (hisLastDesiredKick == 9) {
            if (hisLastReportedArm == 10) {
              sendKickHimOn();
              hisLastDesiredKick = 10;
          }
        }
      }      
}

void processErrors() {
    Serial.println("Method: processErrors");
  // If He has an Arm Error
  if(hisLastReportedArmError == 10) {
    Serial.println("  Processing error blink for His Arm Error");
       // Blink Green light 5 times to indicate He has an Arm Error
          if (digitalRead(GRLIGHT_PIN) == LOW) {
    for (int count = 0; count < 5; count++) {
      digitalWrite(GRLIGHT_PIN, HIGH);
      delay(50);
      digitalWrite(GRLIGHT_PIN, LOW);
      delay(50);
    }
    }
          if (digitalRead(GRLIGHT_PIN) == HIGH) {
    for (int count = 0; count < 5; count++) {
      digitalWrite(GRLIGHT_PIN, LOW);
      delay(50);
      digitalWrite(GRLIGHT_PIN, HIGH);
      delay(50);
    }
   }
  }
   
  // If He has a DeadError
  if(hesDeadError == 10) {
    Serial.println("  Processing error blink for HesDeadError");
       // Blink Red light 5 times to indicate He has a DeadError
          if (digitalRead(REDLIGHT_PIN) == LOW) {
    for (int count = 0; count < 5; count++) {
      digitalWrite(REDLIGHT_PIN, HIGH);
      delay(50);
      digitalWrite(REDLIGHT_PIN, LOW);
      delay(50);
    }
    }
          if (digitalRead(REDLIGHT_PIN) == HIGH) {
    for (int count = 0; count < 5; count++) {
      digitalWrite(REDLIGHT_PIN, LOW);
      delay(50);
      digitalWrite(REDLIGHT_PIN, HIGH);
      delay(50);
    }
   }
  }
}

void processKickMeRequest() {
    Serial.println("Method: processKickMeRequest");
    if (myLastDesiredKick ==10) {
      if (myServoQueue == 9) {
        if (myArmSwitch == 10) {
          if (myLastReportedArmError == 9) {
            myServoQueue = 10;
          }
        }
      }
    }
}

void processServoQueue() {
  Serial.println("Method: processServoQueue");
  if (myServoQueue == 10) {
    if (digitalRead(MOTION_PIN) == HIGH) {
                Serial.println("Running Self Servo");
          servo_14.attach(14);
          servo_14.write(150);
           delay(500);
           servo_14.write(5);
           Serial.println("Self servo has run");
           myServoQueue=9;
           delay(1000);    // delay for bot to fall
             if (analogRead(ARMBTN_PIN) <1000) {
             Serial.println("No MyKick Errors");
        }
         if (analogRead(ARMBTN_PIN) > 4000) {
         sendReportArmErrorOn();
         myLastReportedArmError=10;
         delay(1500);    // delay between reporting
         Serial.println("There is a MyKick Error"); 
        }
     sendKickMeOff();
         delay(1000);    // delay after reporting
     myLastDesiredKick=9;
    }
  }
}

void aliveTimer() {
    Serial.println("Method: aliveTimer");
  myAliveTimer=myAliveTimer+1;
    Serial.print("myAliveTimer = ");
    Serial.println(myAliveTimer);
  hisAliveTimer=hisAliveTimer+1;
    Serial.print("hisAliveTimer = ");
    Serial.println(hisAliveTimer);
  if (myAliveTimer==120) {       //send ImAlive every 2 minutes
    sendImAlive();
    Serial.println("Resetting myAliveTimer to 1");
    myAliveTimer=1;
  }
  if (hisAliveTimer>180) {       //check if hes alive after 3 minutes
    if (hesDeadError==9) {
      hesDeadError=10;
    }
  }
}

void checkWiFi() {
  Serial.println("Method: checkWiFi");
  wifiTimer=wifiTimer+1;
  Serial.print("wifiTimer = ");
  Serial.println(wifiTimer);
    if (wifiTimer==60) {
    wifiTimer=1;   
      if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        digitalWrite(led, LOW);
        delay(5000);
        connectToWiFi();
        whoAmI();
        connectToAWS();
      } else {
    Serial.println("WiFi still connected!");
    }
  }
}


JsonObject& parseJSON(char *json) {

  StaticJsonBuffer<800> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {
    Serial.println("parseObject() failed");
  }
  return root;
  }

void loop() {
  Serial.println("Method: Main Loop");  
  Serial.println("  Part1: Bootup Check In");
    if (bootUpCheckInState == 10) {
    Serial.println("    Already Bootup Checked in");
  }
  if (bootUpCheckInState == 9) {
    bootUpCheckIn();
  }
   
  Serial.println("  Part2: Check for callback");
  if (msgReceived == 1) {
    msgReceived = 0;
    Serial.print("Received Message:");
    Serial.println(rcvdPayload);
    JsonObject& root = parseJSON(rcvdPayload);
    handleMessage(root);
  }
  
  Serial.println("  Part3: Check Arm Switch");
    checkArmSwitch();

  Serial.println("  Part4: Check Kick Button");
    checkKickButton();    
    
  Serial.println("  Part5: Process Errors");
    processErrors();
    
  Serial.println("  Part6: Process Kick Me Request");
    processKickMeRequest();

  Serial.println("  Part7: Servo Queue");
    processServoQueue();
    
  Serial.println("  Part8: Alive Timer");
    aliveTimer();

  Serial.println("  Part9: Check WiFi every minute");
    checkWiFi();

  delay(POLLING_DELAY); 
}
