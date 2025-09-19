// Pinos do Sensor Ultrassônico A
const int trigA = 9;
const int echoA = 10;

// Pinos do Sensor Ultrassônico B
const int trigB = 11;
const int echoB = 12;

// Distância mínima para considerar que há alguém (em cm)
const int limiteDistancia = 50;  

// Variáveis de estado
bool detectA = false;
bool detectB = false;

unsigned long tempoA = 0;
unsigned long tempoB = 0;

void setup() {
  Serial.begin(9600);

  pinMode(trigA, OUTPUT);
  pinMode(echoA, INPUT);

  pinMode(trigB, OUTPUT);
  pinMode(echoB, INPUT);

  Serial.println("Sistema iniciado...");
}

void loop() {
  long distanciaA = lerUltrassom(trigA, echoA);
  long distanciaB = lerUltrassom(trigB, echoB);


  Serial.print(distanciaA);
  Serial.print("     ");
  Serial.println(distanciaB);
  // Verifica se alguém passou no sensor A
  if (distanciaA > 0 && distanciaA < limiteDistancia && !detectA) {
    detectA = true;
    tempoA = millis();
  }

  // Verifica se alguém passou no sensor B
  if (distanciaB > 0 && distanciaB < limiteDistancia && !detectB) {
    detectB = true;
    tempoB = millis();
  }

  // Analisa a ordem dos sensores
  if (detectA && detectB) {
    if (tempoA < tempoB) {
      Serial.println("Pessoa entrou.");
    } else {
      Serial.println("Pessoa saiu.");
    }

    // Reseta estados
    detectA = false;
    detectB = false;
  }

  // Reseta caso fique travado muito tempo
  if ((detectA || detectB) && (millis() - max(tempoA, tempoB) > 2000)) {
    detectA = false;
    detectB = false;
  }

  delay(50);
}

// Função para medir distância com ultrassônico
long lerUltrassom(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duracao = pulseIn(echoPin, HIGH, 20000L); // timeout de 20ms (~3m)
  if (duracao == 0) return -1; // sem leitura

  long distancia = duracao * 0.034 / 2;
  return distancia;
}
