import requests
import time

ESP32_IP = "192.168.1.109"  # 🔹 Substitua pelo IP do seu ESP32
URL_controleAR = f"http://{ESP32_IP}/controleAR"
URL_DADOS = f"http://{ESP32_IP}/dados"

def ligar_controleAR():
    r = requests.get(URL_controleAR, params={"estado": "on"})
    print("Resposta:", r.text)

def desligar_controleAR():
    r = requests.get(URL_controleAR, params={"estado": "off"})
    print("Resposta:", r.text)

def ler_dados():
    r = requests.get(URL_DADOS)
    print("Dados:", r.json())

# --- Exemplo de uso ---
while True:
    print("\n1️⃣ Ligando controleAR...")
    ligar_controleAR()
    time.sleep(2)

    print("\n2️⃣ Lendo sensores...")
    ler_dados()
    time.sleep(2)

    print("\n3️⃣ Desligando controleAR...")
    desligar_controleAR()
    time.sleep(2)
