#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <Arduino.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <PubSubClient.h>

const char* BROKER_USER = "ferr0084@gmail.com";
const char* BROKER_PASS = "a966e133";
const char* BROKER = "mqtt.dioty.co";

const char* ssid     = "NETGEAR37";
const char* password = "gentleraven032";


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

const char* url = "https://raw.githubusercontent.com/jferry68/bots/master/server/LYNGOH.json";
//test string in LYNGOH.json is {"Aarm":0,"Akick":1,"Aservo":2,"Aerror":3,"Barm":4,"Bkick":5,"Bservo":6,"Berror":7}
//const char* data = {"Aarm":0,"Akick":0,"Aservo":0,"Aerror":0,"Barm":1,"Bkick":0,"Bservo":0,"Berror":0}

int LED_BUILTIN = 2;

//Define pin numbers to be used
int WIFILIGHT_PIN = 13;
int REDLIGHT_PIN = 18;
int GRLIGHT_PIN = 17;
int Motion_PIN = 16;
int KICKBTN_PIN = 21;
int ARMBTN_PIN = 26;
Servo servo_14;

WiFiMulti wifiMulti;
WiFiClient wifiClient;
HTTPClient http;
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Connecting to ");
    Serial.println(BROKER);
    // Attempt to connect
    if (client.connect("arduinoClient", BROKER_USER, BROKER_PASS)) {
      Serial.print("Connected to ");
      Serial.println(BROKER);
      // Once connected, publish an announcement...
      client.publish("outTopic","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {

  servo_14.attach(14);
  servo_14.write(0);

  //Define pin numbers as input or output
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(WIFILIGHT_PIN, OUTPUT);
  pinMode(REDLIGHT_PIN, OUTPUT);
  pinMode(GRLIGHT_PIN, OUTPUT);
  pinMode(Motion_PIN, INPUT);
  pinMode(KICKBTN_PIN, INPUT);
  pinMode(ARMBTN_PIN, INPUT);
  Serial.begin(115200);

  client.setServer(BROKER, 1883);
  client.setCallback(callback);

 // setupWiFiAccessPoints();
 // initWiFiConnection();
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  http.setReuse(true);
}

/**
   add all the configured access points
*/
void setupWiFiAccessPoints() {
  int i;
  for (i = 0; i < 4; i++) {
    Serial.print("Adding AP: ");
    Serial.println(SSIDS[i]);
    wifiMulti.addAP(SSIDS[i], PWDS[i]);
  }
}

/**
   Attempt to connect to one of the configured access points
*/
void initWiFiConnection() {
  Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("");
    Serial.print("AP: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to wifi");
    digitalWrite(LED_BUILTIN, LOW);
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

}

void oldLoop(){
    //Check the current connection status
  if (wifiMulti.run() == WL_CONNECTED) {

    http.begin(url);

    //Make the request
    int httpCode = http.GET();

    //Check for the returning code
    if (httpCode == 200) {

      String input = http.getString();
      // Create a char array big enough including the terminating NULL
      int bufLength = input.length() + 1;
      char json[bufLength];
      input.toCharArray(json, bufLength);

      Serial.println(json);
      delay(2000);

      StaticJsonDocument<200> doc;
      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, json);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }

      // Get the root object in the document
      JsonObject root = doc.as<JsonObject>();

      http.end();

      handleEvents(root, doc);
      updateBotState(root);
    } else {
      // TODO: handle unsuccessful GET
    }
  }
}

void printDoc(StaticJsonDocument<200> doc){
    serializeJson(doc, Serial);
    Serial.println("");
}

/**
   Takes in a JsonObject root and calls
*/
void handleEvents(JsonObject root, StaticJsonDocument<200> doc) {
  printDoc(doc);
  serializeJson(doc, Serial);
  if (root["Barm"] == 1) {
    digitalWrite(GRLIGHT_PIN, HIGH);
    Serial.println("B arm is up");
    delay(1000);

  } else {
    digitalWrite(GRLIGHT_PIN, LOW);

  }
  if (digitalRead(KICKBTN_PIN) == 1 && root["Barm"] == 1) {
    if (root["Berror"] == 0) {
      printDoc(doc);
      root["Akick"] = 1;
      printDoc(doc);
      Serial.println("Button has been pushed");
      delay(1000);

    }

  }
  if (digitalRead(ARMBTN_PIN) == 1) {
    printDoc(doc);
    root["Aarm"] = 1;
    printDoc(doc);
    Serial.println("Self arm is up");
    delay(1000);

  }
  if (root["Akick"] == 1 && root["Bservo"] == 1) {
    digitalWrite(REDLIGHT_PIN, HIGH);
    Serial.println("B responded to kick command and queued to kick");
    delay(1000);

  }
  if (root["Akick"] == 0 && root["Bservo"] == 0) {
    digitalWrite(REDLIGHT_PIN, LOW);
    Serial.println("B kick not queued or it completed successfully");

  }
  if (root["Bkick"] == 1) {
    printDoc(doc);
    root["Aservo"] = 1;
    printDoc(doc);
    Serial.println("B sent message for A to queue a kick");

  }
  if (root["Aservo"] == 1 && digitalRead(Motion_PIN) == HIGH) {
    printDoc(doc);
    root["Aservo"] = 0;
    printDoc(doc);
    servo_14.write(180);
    delay(500);
    servo_14.write(0);
    delay(2000);
    Serial.println("Self servo has run");
    if (digitalRead(ARMBTN_PIN) == 1) {
      printDoc(doc);
      root["Aerror"] = 1;
      printDoc(doc);
      Serial.println("Self servo has failed to kick or self robot didnt fall ");

    } else {
      printDoc(doc);
      root["Aarm"] = 0;
      printDoc(doc);
      Serial.println("Self robot has fallen");

    }

  }
  if (root["Berror"] == 1) {
    Serial.println("B servo has failed to kick or B robot didnt fall ");
    for (int count = 0; count < 5; count++) {
      digitalWrite(REDLIGHT_PIN, LOW);
      delay(250);
      digitalWrite(REDLIGHT_PIN, HIGH);
      delay(250);
    }

  }
  if (root["Aerror"] == 1) {
    Serial.println("Indicating self kick and robot error");
    for (int count = 0; count < 5; count++) {
      digitalWrite(GRLIGHT_PIN, LOW);
      delay(250);
      digitalWrite(GRLIGHT_PIN, HIGH);
      delay(250);
    }
  }
}

/**
   Send an update to the bot state on the server
*/
void updateBotState(JsonObject root) {
  Serial.println("Updating bot state...");
}
