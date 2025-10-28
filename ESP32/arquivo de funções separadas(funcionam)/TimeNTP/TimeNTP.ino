// ESP32 - obter horário de Brasília (BRT / UTC-3) via NTP
// Compile com framework Arduino para ESP32

#include <WiFi.h>
#include <time.h>

const char* ssid     = "Dionizio";
const char* password = "09041907";

// Offset de Brasília: UTC-3 -> -3 * 3600 segundos
const long  gmtOffset_sec = -3 * 3600;
const int   daylightOffset_sec = 0; // Brasil atualmente sem DST

void printLocalTime(){
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Falha ao obter hora local");
    return;
  }

  // Formatos comuns:
  char buf[64];
  // Ex: 2025-10-21 14:30:15
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.print("Hora local (Brasília): ");
  Serial.println(buf);

  // Exemplo com nome do dia e fuso
  char buf2[80];
  strftime(buf2, sizeof(buf2), "%A, %d %B %Y %H:%M:%S", &timeinfo);
  Serial.println(buf2);
}

void connectWiFi(){
  Serial.printf("Conectando em %s ", ssid);
  WiFi.begin(ssid, password);
  int counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if(counter > 60){ // timeout ~30s
      Serial.println("\nNão conseguiu conectar ao WiFi. Reinicie ou verifique credenciais.");
      return;
    }
  }
  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

void setup(){
  Serial.begin(115200);
  delay(1000);

  connectWiFi();

  // Opcional: setar variável TZ (nem sempre necessária/efetiva em todas builds)
  // setenv("TZ", "GMT-3", 1);
  // tzset();

  // Inicializa NTP com offsets. Usa servidores NTP públicos.
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
}

void loop(){
  // Atualiza e imprime a cada 10 segundos
  printLocalTime();
  delay(100);
}
