#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char ssid[] = "Laica-IoT";  //  your network SSID (name)
const char pass[] = "Laica321";       // your network password

static const char ntpServerName[] = "pool.ntp.br";   // Servidor do horário

const int timeZone = -3;     // Fuso horário nosso


WiFiUDP Udp;
unsigned int localPort = 8888;

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void setup()
{
  Serial.begin(9600);
  while (!Serial) ; 
  delay(250);
  Serial.println("TimeNTP Example");
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

time_t prevDisplay = 0; 

void loop()
{
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { 
      prevDisplay = now();
      digitalClockDisplay();
    }
  }
}




