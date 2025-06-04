import tkinter as tk
from tkinter import scrolledtext, messagebox, simpledialog
import serial
import serial.tools.list_ports
import threading
import time # Certifique-se que 'time' está importado
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

# Para dados do PID (plotagem)
pid_data_tempo = []
pid_data_pos = []
pid_data_erro = []
pid_data_termo_p = []
pid_data_termo_i = []
pid_data_termo_d = []
pid_data_correcao = []

tempo_inicial_pid_plot = 0
plotando_pid_ativo = False

_janela_grafico_pid = None
_canvas_grafico_pid = None
_ax_pid = None
MAX_PID_PONTOS = 200

# Variáveis globais de comunicação e sensores
arduino = None
thread_leitura = None
rodando_leitura = False
dados_sensores_para_plotar = []

# --- NOVO: Variáveis para os campos de entrada e labels das constantes PID ---
entry_kp_var = None
entry_ki_var = None
entry_kd_var = None
entry_kp_widget = None # Referência ao widget Entry
entry_ki_widget = None # Referência ao widget Entry
entry_kd_widget = None # Referência ao widget Entry

label_kp_atual_var = None
label_ki_atual_var = None
label_kd_atual_var = None

# --- NOVO: Botões para controle de constantes PID ---
btn_enviar_kp = None
btn_enviar_ki = None
btn_enviar_kd = None
btn_ler_pid_consts = None

# --- Funções de Comunicação Serial ---
def encontrar_portas_seriais():
    """Encontra e retorna uma lista de portas seriais disponíveis."""
    return [port.device for port in serial.tools.list_ports.comports()]

def conectar_serial():
    """Tenta conectar à porta serial selecionada."""
    global arduino, rodando_leitura, thread_leitura
    porta_selecionada = porta_serial_var.get()
    if not porta_selecionada or porta_selecionada == "Nenhuma porta encontrada":
        messagebox.showerror("Erro", "Nenhuma porta serial válida selecionada.")
        return

    try:
        arduino = serial.Serial(porta_selecionada, 9600, timeout=1)
        time.sleep(2)
        if arduino.is_open:
            log_console(f"Conectado a {porta_selecionada}")
            status_conexao_var.set(f"Conectado: {porta_selecionada}")
            btn_conectar.config(text="Desconectar", command=desconectar_serial)
            
            rodando_leitura = True
            thread_leitura = threading.Thread(target=ler_da_serial, daemon=True)
            thread_leitura.start()
            habilitar_controles(True)
            solicitar_constantes_pid_do_robo() # Solicita constantes ao conectar
        else:
            messagebox.showerror("Erro de Conexão", f"Não foi possível abrir a porta {porta_selecionada}.")
            habilitar_controles(False)
            
    except serial.SerialException as e:
        messagebox.showerror("Erro de Conexão", f"Erro ao conectar: {e}")
        log_console(f"Falha ao conectar a {porta_selecionada}: {e}")
        habilitar_controles(False)

def desconectar_serial():
    """Fecha a conexão serial."""
    global arduino, rodando_leitura, plotando_pid_ativo
    rodando_leitura = False
    plotando_pid_ativo = False # Para plotagem PID também
    if _janela_grafico_pid and _janela_grafico_pid.winfo_exists():
        _ao_fechar_janela_pid()


    if thread_leitura and thread_leitura.is_alive():
        thread_leitura.join(timeout=1)

    if arduino and arduino.is_open:
        arduino.close()
        log_console("Desconectado.")
        status_conexao_var.set("Desconectado")
    arduino = None
    btn_conectar.config(text="Conectar", command=conectar_serial)
    habilitar_controles(False)
    # Limpar labels de constantes PID ao desconectar
    if label_kp_atual_var: label_kp_atual_var.set("Kp lido: N/A")
    if label_ki_atual_var: label_ki_atual_var.set("Ki lido: N/A")
    if label_kd_atual_var: label_kd_atual_var.set("Kd lido: N/A")


def enviar_comando(comando_char): # Esta função envia um único caractere
    """Envia um comando de um único caractere para o Arduino."""
    if arduino and arduino.is_open:
        try:
            arduino.write(comando_char.encode('utf-8'))
            log_console(f"Comando enviado: {comando_char}")
        except serial.SerialException as e:
            log_console(f"Erro ao enviar comando '{comando_char}': {e}")
            messagebox.showerror("Erro de Envio", f"Não foi possível enviar o comando '{comando_char}': {e}")
    else:
        log_console("Não conectado. Envio falhou.")
        messagebox.showwarning("Aviso", "Não conectado ao robô. Conecte primeiro.")

# --- NOVO: Funções para enviar e solicitar constantes PID ---
def enviar_nova_constante_pid(constante_char, valor_str):
    """Envia uma nova constante PID para o robô."""
    if not arduino or not arduino.is_open:
        messagebox.showwarning("Aviso", "Não conectado ao robô.")
        return
    try:
        valor_float = float(valor_str) 
        comando = f"{constante_char}{valor_float}\n" # Adiciona newline!
        arduino.write(comando.encode('utf-8'))
        log_console(f"Comando enviado: {comando.strip()}")
    except ValueError:
        messagebox.showerror("Erro de Valor", 
                             f"Valor '{valor_str}' inválido para {constante_char}. Deve ser um número (ex: 0.05).")
    except Exception as e:
        log_console(f"Erro ao enviar constante PID {constante_char}: {e}")
        messagebox.showerror("Erro de Envio", f"Não foi possível enviar o comando: {e}")

def solicitar_constantes_pid_do_robo():
    """Envia o comando 'G' para pedir as constantes PID ao robô."""
    if not arduino or not arduino.is_open:
        # Não mostrar warning se for chamado automaticamente ao conectar e não estiver conectado ainda
        # messagebox.showwarning("Aviso", "Não conectado ao robô.") 
        return
    try:
        comando = "G\n" # Adiciona newline!
        arduino.write(comando.encode('utf-8'))
        log_console(f"Comando enviado: {comando.strip()}")
    except Exception as e:
        log_console(f"Erro ao enviar comando 'G': {e}")
        messagebox.showerror("Erro de Envio", f"Não foi possível enviar o comando 'G': {e}")

def ler_da_serial():
    """Lê dados da porta serial continuamente em uma thread."""
    global rodando_leitura
    buffer_leitura = ""
    while rodando_leitura:
        if arduino and arduino.is_open and arduino.in_waiting > 0:
            try:
                byte_lido = arduino.read(arduino.in_waiting)
                dados_recebidos = byte_lido.decode('utf-8', errors='replace')
                buffer_leitura += dados_recebidos

                while '\n' in buffer_leitura:
                    linha, buffer_leitura = buffer_leitura.split('\n', 1)
                    linha_strip = linha.strip() 
                    if linha_strip: 
                        log_console(f"Robô: {linha_strip}")
                        processar_linha_recebida(f"Robô: {linha_strip}")
            except serial.SerialException as e:
                log_console(f"Erro ao ler da serial: {e}")
                # Considerar chamar desconectar_serial() ou uma função de tratamento de erro mais robusta
                if rodando_leitura: # Evitar chamar desconectar_serial se já estiver desconectando
                    desconectar_serial() 
                break 
            except Exception as e:
                log_console(f"Erro inesperado na leitura: {e}")
        time.sleep(0.01)

def processar_linha_recebida(linha_com_prefixo_robo):
    """Processa uma linha recebida do robô, identificando seu tipo."""
    global plotando_pid_ativo, tempo_inicial_pid_plot
    global pid_data_pos, pid_data_erro, pid_data_termo_p, pid_data_termo_i
    global pid_data_termo_d, pid_data_correcao, pid_data_tempo
    # Globais para atualizar labels de Kp, Ki, Kd
    global label_kp_atual_var, label_ki_atual_var, label_kd_atual_var
    global entry_kp_var, entry_ki_var, entry_kd_var


    linha_original = ""
    if linha_com_prefixo_robo.startswith("Robô: "):
        linha_original = linha_com_prefixo_robo[len("Robô: "):].strip()
    else:
        linha_original = linha_com_prefixo_robo.strip()

    if not linha_original:
        return

    if linha_original.startswith("PID:"):
        if not plotando_pid_ativo:
            return
        if not pid_data_tempo:
            tempo_inicial_pid_plot = time.time()
        try:
            valores_str = linha_original.split(':')[1].split(',')
            if len(valores_str) == 6:
                pos_val = int(valores_str[0])
                erro_val = int(valores_str[1])
                p_val = float(valores_str[2]) 
                i_val = float(valores_str[3]) 
                d_val = float(valores_str[4]) 
                correcao_val = int(valores_str[5])
                tempo_atual_relativo = time.time() - tempo_inicial_pid_plot
                pid_data_tempo.append(tempo_atual_relativo)
                pid_data_pos.append(pos_val)
                pid_data_erro.append(erro_val)
                pid_data_termo_p.append(p_val)
                pid_data_termo_i.append(i_val)
                pid_data_termo_d.append(d_val)
                pid_data_correcao.append(correcao_val)
                while len(pid_data_tempo) > MAX_PID_PONTOS:
                    pid_data_tempo.pop(0); pid_data_pos.pop(0); pid_data_erro.pop(0)
                    pid_data_termo_p.pop(0); pid_data_termo_i.pop(0); pid_data_termo_d.pop(0)
                    pid_data_correcao.pop(0)
            else:
                log_console(f"Formato PID inesperado: {linha_original}")
        except Exception as e:
            log_console(f"Erro ao processar dados PID: {e} - Linha: {linha_original}")
    
    # --- NOVO: Processar respostas de atualização de Kp, Ki, Kd e leitura ---
    elif linha_original.startswith("Kp atualizado para: "):
        try:
            kp_lido = linha_original.split(':')[1].strip()
            if label_kp_atual_var: label_kp_atual_var.set(f"Kp lido: {kp_lido}")
        except Exception as e:
            log_console(f"Erro ao parsear confirmação de Kp: {e} | Linha: {linha_original}")
    
    elif linha_original.startswith("Ki atualizado para: "):
        try:
            ki_lido = linha_original.split(':')[1].strip()
            if label_ki_atual_var: label_ki_atual_var.set(f"Ki lido: {ki_lido}")
        except Exception as e:
            log_console(f"Erro ao parsear confirmação de Ki: {e} | Linha: {linha_original}")

    elif linha_original.startswith("Kd atualizado para: "):
        try:
            kd_lido = linha_original.split(':')[1].strip()
            if label_kd_atual_var: label_kd_atual_var.set(f"Kd lido: {kd_lido}")
        except Exception as e:
            log_console(f"Erro ao parsear confirmação de Kd: {e} | Linha: {linha_original}")
    
    elif linha_original.startswith("PID_CONSTS:"):
        try:
            valores_str = linha_original.split(':')[1].split(',')
            if len(valores_str) == 3:
                kp_lido = valores_str[0].strip()
                ki_lido = valores_str[1].strip()
                kd_lido = valores_str[2].strip()
                
                if label_kp_atual_var: label_kp_atual_var.set(f"Kp lido: {kp_lido}")
                if label_ki_atual_var: label_ki_atual_var.set(f"Ki lido: {ki_lido}")
                if label_kd_atual_var: label_kd_atual_var.set(f"Kd lido: {kd_lido}")
                
                if entry_kp_var: entry_kp_var.set(kp_lido)
                if entry_ki_var: entry_ki_var.set(ki_lido)
                if entry_kd_var: entry_kd_var.set(kd_lido)
                log_console(f"Constantes PID lidas: Kp={kp_lido}, Ki={ki_lido}, Kd={kd_lido}")
            else:
                log_console(f"Formato PID_CONSTS inesperado: {linha_original}")
        except Exception as e:
            log_console(f"Erro ao processar PID_CONSTS: {e} | Linha: {linha_original}")


# --- Funções da Interface Gráfica (GUI) ---
def log_console(mensagem):
    """Adiciona uma mensagem à área de texto do console."""
    if console_output and console_output.winfo_exists(): # Verifica se o widget ainda existe
        console_output.config(state=tk.NORMAL)
        console_output.insert(tk.END, mensagem + "\n")
        console_output.see(tk.END)
        console_output.config(state=tk.DISABLED)

def habilitar_controles(estado):
    """Habilita ou desabilita os botões de comando e campos de PID."""
    global btn_enviar_kp, btn_enviar_ki, btn_enviar_kd, btn_ler_pid_consts # Garante que são as globais
    global entry_kp_widget, entry_ki_widget, entry_kd_widget

    componentes_botoes_comando = [
        btn_ler_sensores, btn_ligar_motores, btn_parar_motores,
        btn_seguir_linha_rapido, btn_calibrar, btn_seguir_linha_lento,
        btn_plotar_sensores, btn_plot_pid, 
    ]
    componentes_botoes_pid = [ # Botões de ajuste PID
        btn_enviar_kp, btn_enviar_ki, btn_enviar_kd, btn_ler_pid_consts
    ]
    componentes_entrada_pid = [ # Campos de entrada PID
        entry_kp_widget, entry_ki_widget, entry_kd_widget
    ]

    for comp_list in [componentes_botoes_comando, componentes_botoes_pid]:
        for comp in comp_list:
            if comp: comp.config(state=(tk.NORMAL if estado else tk.DISABLED))
    
    for comp_entry in componentes_entrada_pid:
        if comp_entry: comp_entry.config(state=(tk.NORMAL if estado else tk.DISABLED))


def extrair_e_plotar_sensores():
    """Extrai os últimos dados de sensores do console e plota."""
    global dados_sensores_para_plotar
    texto_console = console_output.get("1.0", tk.END)
    linhas_do_log = texto_console.strip().split('\n')
    
    valores_sensores = []
    idx_leitura_inicio = -1
    marcador_inicio_leitura = "Robô: Leitura dos sensores:"
    marcador_fim_leitura = "Robô: ----------------------"
    prefixo_linha_sensor = "Robô: Sensor "

    for i in range(len(linhas_do_log) - 1, -1, -1):
        if linhas_do_log[i].strip() == marcador_inicio_leitura:
            idx_leitura_inicio = i
            break
    
    if idx_leitura_inicio != -1:
        sensores_coletados_nesta_sessao = 0
        for i in range(idx_leitura_inicio + 1, len(linhas_do_log)):
            linha_atual = linhas_do_log[i].strip()
            if linha_atual == marcador_fim_leitura:
                break 
            if linha_atual.startswith(prefixo_linha_sensor) and ":" in linha_atual:
                try:
                    partes = linha_atual.split(':')
                    if len(partes) > 2: 
                        valor_str = partes[2].strip()
                        valores_sensores.append(int(valor_str))
                        sensores_coletados_nesta_sessao += 1
                    else:
                        log_console(f"Formato inesperado na linha do sensor: {linha_atual}")
                except ValueError: log_console(f"Valor inválido na linha do sensor: {linha_atual}")
                except IndexError: log_console(f"Formato inesperado na linha do sensor: {linha_atual}")
            if sensores_coletados_nesta_sessao >= 8: break 
    
    if len(valores_sensores) == 8:
        dados_sensores_para_plotar = valores_sensores
        log_console(f"Dados extraídos para plotagem: {dados_sensores_para_plotar}")
        plotar_sensores_popup()
    else:
        messagebox.showinfo("Plotar Sensores", 
                            f"Não foram encontrados 8 valores de sensores no log recente.\n"
                            f"Encontrados: {len(valores_sensores)} ({valores_sensores}).\n"
                            "Use o comando '0' e tente novamente.")
        log_console(f"Falha na plotagem QTR: {len(valores_sensores)}/8. Dados: {valores_sensores}")


def _ao_fechar_janela_pid():
    global plotando_pid_ativo, _janela_grafico_pid, _canvas_grafico_pid, _ax_pid
    plotando_pid_ativo = False
    if btn_plot_pid and btn_plot_pid.winfo_exists():
         btn_plot_pid.config(text="Iniciar Plotagem PID")
    if _janela_grafico_pid:
        _janela_grafico_pid.destroy()
    _janela_grafico_pid = None
    _canvas_grafico_pid = None
    if _ax_pid: plt.close(_ax_pid.figure)
    _ax_pid = None

def criar_janela_grafico_pid():
    global _janela_grafico_pid, _canvas_grafico_pid, _ax_pid
    if _janela_grafico_pid is not None and _janela_grafico_pid.winfo_exists():
        _janela_grafico_pid.lift()
        return
    _janela_grafico_pid = tk.Toplevel(app)
    _janela_grafico_pid.title("Gráfico do Controle PID em Tempo Real")
    _janela_grafico_pid.geometry("900x700")
    fig, ax = plt.subplots()
    _ax_pid = ax
    _ax_pid.set_title("Dinâmica do Controle PID"); _ax_pid.set_xlabel("Tempo (s)")
    _ax_pid.set_ylabel("Valor"); _ax_pid.grid(True)
    _ax_pid.plot([], [], label='Erro', marker='o', markersize=2, linestyle='-')
    _ax_pid.plot([], [], label='Termo P', marker='o', markersize=2, linestyle='-')
    _ax_pid.plot([], [], label='Termo I', marker='o', markersize=2, linestyle='-')
    _ax_pid.plot([], [], label='Termo D', marker='o', markersize=2, linestyle='-')
    _ax_pid.plot([], [], label='Correção PID', marker='o', markersize=2, linestyle='-', linewidth=2)
    _ax_pid.plot([], [], label='Posição Linha', marker='.', markersize=2, linestyle='--')
    _ax_pid.legend(loc='upper left')
    _canvas_grafico_pid = FigureCanvasTkAgg(fig, master=_janela_grafico_pid)
    _canvas_grafico_pid.draw()
    _canvas_grafico_pid.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
    _janela_grafico_pid.protocol("WM_DELETE_WINDOW", _ao_fechar_janela_pid)
    _janela_grafico_pid.lift()

def atualizar_grafico_pid():
    global plotando_pid_ativo, _ax_pid, _canvas_grafico_pid
    if not plotando_pid_ativo or _ax_pid is None or _canvas_grafico_pid is None or \
       not _janela_grafico_pid or not _janela_grafico_pid.winfo_exists():
        return
    if not pid_data_tempo:
         if plotando_pid_ativo: app.after(200, atualizar_grafico_pid) # Aumentei um pouco o tempo de retry
         return
    lines = _ax_pid.get_lines()
    lines[0].set_data(pid_data_tempo, pid_data_erro)
    lines[1].set_data(pid_data_tempo, pid_data_termo_p)
    lines[2].set_data(pid_data_tempo, pid_data_termo_i)
    lines[3].set_data(pid_data_tempo, pid_data_termo_d)
    lines[4].set_data(pid_data_tempo, pid_data_correcao)
    lines[5].set_data(pid_data_tempo, pid_data_pos)
    _ax_pid.relim(); _ax_pid.autoscale_view(True, True, True)
    _ax_pid.legend(loc='best') # 'best' pode ser melhor que 'upper left' dinamicamente
    _canvas_grafico_pid.draw_idle()
    if plotando_pid_ativo: app.after(100, atualizar_grafico_pid)

def toggle_plotagem_pid():
    global plotando_pid_ativo, tempo_inicial_pid_plot
    if not arduino or not arduino.is_open:
        messagebox.showwarning("Aviso", "Conecte ao robô primeiro!")
        return
    if not plotando_pid_ativo:
        plotando_pid_ativo = True
        btn_plot_pid.config(text="Parar Plotagem PID")
        pid_data_tempo.clear(); pid_data_pos.clear(); pid_data_erro.clear()
        pid_data_termo_p.clear(); pid_data_termo_i.clear(); pid_data_termo_d.clear()
        pid_data_correcao.clear(); tempo_inicial_pid_plot = 0
        criar_janela_grafico_pid(); atualizar_grafico_pid()
        log_console("Plotagem PID iniciada. Coloque o robô para seguir a linha.")
    else:
        plotando_pid_ativo = False
        btn_plot_pid.config(text="Iniciar Plotagem PID")
        log_console("Plotagem PID parada.")

_janela_grafico_sensores = None
_canvas_grafico_sensores = None

def plotar_sensores_popup(): # Mesma função de antes
    global dados_sensores_para_plotar, _janela_grafico_sensores, _canvas_grafico_sensores
    if not dados_sensores_para_plotar or len(dados_sensores_para_plotar) != 8:
        messagebox.showinfo("Plotar Sensores", "Sem dados suficientes (8 valores). Use o comando '0'.")
        return
    if _janela_grafico_sensores is None or not _janela_grafico_sensores.winfo_exists():
        _janela_grafico_sensores = tk.Toplevel(app); _janela_grafico_sensores.title("Valores dos Sensores QTR")
        fig, ax = plt.subplots()
        barras = ax.bar(range(8), dados_sensores_para_plotar, color='skyblue')
        ax.set_xticks(range(8)); ax.set_xticklabels([f'S{i}' for i in range(8)])
        ax.set_ylabel('Valor do Sensor'); ax.set_title('Leitura dos Sensores QTR')
        min_val = 0 # Sensores QTR geralmente não são negativos
        max_val = max(dados_sensores_para_plotar) if dados_sensores_para_plotar else 1000
        ax.set_ylim([min_val, max(1000, max_val + 100)])
        for i, barra in enumerate(barras):
            altura = barra.get_height()
            ax.text(barra.get_x() + barra.get_width()/2., altura, f'{dados_sensores_para_plotar[i]}', ha='center', va='bottom')
        _canvas_grafico_sensores = FigureCanvasTkAgg(fig, master=_janela_grafico_sensores)
        _canvas_grafico_sensores.draw(); _canvas_grafico_sensores.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        def ao_fechar_grafico_sensores(): # Renomeado para evitar conflito
            global _janela_grafico_sensores, _canvas_grafico_sensores
            if _janela_grafico_sensores: _janela_grafico_sensores.destroy()
            _janela_grafico_sensores = None; _canvas_grafico_sensores = None
            plt.close(fig) # Fecha a figura específica
        _janela_grafico_sensores.protocol("WM_DELETE_WINDOW", ao_fechar_grafico_sensores)
    else:
        ax = _canvas_grafico_sensores.figure.axes[0]; ax.clear()
        barras = ax.bar(range(8), dados_sensores_para_plotar, color='skyblue')
        ax.set_xticks(range(8)); ax.set_xticklabels([f'S{i}' for i in range(8)])
        ax.set_ylabel('Valor do Sensor'); ax.set_title('Leitura dos Sensores QTR')
        min_val = 0
        max_val = max(dados_sensores_para_plotar) if dados_sensores_para_plotar else 1000
        ax.set_ylim([min_val, max(1000, max_val + 100)])
        for i, barra in enumerate(barras):
            altura = barra.get_height()
            ax.text(barra.get_x() + barra.get_width()/2., altura, f'{dados_sensores_para_plotar[i]}', ha='center', va='bottom')
        _canvas_grafico_sensores.draw()
    _janela_grafico_sensores.lift()

# --- Configuração da Janela Principal ---
app = tk.Tk()
app.title("Controlador Robô Seguidor de Linha v2.0") # Nova versão!

# Frame para Conexão
frame_conexao = tk.LabelFrame(app, text="Conexão Serial", padx=10, pady=10)
frame_conexao.pack(padx=10, pady=5, fill="x")
porta_serial_var = tk.StringVar(app)
status_conexao_var = tk.StringVar(app); status_conexao_var.set("Desconectado")
portas_disponiveis = encontrar_portas_seriais()
porta_serial_var.set(portas_disponiveis[0] if portas_disponiveis else "Nenhuma porta encontrada")
tk.Label(frame_conexao, text="Porta:").pack(side=tk.LEFT, padx=(0,5))
dropdown_portas = tk.OptionMenu(frame_conexao, porta_serial_var, *(portas_disponiveis if portas_disponiveis else ["Nenhuma porta encontrada"]))
dropdown_portas.pack(side=tk.LEFT, padx=5)
btn_atualizar_portas = tk.Button(frame_conexao, text="Atualizar Portas", 
                                command=lambda: dropdown_portas['menu'].delete(0, 'end') or \
                                [dropdown_portas['menu'].add_command(label=p, command=tk._setit(porta_serial_var, p)) for p in encontrar_portas_seriais()] or \
                                (porta_serial_var.set(encontrar_portas_seriais()[0]) if encontrar_portas_seriais() else porta_serial_var.set("Nenhuma porta encontrada")))
btn_atualizar_portas.pack(side=tk.LEFT, padx=5)
btn_conectar = tk.Button(frame_conexao, text="Conectar", command=conectar_serial, width=12)
btn_conectar.pack(side=tk.LEFT, padx=5)
tk.Label(frame_conexao, textvariable=status_conexao_var).pack(side=tk.LEFT, padx=5)

# Frame para Comandos
frame_comandos = tk.LabelFrame(app, text="Comandos do Robô", padx=10, pady=10)
frame_comandos.pack(padx=10, pady=5, fill="x")
btn_ler_sensores = tk.Button(frame_comandos, text="0: Ler Sensores", command=lambda: enviar_comando('0'), width=20)
btn_ler_sensores.grid(row=0, column=0, padx=5, pady=2, sticky="ew")
btn_ligar_motores = tk.Button(frame_comandos, text="1: Ligar Motores", command=lambda: enviar_comando('1'), width=20)
btn_ligar_motores.grid(row=0, column=1, padx=5, pady=2, sticky="ew")
btn_parar_motores = tk.Button(frame_comandos, text="2: Parar Motores", command=lambda: enviar_comando('2'), width=20)
btn_parar_motores.grid(row=0, column=2, padx=5, pady=2, sticky="ew")
btn_seguir_linha_rapido = tk.Button(frame_comandos, text="3: Seguir (Rápido)", command=lambda: enviar_comando('3'), width=20) # Texto mais curto
btn_seguir_linha_rapido.grid(row=1, column=0, padx=5, pady=2, sticky="ew")
btn_calibrar = tk.Button(frame_comandos, text="4: Calibração", command=lambda: enviar_comando('4'), width=20)
btn_calibrar.grid(row=1, column=1, padx=5, pady=2, sticky="ew")
btn_seguir_linha_lento = tk.Button(frame_comandos, text="5: Seguir (Lento)", command=lambda: enviar_comando('5'), width=20) # Texto mais curto
btn_seguir_linha_lento.grid(row=1, column=2, padx=5, pady=2, sticky="ew")

# --- NOVO: Frame para Ajuste de Constantes PID ---
frame_pid_ajuste = tk.LabelFrame(app, text="Ajuste de Constantes PID", padx=10, pady=10)
frame_pid_ajuste.pack(padx=10, pady=5, fill="x")

# Kp
tk.Label(frame_pid_ajuste, text="Kp:").grid(row=0, column=0, padx=(0,2), pady=2, sticky="w")
entry_kp_var = tk.StringVar(value="0.06") 
entry_kp_widget = tk.Entry(frame_pid_ajuste, textvariable=entry_kp_var, width=10)
entry_kp_widget.grid(row=0, column=1, padx=2, pady=2)
btn_enviar_kp = tk.Button(frame_pid_ajuste, text="Enviar Kp", width=10,
                          command=lambda: enviar_nova_constante_pid('P', entry_kp_var.get()))
btn_enviar_kp.grid(row=0, column=2, padx=(5,2), pady=2)
label_kp_atual_var = tk.StringVar(value="Kp lido: N/A")
tk.Label(frame_pid_ajuste, textvariable=label_kp_atual_var).grid(row=0, column=3, padx=(5,0), pady=2, sticky="w")

# Ki
tk.Label(frame_pid_ajuste, text="Ki:").grid(row=1, column=0, padx=(0,2), pady=2, sticky="w")
entry_ki_var = tk.StringVar(value="0.0001")
entry_ki_widget = tk.Entry(frame_pid_ajuste, textvariable=entry_ki_var, width=10)
entry_ki_widget.grid(row=1, column=1, padx=2, pady=2)
btn_enviar_ki = tk.Button(frame_pid_ajuste, text="Enviar Ki", width=10,
                          command=lambda: enviar_nova_constante_pid('I', entry_ki_var.get()))
btn_enviar_ki.grid(row=1, column=2, padx=(5,2), pady=2)
label_ki_atual_var = tk.StringVar(value="Ki lido: N/A")
tk.Label(frame_pid_ajuste, textvariable=label_ki_atual_var).grid(row=1, column=3, padx=(5,0), pady=2, sticky="w")

# Kd
tk.Label(frame_pid_ajuste, text="Kd:").grid(row=2, column=0, padx=(0,2), pady=2, sticky="w")
entry_kd_var = tk.StringVar(value="0.3") 
entry_kd_widget = tk.Entry(frame_pid_ajuste, textvariable=entry_kd_var, width=10)
entry_kd_widget.grid(row=2, column=1, padx=2, pady=2)
btn_enviar_kd = tk.Button(frame_pid_ajuste, text="Enviar Kd", width=10,
                          command=lambda: enviar_nova_constante_pid('D', entry_kd_var.get()))
btn_enviar_kd.grid(row=2, column=2, padx=(5,2), pady=2)
label_kd_atual_var = tk.StringVar(value="Kd lido: N/A")
tk.Label(frame_pid_ajuste, textvariable=label_kd_atual_var).grid(row=2, column=3, padx=(5,0), pady=2, sticky="w")

btn_ler_pid_consts = tk.Button(frame_pid_ajuste, text="Ler Constantes PID Atuais do Robô",
                                command=solicitar_constantes_pid_do_robo) # Lambda não é necessário aqui
btn_ler_pid_consts.grid(row=3, column=0, columnspan=4, padx=5, pady=10, sticky="ew")


# Frame para Plotar Dados (Sensores e PID)
frame_plot = tk.LabelFrame(app, text="Visualização de Dados", padx=10, pady=10)
#frame_plot.pack(padx=10, pady=5, fill="x",_after=frame_pid_ajuste) # Organiza depois do ajuste PID
frame_plot.pack(padx=10, pady=5, fill="x", after=frame_pid_ajuste) # Organiza depois do ajuste PID

btn_plotar_sensores = tk.Button(frame_plot, text="Plotar Última Leitura de Sensores", command=extrair_e_plotar_sensores, width=30)
btn_plotar_sensores.pack(pady=5)
btn_plot_pid = tk.Button(frame_plot, text="Iniciar Plotagem PID", command=toggle_plotagem_pid, width=30)
btn_plot_pid.pack(pady=5)

# Frame para Console de Saída
frame_console = tk.LabelFrame(app, text="Console", padx=10, pady=10)
frame_console.pack(padx=10, pady=10, fill="both", expand=True)
console_output = scrolledtext.ScrolledText(frame_console, height=10, state=tk.DISABLED, font=("Consolas", 9)) # Reduzi altura e ajustei fonte
console_output.pack(fill="both", expand=True)

# Inicialmente desabilita os botões de controle
habilitar_controles(False)

# Ação ao fechar a janela principal
def ao_fechar_janela_principal():
    desconectar_serial() # Isso já para a plotagem PID e fecha a janela se existir
    app.quit()
    # app.destroy() # app.quit() geralmente é suficiente para terminar o mainloop

app.protocol("WM_DELETE_WINDOW", ao_fechar_janela_principal)
app.mainloop()