#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include "wifi_config.h"
#include "lambda_config.h"
#include "certs.h"
#include "constants.h"

// -------------------------------
// Buffer serial
// -------------------------------
String serialBuffer = "";
bool mensajeCompleto = false;

// -------------------------------
// Setup
// -------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println(MSG_INIT);
  conectarWiFi();
}

// -------------------------------
// Loop principal
// -------------------------------
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(MSG_WIFI_RECONNECT);
    conectarWiFi();
  }

  leerMensajeSerial();

  if (mensajeCompleto) {
    String payload = construirPayload(serialBuffer);
    enviarPOST(payload);
    serialBuffer = "";
    mensajeCompleto = false;
  }

  delay(100);
}

// -------------------------------
// Función: Conexión WiFi con reconexión
// -------------------------------
void conectarWiFi() {
  WiFi.begin(ssid, password);
  Serial.print(MSG_WIFI_CONNECTING);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println(MSG_WIFI_CONNECTED);
  Serial.print(MSG_WIFI_IP);
  Serial.println(WiFi.localIP());
}

// -------------------------------
// Función: Leer datos por serial hasta sentinela 'E'
// -------------------------------
void leerMensajeSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == 'E') {          // Sentinela de fin
      mensajeCompleto = true;
      Serial.println(MSG_DATA_RECEIVED + serialBuffer);
      return;
    }
    serialBuffer += c;
  }
}

// -------------------------------
// Función: Construir JSON agrupado
// -------------------------------
String construirPayload(String data) {
  String json = "{ \"data\": {";

  int start = 0;
  bool firstPair = true;

  while (true) {
    int commaIndex = data.indexOf(',', start);
    if (commaIndex == -1) break;

    String pair = data.substring(start, commaIndex);
    int dashIndex = pair.indexOf('-');

    if (dashIndex > 0) {
      String key = pair.substring(0, dashIndex);
      String value = pair.substring(dashIndex + 1);

      if (!firstPair) json += ",";
      json += "\"" + key + "\":\"" + value + "\"";
      firstPair = false;
    }

    start = commaIndex + 1;
  }

  json += "} }";

  Serial.println(MSG_PAYLOAD_BUILT + json);
  return json;
}

// -------------------------------
// Función: Enviar POST
// -------------------------------
void enviarPOST(String payload) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    WiFiClientSecure client;
    client.setInsecure();  // Prototipo

    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      Serial.print(MSG_HTTP_CODE);
      Serial.println(httpResponseCode);
      Serial.println(MSG_HTTP_RESPONSE + http.getString());
    } else {
      Serial.print(MSG_HTTP_ERROR);
      Serial.println(http.errorToString(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println(MSG_NO_WIFI);
  }
}
