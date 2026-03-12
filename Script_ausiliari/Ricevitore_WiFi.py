import socket

UDP_IP = "0.0.0.0"
UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Ricevitore in ascolto su {UDP_PORT}...")

while True:
    try:
        data, addr = sock.recvfrom(1024)
        print(f"Ricevuti {len(data)} byte da {addr}")
        # Se i dati sono corretti (66 o 70 byte)
        if len(data) >= 66:
            # Per ora stampiamo solo i primi 4 byte come test
            print(f"  Dato Header: {data[:4].hex()}")
    except Exception as e:
        print(f"Errore: {e}")