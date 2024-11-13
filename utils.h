#include <Arduino_BuiltIn.h>

#include "certs.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"

#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("incoming: ");
  Serial.println(topic);

  // Converte o payload em uma string
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  if(message == "L"){
    digitalWrite(33, HIGH);
  }
  else if(message == "D"){
    digitalWrite(33, LOW);
  }
  Serial.println("Received message: " + message);
}

void connectAWS() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(Wifi_SSID, Wifi_Senha);

  Serial.println("Conectando no Wi-Fi");

  while (WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Conectando no Broker");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("Nao foi possivel fazer a reconeccao");
    return;
  }

  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("Broker Conectado!");
}

void publishMessage(int metricsValue) {
  StaticJsonDocument<200> doc;
  doc["metrics"] = metricsValue;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}