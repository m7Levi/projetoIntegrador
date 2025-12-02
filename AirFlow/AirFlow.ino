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




#include "PinDefinitionsAndMore.h" // Define macros for input and output pin etc.

#if !defined(RAW_BUFFER_LENGTH)
// For air condition remotes it may require up to 750. Default is 200.
#  if !((defined(RAMEND) && RAMEND <= 0x4FF) || (defined(RAMSIZE) && RAMSIZE < 0x4FF))
#define RAW_BUFFER_LENGTH  700 // we require 2 buffer of this size for this example
#  endif
#endif

#define EXCLUDE_EXOTIC_PROTOCOLS // saves around 650 bytes program memory if all other protocols are active

#include <IRremote.hpp>

struct storedIRDataStruct {
    IRData receivedIRData;

    uint8_t rawCode[RAW_BUFFER_LENGTH]; // The durations if raw
    uint8_t rawCodeLength; // The length of the code
} sStoredIRData;

bool receptor();
void emissor(storedIRDataStruct *aIRDataToSend);




#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif

#include <DHT.h>
#include "Adafruit_VL53L0X.h"


#include <Adafruit_SSD1306.h> //Inclusão da biblioteca Adafruit_SSD1306.h

Adafruit_SSD1306 displayLED; //Criação do objeto display, que será chamado para realizar as intruções da biblioteca Adafruit_SSD1306.h


// ===================================================
// ========== CONFIGURAÇÕES DO PROJETO ===============
// ===================================================

// --- CONFIGURE A REDE WIFI AQUI ---
const char* ssid = "FFcrono205";
const char* password = "FFcrono205";

// --- Cria uma instância do servidor na porta 80 (HTTP padrão) ---
WebServer server(80);

// --- CONFIGURE O FUSO HORÁRIO AQUI ---
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

  char buf[64];
  char buf2[80];

struct tm timeinfo;

int minutos;



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
const int LIMITE_DIST = 50; // Distância mínima em cm para detecção

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
int contarPessoas = 1;
float temperatura;
bool arCondicionado = false;


// --- Variáveis para controle de tempo (não-bloqueante) ---
unsigned long previousMillisDetect = 0;
unsigned long previousMillisDHT = 0;
const long intervalDetect = 250; // Intervalo para checar os sensores de distância
const long intervalDHT = 2000;   // Intervalo para ler o sensor DHT11
String estadoAlgoritmo;




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
int MultLCD = 4;  

String ipString;

// ===================================================
// ========== SETUP ==================================
// ===================================================
void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  Serial.println("Setup");
  
  tcaselect(MultLCD);
  displayLED.begin(); //Inicia o display OLED
  displayLED.clearDisplay(); //Limpa as mensagens do display
  delay(100);
  // --- inicialização do IR ------------------------------------------------------------------------------------------

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    #if defined(__AVR_ATmega32U4__) || defined(SERIAL_PORT_USBVIRTUAL) || defined(SERIAL_USB) /*stm32duino*/|| defined(USBCON) /*STM32_stm32*/ \
        || defined(SERIALUSB_PID)  || defined(ARDUINO_ARCH_RP2040) || defined(ARDUINO_attiny3217)
        delay(4000); // To be able to connect Serial monitor after reset or power up and before first print out. Do not wait for an attached Serial Monitor!
    #endif
    // IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
    IrReceiver.start();

  // --- Inicialização dos sensores e pinos --------------------------------------------------------------------------
  dht.begin();

  delay(50);

  display("Iniciando sensores via TCA9548A...");
  delay(500);
  // Sensor A - Canal 0
  tcaselect(MultSensorA);
  if (!loxA.begin()) {
    display("Falha ao iniciar Sensor A");
    Serial.println("Falha ao iniciar Sensor A");
    // while (1);
  } else {
      display("Sensor A OK");
      Serial.println("Sensor A OK");
    }

  delay(1000);

  // Sensor B - Canal 1
  tcaselect(MultSensorB);
  if (!loxB.begin()) {
    display("Falha ao iniciar Sensor B");
    Serial.println("Falha ao iniciar Sensor B");
    // while (1);
  }
  else {
    display("Sensor B OK");
    Serial.println("Sensor B OK");
  }
  delay(1000);
  // --- Conexão com o WiFi ---------------------------------------------------------------------------------------------------
  display("Conectando a ");
  Serial.println("Conectando a " + String(ssid));
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    String ponto = "." + ponto;
    display(ponto);
  }
  display("\nWiFi conectado!");
  IPAddress ip = WiFi.localIP();
  ipString = ip.toString();
  display("Endereco de IP: " + ipString);
  delay(1500);
  // Define as rotas do servidor
  server.on("/", handleRoot);
  server.on("/dados", handleDadosSensor);
  server.on("/controleAR", handleArControl);

  // Inicia o servidor
  server.begin();
  display("Servidor HTTP iniciado!");

  delay(1000);

//======================================================HORARIO====================================================================

  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");

  display("Sincronizando horário com NTP...");
  delay(500);
  // espera até sincronizar (ou timeout)
  struct tm timeinfo;
  int attempts = 0;
  while (!getLocalTime(&timeinfo) && attempts < 30) { // espera até ~15s
    display(".");
    delay(500);
    attempts++;
  }
  
  if (getLocalTime(&timeinfo)) {
    display("Horario sincronizado!");
    printLocalTime();
  } else {
    display("Não foi possível sincronizar horário via NTP.");
  }

  delay(500);
  
  display("calibre o controle");

  receptor();
  while (receptor() == false);
  
  
  display("IP: " + ipString);
  Serial.println("iniciando sistema"); 
  
  delay(500);


//================================================================================================================================

}


// ===================================================
// ========== LOOP PRINCIPAL =========================
// ===================================================


void loop() {
  // Esta função verifica se algum cliente fez uma requisição ao servidor
  server.handleClient();
  display("IP: " + ipString + "   | pessoas: " + String(contarPessoas));
  algoritmo();
}