# --- CONFIGURAÇÃO INICIAL E IMPORTAÇÕES ---
# ⚠️ ESTAS CONFIGURAÇÕES DEVEM VIR ANTES DE QUALQUER IMPORT DO KIVY!
from kivy.config import Config

# Desativa redimensionamento (opcional para mobile)
Config.set('graphics', 'resizable', '0')
Config.set('kivy', 'desktop', '0')  # força modo mobile
# NÃO fixamos width/height nem desativamos DPI — deixamos o Kivy escalar naturalmente
Config.set('graphics', 'fullscreen', 'auto')

# --- IMPORTS ---
import kivy
kivy.require('2.0.0')

from kivy.app import App
from kivy.metrics import dp
from kivy.uix.screenmanager import ScreenManager, Screen
from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.gridlayout import GridLayout
from kivy.uix.image import Image
from kivy.uix.label import Label
from kivy.uix.widget import Widget
from kivy.uix.textinput import TextInput
from kivy.uix.button import Button
from kivy.uix.scrollview import ScrollView
from kivy.uix.behaviors import ButtonBehavior
from kivy.graphics import Color, RoundedRectangle, Line
from kivy.clock import Clock
from kivy.utils import get_color_from_hex
from kivy.core.text import LabelBase
from kivy.core.window import Window

import requests

# --- CONFIGURAÇÃO DO TECLADO MOBILE ---
Window.softinput_mode = 'below_target'

# --- REGISTRO DAS FONTES ---
LabelBase.register(name='Futura', 
                   fn_regular='fonts/HFUNP-Regular.TTF',
                   fn_bold='fonts/HFUNB-Bold.TTF')
LabelBase.register(name='FuturaNegrito', fn_regular='fonts/futura_negrito.otf')
LabelBase.register(name='OpenSans', fn_regular='fonts/OpenSans-Bold.ttf')


# --- VARIÁVEIS GLOBAIS DO ESP32 ---
ESP32_IP = None         # Será definido na tela de login
estado_ar = "off"       # Estado controlado pelo app ("on" / "off")


# --- FUNÇÕES DE COMUNICAÇÃO COM O ESP32 ---
def buscar_dados_do_esp():
    """
    Retorna um dicionário com os dados do ESP (ou None em caso de falha).
    Endpoint esperado: http://<ESP32_IP>/dados
    JSON esperado (exemplo): {"temperatura": 26.4, "pessoas": 2, "horario": "14:22", "temperaturaCidade": 28, "estado": "on"}
    """
    global ESP32_IP
    if not ESP32_IP:
        # IP não definido ainda
        return None

    url = f"http://{ESP32_IP}/dados"
    try:
        resp = requests.get(url, timeout=3)  # timeout curto para não travar a UI por muito tempo
        if resp.status_code == 200:
            try:
                dados = resp.json()
            except ValueError:
                print("Resposta do ESP não é JSON válido.")
                return None
            return dados
        else:
            print(f"Erro HTTP ao buscar dados do ESP: {resp.status_code}")
            return None
    except requests.exceptions.RequestException as e:
        print("Erro de conexão ao buscar dados do ESP:", e)
        return None


def enviar_estado_para_esp(novo_estado):
    """
    Envia o comando para o ESP (/controleAR?estado=on|off).
    O ESP responde apenas com status 200 quando bem sucedido.
    """
    global ESP32_IP
    if not ESP32_IP:
        print("ESP32_IP não definido — não é possível enviar comando.")
        return False

    url = f"http://{ESP32_IP}/controleAR"
    try:
        resp = requests.get(url, params={"estado": novo_estado}, timeout=3)
        if resp.status_code == 200:
            print(f"Comando enviado com sucesso: {novo_estado}")
            return True
        else:
            print(f"Falha ao enviar comando. HTTP {resp.status_code}")
            return False
    except requests.exceptions.RequestException as e:
        print("Erro ao enviar comando para o ESP:", e)
        return False


# --- WIDGET PERSONALIZADO PARA A BARRA DE PROGRESSO ---
class CustomProgressBar(Widget):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.progress = 0
        with self.canvas:
            Color(rgb=get_color_from_hex('#f6f6f6'))
            self.bg_rect = RoundedRectangle(pos=self.pos, size=self.size, radius=[dp(5)])
            Color(rgb=get_color_from_hex('#15213d'))
            self.progress_rect = RoundedRectangle(pos=self.pos, size=(0, self.height), radius=[dp(5)])
        self.bind(pos=self.update_rect, size=self.update_rect)

    def update_rect(self, *args):
        self.bg_rect.pos = self.pos
        self.bg_rect.size = self.size
        self.progress_rect.pos = self.pos
        self.set_progress(self.progress)

    def set_progress(self, value):
        self.progress = value
        progress_width = self.width * (self.progress / 100.0) if self.width else 0
        self.progress_rect.size = (progress_width, self.height)


# --- WIDGET AUXILIAR PARA O OLHINHO E BOTOES DE IMAGEM ---
class ImageButton(ButtonBehavior, Image):
    pass


# --- TELA DE CARREGAMENTO ---
class LoadingScreen(Screen):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        layout_principal = FloatLayout()
        layout_principal.add_widget(Image(source='images/background.png', allow_stretch=True, keep_ratio=False))
        box_central = BoxLayout(
            orientation='vertical',
            size_hint=(0.5, 0.3),
            pos_hint={'center_x': 0.5, 'center_y': 0.55},
            spacing=dp(15)
        )
        box_central.add_widget(Image(source='images/logo.png'))
        self.progress_bar = CustomProgressBar(size_hint_y=None, height=dp(10))
        box_central.add_widget(self.progress_bar)
        layout_principal.add_widget(box_central)
        self.add_widget(layout_principal)


# --- TELA DE LOGIN ---
class LoginScreen(Screen):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        layout_fundo = FloatLayout()
        layout_fundo.add_widget(Image(source='images/background.png', allow_stretch=True, keep_ratio=False))
        
        scroll = ScrollView(size_hint=(1, 1))
        layout_principal = BoxLayout(
            orientation='vertical',
            padding=[dp(35), dp(20), dp(35), dp(20)],
            spacing=dp(15),
            size_hint_y=None
        )
        layout_principal.bind(minimum_height=layout_principal.setter('height'))

        # Cabeçalho
        barra_superior = BoxLayout(size_hint_y=None, height=dp(50))
        label_bem_vindo = Label(
            markup=True,
            text='[color=#a2a2a2]Bem vindo ao AirFlow [/color][color=#15213d][b]- IFRN[/b][/color]',
            font_name='Futura',
            font_size='20sp',
            halign='left',
            valign='top'
        )
        label_bem_vindo.bind(size=label_bem_vindo.setter('text_size'))
        barra_superior.add_widget(label_bem_vindo)
        barra_superior.add_widget(Image(source='images/parte_logo.png', size_hint_x=None, width=dp(100)))
        layout_principal.add_widget(barra_superior)

        # Título
        layout_principal.add_widget(Label(
            text='Primeiro\nacesso',
            font_name='FuturaNegrito',
            font_size='60sp',
            color=get_color_from_hex('#292929'),
            size_hint_y=None,
            height=dp(120),
            line_height=0.8,
            halign='left',
            text_size=(dp(300), None)
        ))
        
        layout_principal.add_widget(Widget(size_hint_y=None, height=dp(20)))
        layout_principal.add_widget(Image(source='images/usuario.png', size_hint_y=None, height=dp(200)))
        layout_principal.add_widget(Widget(size_hint_y=None, height=dp(20)))

        # Campos
        layout_principal.add_widget(self.create_text_field("Qual o nome do seu comodo?", "Crie um apelidio para seu comodo ou sala"))
        layout_principal.add_widget(self.create_password_field("Digite o IP do seu ESP8266 (Inclua os pontos ao digitar)", "Digite o IP presente no seu ESP8266"))
        layout_principal.add_widget(Widget(size_hint_y=None, height=dp(10)))

        # Botão Verificar
        btn_verificar = Button(
            text='Verificar',
            font_name='OpenSans',
            size_hint=(1, None),
            height=dp(50),
            background_normal='',
            background_color=(0, 0, 0, 0)
        )
        with btn_verificar.canvas.before:
            Color(rgb=get_color_from_hex('#15213d'))
            self.btn_rect = RoundedRectangle(pos=btn_verificar.pos, size=btn_verificar.size, radius=[dp(25)])
        btn_verificar.bind(pos=self._update_button_rect, size=self._update_button_rect)
        btn_verificar.bind(on_press=self.verify_credentials)
        layout_principal.add_widget(btn_verificar)

        layout_principal.add_widget(Label(
            text='[u]Estou com dificuldades no login[/u]',
            markup=True,
            font_name='Futura',
            font_size='12sp',
            color=get_color_from_hex('#292929'),
            size_hint_y=None,
            height=dp(30)
        ))
        
        scroll.add_widget(layout_principal)
        layout_fundo.add_widget(scroll)
        self.add_widget(layout_fundo)

    def create_text_field(self, label_text, hint_text):
        container = BoxLayout(orientation='vertical', size_hint_y=None, height=dp(85), spacing=dp(8))
        container.add_widget(Label(
            text=label_text,
            font_name='Futura',
            font_size='15sp',
            color=get_color_from_hex('#292929'),
            halign='left',
            text_size=(dp(300), None)
        ))
        text_input = TextInput(
            hint_text=hint_text,
            multiline=False,
            size_hint_y=None,
            height=dp(48),
            background_normal='',
            background_color=(0, 0, 0, 0),
            padding=[dp(15), dp(15), dp(15), dp(15)],
            font_name='OpenSans'
        )
        with text_input.canvas.before:
            Color(rgb=get_color_from_hex('#d6d6d6'))
            text_input.bg_rect = RoundedRectangle(pos=text_input.pos, size=text_input.size, radius=[dp(24)])
            Color(rgb=get_color_from_hex("#a2a2a2"))
            text_input.line_rect = Line(width=1, rounded_rectangle=(text_input.x, text_input.y, text_input.width, text_input.height, dp(24)))
        text_input.bind(pos=self._update_textinput_rect, size=self._update_textinput_rect)
        container.add_widget(text_input)

        # 👇 Salva o TextInput do nome do cômodo
        if "nome do seu comodo" in label_text.lower():
            self.room_input = text_input

        return container

    def create_password_field(self, label_text, hint_text):
        container = BoxLayout(orientation='vertical', size_hint_y=None, height=dp(85), spacing=dp(8))
        container.add_widget(Label(
            text=label_text,
            font_name='Futura',
            font_size='15sp',
            color=get_color_from_hex('#292929'),
            halign='left',
            text_size=(dp(300), None)
        ))
        field_layout = FloatLayout(size_hint_y=None, height=dp(48))
        self.password_input = TextInput(
            hint_text=hint_text,
            password=True,
            multiline=False,
            size_hint=(1, 1),
            pos_hint={'center_x': 0.5, 'center_y': 0.5},
            background_normal='',
            background_color=(0, 0, 0, 0),
            padding=[dp(15), dp(15), dp(45), dp(15)],
            font_name='OpenSans'
        )
        with self.password_input.canvas.before:
            Color(rgb=get_color_from_hex('#d6d6d6'))
            self.password_input.bg_rect = RoundedRectangle(pos=self.password_input.pos, size=self.password_input.size, radius=[dp(24)])
            Color(rgb=get_color_from_hex('#a2a2a2'))
            self.password_input.line_rect = Line(width=1, rounded_rectangle=(self.password_input.x, self.password_input.y, self.password_input.width, self.password_input.height, dp(24)))
        self.password_input.bind(pos=self._update_textinput_rect, size=self._update_textinput_rect)
        
        self.eye_icon = ImageButton(
            source='images/olho_fechado.png',
            size_hint=(None, None),
            size=(dp(24), dp(24)),
            pos_hint={'right': 0.95, 'center_y': 0.5},
            on_press=self.toggle_password_visibility
        )
        field_layout.add_widget(self.password_input)
        field_layout.add_widget(self.eye_icon)
        container.add_widget(field_layout)
        return container
    
    def toggle_password_visibility(self, instance):
        self.password_input.password = not self.password_input.password
        self.eye_icon.source = 'images/olho_aberto.png' if not self.password_input.password else 'images/olho_fechado.png'

    def _update_button_rect(self, instance, value):
        self.btn_rect.pos = instance.pos
        self.btn_rect.size = instance.size
            
    def _update_textinput_rect(self, instance, value):
        instance.bg_rect.pos = instance.pos
        instance.bg_rect.size = instance.size
        instance.line_rect.rounded_rectangle = (instance.x, instance.y, instance.width, instance.height, dp(24))

    def verify_credentials(self, instance):
        # Pega o nome do cômodo do primeiro campo de texto
        room_name = self.room_input.text.strip() or "Este ambiente"
        # Passa o nome para a MainScreen
        main_screen = self.manager.get_screen('main')
        main_screen.update_room_name(room_name)

        # Define global ESP32_IP a partir do campo de IP (password_input)
        global ESP32_IP
        ip_text = self.password_input.text.strip()
        ESP32_IP = ip_text if ip_text else None
        print("ESP32_IP definido:", ESP32_IP)

        # Vai para a tela principal
        self.manager.current = 'main'


# --- TELA PRINCIPAL ---
class MainScreen(Screen):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        layout_fundo = FloatLayout()
        layout_fundo.add_widget(Image(source='images/background.png', allow_stretch=True, keep_ratio=False))
        layout_principal = BoxLayout(
            orientation='vertical',
            padding=[dp(30), dp(15), dp(30), dp(15)],
            spacing=dp(15)
        )

        # Barra superior
        barra_superior = BoxLayout(size_hint_y=None, height=dp(30))
        barra_superior.add_widget(Image(source='images/parte_logo.png', size_hint_x=None, width=dp(80)))
        barra_superior.add_widget(Widget())
        btn_projeto = Button(
            text='[u]O projeto[/u]',
            markup=True,
            font_name='Futura',
            font_size='14sp',
            color=get_color_from_hex('#15213d'),
            background_normal='',
            background_color=(0, 0, 0, 0),
            size_hint_x=None,
            width=dp(120)
        )
        btn_projeto.bind(on_press=self.open_project)
        barra_superior.add_widget(btn_projeto)
        layout_principal.add_widget(barra_superior)

        # Saudação
        layout_principal.add_widget(Label(
            text="Bem vindo(a)",
            font_name='FuturaNegrito',
            font_size='45sp',
            color=get_color_from_hex('#292929'),
            halign='left',
            valign='top',
            size_hint=(1, None),
            height=dp(50),
            text_size=(dp(300), None)
        ))
        
        # Cards
        layout_principal.add_widget(self.create_temperature_card(size_hint_y=0.65))
        layout_principal.add_widget(self.create_info_grid())

        layout_fundo.add_widget(layout_principal)
        self.add_widget(layout_fundo)

        # FLAG para controlar agendamento de updates (evita múltiplos agendamentos)
        self._updates_scheduled = False

    def on_pre_enter(self, *args):
        # Quando a tela principal for exibida, agenda as atualizações periódicas (uma vez)
        if not self._updates_scheduled:
            Clock.schedule_interval(self.update_data, 2)  # atualiza a cada 2 segundos
            self._updates_scheduled = True

    def open_project(self, instance):
        if self.manager:
            self.manager.current = 'project'

    def create_temperature_card(self, size_hint_y):
        card = BoxLayout(
            size_hint_y=size_hint_y,
            padding=[dp(50), dp(15), dp(15), dp(15)],
            orientation='horizontal'
        )
        with card.canvas.before:
            Color(rgb=get_color_from_hex('#c4c4c4'))
            card.rect = RoundedRectangle(pos=card.pos, size=card.size, radius=[dp(20)])
        card.bind(pos=self._update_card_rect, size=self._update_card_rect)

        text_section = BoxLayout(orientation='vertical', spacing=dp(5), size_hint_x=0.6)

        text_section.add_widget(Label(
            text="Voce esta [b]on-line[/b]",
            markup=True,
            font_name='Futura',
            font_size='15sp',
            color=get_color_from_hex('#5b5b5b'),
            halign='left',
            valign='top',
            size_hint_y=None,
            height=dp(25),
            text_size=(dp(200), None)
        ))
        
        self.room_name_label = Label(
            text="Sua sala esta com a temperatura de:",
            markup=True,
            font_name='Futura',
            font_size='15sp',
            color=get_color_from_hex('#5b5b5b'),
            halign='left',
            valign='top',
            size_hint_y=None,
            height=dp(25),
            text_size=(dp(200), None)
        )
        text_section.add_widget(self.room_name_label)

        # 👇 Aqui guardamos o label da temperatura como atributo da tela
        self.temperature_label = Label(
            text="---",
            font_name='Futura',
            font_size='90sp',
            color=get_color_from_hex('#292929'),
            halign='left',
            text_size=(dp(200), None)
        )
        text_section.add_widget(self.temperature_label)

        text_section.add_widget(Label(
            text="Máxima de hoje 28°\nMinima de hoje 23°",
            font_name='Futura',
            font_size='20sp',
            color=get_color_from_hex('#5b5b5b'),
            size_hint_y=None,
            height=dp(40),
            line_height=1,
            halign='left',
            text_size=(dp(200), None)
        ))
        card.add_widget(text_section)
        card.add_widget(Image(source='images/floco_de_neve.png', size_hint_x=0.4))
        return card

    def create_info_grid(self):
        spacing = dp(15)
        grid = GridLayout(cols=2, spacing=spacing, size_hint_y=None, height=dp(300))  # altura fixa relativa

        # Primeiro card (Pessoas) — criado manualmente para podermos atualizar o valor dinamicamente
        pessoas_card = BoxLayout(orientation='vertical', padding=dp(15), spacing=dp(5))
        with pessoas_card.canvas.before:
            Color(rgb=get_color_from_hex('#c4c4c4'))
            pessoas_card.rect = RoundedRectangle(pos=pessoas_card.pos, size=pessoas_card.size, radius=[dp(20)])
        pessoas_card.bind(pos=self._update_card_rect, size=self._update_card_rect)

        pessoas_card.add_widget(Label(
            text='Tem aproximadamente',
            font_name='Futura',
            font_size='14sp',
            color=get_color_from_hex('#5b5b5b'),
            halign='center',
            valign='middle',
            text_size=(dp(1500), None),
            size_hint_y=None,
            height=dp(30)
        ))

        # Label dinâmico com o número de pessoas (atributo da tela)
        self.people_count_label = Label(
            text='--',
            font_name='Futura',
            font_size='80sp',
            color=get_color_from_hex('#292929'),
            halign='center',
            valign='middle',
            size_hint_y=None,
            height=dp(70)
        )
        pessoas_card.add_widget(self.people_count_label)

        pessoas_card.add_widget(Label(
            text='pessoas na sala',
            font_name='Futura',
            font_size='14sp',
            color=get_color_from_hex('#5b5b5b'),
            halign='center',
            valign='top',
            text_size=(dp(150), None),
            size_hint_y=None,
            height=dp(15)
        ))

        grid.add_widget(pessoas_card)

        # Card de status (liga/desliga)
        grid.add_widget(self.DesligadoCard())

        # Card próximo desligamento (mantido igual)
        grid.add_widget(self.NextShutdownCard())

        # Card economia (mantido igual)
        grid.add_widget(self.InfoCard('Economia de', '15%', 'em relacao\nao ultimo mes'))

        return grid

    def _update_card_rect(self, instance, value):
        instance.rect.pos = instance.pos
        instance.rect.size = instance.size

    def InfoCard(self, sub_text_top='', main_text='', sub_text_bottom=''):
        card = BoxLayout(orientation='vertical', padding=dp(15), spacing=dp(5))
        with card.canvas.before:
            Color(rgb=get_color_from_hex('#c4c4c4'))
            card.rect = RoundedRectangle(pos=card.pos, size=card.size, radius=[dp(20)])
        card.bind(pos=self._update_card_rect, size=self._update_card_rect)
        
        if sub_text_top:
            card.add_widget(Label(
                text=sub_text_top,
                font_name='Futura',
                font_size='14sp',
                color=get_color_from_hex('#5b5b5b'),
                halign='center',
                valign='middle',
                text_size=(dp(1500), None),
                size_hint_y=None,
                height=dp(30)
            ))
        else:
            card.add_widget(Widget(size_hint_y=None, height=dp(20)))

        card.add_widget(Label(
            text=main_text,
            font_name='Futura',
            font_size='80sp',
            color=get_color_from_hex('#292929'),
            halign='center',
            valign='middle',
            size_hint_y=None,
            height=dp(70)
        ))
        
        if sub_text_bottom:
            card.add_widget(Label(
                text=sub_text_bottom,
                font_name='Futura',
                font_size='14sp',
                color=get_color_from_hex('#5b5b5b'),
                halign='center',
                valign='top',
                text_size=(dp(150), None),
                size_hint_y=None,
                height=dp(15)
            ))
        else:
            card.add_widget(Widget(size_hint_y=None, height=dp(20)))
        return card
    
    def update_room_name(self, name):
        # Atualiza o label de temperatura com o nome do cômodo
        self.room_name_label.text = f"{name} esta com a temperatura de:"

    def DesligadoCard(self):
        # Criamos um card personalizado que controla seu próprio estado
        card = BoxLayout(orientation='vertical', padding=dp(15), spacing=dp(5))
        with card.canvas.before:
            Color(rgb=get_color_from_hex('#c4c4c4'))
            card.rect = RoundedRectangle(pos=card.pos, size=card.size, radius=[dp(20)])
        card.bind(pos=self._update_card_rect, size=self._update_card_rect)

        # Estado inicial: desligado
        card.is_on = False

        # Criamos o rótulo de texto como atributo do card para poder atualizar depois
        card.status_label = Label(
            text='Desligado',
            font_name='FuturaNegrito',
            font_size='20sp',
            color=get_color_from_hex('#292929'),
            halign='center',
            valign='middle',
            size_hint_y=None,
            height=dp(20)
        )

        # Imagem clicável — CENTRALIZADA!
        card.image_button = ImageButton(
            source='images/desligado.png',  # imagem de "desligado"
            size_hint=(None, None),
            size=(dp(90), dp(90)),
            pos_hint={'center_x': 0.5, 'center_y': 0.0},  # 👈 CENTRALIZA NA ÁREA DO CARD
            on_press=self.toggle_power_state
        )
        # Guardamos uma referência ao card dentro do botão (para acessar depois)
        card.image_button.parent_card = card

        # REMOVIDO o Widget de espaçamento superior — agora a imagem está centralizada
        card.add_widget(card.image_button)
        card.add_widget(card.status_label)

        return card

    def NextShutdownCard(self):
        card = BoxLayout(orientation='vertical', padding=dp(15), spacing=dp(5))
        with card.canvas.before:
            Color(rgb=get_color_from_hex('#c4c4c4'))
            card.rect = RoundedRectangle(pos=card.pos, size=card.size, radius=[dp(20)])
        card.bind(pos=self._update_card_rect, size=self._update_card_rect)

        # Labels do card
        self.shutdown_top_label = Label(
            text="Ligará às",
            font_name='Futura',
            font_size='14sp',
            color=get_color_from_hex('#5b5b5b'),
            halign='center',
            valign='middle',
            text_size=(dp(150), None),
            size_hint_y=None,
            height=dp(30)
        )
        self.shutdown_top_label.bind(size=self.shutdown_top_label.setter('text_size'))

        self.shutdown_main_label = Label(
            text="--h",
            font_name='Futura',
            font_size='80sp',
            color=get_color_from_hex('#292929'),
            halign='center',
            valign='middle',
            size_hint_y=None,
            height=dp(70)
        )
        self.shutdown_main_label.bind(size=self.shutdown_main_label.setter('text_size'))

        self.shutdown_bottom_label = Label(
            text="Sera desliagdo as 16h horas",
            font_name='Futura',
            font_size='14sp',
            color=get_color_from_hex('#5b5b5b'),
            halign='center',
            valign='top',
            text_size=(dp(200), None),
            size_hint_y=None,
            height=dp(15)
        )
        self.shutdown_bottom_label.bind(size=self.shutdown_bottom_label.setter('text_size'))

        card.add_widget(self.shutdown_top_label)
        card.add_widget(self.shutdown_main_label)
        card.add_widget(self.shutdown_bottom_label)

        return card

    def toggle_power_state(self, instance):
        """
        Alterna o estado local do card e envia o comando para o ESP.
        O 'estado' real do ESP é controlado pelo app (envia on/off).
        """
        card = instance.parent_card
        card.is_on = not card.is_on

        # Atualiza variável global e envia comando ao ESP
        global estado_ar
        if card.is_on:
            # Liga
            instance.source = 'images/ligado.png'
            card.status_label.text = 'Ligado'
            # Atualizações visuais locais
            # (a temperatura real virá do ESP nas próximas leituras;
            # aqui colocamos um valor indicativo se desejar.)
            # self.temperature_label.text = "25°"   # removido para evitar sobrescrever dado real
            estado_ar = "on"
            sucesso = enviar_estado_para_esp("on")
            if not sucesso:
                print("Envio do comando LIGAR falhou.")
            
            # Atualiza o card de próximo desligamento → modo LIGADO
            self.shutdown_top_label.text = "Proximo desligamento"
            self.shutdown_main_label.text = "16h"
            self.shutdown_bottom_label.text = "Sera ligado as 16:30h"
            
            print("Enviando comando: LIGAR")
        else:
            # Desliga
            instance.source = 'images/desligado.png'
            card.status_label.text = 'Desligado'
            # self.temperature_label.text = "---"   # não sobrescrevemos o dado ativo
            estado_ar = "off"
            sucesso = enviar_estado_para_esp("off")
            if not sucesso:
                print("Envio do comando DESLIGAR falhou.")

            # Atualiza o card de próximo desligamento → modo DESLIGADO
            self.shutdown_top_label.text = "Ligara as"
            self.shutdown_main_label.text = "--h"
            self.shutdown_bottom_label.text = "Sera desligado as 16h horas"

            print("Enviando comando: DESLIGAR")

    def update_data(self, dt):
        """
        Chamado periodicamente (Clock.schedule_interval) para buscar dados do ESP
        e atualizar labels na tela principal.
        """
        dados = buscar_dados_do_esp()
        if dados:
            # Temperatura
            temp = dados.get('temperatura')
            if temp is None:
                self.temperature_label.text = "---"
            else:
                # Formata para exibir sem muitos decimais, se for float
                try:
                    # se for numérico
                    if isinstance(temp, (float, int)):
                        display_temp = f"{round(temp, 1)}°"
                    else:
                        display_temp = str(temp) + "°" if not str(temp).endswith("°") else str(temp)
                except Exception:
                    display_temp = str(temp)
                self.temperature_label.text = display_temp

            # Pessoas
            pessoas = dados.get('pessoas')
            if pessoas is None:
                self.people_count_label.text = "--"
            else:
                self.people_count_label.text = str(pessoas)

            # Atualiza estado interno com o que o ESP reporta (opcional)
            # estado_reportado = dados.get('estado')  # use se quiser alinhar
            # (no seu requisito, o estado é controlado pelo app, então não forçamos aqui)
        else:
            # Não foi possível obter dados — deixamos o último valor visível
            pass


# --- TELA DO PROJETO ---
class ProjectScreen(Screen):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.is_ac_on = False
        layout_fundo = FloatLayout()
        layout_fundo.add_widget(Image(source='images/background.png', allow_stretch=True, keep_ratio=False))

        scroll = ScrollView(size_hint=(1, 1))
        container = BoxLayout(
            orientation='vertical',
            padding=[dp(30), dp(20), dp(30), dp(30)],
            spacing=dp(15),
            size_hint_y=None
        )
        container.bind(minimum_height=container.setter('height'))

        barra_superior = BoxLayout(size_hint_y=None, height=dp(40))
        barra_superior.add_widget(Image(source='images/parte_logo.png', size_hint_x=None, width=dp(80)))
        barra_superior.add_widget(Widget())
        btn_voltar = Button(
            text='[u]Voltar[/u]',
            markup=True,
            size_hint=(None, None),
            size=(dp(80), dp(30)),
            background_normal='',
            background_color=(0, 0, 0, 0),
            font_name='Futura',
            font_size='14sp',
            color=get_color_from_hex('#15213d')
        )
        btn_voltar.bind(on_press=self.go_back)
        barra_superior.add_widget(btn_voltar)
        container.add_widget(barra_superior)

        container.add_widget(Label(
            text='O projeto',
            font_name='FuturaNegrito',
            font_size='60sp',
            color=get_color_from_hex('#292929'),
            halign='left',
            size_hint_y=None,
            height=dp(90),
            text_size=(dp(300), None)
        ))

        card = BoxLayout(orientation='vertical', size_hint_y=None)
        card.bind(minimum_height=card.setter('height')) 
        
        with card.canvas.before:
            Color(rgb=get_color_from_hex('#d0d0d0'))
            card.rect = RoundedRectangle(pos=card.pos, size=card.size, radius=[dp(20)])
        card.bind(pos=self._update_rect, size=self._update_rect)
        
        # --- MODIFICAÇÃO AQUI ---
        # Usamos um BoxLayout para o wrapper da imagem e aplicamos o RoundedRectangle com a imagem como source nele.
        image_wrapper = BoxLayout(
            size_hint_y=None,
            height=dp(180), # Altura para o wrapper da imagem
            padding=[dp(15), dp(15), dp(15), dp(0)] # Padding interno para separar a imagem das bordas do card principal
        )
        
        # O widget interno onde a imagem arredondada será desenhada
        self.rounded_image_widget = Widget(
            size_hint=(1, 1) # Preenche o image_wrapper
        )
        
        with self.rounded_image_widget.canvas.before:
            Color(rgb=(1,1,1,1)) # Opcional: define um fundo branco se a imagem tiver transparência
            self.rounded_image_rect = RoundedRectangle(
                source='images/Image_ProjectScreen.png',
                pos=self.rounded_image_widget.pos,
                size=self.rounded_image_widget.size,
                radius=[dp(15)] # Aumentamos o raio para bordas mais suaves
            )
        
        self.rounded_image_widget.bind(pos=self._update_rounded_image_rect, size=self._update_rounded_image_rect)
        
        image_wrapper.add_widget(self.rounded_image_widget)
        card.add_widget(image_wrapper)
        # --- FIM DA MODIFICAÇÃO ---

        texto = (
            'O projeto "Sistema de Gerenciamento de Refrigeracao de Ambientes" surgiu da observacao do alto consumo '
            'de energia eletrica provocado pelo uso de equipamentos de climatizacao em ambientes escolares, como o IFRN Natal '
            'Central. Esse cenario despertou o interesse do grupo em buscar alternativas tecnologicas que promovessem a '
            'eficiencia energetica. A motivacao foi, portanto, desenvolver um sistema automatizado com sensores e '
            'microcontroladores, capaz de monitorar e otimizar o uso desses aparelhos nas salas de aula.\n\n'
            'A problematica central do projeto se concentra na falta de mecanismos inteligentes para a utilizacao racional '
            'do ar-condicionado, considerando fatores como a temperatura, a presença de pessoas e o tempo de uso do '
            'equipamento. A proposta busca justificar-se pela necessidade de reduzir o desperdicio de energia eletrica, '
            'contribuindo para a sustentabilidade e para a diminuicao dos custos operacionais da instituicao. O projeto '
            'tambem pode ser replicado em outras unidades escolares, servindo como um modelo inovador de gestao energetica.\n\n'
            'O grupo responsavel pelo projeto é composto por: Levi Ramos Matias Resende, Luis Felipe De Oliveira Barros, '
            'Thabatha Vitoria Ferreira Morais e Vinicius Da Silva Dionizio.'
        )
        
        text_container = BoxLayout(orientation='vertical', padding=[dp(20), dp(15), dp(20), dp(20)], size_hint_y=None)
        lbl = Label(
            text=texto,
            font_name='Futura',
            font_size='15sp',
            color=get_color_from_hex('#555555'),
            halign='left',
            valign='top',
            size_hint_y=None,
            text_size=(dp(280), None)
        )
        lbl.bind(texture_size=lambda inst, size: setattr(lbl, 'height', size[1]))
        text_container.add_widget(lbl)
        text_container.bind(minimum_height=text_container.setter('height'))
        
        card.add_widget(text_container)
        
        container.add_widget(card)
        scroll.add_widget(container)
        layout_fundo.add_widget(scroll)
        self.add_widget(layout_fundo)

    # Nova função para atualizar o retângulo arredondado da imagem
    def _update_rounded_image_rect(self, instance, value):
        self.rounded_image_rect.pos = instance.pos
        self.rounded_image_rect.size = instance.size

    def _update_rect(self, instance, value):
        instance.rect.pos = instance.pos
        instance.rect.size = instance.size

    def go_back(self, instance):
        if self.manager:
            self.manager.current = 'main'


# --- APLICAÇÃO PRINCIPAL ---
class ProjetoIntegradorApp(App):
    def build(self):
        # NÃO definimos Window.density nem system_size — deixamos o Kivy lidar
        self.sm = ScreenManager()
        self.sm.add_widget(LoadingScreen(name='loading'))
        self.sm.add_widget(LoginScreen(name='login'))
        self.sm.add_widget(MainScreen(name='main'))
        self.sm.add_widget(ProjectScreen(name='project'))
        return self.sm

    def on_start(self):
        # Simula barra de progresso e, depois, vai para a tela de login
        Clock.schedule_interval(self.update_progress, 0.05)

    def update_progress(self, dt):
        progress_bar = self.sm.get_screen('loading').progress_bar
        current_value = progress_bar.progress
        if current_value < 100:
            progress_bar.set_progress(current_value + 4)
        else:
            Clock.unschedule(self.update_progress)
            self.sm.current = 'login'


if __name__ == '__main__':
    ProjetoIntegradorApp().run()
