#include "DHT.h"

// Defina o pino do sinal do DHT11
#define DHTPIN 18

// Escolha o tipo de sensor: DHT11 ou DHT22
#define DHTTYPE DHT11

// Cria um objeto DHT
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando leitura do sensor DHT11...");
  
  dht.begin();
}

void loop() {
  // Aguarda um pouco entre as medições
  delay(200);

  // Lê a umidade
  float h = dht.readHumidity();
  // Lê a temperatura em Celsius
  float t = dht.readTemperature();
  // Lê a temperatura em Fahrenheit (opcional)
  float f = dht.readTemperature(true);

  // Verifica se houve falha na leitura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Falha ao ler o sensor DHT11!");
    return;
  }

  // Mostra os valores no monitor serial
  Serial.println("=================================");
  Serial.print("Umidade: ");
  Serial.print(h);
  Serial.println(" %");

  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.println(" °C");

  Serial.print("Temperatura: ");
  Serial.print(f);
  Serial.println(" °F");
}
