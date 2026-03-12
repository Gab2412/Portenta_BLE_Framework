import socket
import struct
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from collections import deque

# Configurazione
UDP_PORT = 5005
MAX_SAMPLES = 500 # 5 secondi di dati a schermo
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(("0.0.0.0", UDP_PORT))
sock.setblocking(False)

# Buffer per i 3 canali
history = [deque([0]*MAX_SAMPLES, maxlen=MAX_SAMPLES) for _ in range(3)]

fig, axes = plt.subplots(3, 1, sharex=True, figsize=(10, 7))
lines = [axes[i].plot(history[i], lw=1)[0] for i in range(3)]

for ax in axes:
    ax.set_ylim(-10000, 10000)
    ax.grid(True, alpha=0.3)
axes[2].set_xlabel("Campioni (100Hz)")

def update(frame):
    try:
        while True: # Leggi tutti i pacchetti nel buffer UDP
            data, addr = sock.recvfrom(1024)
            if len(data) < 66: continue
            
            ptr = 0
            for ch in range(3):
                # Ricostruisci i 10 campioni
                current_val = struct.unpack('<i', data[ptr:ptr+4])[0]
                history[ch].append(current_val)
                ptr += 4
                
                for _ in range(9):
                    delta = struct.unpack('<h', data[ptr:ptr+2])[0]
                    current_val += delta
                    history[ch].append(current_val)
                    ptr += 2
                    
            for i in range(3):
                lines[i].set_ydata(history[i])
                
    except BlockingIOError:
        pass
    return lines

ani = FuncAnimation(fig, update, interval=20, blit=True)
plt.tight_layout()
plt.show()