#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

//set up global variables
String JKF_wifissid = "JKFERRY2";  //JOEFERRY wi-fi in Seattle
String JKF_wifipw = "f3rryl1nk";
String TFW_wifissid = "BigHoops";  //TOMFERRY wi-fi in Wallace
String TFW_wifipw = "Wallace_PW";
String TFP_wifissid = "Engenius";  //TOMFERRY wi-fi in Phoenix
String TFP_wifipw = "tinker18";
const char* url = "https://raw.githubusercontent.com/jferry68/bots/master/LYNGOH.json";
//CONTROLSTRING format from LYNGOH.json looks like {Aa1Ak2As3Ae4Ba5Bk6Bs7Be8}

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
  Serial.begin(115200);  // Set baud rate for Serial Monitor
  delay(5000);

  while ((!(WiFi.status() == WL_CONNECTED))) {
    if (!(WiFi.status() == WL_CONNECTED)) {
    WiFi.begin("JKFERRY2", "f3rryl1nk");
    Serial.println("Trying JKFERRY2...");
    delay(5000);
    }
    else if (!(WiFi.status() == WL_CONNECTED)) {
    WiFi.begin("TFW_wifissid", "TFW_wifipw");
    Serial.println("Trying TFW_wifissid...");
    delay(5000);
    }
    else if (!(WiFi.status() == WL_CONNECTED)) {
    WiFi.begin("TFP_wifissid", "TFP_wifipw");
    Serial.println("Trying TFP_wifissid...");
    delay(5000);
    }
    else
    {
    Serial.println("Error Connecting to Wi-Fi network");
    }
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to Wi-Fi network ");
    Serial.print((WiFi.SSID()));
    Serial.println(" ");
    digitalWrite(WIFILIGHT_PIN, HIGH);
  }
  else {
    Serial.println("Error Connecting to Wi-Fi network");
    digitalWrite(WIFILIGHT_PIN, LOW);
  }
}

void loop()
{
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    http.begin(url); //Specify the URL
    int httpCode = http.GET();                            //Make the request

    if (httpCode > 0) {                                   //Check for the returning code

      String CONTROLSTRING = http.getString();
      Serial.println(CONTROLSTRING);
      Serial.println(total);
      delay(5000);

    //int Aarm;
    //int Akick;
    //Aarm = com.substring(3,3)
    //Akick = com.substring(6,6)

    //int total = Aarm + Akick;  //check to see that these are really integers converted from string
    //Serial.println(total);
     }
   }
 }
