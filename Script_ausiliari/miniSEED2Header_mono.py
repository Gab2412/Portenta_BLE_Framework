from pathlib import Path

import obspy
import numpy as np

# 1. Upload file miniSEED
base_dir = Path(__file__).resolve().parent
input_file = base_dir / "data" / "dati_di_prova.mseed"
output_file = base_dir.parent / "shared_lib" / "seismic_data_mono.h"
output_file.parent.mkdir(parents=True, exist_ok=True)

st = obspy.read(str(input_file))
trace = st[0]
data = trace.data.astype(np.int32) # Convertiamo in interi a 32 bit

# 2. Limite campioni per non saturare la memoria Flash (1000 campioni = 10 secondi)
max_samples = 10000 #100 secondi
data_slice = data[:max_samples]

# 3. Conversione in header C
with open(output_file, "w") as f:
    f.write("#ifndef SEISMIC_DATA_MONO_H\n#define SEISMIC_DATA_MONO_H\n\n")
    f.write("#include <stdint.h>\n\n")
    f.write(f"static const int SEISMIC_DATA_LEN = {len(data_slice)};\n")
    f.write("static const int32_t SEISMIC_SAMPLES[] = {\n")
    
    # Scrive i dati formattati
    for i, val in enumerate(data_slice):
        f.write(f"{val},")
        if (i + 1) % 10 == 0: f.write("\n")
        
    f.write("\n};\n\n#endif")

print(f"File {output_file.name} generato con {len(data_slice)} campioni")