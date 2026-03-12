import socket
import struct
import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.animation import FuncAnimation
from collections import deque

# --- CONFIGURAZIONE ---
UDP_IP = "0.0.0.0"
UDP_PORT = 5005
SAMPLING_RATE = 100
MAX_SAMPLES_PLOT = 500 # 5 secondi visibili

# ORA DI INIZIO (Prendila dal tuo script di conversione MiniSEED)
# Esempio: datetime.datetime(2024, 3, 12, 10, 0, 0)
START_TIME_DATA = datetime.datetime.now() 

# --- STATO ---
sample_counter = 0
# Buffer per i dati
history = [deque([0]*MAX_SAMPLES_PLOT, maxlen=MAX_SAMPLES_PLOT) for _ in range(3)]
# Buffer per i timestamp (uno per ogni campione)
time_history = deque([START_TIME_DATA + datetime.timedelta(seconds=i/SAMPLING_RATE) 
                     for i in range(MAX_SAMPLES_PLOT)], maxlen=MAX_SAMPLES_PLOT)

# Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False)

# --- GRAFICA ---
fig, (axZ, axN, axE) = plt.subplots(3, 1, sharex=True, figsize=(12, 8))
fig.canvas.manager.set_window_title('INGV - Analisi Sismica Real-Time (3 Canali)')

# Setup canali con nomi standard sismologici
lines = [
    axZ.plot(time_history, history[0], color='#d32f2f', lw=1.5, label='Componente Z (Verticale)')[0],
    axN.plot(time_history, history[1], color='#388e3c', lw=1.5, label='Componente N (Nord-Sud)')[0],
    axE.plot(time_history, history[2], color='#1976d2', lw=1.5, label='Componente E (Est-Ovest)')[0]
]

# Formattazione assi
for ax in [axZ, axN, axE]:
    ax.set_ylim(-1000, 1000) # Counts
    ax.grid(True, alpha=0.3, linestyle='--')
    ax.legend(loc='upper right')
    # Formattatore per l'ora (HH:MM:SS)
    ax.xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))

plt.xticks(rotation=15)

def update(frame):
    global sample_counter
    
    try:
        # Leggiamo i dati dal Gateway
        while True:
            data, addr = sock.recvfrom(1024)
            if len(data) >= 66:
                ptr = 0
                for ch in range(3):
                    # Ricostruzione pacchetto (Base + 9 Delta)
                    current_val = struct.unpack('<i', data[ptr:ptr+4])[0]
                    history[ch].append(current_val)
                    
                    # Se è il primo canale, aggiorniamo il tempo per tutti
                    if ch == 0:
                        sample_time = START_TIME_DATA + datetime.timedelta(seconds=sample_counter / SAMPLING_RATE)
                        time_history.append(sample_time)
                    
                    ptr += 4
                    for i in range(9):
                        delta = struct.unpack('<h', data[ptr:ptr+2])[0]
                        current_val += delta
                        history[ch].append(current_val)
                        
                        # Aggiungiamo il tempo per ogni micro-campione delta
                        if ch == 0:
                            sample_counter += 1
                            sample_time = START_TIME_DATA + datetime.timedelta(seconds=sample_counter / SAMPLING_RATE)
                            time_history.append(sample_time)
                            
                        ptr += 2
                
                # Incremento counter base (i delta sono già contati sopra)
                sample_counter += 1 

            # Aggiornamento dati delle linee
            for i in range(3):
                lines[i].set_data(time_history, history[i])
            
            # AGGIORNAMENTO DINAMICO DELL'ASSE X (Lo fa scorrere)
            axE.set_xlim(time_history[0], time_history[-1])
                
    except BlockingIOError:
        pass
    except Exception as e:
        print(f"Errore: {e}")
    
    return lines

# Nota: blit=False è fondamentale per permettere all'asse X di aggiornarsi!
ani = FuncAnimation(fig, update, interval=50, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()