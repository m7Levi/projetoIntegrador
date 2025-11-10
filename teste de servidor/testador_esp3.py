import requests
import time

temperatura = ""
pessoas = "" 
horario = "" 
temperaturaCidade = "" # precisa ser criado
estado = "" # preciso verificar no codigo original


ESP32_IP = "192.168.1.109"  # 🔹 Substitua pelo IP do seu ESP32
URL_controleAR = f"http://{ESP32_IP}/controleAR"
URL_DADOS = f"http://{ESP32_IP}/dados"

def ligar_controleAR():
    response = requests.get(URL_controleAR, params={"estado": "on"})
    print("Resposta:", r.text)

def desligar_controleAR():
    response = requests.get(URL_controleAR, params={"estado": "off"})
    print("Resposta:", r.text)


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
            
            global temperatura = dados.get('temperatura')
            global pessoas = dados.get('pessoas')
            global horario = dados.get('horario')
            global temperaturaCidade = dados.get('temperaturaCidade')
            global estado = dados.get('estado') #essa eu preciso verificar no codigo original
            
            print("Dados recebidos com sucesso!")
            print(f"  Temperatura: {temperatura} °C")
            print(f"  horario: {horario}")
            print(f"  pessoas: {pessoas}")
            print(f"  pessoas: {temperaturaCidade}")
            print(f"  estado ar condicionado: {estado}")
            
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
        

        #Atribuindo os valores do esp no python

        if dados_sensores:
            # Aqui você atualizaria a sua interface gráfica (Labels, botões, etc.)
            # Ex: meu_label_temp.text = f"{dados_sensores['temperatura']} °C"
            pass
            
        time.sleep(1)