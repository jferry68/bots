// updated 12/18/2018 11:54pm

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <Base64.h>
#include <Arduino.h>
#include <HTTPClient.h>

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

const char* url = "https://raw.githubusercontent.com/jferry68/bots/master/server/LYNGOH_Bposted.json";
//test string in LYNGOH_Bposted.json is like {"Barm":0,"Bkick":0,"Bservo":0,"Berror":0}

//Define pin numbers to be used
#define led 2          //onboard blue light
Servo servo_14;
int REDLIGHT_PIN = 18;
int GRLIGHT_PIN = 17;
int Motion_PIN = 16;
int KICKBTN_PIN = 34;
int ARMBTN_PIN = 35;

WiFiMulti wifiMulti;
HTTPClient http;

void setup() {
  Serial.begin(115200);
  pinMode(ARMBTN_PIN, INPUT);
  pinMode(KICKBTN_PIN, INPUT);
  pinMode(Motion_PIN, INPUT);
  pinMode(REDLIGHT_PIN, OUTPUT);
  pinMode(GRLIGHT_PIN, OUTPUT);
  pinMode(led, OUTPUT);
  servo_14.attach(14);
  servo_14.write(5);    //sets servo to 5 degrees

  setupWiFiAccessPoints();
  initWiFiConnection();
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
    digitalWrite(led, HIGH);
    Serial.println("");
    Serial.print("AP: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to wifi");
    digitalWrite(led, LOW);
  }
}

void loop() {

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

      handleEvents(root);
      updateBotState(root);
    } else {
      // TODO: handle unsuccessful GET
    }
  }
}

void handleEvents(JsonObject root) {
  Serial.println("Handling events...");

  if (root["Barm"] == 1) {
    digitalWrite(GRLIGHT_PIN, HIGH);
    Serial.println("B arm is up");
    delay(1000);
  } else {
    digitalWrite(GRLIGHT_PIN, LOW);
  }

    if (analogRead(KICKBTN_PIN) < 4000 && root["Barm"] == 0) {
    if (root["Berror"] == 0 && root["Bservo"] == 0) {
      root["Akick"] = 0;
      digitalWrite(REDLIGHT_PIN, LOW);
      digitalWrite(GRLIGHT_PIN, LOW);
      Serial.println("B reported back B kicked and B arm is down with no error");
      delay(1000);
    }
  }
  
  if (analogRead(KICKBTN_PIN) > 4000 && root["Barm"] == 1) {
    if (root["Berror"] == 0) {
      root["Akick"] = 1;
      Serial.println("Self button has been pushed");
      delay(1000);
    }
  }

  if (analogRead(ARMBTN_PIN) > 4000) {
    root["Aarm"] = 1;
    Serial.println("Self arm is up");
    delay(1000);
    }
      else {
    root["Aarm"] = 0;
    Serial.println("Self arm is down");
  }
  
  if (analogRead(ARMBTN_PIN) < 4000 && root["Aservo"] == 0)  {
    root["Aerror"] = 0;
    digitalWrite(REDLIGHT_PIN, LOW);
    Serial.println("no self error or error is cleared"); 
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
    root["Aservo"] = 1;
    Serial.println("B sent message for A to queue a kick");

  }
  if (root["Aservo"] == 1 && digitalRead(Motion_PIN) == HIGH) {   //run servo if both true
    runServo(root);
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

  void runServo(JsonObject root) {
  if (analogRead(ARMBTN_PIN) > 4000) {
    servo_14.attach(14);
    servo_14.write(150);
    delay(500);
    servo_14.write(5);
    Serial.println("Self servo has run");
    root["Aservo"] = 0;
    delay(3000);
  }  
    if (analogRead(ARMBTN_PIN) > 4000) {
      root["Aerror"] = 1;
      Serial.println("Self servo has failed to kick or self robot didnt fall ");
    } else {
      root["Aarm"] = 0;
      Serial.println("Self arm is down and robot has fallen");
   }
 }

  /**
   Send an update to the bot state on the server
*/
void updateBotState(JsonObject root) {
  Serial.println("Updating bot state...");
  //make sure to update LYNGOH_Aposted.json
  
  
  //int Bkickval = root["Bkick"];
  //Serial.println(Bkickval);         //testing the extraction of one of the root arrays. should give a 5
  //delay(5000);

  //update values in array, eventually done through logic tests
  //root["Aarm"] = 7;
  //root["Akick"] = 6;
  //root["Aservo"] = 5;
  //root["Aerror"] = 4;
  //root["Barm"] = 3;
  //root["Bkick"] = 2;
  //root["Bservo"] = 1;
  //root["Berror"] = 0;

  //char toEncode[85];
  //root.printTo(toEncode);
  //Serial.println(toEncode);
  //delay(5000);

  //int inputLen = sizeof(toEncode);
  //int encodedLen = base64_enc_len(inputLen);
  // char encoded[encodedLen];

  //base64_encode(encoded, toEncode, inputLen);

  //Serial.print(toEncode); Serial.print(" = ");
  // Serial.println(encoded);
  //delay(5000);
}
