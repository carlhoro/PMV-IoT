#include <DFRobot_PN532.h>
#include <EEPROM.h>
#include <GFButton.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "RTClib.h"

// -------------------------------
// Headers propios
// -------------------------------
#include "pins.h"
#include "states.h"
#include "config.h"
#include "protocol.h"

// -------------------------------
// Periféricos
// -------------------------------
RTC_DS3231 reloj3231;
LiquidCrystal_I2C lcd(0x27, 20, 4);
SoftwareSerial WIFI(15, 14);
DFRobot_PN532_IIC nfc(2, 0);

// -------------------------------
// Botones
// -------------------------------
GFButton btnExit       (PIN_BTN_EXIT);
GFButton btnInicio     (PIN_BTN_INICIO);
GFButton btnConfig     (PIN_BTN_CONFIG);
GFButton btnLpOK       (PIN_BTN_LP_OK);
GFButton btnInmersion1 (PIN_BTN_INM1);
GFButton btnInmersion2 (PIN_BTN_INM2);

// -------------------------------
// Variables de proceso
// -------------------------------
String arrayVar[10];
int    pHArray[ARRAY_LENGTH];

bool estadoLedI1  = LOW;
bool estadoLedI2  = LOW;
bool estadoLedLp  = LOW;
bool puerta       = false;

unsigned long currentMillis  = 0;
unsigned long previousMillis = 0;

EstadoProceso estado = HOME;
String UIDleido = "";
String lema = "RANIOT Biomedics";

// -------------------------------
// Setup
// -------------------------------
void setup() {
  pinMode(PIN_LED_INM1, OUTPUT);
  pinMode(PIN_LED_INM2, OUTPUT);
  pinMode(PIN_LED_LP,   OUTPUT);
  pinMode(PIN_BUZZER,   OUTPUT);

  Serial.begin(115200);
  WIFI.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.clear();

  Wire.begin();

  while (!nfc.begin()) {
    lcd.print("Inicio RFID fallo");
    delay(100);
  }

  if (!reloj3231.begin()) {
    lcd.print("No se haya RTC");
    while (1);
  }

  pantalla("Iniciando.", 5, "Espere por favor", 2);
  delay(5000);

  WIFI.print("validar,");
  pantalla("Reprocesados", 4, "de alto nivel.", 3);
}

// -------------------------------
// Loop principal
// -------------------------------
void loop() {

  if (Serial.available() > 0) {
    char dato[5];
    dato[Serial.readBytesUntil('\n', dato, sizeof(dato) - 1)] = 0;

    if (dato[0] == 'A') {
      puerta = true;
      Serial.write('1');
    } else if (dato[0] == 'C') {
      Serial.write('0');
    }
  }

  switch (estado) {

    case HOME:
      if (puerta || btnInicio.isPressed()) {
        hora();
        fecha();
        puerta = false;

        WIFI.print("conectar,");
        lcd.backlight();
        pantalla("Id personal.", 4, "Acerque tarjeta.", 2);

        if (leerRFID()) {
          buzzOk();
          if (compararColUID()) {
            estado = END;
          } else {
            pantalla("ERROR: ID invalida", 1, "Presione iniciar", 2);
            buzzNotOk();
            estado = LP;
          }
        }
      }
      break;

    case END:
      pantalla("Id endoscopio.", 3, "Acerque tag.", 4);
      if (leerRFID()) {
        buzzOk();
        estado = compararEndUID() ? LP : HOME;
      }
      break;

    case LP:
      pantalla("Revisar limp previa.", 0, "Presiona OK.", 4);
      if (fnLp()) {
        buzzOk();
        estado = JE;
      } else {
        WIFI.print("reset,");
        buzzNotOk();
        lcd.noBacklight();
        estado = HOME;
      }
      break;

    case JE:
      pantalla("Realiza la inmersion", 0, "y Presiona OK.", 3);
      if (btnInmersion1.isPressed()) {
        buzzOk();
        fnInmersion();
        estado = DES;
      }
      break;

    case DES:
      pantalla("Inmers. desinfeccion", 0, "y Presiona OK.", 3);
      if (btnInmersion2.isPressed()) {
        buzzOk();
        fnInmersion2();
        estado = ALM;
      }
      break;

    case ALM:
      agregarIndicadorYEnviar();
      pantalla("Reprocesados", 4, "de alto nivel.", 3);
      estado = HOME;
      break;
  }
}

// -------------------------------
// Funciones auxiliares (sin cambios)
// -------------------------------
void hora() {
  DateTime now = reloj3231.now();
  arrayVar[FIELD_A] = String(now.hour()) + ":" + String(now.minute());
}

void fecha() {
  DateTime now = reloj3231.now();
  arrayVar[FIELD_B] = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
}

void agregarIndicadorYEnviar() {
  char keys[] = {'A','B','C','D','E','F','G','H','I'};
  for (int i = 0; i < 9; i++) {
    WIFI.print(String(keys[i]) + "-" + arrayVar[i] + ",");
  }
  WIFI.print(SERIAL_END_CHAR);
  WIFI.print(",");
  pantalla("Enviando informacion", 0, "", 0);
  delay(2000);
}

void fnInmersion2() {
  pantalla("desinfectante.", 3, "Sumergido.", 5);

  int i = 0;
  unsigned long totalInmersion2 = 0;
  long extemporaneo = 0;
  unsigned long tiempoInicio = millis();

  while (millis() < tiempoInicio + T_INMERSION_2) {
    static unsigned long samplingTime = millis();
    digitalWrite(PIN_LED_INM2, HIGH);

    if (millis() - samplingTime > SAMPLING_INTERVAL) {
      pHArray[i] = analogRead(SENSOR_PH_PIN);
      arrayVar[FIELD_I] = avergearray(pHArray);

      if (i == ARRAY_LENGTH - 1) i = 0;
      i++;
      samplingTime = millis();
    }
  }

  digitalWrite(PIN_LED_INM2, LOW);
  digitalWrite(PIN_BUZZER, HIGH);

  tiempoInicio = millis();
  pantalla("Tiempo cumplido.", 2, "Retire el equipo", 2);

  while (!btnInmersion2.isPressed()) {
    extemporaneo = tiempoInicio - millis();
  }

  digitalWrite(PIN_BUZZER, LOW);
  delay(100);

  totalInmersion2 = (T_INMERSION_2 - extemporaneo) / 1000;
  arrayVar[FIELD_H] = totalInmersion2;
}

void fnInmersion() {
  pantalla("Jabon enzimatico", 2, "Sumergido.", 5);

  int i = 0;
  unsigned long totalInmersion1 = 0;
  long extemporaneo = 0;
  unsigned long tiempoInicio = millis();

  while (millis() < tiempoInicio + T_INMERSION_1) {
    static unsigned long samplingTime = millis();
    digitalWrite(PIN_LED_INM1, HIGH);

    if (millis() - samplingTime > SAMPLING_INTERVAL) {
      pHArray[i] = analogRead(SENSOR_PH_PIN);
      arrayVar[FIELD_G] = avergearray(pHArray);

      if (i == ARRAY_LENGTH - 1) i = 0;
      i++;
      samplingTime = millis();
    }
  }

  digitalWrite(PIN_LED_INM1, LOW);
  digitalWrite(PIN_BUZZER, HIGH);

  tiempoInicio = millis();
  pantalla("Tiempo cumplido.", 2, "Retire el equipo", 2);

  while (!btnInmersion1.isPressed()) {
    extemporaneo = tiempoInicio - millis();
  }

  digitalWrite(PIN_BUZZER, LOW);
  delay(100);

  totalInmersion1 = (T_INMERSION_1 - extemporaneo) / 1000;
  arrayVar[FIELD_F] = totalInmersion1;
}

bool fnLp() {
  while (!btnExit.isPressed()) {
    currentMillis = millis();

    if (currentMillis - previousMillis >= T_PARPADEO) {
      previousMillis = currentMillis;
      estadoLedLp = !estadoLedLp;
      digitalWrite(PIN_LED_LP, estadoLedLp);
    }

    if (btnLpOK.isPressed()) {
      digitalWrite(PIN_LED_LP, LOW);
      arrayVar[FIELD_E] = "OK";
      return true;
    }
  }

  digitalWrite(PIN_LED_LP, LOW);
  lcd.noBacklight();
  return false;
}

bool leerRFID() {
  while (!btnExit.isPressed()) {
    if (nfc.scan()) {
      UIDleido = nfc.readUid();
      return true;
    }
  }
  return false;
}

bool compararColUID() {
  String UIDguardada = "";

  for (int i = 0; i < 4; i++) {
    UIDguardada = "";
    for (int j = 0; j < 8; j++) {
      UIDguardada += char(EEPROM.read(j + (8 * i)));
    }
    if (UIDleido == UIDguardada) {
      arrayVar[FIELD_C] = UIDleido;
      return true;
    }
  }
  return false;
}

bool compararEndUID() {
  String UIDguardada;

  for (int i = 0; i < 8; i++) {
    UIDguardada = "";
    for (int j = 0; j < 8; j++) {
      UIDguardada += char(EEPROM.read(j + 32 + (8 * i)));
    }
    if (UIDleido == UIDguardada) {
      arrayVar[FIELD_D] = UIDleido;
      return true;
    }
  }
  return false;
}

void buzzOk() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(80);
  digitalWrite(PIN_BUZZER, LOW);
}

void buzzNotOk() {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(80);
  digitalWrite(PIN_BUZZER, LOW);
  delay(80);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(500);
  digitalWrite(PIN_BUZZER, LOW);
}

void pantalla(String ln2, int p2, String ln3, int p3) {
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print(lema);
  lcd.setCursor(p2, 2);
  lcd.print(ln2);
  lcd.setCursor(p3, 3);
  lcd.print(ln3);
}

float avergearray(int* arr) {
  long amount = 0;

  for (int j = 0; j < ARRAY_LENGTH - 1; j++) {
    amount += arr[j];
  }

  double avg = amount / (ARRAY_LENGTH - 1);
  float voltage = avg * 5.0 / 1024;
  return (3.5 * voltage) + OFFSET_PH;
}
