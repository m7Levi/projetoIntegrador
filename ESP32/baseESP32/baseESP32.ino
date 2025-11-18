// ===================================================
// ========== BIBLIOTECAS E INCLUDES GERAIS ==========
// ===================================================
#include <Arduino.h>
#include <Wire.h>

#include <WiFi.h>              // Biblioteca Wi-Fi para ESP32
#include <WebServer.h>         // Biblioteca de servidor HTTP para ESP32
#include <ArduinoJson.h>       // Para criar e enviar JSON
#include <WiFiUdp.h>           // Para comunicação UDP (necessário ao NTP)

#include <time.h>

#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif


#include <DHT.h>
#include "Adafruit_VL53L0X.h"

// ===================================================
// ========== CONFIGURAÇÕES DO PROJETO ===============
// ===================================================

// --- CONFIGURE A REDE WIFI AQUI ---
const char* ssid = "Dionizio";
const char* password = "09041907";

// --- Cria uma instância do servidor na porta 80 (HTTP padrão) ---
WebServer server(80);

// --- CONFIGURE O FUSO HORÁRIO AQUI ---
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

  char buf[64];
  char buf2[80];

struct tm timeinfo;



// ==== TEMPERATURA EXTERNA (OpenWeather) ====

const char* apiKey = "7a6c7c1ef1b24bdfa977ec15e6eb9695";  // <-- Troque pela sua API KEY
const char* cidade = "Natal";
const char* pais =   "BR";

float temperaturaExterna = 0;
float umidadeExterna = 0;


// --- PINOS (ESP32) ---
#define DHTPIN 18       // Pino do sensor DHT11

// --- CONSTANTES ---
#define DHTTYPE DHT11        // Define o tipo do sensor DHT
const int LIMITE_DIST = 800; // Distância mínima em cm para detecção

// ===================================================
// ========== OBJETOS E VARIÁVEIS GLOBAIS ==========
// ===================================================

// --- Sensores ---
DHT dht(DHTPIN, DHTTYPE);

Adafruit_VL53L0X loxA = Adafruit_VL53L0X();
Adafruit_VL53L0X loxB = Adafruit_VL53L0X();


// --- Variáveis de estado para sensores ---
bool detectA = false;
bool detectB = false;
unsigned long tempoA = 0;
unsigned long tempoB = 0;
int contarPessoas = 0;
float temperatura;
bool arCondicionado = false;

uint8_t sCommand = 0x90;
uint8_t sRepeats = 1;

// --- Variáveis para controle de tempo (não-bloqueante) ---
unsigned long previousMillisDetect = 0;
unsigned long previousMillisDHT = 0;
const long intervalDetect = 250; // Intervalo para checar os sensores de distância
const long intervalDHT = 2000;   // Intervalo para ler o sensor DHT11





// ===================================================
// ========== PROTÓTIPOS DAS FUNÇÕES ===============
// ===================================================


// ===================================================
// ========== MULTPLEXADOR ===========================
// ===================================================

// Endereço padrão do TCA9548A
#define TCAADDR 0x70

// Função para selecionar canal no TCA9548A
void tcaselect(uint8_t i) {
  // if (i > 7) return;
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();
  delay(5);
}

int MultSensorA = 1;
int MultSensorB = 0;  

// ===================================================
// ========== SETUP ==================================
// ===================================================
void setup() {


  Serial.begin(115200);
  delay(500);
  Serial.println("\nSistema iniciado no ESP8266...");

  // --- inicialização do IR ------------------------------------------------------------------------------------------

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

  // --- Inicialização dos sensores e pinos --------------------------------------------------------------------------
  dht.begin();

  Wire.begin(21, 22); 

  delay(50);

  Serial.println("Iniciando sensores via TCA9548A...");

  // Sensor A - Canal 0
  tcaselect(MultSensorA);
  if (!loxA.begin()) {
    Serial.println("Falha ao iniciar Sensor A");
    // while (1);
  } else Serial.println("Sensor A OK");

  delay(250);

  // Sensor B - Canal 1
  tcaselect(MultSensorB);
  if (!loxB.begin()) {
    Serial.println("Falha ao iniciar Sensor B");
    // while (1);
  }
  else Serial.println("Sensor B OK");

  // --- Conexão com o WiFi -------------------------------------------------------------------------------------
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
  Serial.print("Endereco de IP: ");
  Serial.println(WiFi.localIP());

  // Define as rotas do servidor
  server.on("/", handleRoot);
  server.on("/dados", handleDadosSensor);
  server.on("/controleAR", handleArControl);
  // Inicia o servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado!");

//==============================HORARIO====================================================================

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  Serial.println("Sincronizando horário com NTP...");
  // espera até sincronizar (ou timeout)
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 30) { // espera até ~15s
    Serial.print(".");
    delay(500);
    attempts++;
  }
  Serial.println();

  if (getLocalTime(&timeinfo)) {
    Serial.println("Horario sincronizado!");
    printLocalTime();
  } else {
    Serial.println("Não foi possível sincronizar horário via NTP.");
  }

//================================================================================================================================

}


// ===================================================
// ========== LOOP PRINCIPAL =========================
// ===================================================


void loop() {
  // Esta função verifica se algum cliente fez uma requisição ao servidor
  server.handleClient();
  debug();
  // printLocalTime();
}