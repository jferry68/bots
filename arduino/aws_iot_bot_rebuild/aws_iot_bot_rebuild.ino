#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>

// https://github.com/aws-samples/aws-iot-workshop
// Make sure you install the certs/keys where specified!

AWS_IOT AWS_CLIENT;

int NETWORK = 1;

// define possible wifi access points to connect to
const char* SSIDS[] = {
  "NETGEAR37",
  "JKFERRY2",
  "bighoops",
  "Engenius"
};

// corresponding passwords for the wifi access points
const char* PWDS[] = {
  "gentleraven032",
  "f3rryl1nk",
  "123babe123",
  "tinker18"
};


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
char CLIENT_ID[] = "amebaClient";

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

char* SUBSCRIBE_TOPICS[2][2] =
{
  {
    "$aws/things/BotA/shadow/update/accepted",
    "$aws/things/BotA/shadow/update/rejected"
  },
  {
    "$aws/things/BotB/shadow/update/accepted",
    "$aws/things/BotB/shadow/update/rejected"
  }
};

int status = WL_IDLE_STATUS;
int tick = 0, msgCount = 0, msgReceived = 0;
char payload[512];
char rcvdPayload[512];
char *payloadTopic;
const char desiredStateFmt[] = "{\"state\":{\"desired\":{\"kick\":%i,\"arm\":%i,\"servo\":%i}}}";

char kickMsgOn[] = "{\"state\":{\"desired\":{\"kick\": 10}}}";
char kickMsgOff[] = "{\"state\":{\"desired\":{\"kick\": 9}}}";
char armMsgOn[] = "{\"state\":{\"reported\":{\"arm\": 10}}}";
char armMsgOff[] = "{\"state\":{\"reported\":{\"arm\": 9}}}";
char armErrorOn[] = "{\"state\":{\"reported\":{\"error\": 10}}}";
char armErrorOff[] = "{\"state\":{\"reported\":{\"error\": 9}}}";
char kickReportOn[] = "{\"state\":{\"reported\":{\"kick\": 10}}}";
char kickReportOff[] = "{\"state\":{\"reported\":{\"kick\": 9}}}";

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

  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SSIDS[NETWORK]);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(SSIDS[NETWORK], PWDS[NETWORK]);
    // wait 5 seconds for connection:
    delay(5000);
  }

  Serial.println("Connected to wifi");
    digitalWrite(led, HIGH);
  return (status == WL_CONNECTED);
}

void subscribeToTopics(int who) {
  for (int i = 0; i < 2; i++) {
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

//void sendReportArmUp() {
//  Serial.println("Method: sendReportArmUp");
//  // publish the message
//  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armMsgOn) == 0) {
//    Serial.print("Published Arm Up Message:");
//  } else {
//    Serial.print("Arm Up Publish failed:");
//      publishMessageError();
//  }
//  Serial.println(armMsgOn);
//}

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
            Serial.println("now going to report my arm error off");
            delay(1000);
            sendReportArmErrorOff();
            myLastReportedArmError = 9;
              Serial.print("  myLastReportedArmError SHOULD be 9 and is now = ");
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
      delay(1000);
    }
          if (digitalRead(GRLIGHT_PIN) == HIGH) {
    for (int count = 0; count < 5; count++) {
      digitalWrite(GRLIGHT_PIN, LOW);
      delay(50);
      digitalWrite(GRLIGHT_PIN, HIGH);
      delay(50);
    }
      delay(1000);
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
     myLastDesiredKick=9;
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

  Serial.println("  Part6: Servo Queue");
    processServoQueue();

  delay(POLLING_DELAY); 
}
