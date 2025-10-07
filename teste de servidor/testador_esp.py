import requests
import json
import time

# --- COLOQUE AQUI O IP DO SEU ESP8266 ---
# Este IP muda dependendo da sua rede e a cada reconexão do ESP
# Você o pega no Monitor Serial da Arduino IDE
ESP_IP = "192.168.3.18" 
URL_DADOS = f"http://{ESP_IP}/dados"
# ------------------------------------------

def buscar_dados_do_esp():
    """
    Faz uma requisição HTTP para o ESP8266 e retorna os dados dos sensores.
    """
    try:
        # O timeout é importante para não travar seu app se o ESP estiver offline
        response = requests.get(URL_DADOS, timeout=5)
        
        # Verifica se a requisição foi bem sucedida (código 200)
        if response.status_code == 200:
            dados = response.json() # A biblioteca requests já converte JSON para um dicionário Python
            print("Dados recebidos com sucesso!")
            print(f"  Temperatura: {dados.get('temperatura')} °C")
            print(f"  Umidade: {dados.get('umidade')} %")
            print(f"  QUEM É A NAMORADA MAIS LINDA DE TODO O MUNDO: {dados.get('Namorada mais linda do mundo')}")
            return dados
        else:
            print(f"Erro ao contatar o ESP. Status Code: {response.status_code}")
            return None

    except requests.exceptions.RequestException as e:
        print(f"Erro de conexão com o ESP8266. Verifique o IP e a conexão Wi-Fi.")
        print(f"Detalhe do erro: {e}")
        return None

# Exemplo de como você usaria isso em um loop no seu app
if __name__ == "__main__":
    while True:
        print("\nBuscando novos dados...")
        dados_sensores = buscar_dados_do_esp()
        
        if dados_sensores:
            # Aqui você atualizaria a sua interface gráfica (Labels, botões, etc.)
            # Ex: meu_label_temp.text = f"{dados_sensores['temperatura']} °C"
            pass
            
        time.sleep(1)