#include <ESP8266WiFi.h>
#include <AsyncMqttClient.h>
#include <Ticker.h>

#define WIFI_SSID "_ahora-no2.4"
#define WIFI_PASSWORD "uN2444%&"

#define MQTT_HOST IPAddress(192, 168, 2, 100)
#define MQTT_PORT 1883

#define MQTT_PUB_SENSOR "sensor1/datos"

float dato = 79.1;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;
Ticker wifiReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;

unsigned long previousMillis = 0;
const long interval = 10000;


void connectToWiFi() {
  Serial.println("Conectando al WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}


void onWiFiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Conectado al WiFi.");
  connectToMqtt();
}

void onWiFiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Desconectado del WiFi.");
  mqttReconnectTimer.detach();
  wifiReconnectTimer.once(2, connectToWiFi);
}

void connectToMqtt() {
  Serial.println("Conectando al MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Conectado al MQTT.");
  Serial.print("Sesión: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Desconectado del MQTT.");

  if(WiFi.isConnected()){
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Paquete publicado.");
  Serial.print(" ID paquete: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWiFiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWiFiDisconnect);
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWiFi();

}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;

    uint16_t packetIdPub = mqttClient.publish(MQTT_PUB_SENSOR, 1, true, 
    String(dato).c_str());
    Serial.printf("Paquete publicado: %s, QoS 1, packetId: $i",
    MQTT_PUB_SENSOR, packetIdPub);
  }

}
