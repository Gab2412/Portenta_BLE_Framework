from pathlib import Path
import obspy
import numpy as np

# 1. Configurazione percorsi
base_dir = Path(__file__).resolve().parent
input_file = base_dir / "data" / "dati_di_prova.mseed"
output_file = base_dir.parent / "shared_lib" / "seismic_data.h"
output_file.parent.mkdir(parents=True, exist_ok=True)

# 2. Lettura MiniSEED
st = obspy.read(str(input_file))

# Ordiniamo i canali (solitamente arrivano come E, N, Z o 1, 2, 3)
st.sort() 

# Prendiamo i primi 3 canali disponibili
if len(st) < 3:
    print(f"ATTENZIONE: Il file contiene solo {len(st)} canali. Ne servono 3.")
    # In caso manchino canali, potresti duplicare l'ultimo o uscire
    exit()

traces = st[:3]

# Troviamo la lunghezza minima tra i 3 canali e applichiamo il limite massimo
max_allowed = 10000 
min_len = min(len(tr.data) for tr in traces)
actual_len = min(min_len, max_allowed)

# 3. Creazione del file Header C
with open(output_file, "w") as f:
    f.write("#ifndef SEISMIC_DATA_H\n#define SEISMIC_DATA_H\n\n")
    f.write("#include <stdint.h>\n\n")
    
    # Informazioni di debug nell'header
    f.write(f"// Dati estratti da: {input_file.name}\n")
    for i, tr in enumerate(traces):
        f.write(f"// Canale {i}: {tr.stats.channel} ({tr.stats.starttime})\n")
    
    f.write(f"\nstatic const int SEISMIC_DATA_LEN = {actual_len};\n")
    
    # Definiamo l'array bidimensionale: [canale][campione]
    f.write(f"static const int32_t SEISMIC_SAMPLES[3][{actual_len}] = {{\n")
    
    for ch_idx, tr in enumerate(traces):
        f.write("  { // Inizio Canale " + str(ch_idx) + "\n    ")
        
        # Estraiamo i dati e convertiamo in int32
        data_slice = tr.data[:actual_len].astype(np.int32)
        
        for i, val in enumerate(data_slice):
            f.write(f"{val},")
            # Andiamo a capo ogni 10 valori per leggibilità
            if (i + 1) % 10 == 0 and (i + 1) < actual_len:
                f.write("\n    ")
        
        f.write("\n  }" + ("," if ch_idx < 2 else "") + "\n")
        
    f.write("};\n\n#endif")

print(f"Successo! Generato {output_file.name}")
print(f"Canali inclusi: {[tr.stats.channel for tr in traces]}")
print(f"Lunghezza campioni: {actual_len}")