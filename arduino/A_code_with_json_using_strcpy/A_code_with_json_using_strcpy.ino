#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

//possible wifi connections
//"JKFERRY2" "f3rryl1nk"
//"bighoops" "123babe123"
//"Engenius" "tinker18"

const char* url = "https://raw.githubusercontent.com/jferry68/bots/master/LYNGOH.json";
//test string in LYNGOH.json is "{\"Aarm\":0,\"Akick\":1,\"Aservo\":2,\"Aerror\":3,\"Barm\":4,\"Bkick\":5,\"Bservo\":6,\"Berror\":7}"

//Define pin numbers to be used
int WIFILIGHT_PIN = 13;
int REDLIGHT_PIN = 18;
int GRLIGHT_PIN = 17;
int Motion_PIN = 16;
int KICKBTN_PIN = 15;
int ARMBTN_PIN = 26;


void setup() {

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

  WiFi.begin("Engenius", "tinker18");
  Serial.print("Trying to connect to ");
  Serial.println("Engenius");
  delay(5000);

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin("bighoops", "123babe123");
    Serial.print("Trying to connect to ");
    Serial.println("bighoops");
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
    //int httpCode = http.GET();                            //Make the request
    char httpCode = http.GET();                            //Make the request

    if (httpCode > 0) {                                   //Check for the returning code

      String input = http.getString();

      String control(String &input);
        char json[input.length +1()]; // Create a char array big enough including the terminating NULL
        strcpy(json, input.c_str()); // Place incoming route in char array
      
        //char json[] = http.getString();
        //char json[] = "{\"Aarm\":0,\"Akick\":1,\"Aservo\":2,\"Aerror\":3,\"Barm\":4,\"Bkick\":5,\"Bservo\":6,\"Berror\":7}";
        Serial.println(json);
        delay(5000);

        StaticJsonBuffer<200> jsonBuffer;
        //DynamicJsonBuffer jsonBuffer;

        JsonObject& root = jsonBuffer.parseObject(json);

        if (!root.success()) {                            //Check for errors in parsing
          Serial.println("Parsing failed");
          delay(5000);
          return;
        }

        int Bkickval = root["Bkick"];
        Serial.println(Bkickval);         //testing the extraction of one of the root arrays. should give a 5
        delay(5000);
      }
    }
  }
