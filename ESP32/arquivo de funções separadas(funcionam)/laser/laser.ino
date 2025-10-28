#include <Wire.h>
#include "Adafruit_VL53L0X.h"

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

// Instâncias dos sensores
Adafruit_VL53L0X loxA = Adafruit_VL53L0X();
Adafruit_VL53L0X loxB = Adafruit_VL53L0X();

// Distância limite para detecção (em mm)
const int LIMITE_DIST = 800;

bool detectA = false;
bool detectB = false;
unsigned long tempoA = 0;
unsigned long tempoB = 0;
int contadorDentro = 0;

void setup() {
  Serial.begin(115200);
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

  Serial.println("Sistema pronto!");
}

void loop() {
  VL53L0X_RangingMeasurementData_t measureA;
  VL53L0X_RangingMeasurementData_t measureB;

  // Variável para lembrar quem detectou primeiro
  static char primeiro = 0;  // 'A', 'B' ou 0 (nenhum)

  // Leitura dos sensores
  tcaselect(MultSensorA);
  loxA.rangingTest(&measureA, false);
  long distA = (measureA.RangeStatus != 4) ? measureA.RangeMilliMeter / 10 : -1;

  tcaselect(MultSensorB);
  loxB.rangingTest(&measureB, false);
  long distB = (measureB.RangeStatus != 4) ? measureB.RangeMilliMeter / 10 : -1;

  Serial.print(String(distA) + "    " + String(distB));

  // --- DETECÇÃO ---

  // Sensor A detecta
  if (distA > 0 && distA < LIMITE_DIST && !detectA) {
    detectA = true;
    Serial.println(" -> Sensor A detectou!");
    if (primeiro == 0) primeiro = 'A'; // marca quem detectou primeiro
  }

  // Sensor B detecta
  if (distB > 0 && distB < LIMITE_DIST && !detectB) {
    detectB = true;
    Serial.println(" -> Sensor B detectou!");
    if (primeiro == 0) primeiro = 'B'; // marca quem detectou primeiro
  }

  // --- AVALIAÇÃO DE PASSAGEM ---

  if (detectA && detectB) {
    if (primeiro == 'A') {
      contadorDentro++;
      Serial.print("ENTRADA detectada | Total: ");
      Serial.println(contadorDentro);
    } else if (primeiro == 'B') {
      contadorDentro = max(0, contadorDentro - 1);
      Serial.print("SAÍDA detectada | Total: ");
      Serial.println(contadorDentro);
    }

    // Reseta o ciclo
    detectA = false;
    detectB = false;
    primeiro = 0;
    delay(750); // evita múltiplos contadores na mesma passagem
  }

  // --- LIMPEZA CASO TRAVE ---
  static unsigned long ultimaLeitura = millis();
  if (millis() - ultimaLeitura > 500) {
    detectA = false;
    detectB = false;
    primeiro = 0;
  }
  ultimaLeitura = millis();

  Serial.print("   | Total: ");
  Serial.println(contadorDentro);
  delay(20);
}
