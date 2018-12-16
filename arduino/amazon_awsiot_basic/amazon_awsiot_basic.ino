/*
 Basic Amazon AWS IoT example
*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
char ssid[] = "NETGEAR37";     // your network SSID (name)
char pass[] = "gentleraven032";  // your network password
int status  = WL_IDLE_STATUS;    // the Wifi radio's status

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

WiFiMulti wifiMulti;

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

#define THING_NAME "ameba"

char mqttServer[]     = "ac0pct10qk7h9-ats.iot.us-west-2.amazonaws.com";
char clientId[]       = "amebaClient";
char publishTopic[]   = "$aws/things/ameba/shadow/update";
char publishPayload[MQTT_MAX_PACKET_SIZE];
const char* subscribeTopic[5] = {
  "$aws/things/ameba/shadow/update/accepted",
  "$aws/things/ameba/shadow/update/rejected",
  "$aws/things/ameba/shadow/update/delta",
  "$aws/things/ameba/shadow/get/accepted",
  "$aws/things/ameba/shadow/get/rejected"
};

/* root CA can be download here:
 *  https://www.symantec.com/content/en/us/enterprise/verisign/roots/VeriSign-Class%203-Public-Primary-Certification-Authority-G5.pem
 **/
const char* rootCABuff = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIE0zCCA7ugAwIBAgIQGNrRniZ96LtKIVjNzGs7SjANBgkqhkiG9w0BAQUFADCB\n" \
"yjELMAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQL\n" \
"ExZWZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJp\n" \
"U2lnbiwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxW\n" \
"ZXJpU2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0\n" \
"aG9yaXR5IC0gRzUwHhcNMDYxMTA4MDAwMDAwWhcNMzYwNzE2MjM1OTU5WjCByjEL\n" \
"MAkGA1UEBhMCVVMxFzAVBgNVBAoTDlZlcmlTaWduLCBJbmMuMR8wHQYDVQQLExZW\n" \
"ZXJpU2lnbiBUcnVzdCBOZXR3b3JrMTowOAYDVQQLEzEoYykgMjAwNiBWZXJpU2ln\n" \
"biwgSW5jLiAtIEZvciBhdXRob3JpemVkIHVzZSBvbmx5MUUwQwYDVQQDEzxWZXJp\n" \
"U2lnbiBDbGFzcyAzIFB1YmxpYyBQcmltYXJ5IENlcnRpZmljYXRpb24gQXV0aG9y\n" \
"aXR5IC0gRzUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCvJAgIKXo1\n" \
"nmAMqudLO07cfLw8RRy7K+D+KQL5VwijZIUVJ/XxrcgxiV0i6CqqpkKzj/i5Vbex\n" \
"t0uz/o9+B1fs70PbZmIVYc9gDaTY3vjgw2IIPVQT60nKWVSFJuUrjxuf6/WhkcIz\n" \
"SdhDY2pSS9KP6HBRTdGJaXvHcPaz3BJ023tdS1bTlr8Vd6Gw9KIl8q8ckmcY5fQG\n" \
"BO+QueQA5N06tRn/Arr0PO7gi+s3i+z016zy9vA9r911kTMZHRxAy3QkGSGT2RT+\n" \
"rCpSx4/VBEnkjWNHiDxpg8v+R70rfk/Fla4OndTRQ8Bnc+MUCH7lP59zuDMKz10/\n" \
"NIeWiu5T6CUVAgMBAAGjgbIwga8wDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8E\n" \
"BAMCAQYwbQYIKwYBBQUHAQwEYTBfoV2gWzBZMFcwVRYJaW1hZ2UvZ2lmMCEwHzAH\n" \
"BgUrDgMCGgQUj+XTGoasjY5rw8+AatRIGCx7GS4wJRYjaHR0cDovL2xvZ28udmVy\n" \
"aXNpZ24uY29tL3ZzbG9nby5naWYwHQYDVR0OBBYEFH/TZafC3ey78DAJ80M5+gKv\n" \
"MzEzMA0GCSqGSIb3DQEBBQUAA4IBAQCTJEowX2LP2BqYLz3q3JktvXf2pXkiOOzE\n" \
"p6B4Eq1iDkVwZMXnl2YtmAl+X6/WzChl8gGqCBpH3vn5fJJaCGkgDdk+bW48DW7Y\n" \
"5gaRQBi5+MHt39tBquCWIMnNZBU4gcmU7qKEKQsTb47bDN0lAtukixlE0kF6BWlK\n" \
"WE9gyn6CagsCqiUXObXbf+eEZSqVir2G3l6BFoMtEMze/aiCKm0oHw0LxOXnGiYZ\n" \
"4fQRbxC1lfznQgUy286dUV4otp6F01vvpX1FQHKOtw5rDgb7MzVIcbidJ4vEZV8N\n" \
"hnacRHr2lVz2XTIIM6RUthg/aFzyQkqFOFSDX9HoLPKsEdao7WNq\n" \
"-----END CERTIFICATE-----\n";

/* Fill your certificate.pem.crt wiht LINE ENDING */
const char* certificateBuff = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDWjCCAkKgAwIBAgIVAPNpJ/z1wIoX8GnhiwmMW9HuYaWPMA0GCSqGSIb3DQEB\n" \
"CwUAME0xSzBJBgNVBAsMQkFtYXpvbiBXZWIgU2VydmljZXMgTz1BbWF6b24uY29t\n" \
"IEluYy4gTD1TZWF0dGxlIFNUPVdhc2hpbmd0b24gQz1VUzAeFw0xODEyMTUxODE0\n" \
"NDZaFw00OTEyMzEyMzU5NTlaMB4xHDAaBgNVBAMME0FXUyBJb1QgQ2VydGlmaWNh\n" \
"dGUwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDFFtEPSZ6VRtT22zC0\n" \
"qPCQJKLjCI3JQFsPVxNlmvG2UKXXiN0VsfjqEkwUk73Z9uUlupXdC7W9K75hNoLn\n" \
"mV+k2QlxHpPxrf0UrL+EFD7bo3A6n7jABQIXwXQGUVa+okgtM/zX/AWwF1+rQNgQ\n" \
"09PBRYnN7QiEdm+nVgu+xMzk4PPke3bttja7qxGPZGj41C7sLOVfzGSsr0CnpdHY\n" \
"0IyqK79tdkoHwu+opk1Ft0Qwo5bRlh5iU/kN26zrLRIzhha0ZDTwSsjKJHYeOM/R\n" \
"MS2AGhtrzdf/1bVq4rAkk0bOISs0jcgxhKRAu3hka8XmMe1gixpO5oygfl5rrYGU\n" \
"PhRpAgMBAAGjYDBeMB8GA1UdIwQYMBaAFGsvm9n7o67xisX8AgRcreMo30tVMB0G\n" \
"A1UdDgQWBBTfc606q4NgvRIKtlRIy+3TZlIXpjAMBgNVHRMBAf8EAjAAMA4GA1Ud\n" \
"DwEB/wQEAwIHgDANBgkqhkiG9w0BAQsFAAOCAQEAOISZaAOQ5iTEVRw907dHIXXO\n" \
"cBrKcozKM8oaukJ6YYDTqxiIH0Rb7l33/e9/4F9Y/uMabWNKNaoW9sNRpRX5bafA\n" \
"ff3hfcECdo8074SpgjzLZmAO5Uxkfxm3hhNqajq3Yq/zUKRI9PiuduXbZLR/LYiK\n" \
"8GrFbfM3LX7CJIL7kYpZRZuQ2nB+ZOoPAC6ruj3+negDubd6DbTaHm43yzedWvGc\n" \
"HpeG7sLeKktiCP6vDOzrDR26FIwktUtL3GB1vzyJK9HFsj1LJoehaaASTb2L+YfB\n" \
"aTJSt2Q6AmlV+466745dKslXycnwsyVr+qOQmJvfJEM8ybOr0CAkPMqb8nSIcQ==\n" \
"-----END CERTIFICATE-----\n";

/* Fill your private.pem.key wiht LINE ENDING */
const char* privateKeyBuff = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEAxRbRD0melUbU9tswtKjwkCSi4wiNyUBbD1cTZZrxtlCl14jd\n" \
"FbH46hJMFJO92fblJbqV3Qu1vSu+YTaC55lfpNkJcR6T8a39FKy/hBQ+26NwOp+4\n" \
"wAUCF8F0BlFWvqJILTP81/wFsBdfq0DYENPTwUWJze0IhHZvp1YLvsTM5ODz5Ht2\n" \
"7bY2u6sRj2Ro+NQu7CzlX8xkrK9Ap6XR2NCMqiu/bXZKB8LvqKZNRbdEMKOW0ZYe\n" \
"YlP5Ddus6y0SM4YWtGQ08ErIyiR2HjjP0TEtgBoba83X/9W1auKwJJNGziErNI3I\n" \
"MYSkQLt4ZGvF5jHtYIsaTuaMoH5ea62BlD4UaQIDAQABAoIBAQC4Vc7S7ZhVJhqp\n" \
"h/jV/+x/5MQ0dBuoz3hbsKWDspfAXU2/eVLOp9aXluVxmX1ID5Pi5dClHf6tH+zA\n" \
"H6vv0ZOTKyuZkW/t6z15yNiXQBhKdWLScfDqJ8T9UwKhphHr2vyGyskBecYYHMRt\n" \
"+AgeNQEpE1EAOWJuAhgL+n6hvo+kwNx+lghVRR/bMLmtPnzXV5X7/0R3qpYRMonW\n" \
"hW238BSi9IWWyMiCjSG4z5QjXMU34A9MXaAzXFUOMiRzh5ptdm0tI924Uy4DD+t9\n" \
"EoTvoVmCSi1TI5adU2lKKQ5BVJcU5mSPRJaAU8Q8/87umyHn/CTbB9muPTWdZN5N\n" \
"H8C1iC8BAoGBAO3XePIhX6TP73mE1CjgMdv+ra+kBu2c2LOYJvNVEmF/4U42huF0\n" \
"Vk5NCpfzBJgQGimL9wDXJuzuIi6jWvwN3laKNdwrgbxb9ZD5akSqla/eDnqIZgl1\n" \
"uexiJK6da+aDE3i7SHqItRaaFMhzvKydr42TsvvivBzGDHAct6DuOrCdAoGBANQi\n" \
"2bpGxN+zqmZmaVEtu1O28npmG5wZfdSXJmCha+xny/oWmXoJRuyTd5HT3RG+8+nH\n" \
"VqBhKJoUVaXzLAoH6lQ68wd17hvoRyhyh9YM/OGEwu0lFsa3o+ButGpDcy94+L3x\n" \
"lMyJS0p9ibP0yPUNT953l2oKghfNgR4lRVbnVEs9AoGAKVbdV8CDPkNCDW4P7jbh\n" \
"k6pViATLi3tKkxSsGHk946J1h8GkegVdhuiPIdAEiWWrFnOUcBn1UxMk79RG2S6T\n" \
"6ZjGOcl6GEInkZY4aUz5DYNcIuIYUiabNUHtvEfyXDOGK+LpexrN1urbF+g98AbM\n" \
"X6A/aNaE+b9WpBnptNgTbWUCgYEAkySBRXXlBQ3IOjL7BjXkY9LYKhI9QG9mGlml\n" \
"Yw1UoKv2jkeyuTVKHrrDC0Gba725zgRYj/IUiZtlNTCQ8XxAC6DABgLE4FKjWOjo\n" \
"U466sWq3EmrRe5DReNERD0wtz1Xno0i4X48zzD17kHlQErnjFnp5ajJTvlLrJo/P\n" \
"otHeZDkCgYEA3TnfhX6Zi7J+nM7MsNSWltnip4FbKDHYn5aIlTvpOPEo7d9UnCEz\n" \
"3izV/58CJPMHtp5ZDdRME527i+TyilgQc5xP9AXm6e0F6kVQVMghBv3aUr2l9AKE\n" \
"YHlqDmPI0fhZ1w+FSlmj6iqrW68OtHvQlh9KnMsRzxrpSn1kRMmYPcM=\n" \
"-----END RSA PRIVATE KEY-----\n";

int led_pin = 21;
int led_state = 0;

void updateLedState(int desired_led_state) {
  printf("change led_state to %d\r\n", desired_led_state);
  led_state = desired_led_state;
  digitalWrite(led_pin, led_state);

  sprintf(publishPayload, "{\"state\":{\"reported\":{\"led\":%d}},\"clientToken\":\"%s\"}",
    led_state,
    clientId
  );
  client.publish(publishTopic, publishPayload);
  printf("Publish [%s] %s\r\n", publishTopic, publishPayload);
}

void callback(char* topic, byte* payload, unsigned int length) {
  char buf[MQTT_MAX_PACKET_SIZE];
  char *pch;
  int desired_led_state;

  strncpy(buf, (const char *)payload, length);
  buf[length] = '\0';
  printf("Message arrived [%s] %s\r\n", topic, buf);

  if ((strstr(topic, "/shadow/get/accepted") != NULL) || (strstr(topic, "/shadow/update/accepted") != NULL)) {
    // payload format: {"state":{"reported":{"led":1},"desired":{"led":0}},"metadata":{"reported":{"led":{"timestamp":1466996558}},"desired":{"led":{"timestamp":1466996558}}},"version":7,"timestamp":1466996558}
    pch = strstr(buf, "\"desired\":{\"led\":");
    if (pch != NULL) {
      pch += strlen("\"desired\":{\"led\":");
      desired_led_state = *pch - '0';
      if (desired_led_state != led_state) {
        updateLedState(desired_led_state);
      }
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientId)) {
      Serial.println("connected");

      for (int i=0; i<5; i++) {
        client.subscribe(subscribeTopic[i]);
      }

      sprintf(publishPayload, "{\"state\":{\"reported\":{\"led\":%d}},\"clientToken\":\"%s\"}",
        led_state,
        clientId
      );
      client.publish(publishTopic, publishPayload);
      printf("Publish [%s] %s\r\n", publishTopic, publishPayload);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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
    Serial.println("");
    Serial.print("AP: ");
    Serial.println(WiFi.SSID());
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Failed to connect to wifi");
  }
}

void setup()
{
  Serial.begin(115200);
  delay(5000);
  pinMode(led_pin, OUTPUT);
  digitalWrite(led_pin, led_state);

  setupWiFiAccessPoints();
  initWiFiConnection();

  wifiClient.setCACert(rootCABuff);
  wifiClient.setCertificate(privateKeyBuff);

  client.setServer(mqttServer, 8883);
  client.setCallback(callback);

  // Allow the hardware to sort itself out
  delay(1500);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
    delay(500);
    Serial.println("starting client loop");
    client.loop();
}
