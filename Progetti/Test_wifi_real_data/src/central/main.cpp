#include <ArduinoBLE.h>
#include <WiFi.h>
#include <WiFiUdp.h>

char ssid[] = "OBS_Earth_Lab";    
char pass[] = "seismofaults2019"; 
const char * udpAddress = "10.201.9.20"; 
const int udpPort = 5005;
const int localPort = 5005; // Porta locale per ascolto/invio

const char * serviceUuid = "19b10000-e8f2-537e-4f6c-d104768a1214";
const char * dataCharUuid = "19b10001-e8f2-537e-4f6c-d104768a1214";

WiFiUDP udp;
bool bleConnected = false;
bool wifiConnected = false;
BLEDevice peripheralDevice;
BLECharacteristic dataChar;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\n--- GATEWAY DEBUG START ---");

  if (!BLE.begin()) {
    Serial.println("[ERR] Hardware BLE non risponde!");
    while (1);
  }
  
  Serial.println("[OK] BLE Inizializzato. Avvio scansione...");
  BLE.scanForUuid(serviceUuid);
}

void loop() {
  // FASE 1: CONNESSIONE BLE
  if (!bleConnected) {
    peripheralDevice = BLE.available();
    if (peripheralDevice && peripheralDevice.localName() == "Sismometro_M7") {
      Serial.print("[BLE] Trovato: "); Serial.println(peripheralDevice.address());
      BLE.stopScan();
      if (peripheralDevice.connect()) {
        Serial.println("[BLE] Connesso al sensore!");
        if (peripheralDevice.discoverAttributes()) {
          dataChar = peripheralDevice.characteristic(dataCharUuid);
          if (dataChar) {
            dataChar.subscribe();
            bleConnected = true;
            Serial.println("[BLE] Sottoscrizione OK. Avvio WiFi...");
            WiFi.begin(ssid, pass);
          } else {
            Serial.println("[ERR] Caratteristica non trovata!");
          }
        }
      } else {
        Serial.println("[ERR] Connessione fallita. Riprovo...");
        BLE.scanForUuid(serviceUuid);
      }
    }
  }

  // FASE 2: ATTENDI WIFI E APRI SOCKET UDP
  if (bleConnected && !wifiConnected) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiConnected = true;
      Serial.print("[WIFI] Connesso! IP: ");
      Serial.println(WiFi.localIP());
      
      // FONDAMENTALE: Apriamo il socket UDP locale
      if (udp.begin(localPort)) {
        Serial.println("[UDP] Socket locale aperto correttamente.");
      } else {
        Serial.println("[ERR] Impossibile aprire socket UDP!");
      }
      Serial.println("--> STREAMING ATTIVO");
    }
  }

  // FASE 3: STREAMING DATI CON LOGGING ERRORI
  if (bleConnected && wifiConnected && dataChar) {
    if (dataChar.valueUpdated()) {
      uint8_t buffer[70];
      int len = dataChar.readValue(buffer, 70);
      
      // Debug: beginPacket restituisce 1 se l'host è risolto
      int beginStatus = udp.beginPacket(udpAddress, udpPort);
      if (beginStatus == 1) {
        udp.write(buffer, len);
        if (udp.endPacket()) {
          Serial.print(">"); // Successo
        } else {
          Serial.print("!"); // Fallimento hardware/rete
        }
      } else {
        Serial.print("?"); // IP non raggiungibile o errore DNS
      }
    }
  }

  // GESTIONE DISCONNESSIONE
  if (bleConnected && !peripheralDevice.connected()) {
    Serial.println("\n[AVVISO] Connessione BLE persa!");
    bleConnected = false;
    wifiConnected = false;
    WiFi.disconnect();
    udp.stop(); // Chiudiamo il socket
    BLE.scanForUuid(serviceUuid);
  }
  
  BLE.poll();
  delay(1); // Piccolo respiro per la stabilità del sistema
}