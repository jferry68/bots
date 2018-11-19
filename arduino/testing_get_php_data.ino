#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* url = "https://raw.githubusercontent.com/jferry68/bots/master/LYNGOH.json";
const char* ssid = "JKFERRY2";
const char* password = "f3rryl1nk";

void setup() {

  Serial.begin(115200);
  delay(4000);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

}

void loop() {

  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(url); //Specify the URL
    int httpCode = http.GET();                            //Make the request

    if (httpCode > 0) { //Check for the returning code

        String payload = http.getString();
        Serial.println(httpCode);
        Serial.println(payload);

        // convert the GET response body into a character array
        // json file so far  {"Akick":"0", "Bkick":"1" }
        //TODO: figure out how to convert a string into a JSONMessage array
        //char JSONMessage[] = {0};
        char JSONMessage[] = http.getString();
        /// need a buffer for the parser
        StaticJsonBuffer<300> JSONBuffer;
        JsonObject& parsed = JSONBuffer.parseObject(JSONMessage);

        if (!parsed.success()) {   //Check for errors in parsing

          Serial.println("Parsing failed");
          delay(5000);
          return;

        }

        // get our parameters from the parsed response
        int Akick = parsed["Akick"];
        Serial.println(Akick);
        int Bkick = parsed["Bkick"];
        Serial.println(Bkick);
      }

    else {
      Serial.println("Error on HTTP request");
    }

    http.end(); //Free the resources
  }

  delay(10000);

}
