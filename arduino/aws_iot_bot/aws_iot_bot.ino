#include <AWS_IOT.h>
#include <WiFi.h>

// https://github.com/aws-samples/aws-iot-workshop
// Make sure you install the certs/keys where specified!

AWS_IOT AWS_CLIENT;

int POLLING_DELAY = 5000;

char WIFI_SSID[]="NETGEAR37";
char WIFI_PASSWORD[]="gentleraven032";
char HOST_ADDRESS[]="ac0pct10qk7h9-ats.iot.us-west-2.amazonaws.com";
char CLIENT_ID[]= "amebaClient";
char UPDATE_TOPIC[]= "$aws/things/BotA/shadow/update";
char* SUBSCRIBE_TOPICS[5] = {
  "$aws/things/BotA/shadow/update/accepted",
  "$aws/things/BotA/shadow/update/rejected",
  "$aws/things/BotA/shadow/update/delta",
  "$aws/things/BotA/shadow/get/accepted",
  "$aws/things/BotA/shadow/get/rejected"
};

int status = WL_IDLE_STATUS;
int tick=0,msgCount=0,msgReceived = 0;
char payload[512];
char rcvdPayload[512];

void callBackHandler (char *topicName, int payloadLen, char *payLoad){
    Serial.print("Callback received: ");
    Serial.println(topicName);
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

void loop() {

    // see if we got a callback
    if(msgReceived == 1){
        msgReceived = 0;
        Serial.print("Received Message:");
        Serial.println(rcvdPayload);
    } else {
        Serial.println("Waiting...");
    }
    
    delay(POLLING_DELAY);
}
