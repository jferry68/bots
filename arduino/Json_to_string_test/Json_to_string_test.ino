#include <base64.h>
#include <ArduinoJson.h>

void setup() {

  Serial.begin(115200);
}

void loop() {
  
      // Create a char array big enough including the terminating NULL
      
      String input = "{"Aarm":0, "Akick":1, "Aservo":2, "Aerror":3, "Barm":4, "Bkick":5, "Bservo":6, "Berror":7}";
      int bufLength = input.length() + 1;
      char json[bufLength];
      input.toCharArray(json, bufLength);
      
  //char json[] = '{"Aarm":0, "Akick":1, "Aservo":2, "Aerror":3, "Barm":4, "Bkick":5, "Bservo":6, "Berror":7}';

  Serial.println(json);
  delay(5000);

  StaticJsonBuffer<200> jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(json);

  if (!root.success()) {                            //Check for errors in parsing
    Serial.println("Parsing failed");
    delay(5000);
    return;
  }

  int Bkickval = root["Bkick"];
  Serial.println(Bkickval);         //testing the extraction of one of the root arrays. should give a 5
  delay(5000);

  //update values in array, eventually done through logic tests
  root["Aarm"] = 7;
  root["Akick"] = 6;
  root["Aservo"] = 5;
  root["Aerror"] = 4;
  root["Barm"] = 3;
  root["Bkick"] = 2;
  root["Bservo"] = 1;
  root["Berror"] = 0;

  String toEncode;
  root.printTo(toEncode);
  Serial.println(toEncode);
  delay(5000);

  String outcode = base64::encode(toEncode);
  Serial.println(outcode);
  delay(5000);
}
http.end();
}
}
