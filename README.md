# 📡 Portenta H7 Multi-Node BLE Framework

![C++](https://img.shields.io/badge/Language-C++-blue.svg)
![PlatformIO](https://img.shields.io/badge/Piattaforma-PlatformIO-orange.svg)
![Hardware](https://img.shields.io/badge/Hardware-Arduino_Portenta_H7-00979D.svg)
![Stato](https://img.shields.io/badge/Stato-In_Sviluppo-success.svg)

Questo repository contiene un framework strutturato per lo sviluppo, il test e l'implementazione di reti di sensori basate su **Arduino Portenta H7**. 

Il progetto è stato sviluppato nell'ambito di un tirocinio di ricerca presso l'**INGV**, focalizzato, per il momento, sull'acquisizione dati, la comunicazione wireless tramite Bluetooth Low Energy (BLE) e l'ottimizzazione estrema dei consumi energetici (Deep Sleep) sfruttando l'architettura asimmetrica Dual-Core (Cortex-M7 + Cortex-M4) della scheda.

---

## 🏗️ Architettura del Progetto (Monorepo)

Per gestire una flotta di schede con firmware differenti senza duplicare il codice, il progetto è strutturato come un **Multi-Root Workspace** in PlatformIO.

* **⚙️ Configurazione Centralizzata**: Tutti i progetti ereditano le impostazioni hardware e le librerie base dal file `common_config.ini`. Questo garantisce che tutte le compilazioni siano uniformi (es. ottimizzazione `-O3` attivata per la massima velocità).
* **📁 Workspace VS Code**: Il file `PortentaSpace.code-workspace` gestisce l'interfaccia dell'IDE, separando la cartella Root (per le configurazioni globali e le librerie custom condivise) dai singoli script operativi. Questa architettura permette di gestire decine di script e firmware diversi all'interno di un unico repository: basta selezionare l'Environment desiderato nella barra di stato per riconfigurare istantaneamente i tasti di Build e Upload per lo specifico progetto o core su cui si sta lavorando.


---

## 🚀 Guida all'Installazione

Dato che il progetto usa una struttura avanzata a multi-ambiente, **non** basta aprire semplicemente la cartella principale. Segui questi passaggi per configurare l'IDE correttamente.

### Prerequisiti
1. Installa [Visual Studio Code](https://code.visualstudio.com/).
2. Installa l'estensione **PlatformIO IDE** all'interno di VS Code.
3. (Facoltativo) Installa l'app **nRF Connect** per testare la connettività Bluetooth. 

### Passaggi per il Setup
1. **Clona il repository** sul tuo computer:
   
2. **Apri VS Code**. 
   > ⚠️ **ATTENZIONE**: Per il corretto funzionamento di PlatformIO, **NON** usare il classico "Apri Cartella" (Open Folder) sulla root del repository.

3. Vai nel menu in alto su **File > Open Workspace from File...**

4. Seleziona il file `PortentaSpace.code-workspace` che trovi nella cartella principale.

---

## 🔵 Connettività BLE su Portenta H7

La Portenta H7 utilizza un modulo radio integrato che supporta lo standard **Bluetooth Low Energy (BLE)**. A differenza del Bluetooth classico, il BLE è ottimizzato per trasmissioni a basso consumo di piccoli pacchetti di dati, rendendolo ideale per nodi sensori alimentati a batteria.

### Funzionamento della libreria `ArduinoBLE`
Il framework si basa sulla libreria ufficiale `ArduinoBLE`, che astrae la complessità del protocollo tramite un'architettura a **Servizi** e **Caratteristiche**:

* **Central vs Peripheral**: 
    * Il **Peripheral** (il nostro sensore) espone dei dati.
    * Il **Central** (il nostro hub) scansiona l'etere, trova il peripheral tramite il suo indirizzo o un **UUID** (Unique Identifier) specifico e vi si connette.
* **Servizi e Caratteristiche**: I dati sono organizzati in *Service* (gruppi logici, es. "Dati Ambientali") che contengono una o più *Characteristic* (singoli dati, es. "Temperatura").
* **Protocollo di Scambio**: Il sistema utilizza i metodi `write()` per inviare comandi e `poll()` o i callback di notifica per leggere i dati in tempo reale.

### Integrazione Dual-Core e BLE
Nella configurazione avanzata (V2), lo stack BLE viene inizializzato e gestito interamente dal core **Cortex-M4**. Questo permette al core **Cortex-M7** (molto più energivoro) di rimanere in stato di stop o gestire task di calcolo pesante solo quando strettamente necessario, ottimizzando drasticamente l'efficienza energetica del dispositivo durante le fasi di advertising e connessione.

---

## 📂 Panoramica dei Progetti

Il framework è diviso in step incrementali di complessità. Ogni cartella all'interno di `Progetti/` contiene un esperimento o una versione specifica:

### 1. 💡 Blink (`Progetti/Blink`)
Il classico script di test hardware "Hello World" per verificare il corretto funzionamento, i driver USB e il caricamento del codice sulla Portenta H7.
> Il codice è stato preso dalla documentazione ufficiale Arduino seguendo [questo articolo](https://docs.arduino.cc/tutorials/portenta-h7/setting-up-portenta/)

### 2. 📡 Test BLE Preliminare (`Progetti/test_BLE_preliminare`)
Progetto base utile testare l'attivazione del modulo radio e l'esposizione di un servizio BLE (controllo LED da smartphone o da un altro central). 

> Il codice è stato preso dalla documentazione ufficiale Arduino seguendo [questo articolo](https://docs.arduino.cc/tutorials/portenta-h7/ble-connectivity/)

### 3. 🛜 Connessione BLE Bidirezionale (`Progetti/Connessione_BLE_V1`)
Implementa una comunicazione continua tra due schede Portenta H7:
* **Central**: Scansiona l'ambiente filtrando per un UUID specifico, si connette e resta in ascolto continuo dei dati.
* **Sensor (Peripheral)**: Invia dati (attualmente generati da un sensore fittizio) ogni secondo e riceve contemporaneamente stringhe di testo/comandi inviati dal Central. 

### 4. 🔋 BLE con Deep Sleep & Dual Core (`Progetti/Connessione_BLE_V2`)
*L'attuale fulcro dello sviluppo.* Questo modulo esplora il risparmio energetico spinto demandando i compiti di rete al core a basso consumo (M4).

* **Gestione M7 (Bootloader)**: Il core principale (M7) viene utilizzato esclusivamente per "svegliare" il core secondario, dopodiché viene messo in ibernazione profonda a tempo indeterminato tramite `rtos::ThisThread::flags_wait_any`.
* **Gestione M4 (Operativo)**: Il core M4 gestisce l'intero stack radio BLE in autonomia. L'invio dei dati è ottimizzato: avviene periodicamente o asincronamente se il Central invia un comando di lettura anticipata. Nei tempi morti, il thread sfrutta i cicli di idle di Mbed OS per abbattere ulteriormente gli assorbimenti.

---


## 🧪 Setup Sperimentale

La validazione del sistema e la caratterizzazione dei consumi energetici per la versione **V2 (Deep Sleep)** vengono effettuate in un ambiente di test controllato. 

* **Metodologia di Misura**: La Portenta H7 viene alimentata tramite un **alimentatore stabilizzato da banco**. La corrente assorbita viene misurata in tempo reale utilizzando un **multimetro digitale** posto in serie alla linea di alimentazione, permettendo di monitorare i picchi durante la trasmissione BLE e i minimi durante le fasi di ibernazione dei core.
* **Sorgente Dati**: In questa fase dello sviluppo, i dati trasmessi dal nodo sensore sono **generati via software** (dati fittizi). Questo approccio permette di isolare il consumo dello stack radio e della logica Dual-Core, eliminando le variabili di assorbimento della sensoristica esterna non ancora integrata.

---
Questo repository e la relativa documentazione nascono dall'esigenza di sistematizzare l'architettura del lavoro svolto. L'obiettivo è garantire la piena tracciabilità delle scelte progettuali e permettere una rapida replica dell'ambiente di sviluppo e della struttura del firmware in scenari futuri o per ulteriori sviluppi del progetto.