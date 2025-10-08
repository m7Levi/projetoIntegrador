// ===================================================
// ========== BIBLIOTECAS E INCLUDES GERAIS ==========
// ===================================================
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

// ===================================================
// ========== CONFIGURAÇÕES DO PROJETO ===============
// ===================================================

// --- CONFIGURE A REDE WIFI AQUI ---
const char* ssid = "Dionizio_2G";
const char* password = "88114857";

// --- PINOS (NodeMCU) ---
#define LEDPIN 0       // LED IR emissor
#define IR_RECV_PIN 4  // Pino para receptor IR
#define TRIG_A 14      // Trigger sensor ultrassônico A
#define ECHO_A 12      // Echo sensor ultrassônico A
#define TRIG_B 13       // Trigger sensor ultrassônico B
#define ECHO_B 15       // Echo sensor ultrassônico B
#define DHTPIN 5       // Pino do sensor DHT11

// --- CONSTANTES ---
#define DHTTYPE DHT11        // Define o tipo do sensor DHT
#define maxLen 650           // Tamanho máximo do buffer para o receptor IR
const int limiteDistancia = 15; // Distância mínima em cm para detecção
int khz = 38;                // Frequência do emissor IR (NEC)

// Dados brutos para o sinal IR (NEC)
const uint16_t rawData[] = {
  9000,4500,560,560,560,560,560,1690,560,
  560,560,560,560,560,560,560,560,560,1690,560,1690,560,560,
  560,1690,560,1690,560,1690,560,1690,560,560,560,560,
  560,560,560,1690,560,1690,560,560,560,560,560,560,
  560,1690,560,1690,560,1690,560,560,560,560,560,1690,560,
  1690,560,1690,560
};

// ===================================================
// ========== OBJETOS E VARIÁVEIS GLOBAIS ==========
// ===================================================

// --- Servidor Web ---
ESP8266WebServer server(80);

// --- Sensores ---
DHT dht(DHTPIN, DHTTYPE);
IRsend irsend(LEDPIN);

// --- Variáveis de estado para sensores ---
bool detectA = false;
bool detectB = false;
unsigned long tempoA = 0;
unsigned long tempoB = 0;
int contarPessoas = 0;
float temperaturaGlobal = 0.0;
float umidadeGlobal = 0.0;

// --- Buffer e variáveis para o receptor IR ---
volatile unsigned long irBuffer[maxLen];
volatile int x = 0;

// --- Variáveis para controle de tempo (não-bloqueante) ---
unsigned long previousMillisDetect = 0;
unsigned long previousMillisDHT = 0;
const long intervalDetect = 250; // Intervalo para checar os sensores de distância
const long intervalDHT = 2000;   // Intervalo para ler o sensor DHT11

// ===================================================
// ========== PROTÓTIPOS DAS FUNÇÕES ===============
// ===================================================
void ICACHE_RAM_ATTR rxIR_Interrupt_Handler(); // <<-- LINHA ADICIONADA PARA CORRIGIR O ERRO

// ===================================================
// ========== FUNÇÕES DO SERVIDOR WEB ================
// ===================================================

// Função para a página raiz, para testar no navegador
void handleRoot() {
  server.send(200, "text/plain", "Servidor do ESP8266 esta no ar! Acesse /dados para ver as leituras.");
}

// Função que será chamada quando o app acessar o endpoint "/dados"
void handleDadosSensor() {
  // Crie um objeto JSON para enviar como resposta
  StaticJsonDocument<256> jsonDocument;
  jsonDocument["temperatura"] = temperaturaGlobal;
  jsonDocument["umidade"] = umidadeGlobal;
  jsonDocument["pessoas"] = contarPessoas;
  //jsonDocument["Namorada mais linda do mundo"] = "Thabatha";

  String jsonString;
  serializeJson(jsonDocument, jsonString);

  // Envia a resposta (código 200 = OK) com o conteúdo JSON
  server.send(200, "application/json", jsonString);
}


// ===================================================
// ========== SETUP ==================================
// ===================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\nSistema iniciado no ESP8266...");

  // --- Inicialização dos sensores e pinos ---
dht.begin();
 // irsend.begin();
  pinMode(14, OUTPUT);
  pinMode(12, INPUT);
  pinMode(13, OUTPUT);
  pinMode(15, INPUT);
  Serial.println("chegou");

  // Ativa interrupção no pino receptor IR
  //attachInterrupt(digitalPinToInterrupt(IR_RECV_PIN), rxIR_Interrupt_Handler, CHANGE);

  // --- Conexão com o WiFi ---
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

  // --- Configuração das rotas do servidor ---
  server.on("/", handleRoot);
  server.on("/dados", handleDadosSensor);

  // --- Inicia o servidor ---
  server.begin();
  Serial.println("Servidor HTTP iniciado");
}


// ===================================================
// ========== LOOP PRINCIPAL =========================
// ===================================================
void loop() {
  // Esta função verifica se algum cliente fez uma requisição ao servidor
  server.handleClient();

  unsigned long currentMillis = millis();

  // --- Tarefas que rodam em segundo plano (não-bloqueante) ---

  // 1. Atualiza a contagem de pessoas
  if (currentMillis - previousMillisDetect >= intervalDetect) {
    previousMillisDetect = currentMillis;
    detectarPessoas();
  }

  // 2. Atualiza a leitura de temperatura e umidade
  if (currentMillis - previousMillisDHT >= intervalDHT) {
    previousMillisDHT = currentMillis;
    atualizarSensorTemperatura();
  }

  // --- Comandos via monitor serial (para debug e testes) ---
  if (Serial.available() > 0) {
    char caractereRecebido = Serial.read();

    if (caractereRecebido == '1') {
      Serial.println("Modo: emissor IR (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        emissor();
      }
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '2') {
      Serial.println("Modo: receptor IR (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        receber_sinal();
        delay(100);
      }
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '3') {
      Serial.println("Modo: Debug detectar pessoas (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        detectarPessoas();
      }
      Serial.println("Comando fechado");
    }

    if (caractereRecebido == '4') {
      Serial.println("Modo: Debug temperatura (digite '0' para sair)");
      while (true) {
        if (Serial.available() && Serial.read() == '0') break;
        atualizarSensorTemperatura();
      }
      Serial.println("Comando fechado");
    }
  }
}

// ===================================================
// ========== FUNÇÕES DOS SENSORES ===================
// ===================================================

void detectarPessoas() {
  long distanciaA = lerUltrassom(TRIG_A, ECHO_A);
  long distanciaB = lerUltrassom(TRIG_B, ECHO_B);

  // Imprime no serial para debug
  // Serial.printf("A: %ld cm | B: %ld cm | Pessoas: %d\n", distanciaA, distanciaB, contarPessoas);

  if (distanciaA > 0 && distanciaA < limiteDistancia && !detectA) {
    detectA = true;
    tempoA = millis();
  }
  if (distanciaB > 0 && distanciaB < limiteDistancia && !detectB) {
    detectB = true;
    tempoB = millis();
  }

  if (detectA && detectB) {
    if (tempoA < tempoB) {
      contarPessoas++;
      Serial.println("Pessoa entrou.");
    } else {
      contarPessoas--;
      Serial.println("Pessoa saiu.");
    }
    detectA = detectB = false;
    delay(200); // Pequeno delay para evitar contagem dupla imediata
  }

  // Timeout para resetar a detecção se apenas um sensor for ativado
  unsigned long lastTempo = max(tempoA, tempoB);
  if ((detectA || detectB) && (millis() - lastTempo > 2000)) {
    detectA = detectB = false;
  }

  if (contarPessoas < 0) contarPessoas = 0;
}

long lerUltrassom(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 20000L); // Timeout 20 ms
  if (duracao == 0) return -1;
  long distancia = duracao * 0.034 / 2;
  return distancia;
}

void atualizarSensorTemperatura() {
  float tempC = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(tempC) || isnan(hum)) {
    Serial.println("Falha ao ler DHT11!");
    return;
  }

  temperaturaGlobal = tempC;
  umidadeGlobal = hum;
  
  // Imprime no serial para debug
 // Serial.printf("Temperatura: %.1f °C | Umidade: %.1f %% | Pessoas: %d\n", temperaturaGlobal, umidadeGlobal, contarPessoas);
}

// ===================================================
// ========== FUNÇÕES DE INFRAVERMELHO ===============
// ===================================================

void emissor() {
  Serial.println(F("Enviando sinal IR (NEC)..."));
  irsend.sendRaw(rawData, sizeof(rawData) / sizeof(rawData[0]), khz);
  delay(1000);
}

void receber_sinal() {
  if (x > 0) {
    digitalWrite(LED_BUILTIN, LOW); // LED acende
    Serial.printf("\nRaw: (%d) ", x - 1);
    detachInterrupt(digitalPinToInterrupt(IR_RECV_PIN));
    for (int i = 1; i < x; i++) {
      unsigned long delta = irBuffer[i] - irBuffer[i - 1];
      Serial.print(delta);
      if (i < x - 1) Serial.print(", ");
    }
    Serial.println();
    x = 0;
    digitalWrite(LED_BUILTIN, HIGH); // LED apaga
    attachInterrupt(digitalPinToInterrupt(IR_RECV_PIN), rxIR_Interrupt_Handler, CHANGE);
  }
}

// ISR de recepção IR (deve ser rápida e estar na IRAM)
void ICACHE_RAM_ATTR rxIR_Interrupt_Handler() {
  if (x >= maxLen) return;
  irBuffer[x++] = micros();
}