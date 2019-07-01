//me crashes after he kicks

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
char errorMsgOn[] = "{\"state\":{\"reported\":{\"error\": 10}}}";
char errorMsgOff[] = "{\"state\":{\"reported\":{\"error\": 9}}}";
char kickReportOn[] = "{\"state\":{\"reported\":{\"kick\": 10}}}";
char kickReportOff[] = "{\"state\":{\"reported\":{\"kick\": 9}}}";

const int led = 2;
const int REDLIGHT_PIN = 18;
const int GRLIGHT_PIN = 17;
const int MOTION_PIN = 16;
const int KICKBTN_PIN = 34;
const int ARMBTN_PIN = 35;
Servo servo_14;

int bootUpCheckedIn = 0;
int myKickError = 9;
int kickHimState = 0;
int kickButtonState = 0;
int lastKickButtonState = 0;
int desiredKickMe = 9;
int desiredKickHim = 9;
int myRedLight = 0;
int lastReportedArm = 9;
int armButtonState = 0;
int hisReportedArm = 9;
int lastReportedKickMe = 9;
int hisReportedkick=0;

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
  int desiredKick = root["state"]["desired"]["kick"].as<int>();
     if (desiredKick == 10) {
     desiredKickMe = 10;
     }
     if (desiredKick == 9) {
     desiredKickMe = 9;
     }
  
//  Serial.print("my desired arm:");                              //will never be needed
//  const char* desiredArm = root["state"]["desired"]["arm"];
//  Serial.println(desiredArm);

//  Serial.print("my desired servo:");                              //will never be needed
//  const char* desiredServo = root["state"]["desired"]["servo"];
//  Serial.println(desiredServo);

  Serial.print("my desired error:");
  const char* desiredError = root["state"]["desired"]["error"];
  Serial.println(desiredError);

  Serial.print("my reported kick:");
  const char* reportedKick = root["state"]["reported"]["kick"];
  Serial.println(reportedKick);

  Serial.print("my reported arm:");
  int reportedArm = root["state"]["reported"]["arm"];
  Serial.println(reportedArm);
  
  Serial.print("my reported servo:");
  const char* reportedServo = root["state"]["reported"]["servo"];
  Serial.println(reportedServo);

  Serial.print("my reported error:");
  const char* reportedError = root["state"]["reported"]["error"];
  Serial.println(reportedError);
}

void handleMessageForHim(JsonObject& root) {
Serial.println("Method: handleMessageForHim");

  Serial.print("his reported kick:");
  int hisReportedKick = root["state"]["reported"]["kick"].as<int>();
  Serial.println(hisReportedKick);
  if (hisReportedKick ==10) {
    sendKickOffHim();
  }

  Serial.println("his desired kick Message:");
    delay(500);                                                                       //>>>>>>>>>>new
  int desiredKick = root["state"]["desired"]["kick"].as<int>();
  Serial.println(desiredKick);
  if (desiredKick == 10) {
    desiredKickHim = 10;
  }
  if (desiredKick == 9) {
    desiredKickHim = 9;
  } 

 //  Serial.print("his desired arm:");                              //will never be needed
 //  const char* desiredArm = root["state"]["desired"]["arm"];
 //  Serial.println(desiredArm);

 // Serial.print("his desired servo:");                              //will never be needed
 // const char* desiredServo = root["state"]["desired"]["servo"];
 //  Serial.println(desiredServo);

  Serial.print("his desired error:");
  const char* desiredError = root["state"]["desired"]["error"];
  Serial.println(desiredError);

  Serial.print("reported arm from Root:");
  int reportedArm = root["state"]["reported"]["arm"].as<int>();
    Serial.println(reportedArm);
  if (reportedArm == 10) {
    hisReportedArm = 10;
  }
  if (reportedArm == 9) {
    hisReportedArm = 9;
  } 
  Serial.print("hisreportedarm assigned from Root:");
  Serial.println(hisReportedArm);
  

  Serial.print("his reported servo:");
  const char* reportedServo = root["state"]["reported"]["servo"];
  Serial.println(reportedServo);

  Serial.print("his reported error:");
  const char* reportedError = root["state"]["reported"]["error"];
  Serial.println(reportedError);
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
  } else if (topic.startsWith(SUBSCRIBE_TOPICS[HIM][0])) {
    handleMessageForHim(root);
    Serial.println("update to him accepted");
  } else if (topic.startsWith(SUBSCRIBE_TOPICS[HIM][1])) {
    Serial.println("update to him rejected");
  } else {
    Serial.println("unknown topic");
  }
}

void runServo() {
Serial.println("Method: runServo");
     if (analogRead(ARMBTN_PIN) > 4000) {
        if (digitalRead(MOTION_PIN) == HIGH) {
          Serial.println("Running Self Servo");
          servo_14.attach(14);
          servo_14.write(150);
           delay(500);
           servo_14.write(5);
           Serial.println("Self servo has run");
           desiredKickMe=9;
           delay(1000);
             if (analogRead(ARMBTN_PIN) <1000) {
             myKickError=9;
             Serial.println("No MyKick Errors");
        }
         if (analogRead(ARMBTN_PIN) > 4000) {
         myKickError=10;
         Serial.println("There is a MyKick Error");    //make error report method  
        }
        sendMyReportedKickOn();
      }
    }
  }
  
void publishFailureBlink() {
  Serial.println("Method: publishFailureBlink");
    for (int count = 0; count < 10; count++) {
      digitalWrite(led, LOW);
      delay(10);
      digitalWrite(led, HIGH);
      delay(10);
    }
}

void sendKickOnHim() {
  Serial.println("Method: sendKickOnHim");
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], kickMsgOn) == 0) {
    kickHimState = 2;
    Serial.print("Published Kick Message:");
      digitalWrite(REDLIGHT_PIN, LOW);
      delay(200);
      digitalWrite(REDLIGHT_PIN, HIGH);
      delay(200);
      digitalWrite(REDLIGHT_PIN, LOW);
      delay(200);
      digitalWrite(REDLIGHT_PIN, HIGH);
      delay(200);
  } else {
    Serial.print("Kick Publish failed:");
    publishFailureBlink();
  }
  Serial.println(kickMsgOn);
}

void sendKickOffHim() {
    Serial.println("Method: sendKickOffHim");
    kickHimState = 0;
    desiredKickHim = 9;
    digitalWrite(REDLIGHT_PIN, LOW);
  // publish the message
       delay(3000);                                //>>>>>>>>>>>>>>>>> new
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], kickMsgOff) == 0) {
    Serial.print("Published Kick Message:");       // crashes me here
  } else {
    Serial.print("Kick Publish failed:");
    publishFailureBlink();
  }
  Serial.println(kickMsgOff);
}

void sendReportArmUp() {
  Serial.println("Method: sendReportArmUp");
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armMsgOn) == 0) {
    lastReportedArm = 10;
    Serial.print("Published Arm Up Message:");
  } else {
    Serial.print("Arm Up Publish failed:");
    publishFailureBlink();
  }
  Serial.println(armMsgOn);
}

void sendReportArmDown() {
  Serial.println("Method: sendReportArmDown");
  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], armMsgOff) == 0) {
    lastReportedArm = 9;
    Serial.print("Published Arm Down Message:");
  } else {
    Serial.print("Arm Down Publish failed:");
    publishFailureBlink();
  }
  Serial.println(armMsgOff);
}

void sendMyReportedKickOn() {
  Serial.println("Method: sendMyReportedKickOn");
   // publish the message
   delay(3000);                                //>>>>>>>>>>>>>>>>> new
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], kickReportOn) == 0) {
    Serial.print("Published MyReportedKickOn Message:");
  } else {
    Serial.print("MyReportedKickOn Publish failed:");
    publishFailureBlink();
  }
  Serial.println(kickReportOn);
  lastReportedKickMe = 10;
}

void sendMyReportedKickOff() {
  Serial.println("Method: sendKickReportedMeOff");
   // publish the message
     delay(3000);                                //>>>>>>>>>>>>>>>>> new
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], kickReportOff) == 0) {
    Serial.print("Published KickReportedMeOff Message:");
  } else {
    Serial.print("KickReportedMeOff Publish failed:");
    publishFailureBlink();
  }
  Serial.println(kickReportOff);
  lastReportedKickMe = 9;
}

void bootUpCheckIn() {
    Serial.println("Method: bootUpCheckIn");
// publish errorMegOff HIM
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], errorMsgOff) == 0) {
    Serial.print("Published Bootup Erroroff HIM Message:");
    Serial.println(errorMsgOff);
  } else {
    Serial.print("Bootup Erroroff HIM Message failed:");
    publishFailureBlink();
  }
// publish errorMegOff ME
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], errorMsgOff) == 0) {
    Serial.print("Published Bootup Erroroff ME Message:");
    Serial.println(errorMsgOff);
  } else {
    Serial.print("Bootup Erroroff ME Message failed:");
    publishFailureBlink();
  }
// publish kickMsgOff HIM
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], kickMsgOff) == 0) {
    Serial.print("Published kickMsgOff HIM:");
    Serial.println(kickMsgOff);
  } else {
    Serial.print("Bootup kickMsgOff HIM message failed:");
    publishFailureBlink();
  }
// publish armMsgOff HIM
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], armMsgOff) == 0) {
    Serial.print("Published armMsgOff HIM:");
    Serial.println(armMsgOff);
  } else {
    Serial.print("Bootup armMsgOff HIM message failed:");
    publishFailureBlink();
  }
     
// publish kickReportOff ME
  if (AWS_CLIENT.publish(UPDATE_TOPIC[ME], kickReportOff) == 0) {
    Serial.print("Published reportKickOff ME:");
    Serial.println(kickReportOff);
  } else {
    Serial.print("Bootup kickReportOff ME message failed:");
    publishFailureBlink();
  }
  
  bootUpCheckedIn = 1;
  delay(3000);                                //>>>>>>>>>>>>>>>>> new
}

void sendStateUpdates() {
Serial.println("Method: SendStateUpdates");
  if (bootUpCheckedIn == 0) {
    bootUpCheckIn();
  }
  // if my kick button has been pushed, send the kick request
  if (kickHimState == 1) {
    sendKickOnHim();
  }
    if (desiredKickMe == 10) {
      runServo();
  }
  if (desiredKickHim == 10) {
     digitalWrite(REDLIGHT_PIN, HIGH);
  }
  if (desiredKickHim == 9) {
    digitalWrite(REDLIGHT_PIN, LOW);
  }
  Serial.print("His Reported Arm in main loop = ");
  Serial.println(hisReportedArm);
  if (hisReportedArm ==10) {
     digitalWrite(GRLIGHT_PIN, HIGH);
  }
  if (hisReportedArm ==9) {
     digitalWrite(GRLIGHT_PIN, LOW);
  }
  if (lastReportedKickMe ==10) {
    if (desiredKickMe ==9) {
      sendMyReportedKickOff();
    }
  }
}

JsonObject& parseJSON(char *json) {

  StaticJsonBuffer<800> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {
    Serial.println("parseObject() failed");
    publishFailureBlink();
  }
  return root;
  }

void checkMyStates() {
  Serial.println("Method: checkMyStates");
    Serial.println("Part1 checkKickButtonState");
  kickButtonState = digitalRead(KICKBTN_PIN);       // read the pushbutton input pin:

  if (kickButtonState != lastKickButtonState) {     // compare the kickButtonState to its previous state

    if (kickButtonState == HIGH) {                  // if the state has changed, increment the counter
      Serial.println("kick button on");
      digitalWrite(REDLIGHT_PIN, HIGH);
      delay(2000);                                  // wait for finger to unpush
      // only request kick if he's ready
      if (kickHimState == 0) {
        kickHimState = 1;
    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("kick button off");
      delay(2000);                                  // wait for finger to unpush
        if (kickHimState == 2) {
        kickHimState = 0;
        sendKickOffHim();
      }
    }
  }
  // save the current state as the last state, for next time through the loop
  lastKickButtonState = kickButtonState;
    Serial.print("Last Kick Button State = ");
    Serial.println(lastKickButtonState);
    Serial.print("Kick Him State = ");
    Serial.println(kickHimState);
}
Serial.println("Part2 checkArmButton");
  armButtonState = analogRead(ARMBTN_PIN);
  Serial.print("analog read of arm button = ");
  Serial.println(armButtonState);
  Serial.print("last reported arm = ");
  Serial.println(lastReportedArm);
  
    if (armButtonState > 4000) {
      if (lastReportedArm == 9) {
      sendReportArmUp();
      }
    }
    if (armButtonState < 1000) {
      if (lastReportedArm == 10) {
      sendReportArmDown();
      }
    }
}

void loop() {
    Serial.println("Method: Main Loop");
    
Serial.println("Part1  Check for callback");
  if (msgReceived == 1) {
    msgReceived = 0;
    Serial.print("Received Message:");
    Serial.println(rcvdPayload);
    JsonObject& root = parseJSON(rcvdPayload);
    handleMessage(root);
  }
Serial.println("Part2  Check My States");  
  checkMyStates();

Serial.println("Part3  Send State Updates");   
  sendStateUpdates();

  delay(1000);
}
