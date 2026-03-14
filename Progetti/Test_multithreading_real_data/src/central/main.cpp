#include <ArduinoBLE.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// Configurazione Rete
char ssid[] = "OBS_Earth_Lab";    
char pass[] = "seismofaults2019"; 
const char* udpAddress = "10.201.9.20"; 
const int udpPort = 5005;

WiFiUDP udp;

// Gestione Multi-Sensore
const int NUM_SENSORS = 2;
const char* targetNames[] = {"Sismometro_01", "Sismometro_02"};
BLEDevice sensors[NUM_SENSORS];
BLECharacteristic dataChars[NUM_SENSORS];
bool connected[NUM_SENSORS] = {false, false};

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Errore BLE!");
    while (1);
  }

  Serial.println("GATEWAY: Avvio WiFi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  udp.begin(5005);
  Serial.println("WiFi Connesso. Avvio scansione sensori...");
  BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
}

void loop() {
  // 1. GESTIONE CONNESSIONI
  BLEDevice peripheral = BLE.available();
  if (peripheral) {
    for (int i = 0; i < NUM_SENSORS; i++) {
      if (!connected[i] && peripheral.localName() == targetNames[i]) {
        BLE.stopScan();
        if (peripheral.connect()) {
          if (peripheral.discoverAttributes()) {
            dataChars[i] = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");
            dataChars[i].subscribe();
            sensors[i] = peripheral;
            connected[i] = true;
            Serial.print(">>> Connesso a "); Serial.println(targetNames[i]);
          }
        }
        BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
      }
    }
  }

  // 2. LETTURA E INOLTRO DATI
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (connected[i]) {
      if (sensors[i].connected()) {
        if (dataChars[i].valueUpdated()) {
          uint8_t bleBuffer[66];
          dataChars[i].readValue(bleBuffer, 66);

          // Inviato via UDP: ID (1 byte) + Dati (66 byte) = 67 byte
          udp.beginPacket(udpAddress, udpPort);
          uint8_t sensorID = (uint8_t)(i + 1); 
          udp.write(sensorID);
          udp.write(bleBuffer, 66);
          udp.endPacket();
        }
      } else {
        Serial.print("!!! Perso "); Serial.println(targetNames[i]);
        connected[i] = false;
      }
    }
  }
  
  BLE.poll();
}