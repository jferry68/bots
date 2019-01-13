#include <AWS_IOT.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Servo.h>

// https://github.com/aws-samples/aws-iot-workshop
// Make sure you install the certs/keys where specified!

AWS_IOT AWS_CLIENT;

int POLLING_DELAY = 5000;

char WIFI_SSID[]="NETGEAR37";
char WIFI_PASSWORD[]="gentleraven032";
char HOST_ADDRESS[]="ac0pct10qk7h9-ats.iot.us-west-2.amazonaws.com";
char CLIENT_ID[]= "amebaClient";
char UPDATE_TOPIC[]= "$aws/things/UniBot/shadow/update";
char GET_TOPIC[]= "$aws/things/UniBot/shadow/get";
char* SUBSCRIBE_TOPICS[5] = {
  "$aws/things/UniBot/shadow/update/accepted",
  "$aws/things/UniBot/shadow/update/rejected",
  "$aws/things/UniBot/shadow/update/delta",
  "$aws/things/UniBot/shadow/get/accepted",
  "$aws/things/UniBot/shadow/get/rejected"
};

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];
char *payloadTopic;

#define led 2          //onboard blue light
Servo servo_14;
int REDLIGHT_PIN = 18;
int GRLIGHT_PIN = 17;
int Motion_PIN = 16;
int KICKBTN_PIN = 34;
int ARMBTN_PIN = 35;

void callBackHandler (char *topicName, int payloadLen, char *payLoad){
    Serial.print("Callback received: ");
    Serial.println(topicName);
    payloadTopic = topicName;
    strncpy(rcvdPayload,payLoad,payloadLen);
    rcvdPayload[payloadLen] = 0;
    msgReceived = 1;
}

boolean connectToWiFi(){
  
    while (status != WL_CONNECTED){
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

boolean connectToAWS(){
  
      if(0 == AWS_CLIENT.connect(HOST_ADDRESS,CLIENT_ID)){
        Serial.println("Connected to AWS");
        delay(1000);

        for (int i=0; i<4; i++) {
          Serial.print(SUBSCRIBE_TOPICS[i]);
          Serial.print(": ");
          if(0 == AWS_CLIENT.subscribe(SUBSCRIBE_TOPICS[i], callBackHandler)){
              Serial.println("Subscribe Successfull");
          } else {
              Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
              while(1);
          }
        }

        if(0 == AWS_CLIENT.subscribe(UPDATE_TOPIC, callBackHandler)){
            Serial.println("Subscribe Successfull");
        } else {
            Serial.println("Subscribe Failed, Check the Thing Name and Certificates");
            while(1);
        }
    } else {
        Serial.println("AWS connection failed, Check the HOST Address");
        while(1);
    }
    
}

void setup() {
  
    WiFi.disconnect(true);
    Serial.begin(115200);
    
    connectToWiFi();
    connectToAWS();
    
    delay(2000);
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

void updateBotState(JsonObject root) {
  Serial.println("Updating bot state...");
}

void handleMessage(char *payload){

      Serial.println(payload);
      delay(2000);

      StaticJsonDocument<200> doc;
      // Deserialize the JSON document
      DeserializationError error = deserializeJson(doc, payload);

      // Test if parsing succeeds.
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }

      // Get the root object in the document
      JsonObject root = doc.as<JsonObject>();

//      Serial.print("ROOT: ");
//      char toPrint[512];
//      root.serializeJsonPretty(doc,toPrint);
//      Serial.printTo(toPrint);
      
      handleEvents(root);
      updateBotState(root);

}

void loop() {

    // see if we got a callback
    if(msgReceived == 1){
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
        handleMessage(rcvdPayload);
    } else {
        Serial.println("Waiting...");
    }
    
    delay(POLLING_DELAY);
}
