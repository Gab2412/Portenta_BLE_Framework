import socket
import struct
import datetime
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from matplotlib.animation import FuncAnimation
from collections import deque

# --- CONFIGURAZIONE RETE ---
UDP_PORT = 5005
MAX_SENSORS = 2
SAMPLING_RATE = 100
MAX_SAMPLES_PLOT = 500 # 5 secondi

# --- CONFIGURAZIONE SCALA (COUNTS) PER OGNI CANALE ---
# Modifica questi valori per decidere quanto "zoomare" ogni traccia
# Formato: [Sensore 1 [Z, N, E], Sensore 2 [Z, N, E]]
SCALING_LIMITS = [
    [2000, 2000, 2000],  # Sensore 1: Limiti per Z, N, E
    [2000, 2000, 2000]   # Sensore 2: Limiti per Z, N, E
]

# --- STATO SISTEMA ---
current_view = 1
start_time_global = datetime.datetime.now()

# Strutture dati per Sensore
histories = [[deque([0]*MAX_SAMPLES_PLOT, maxlen=MAX_SAMPLES_PLOT) for _ in range(3)] for _ in range(MAX_SENSORS)]
time_histories = [deque([start_time_global + datetime.timedelta(seconds=i/SAMPLING_RATE) 
                         for i in range(MAX_SAMPLES_PLOT)], maxlen=MAX_SAMPLES_PLOT) for _ in range(MAX_SENSORS)]
sample_counters = [0] * MAX_SENSORS

# Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", UDP_PORT))
sock.setblocking(False)

# --- GRAFICA ---
fig, axes = plt.subplots(3, 1, sharex=True, figsize=(12, 8))
channel_names = ["Z (Verticale)", "N (Nord-Sud)", "E (Est-Ovest)"]
colors = ['#d32f2f', '#388e3c', '#1976d2']

lines = []
for i in range(3):
    line, = axes[i].plot(time_histories[0], [0]*MAX_SAMPLES_PLOT, color=colors[i], lw=1.2, label=channel_names[i])
    lines.append(line)
    axes[i].grid(True, alpha=0.3, linestyle='--')
    axes[i].legend(loc='upper right')
    axes[i].xaxis.set_major_formatter(mdates.DateFormatter('%H:%M:%S'))

def on_key(event):
    global current_view
    if event.key in ['1', '2']:
        current_view = int(event.key)
        # Quando cambiamo pagina, resettiamo subito i limiti Y
        for i in range(3):
            limit = SCALING_LIMITS[current_view-1][i]
            axes[i].set_ylim(-limit, limit)

fig.canvas.mpl_connect('key_press_event', on_key)

def update(frame):
    global sample_counters
    try:
        while True:
            data, addr = sock.recvfrom(1024)
            if len(data) < 67: continue
            
            s_id = data[0]
            if 1 <= s_id <= MAX_SENSORS:
                s_idx = s_id - 1
                payload = data[1:]
                ptr = 0
                
                for ch in range(3):
                    # Ricostruzione (Base + 9 Delta)
                    val = struct.unpack('<i', payload[ptr:ptr+4])[0]
                    histories[s_idx][ch].append(val)
                    
                    if ch == 0: # Aggiorna tempo solo una volta per pacchetto
                        sample_time = start_time_global + datetime.timedelta(seconds=sample_counters[s_idx] / SAMPLING_RATE)
                        time_histories[s_idx].append(sample_time)
                    
                    ptr += 4
                    for _ in range(9):
                        delta = struct.unpack('<h', payload[ptr:ptr+2])[0]
                        val += delta
                        histories[s_idx][ch].append(val)
                        if ch == 0:
                            sample_counters[s_idx] += 1
                            sample_time = start_time_global + datetime.timedelta(seconds=sample_counters[s_idx] / SAMPLING_RATE)
                            time_histories[s_idx].append(sample_time)
                        ptr += 2
                
                sample_counters[s_idx] += 1

    except BlockingIOError:
        pass

    # --- DISEGNO PAGINA ATTUALE ---
    s_view_idx = current_view - 1
    fig.suptitle(f"STAZIONE SISMICA: {current_view:02d} | ORA: {time_histories[s_view_idx][-1].strftime('%H:%M:%S')}", 
                 fontsize=14, fontweight='bold')

    for i in range(3):
        lines[i].set_data(time_histories[s_view_idx], histories[s_view_idx][i])
        
        # Applichiamo lo scaling fisso per questo sensore/canale
        limit = SCALING_LIMITS[s_view_idx][i]
        axes[i].set_ylim(-limit, limit)
        axes[i].set_ylabel(f"{channel_names[i]}\n[Counts]")

    # Fa scorrere l'asse X
    axes[2].set_xlim(time_histories[s_view_idx][0], time_histories[s_view_idx][-1])
    
    return lines

ani = FuncAnimation(fig, update, interval=50, blit=False, cache_frame_data=False)
plt.tight_layout(rect=[0, 0.03, 1, 0.95])
plt.show()