#include "DHT.h"

// Definições
#define DHTPIN 2        // pino DATA do UC-121 conectado ao digital 2
#define DHTTYPE DHT11   // ou DHT22 se for esse tipo

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
}

void loop() {
  // Espera o tempo mínimo entre leituras do DHT11 (≈ 2s)
  delay(2000);

  float tempC = dht.readTemperature();     // temperatura em Celsius
  float tempF = dht.readTemperature(true); // temperatura em Fahrenheit
  float hum   = dht.readHumidity();        // umidade relativa

  // Verifica se leitura deu certo:
  if (isnan(tempC) || isnan(hum)) {
    Serial.println("Falha ao ler do sensor UC-121 / DHT11!");
    return;
  }

  // Exibe os valores
  Serial.print("Temperatura: ");
  Serial.print(tempC);
  Serial.print(" °C  |  ");
  Serial.print(tempF);
  Serial.print(" °F ");
  Serial.print(" | Umidade: ");
  Serial.print(hum);
  Serial.println(" %");

  // Pode colocar outro delay se quiser
}
