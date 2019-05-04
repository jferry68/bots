#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>

// https://github.com/aws-samples/aws-iot-workshop
// Make sure you install the certs/keys where specified!

AWS_IOT AWS_CLIENT;

NETWORK = 0;

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
char kickMsg[] = "{\"state\":{\"desired\":{\"kick\": 1}}}";

const int led = 2;
Servo servo_14;
const int REDLIGHT_PIN = 18;
const int GRLIGHT_PIN = 17;
const int MOTION_PIN = 16;
const int KICKBTN_PIN = 34;
const int ARMBTN_PIN = 35;

// state of a kick I sent Him
int kickHimState = 0;
// current state of the button
int kickButtonState = 0;
// previous state of the button
int lastKickButtonState = 0;

void callBackHandler(char *topicName, int payloadLen, char *payLoad) {
  Serial.println("Callback received:");
  Serial.println(topicName);
  Serial.println(payLoad);
  payloadTopic = topicName;
  strncpy(rcvdPayload, payLoad, payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1;
}

void whoAmI () {
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
}

boolean connectToAWS() {

  if (0 == AWS_CLIENT.connect(HOST_ADDRESS, CLIENT_ID)) {
    Serial.println("Connected to AWS");
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

  // initialize the LED as an output:
  pinMode(led, OUTPUT);

  pinMode(ARMBTN_PIN, INPUT);
  pinMode(KICKBTN_PIN, INPUT);
  //  pinMode(MOTION_PIN, INPUT);
  pinMode(REDLIGHT_PIN, OUTPUT);
  pinMode(GRLIGHT_PIN, OUTPUT);

  WiFi.disconnect(true);
  Serial.begin(115200);

  connectToWiFi();
  whoAmI();
  connectToAWS();

  delay(2000);
}


void handleMessageForMe(JsonObject& root) {

  Serial.print("desired kick:");
  int desiredKick = root["state"]["desired"]["kick"];
  Serial.println(desiredKick);
  if (desiredKick == 1) {
    digitalWrite(GRLIGHT_PIN, HIGH);
   } else {
    digitalWrite(GRLIGHT_PIN, LOW);
  }

  Serial.print("desired arm:");
  const char* desiredArm = root["state"]["desired"]["arm"];
  Serial.println(desiredArm);

  Serial.print("desired servo:");
  const char* desiredServo = root["state"]["desired"]["servo"];
  Serial.println(desiredServo);

  Serial.print("desired error:");
  const char* desiredError = root["state"]["desired"]["error"];
  Serial.println(desiredError);

  Serial.print("reported kick:");
  const char* reportedKick = root["state"]["reported"]["kick"];
  Serial.println(reportedKick);

  Serial.print("reported arm:");
  const char* reportedArm = root["state"]["reported"]["arm"];
  Serial.println(reportedArm);

  Serial.print("reported servo:");
  const char* reportedServo = root["state"]["reported"]["servo"];
  Serial.println(reportedServo);

  Serial.print("reported error:");
  const char* reportedError = root["state"]["reported"]["error"];
  Serial.println(reportedError);

}

void handleMessageForHim(JsonObject& root) {

  Serial.print("desired kick:");
  int desiredKick = root["state"]["desired"]["kick"];
  Serial.println(desiredKick);
    if (desiredKick == 1) {
    digitalWrite(GRLIGHT_PIN, HIGH);
   } else {
    digitalWrite(GRLIGHT_PIN, LOW);
  }

  Serial.print("desired arm:");
  const char* desiredArm = root["state"]["desired"]["arm"];
  Serial.println(desiredArm);

  Serial.print("desired servo:");
  const char* desiredServo = root["state"]["desired"]["servo"];
  Serial.println(desiredServo);

  Serial.print("desired error:");
  const char* desiredError = root["state"]["desired"]["error"];
  Serial.println(desiredError);

  Serial.print("reported kick:");
  const char* reportedKick = root["state"]["reported"]["kick"];
  Serial.println(reportedKick);

  Serial.print("reported arm:");
  const char* reportedArm = root["state"]["reported"]["arm"];
  Serial.println(reportedArm);

  Serial.print("reported servo:");
  const char* reportedServo = root["state"]["reported"]["servo"];
  Serial.println(reportedServo);

  Serial.print("reported error:");
  const char* reportedError = root["state"]["reported"]["error"];
  Serial.println(reportedError);

}

void handleMessage(JsonObject& root) {
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

void sendKick() {
  Serial.println("Sending kick...");

  // publish the message
  if (AWS_CLIENT.publish(UPDATE_TOPIC[HIM], kickMsg) == 0) {
    digitalWrite(REDLIGHT_PIN, HIGH);
    kickHimState = 2;
    Serial.print("Published Message:");
  } else {
    Serial.print("Publish failed:");
  }
  Serial.println(kickMsg);
}

void sendKickReceived() {
  Serial.println("Sending kick received...");
}

void sendReset() {
  Serial.println("Sending reset...");
}

void sendStateUpdates() {

  // if my button has been pushed, send the kick request
  if (kickHimState == 1) {
    sendKick();
  }

}

JsonObject& parseJSON(char *json) {

  //  Serial.println("Parsing JSON message:");
  //  Serial.println(json);
  //  delay(2000);

  StaticJsonBuffer<800> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {
    Serial.println("parseObject() failed");
  }

  return root;
}


void checkKickButtonState() {

  // read the pushbutton input pin:
  kickButtonState = digitalRead(KICKBTN_PIN);

  // compare the kickButtonState to its previous state
  if (kickButtonState != lastKickButtonState) {

    // if the state has changed, increment the counter
    if (kickButtonState == HIGH) {

      Serial.println("kick button on");
      //digitalWrite(REDLIGHT_PIN, HIGH);

      // only request kick if he's ready
      if (kickHimState == 0) {
        kickHimState = 1;
      }

    } else {
      // if the current state is LOW then the button went from on to off:
      Serial.println("kick button off");
      //digitalWrite(REDLIGHT_PIN, LOW);
    }

    // Delay a little bit to avoid bouncing
    //    delay(50);
  }

  // save the current state as the last state, for next time through the loop
  lastKickButtonState = kickButtonState;
}

void registerStateChanges() {

  // check my kick button
  checkKickButtonState();

}

void loop() {  //looking for button presses

  // see if anything has changed with me
  registerStateChanges();

  // send any necessary updates to my shadow
  sendStateUpdates();

  // see if we got a callback
    Serial.print("Polling. mesgreceived= ");
    Serial.println(msgReceived);
  if (msgReceived == 1) {
    msgReceived = 0;
    Serial.println("Received Message:");
    Serial.println(rcvdPayload);
    JsonObject& root = parseJSON(rcvdPayload);
    handleMessage(root);
  }

  delay(POLLING_DELAY);
}
