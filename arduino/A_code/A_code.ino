#include <WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include <Base64.h>
#include <Arduino.h>
#include <HTTPClient.h>

//possible wifi connections
//"JKFERRY2" "f3rryl1nk"
//"bighoops" "123babe123"
//"Engenius" "tinker18"

//void HttpClient::sendBasicAuth(const char* aUser, const char* aPassword)

const char* url = "https://raw.githubusercontent.com/jferry68/bots/master/LYNGOH.json";
//test string in LYNGOH.json is {"Aarm":0,"Akick":1,"Aservo":2,"Aerror":3,"Barm":4,"Bkick":5,"Bservo":6,"Berror":7}

//Define pin numbers to be used
int WIFILIGHT_PIN = 13;
int REDLIGHT_PIN = 18;
int GRLIGHT_PIN = 17;
int Motion_PIN = 16;
int KICKBTN_PIN = 15;
int ARMBTN_PIN = 26;
Servo servo_14;

void setup() {

  servo_14.attach(14);
  servo_14.write(0);
  
  //Define pin numbers as input or output
  pinMode(WIFILIGHT_PIN, OUTPUT);
  digitalWrite(WIFILIGHT_PIN, LOW);
  pinMode(REDLIGHT_PIN, OUTPUT);
  pinMode(GRLIGHT_PIN, OUTPUT);
  pinMode(Motion_PIN, INPUT);
  pinMode(KICKBTN_PIN, INPUT);
  pinMode(ARMBTN_PIN, INPUT);
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);

  WiFi.begin("makerlab", "smoothpanda");
  Serial.print("Trying to connect to ");
  Serial.println("Engenius");
  delay(5000);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin("JKFERRY2", "f3rryl1nk");
    Serial.print("Trying to connect to ");
    Serial.println("JKFERRY2");
    delay(5000);
  }

  else {
    Serial.println("Failed to connect to wifi");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to wifi");
    Serial.println(WiFi.SSID());
    digitalWrite(WIFILIGHT_PIN, HIGH);
    delay(3000);
  }
}

void loop() {

  if ((WiFi.status() == WL_CONNECTED)) {                  //Check the current connection status

    HTTPClient http;

    http.begin(url);                                      //url address defined above in global variables
    int httpCode = http.GET();                            //Make the request

    if (httpCode > 0) {                                   //Check for the returning code

      String input = http.getString();
      //String input = "{"Aarm":0,"Akick":1,"Aservo":2,"Aerror":3,"Barm":4,"Bkick":5,"Bservo":6,"Berror":7}";
      // Create a char array big enough including the terminating NULL
      int bufLength = input.length() + 1;
      char json[bufLength];
      input.toCharArray(json, bufLength);

      Serial.println(json);
      delay(2000);

      StaticJsonBuffer<200> jsonBuffer;
      //DynamicJsonBuffer jsonBuffer;

      JsonObject& root = jsonBuffer.parseObject(json);

      if (!root.success()) {                            //Check for errors in parsing
        Serial.println("Parsing failed");
        delay(5000);
        return;
      }
    }

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
    //http.end();
    
    if (root["Barm"] == 1) {                          //Start of Logic and Opperations
      digitalWrite(GRLIGHT_PIN, HIGH);
      //digitalWrite(GRLIGHT_PIN) = HIGH;
      Serial.println("B arm is up");
      delay(1000);

    } else {
      digitalWrite(GRLIGHT_PIN, LOW);

    }
    if (digitalRead(KICKBTN_PIN) == 1 && root["Barm"] == 1) {
      if (root["Berror"] == 0) {
        root["Akick"] = 1;
        Serial.println("Button has been pushed");
        delay(1000);

      }

    }
    if (digitalRead(ARMBTN_PIN) == 1) {
      root["Aarm"] = 1;
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
      root["Aservo"] = 1;
      Serial.println("B sent message for A to queue a kick");

    }
    if (root["Aservo"] == 1 && digitalRead(Motion_PIN) == HIGH) {
      root["Aservo"] = 0;
      servo_14.write(180);
      delay(500);
      servo_14.write(0);
      delay(2000);
      Serial.println("Self servo has run");
      if (digitalRead(ARMBTN_PIN) == 1) {
        root["Aerror"] = 1;
        Serial.println("Self servo has failed to kick or self robot didnt fall ");

      } else {
        root["Aarm"] = 0;
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
