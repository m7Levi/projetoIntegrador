#include <WiFi.h>
#include <HTTPClient.h>

// ===== CONFIGURAÇÕES DE REDE =====
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

// ===== COORDENADAS (MODIFIQUE AQUI) =====
// Exemplo: Natal/RN -> latitude -5.795, longitude -35.2094
float latitude = -5.795;
float longitude = -35.2094;
// ========================================

// Função que busca a temperatura atual no Open-Meteo
float pegarTemperatura(float lat, float lon) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Monta a URL com base nas coordenadas
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(lat, 4) +
                 "&longitude=" + String(lon, 4) + "&current_weather=true";

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == 200) {
      String payload = http.getString();
      int tempIndex = payload.indexOf("\"temperature\":");
      if (tempIndex != -1) {
        int start = tempIndex + 14; // pula "temperature":
        int end = payload.indexOf(",", start);
        String tempStr = payload.substring(start, end);
        float temperatura = tempStr.toFloat();
        http.end();
        return temperatura;
      }
    }

    http.end();
  }

  return NAN; // Retorna NaN se der erro
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ Conectado!");
}

void loop() {
  float temperatura = pegarTemperatura(latitude, longitude);

  if (!isnan(temperatura)) {
    Serial.print("🌡️ Temperatura atual: ");
    Serial.print(temperatura);
    Serial.println(" °C");
  } else {
    Serial.println("❌ Erro ao obter temperatura!");
  }

  delay(60000); // Atualiza a cada 60 segundos
}
