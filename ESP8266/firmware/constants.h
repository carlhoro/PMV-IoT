#ifndef CONSTANTS_H
#define CONSTANTS_H

// -------------------------------
// Claves de JSON
// -------------------------------
const char* DATA_KEY = "dataString";

// -------------------------------
// Mensajes de Log
// -------------------------------
const char* MSG_INIT           = "Iniciando ESP8266 Bridge";

const char* MSG_WIFI_CONNECTING = "Conectando a WiFi";
const char* MSG_WIFI_CONNECTED  = "\nWiFi conectado";
const char* MSG_WIFI_IP         = "IP: ";
const char* MSG_WIFI_FAILED     = "\nNo se pudo conectar a WiFi";
const char* MSG_WIFI_RECONNECT  = "WiFi desconectado. Intentando reconectar...";
const char* MSG_NO_WIFI         = "No hay conexión WiFi para enviar POST.";

const char* MSG_PAYLOAD_BUILT   = "Payload construido: ";
const char* MSG_DATA_RECEIVED   = "Datos recibidos: ";

const char* MSG_HTTP_CODE       = "Código de respuesta: ";
const char* MSG_HTTP_RESPONSE   = "Respuesta: ";
const char* MSG_HTTP_ERROR      = "Error en POST: ";

#endif
