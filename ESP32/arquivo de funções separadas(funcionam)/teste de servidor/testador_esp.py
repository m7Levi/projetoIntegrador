import requests
import json
import time

# --- CONFIGURAÇÕES ---
ESP32_IP = "10.132.147.199"  # 🔹 Substitua pelo IP mostrado no Serial Monitor
URL = f"http://{ESP32_IP}/dados"

# --- LOOP PRINCIPAL ---
while True:
    try:
        # Faz a requisição HTTP GET
        response = requests.get(URL, timeout=5)

        # Verifica se a resposta é válida
        if response.status_code == 200:
            dados = response.json()  # Converte JSON em dicionário Python

            # Mostra os dados formatados
            print("\n Dados recebidos do ESP32:")
            print(f" Temperatura: {dados['temperatura']} °C")
            print(f" pessoas: {dados['pessoas']}")
            print(f" horario: {dados['horario']}")

        else:
            print(f"Erro: Código HTTP {response.status_code}")

    except requests.exceptions.RequestException as e:
        print(f"Falha na conexão: {e}")

    # Aguarda 5 segundos antes de buscar novamente
    time.sleep(1)
