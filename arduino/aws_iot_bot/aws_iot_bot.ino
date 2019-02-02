#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>

// https://github.com/aws-samples/aws-iot-workshop
// Make sure you install the certs/keys where specified!

AWS_IOT AWS_CLIENT;

int POLLING_DELAY = 5000;

int ME = -1;
int HIM = -1;
char* MAC[2] = {
  "3C:71:BF:84:B8:0C",
  "mac:address:b"
};
char* BOT_NAME[2] = {
  "BotA",
  "BotB"
};

char WIFI_SSID[] = "NETGEAR37";
char WIFI_PASSWORD[] = "gentleraven032";
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

const int led = 2;
Servo servo_14;
const int REDLIGHT_PIN = 18;
const int GRLIGHT_PIN = 17;
const int Motion_PIN = 16;
const int KICKBTN_PIN = 34;
const int ARMBTN_PIN = 35;

void registerMessage(char *topicName, int payloadLen, char *payLoad) {
  Serial.println("Callback received:");
  Serial.println(topicName);
  Serial.println(payLoad);
  payloadTopic = topicName;
  strncpy(rcvdPayload, payLoad, payloadLen);
  rcvdPayload[payloadLen] = 0;
  msgReceived = 1;
}

void callBackHandler (char *topicName, int payloadLen, char *payLoad) {
  Serial.println("Callback received:");
  registerMessage(topicName, payloadLen, payLoad);
}

void whoAmI () {
  String mac = WiFi.macAddress();
  String s = String(MAC[0]);

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
    Serial.println(WIFI_SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    // wait 5 seconds for connection:
    delay(5000);
  }

  Serial.println("Connected to wifi");

  return (status == WL_CONNECTED);
}

boolean connectToAWS() {

  if (0 == AWS_CLIENT.connect(HOST_ADDRESS, CLIENT_ID)) {
    Serial.println("Connected to AWS");
    delay(1000);

//    char topic[50];

    for (int i = 0; i < 2; i++) {

//      sprintf(topic, SUBSCRIBE_TOPICS_TMPL[i], BOT_NAME[ME]);
//      Serial.print("Subscribing to tmplt: ");
//      Serial.println(topic);

      Serial.print("Subscribing to topic: ");
      Serial.println(SUBSCRIBE_TOPICS[ME][i]);

      if (0 == AWS_CLIENT.subscribe(SUBSCRIBE_TOPICS[ME][i], callBackHandler)) {
        Serial.println("Subscribe Successfull");
      } else {
        Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
        while (1);
      }

    }

  } else {
    Serial.println("AWS connection failed, Check the HOST Address");
    while (1);
  }

}

void setup() {

  WiFi.disconnect(true);
  Serial.begin(115200);

  connectToWiFi();
  whoAmI();
  connectToAWS();

  delay(2000);
}

void handleMessage(JsonObject& root) {
  Serial.print("Handling event for topic: ");
  String topic = String(payloadTopic);
  Serial.println(topic);

  if(topic.startsWith(SUBSCRIBE_TOPICS[ME][1]) == 0){
    Serial.println("update accepted");
  } else if(topic.startsWith(SUBSCRIBE_TOPICS[ME][2]) == 0){
    Serial.println("update rejected");
  } else {
    Serial.println("unknown topic");
  }

  Serial.print("desired kick:");
  const char* desiredKick = root["state"]["desired"]["kick"];
  Serial.println(desiredKick);

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

JsonObject& parseJSON(char *json) {

  Serial.println("Parsing JSON message:");
  Serial.println(json);
  delay(2000);

  StaticJsonBuffer<800> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {
    Serial.println("parseObject() failed");
  }

  return root;
}

void loop() {

  // see if we got a callback
  if (msgReceived == 1) {
    msgReceived = 0;
    Serial.println("Received Message:");
    Serial.println(rcvdPayload);
    JsonObject& root = parseJSON(rcvdPayload);
    handleMessage(root);
  } else {
    Serial.println("Waiting for some action...");
  }

  delay(POLLING_DELAY);
}
