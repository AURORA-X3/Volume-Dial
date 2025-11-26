#include <WiFi.h>
#include <Wire.h>

const char* ssid     = "ESP32_VOLUME_AP";
const char* password = "12345678";
const char* SERVER_IP = "192.168.4.1";
const int SERVER_PORT = 4210;

WiFiClient client;

#define AS5600_ADDR 0x36

#define RAWANGLE1 0x0C     // RAWANGLE1
#define RAWANGLE2 0x0D     // RAWANGLE2

typedef union {
  uint16_t _16byte;
  struct {
    uint16_t byte_1 : 8;
    uint16_t byte_2 : 8;
  } bytes;
} _16byte;

_16byte rawangle;
                    //en, dir
int driver_pins[3] = {27, 14};

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(driver_pins[0], OUTPUT);
  pinMode(driver_pins[1], OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connexion au WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }

  Serial.println("\nConnecté !");
  
  while (!client.connected()) {
      Serial.println("Connexion au serveur...");
      if (client.connect(SERVER_IP, SERVER_PORT)) {
          Serial.println("Connecté au serveur !");
          break;
      }
      delay(500);
  }


  Serial.println("Initialisation du bus I2C...");
  Wire.begin(21, 22); //esp32 wroom 32

  digitalWrite(driver_pins[0], HIGH);
  digitalWrite(driver_pins[1], LOW);

  delay(100);
}

float angle;
bool switchVal = 0;
int lastDir, dir = NULL;
bool trig = 0;
int volume = 0;


int lastSentValue = 0;     // dernière valeur envoyée
unsigned long lastChange = 0; // moment où la valeur a changé
const unsigned long stableTime = 100; // temps stable requis en ms

void loop() {
  wifiReception();
  angleProcessing();

  int newValue = volume; // ici tu pourrais lire un capteur ou changer la variable

  // --- Si stable depuis stableTime ms, envoyer ---
  if (millis() - lastChange >= stableTime) {
    
    
    if (trig == 0) {
      Serial.println(volume);
      wifiTransmission(volume);
      lastSentValue = newValue;
      trig = 1;
    }
  }

  // --- Vérifier si la valeur a changé ---
  if (newValue != lastSentValue) {
    lastChange = millis();  // reset timer
    lastSentValue = newValue;
    trig = 0;
  }



  delay(1);
}

void angleProcessing(){
  rawangle.bytes.byte_2 = readRegister(AS5600_ADDR, RAWANGLE1); //MSB
  rawangle.bytes.byte_1 = readRegister(AS5600_ADDR, RAWANGLE2); //LSB

  angle = float(rawangle._16byte) / 4095 * 360;

  if(switchVal == 0){
    switchVal = 1;
    lastDir = angle;
  }else{
    switchVal = 0;
    dir = angle;
  }

  if(lastDir != NULL && dir != NULL){


    if((dir - lastDir > 0 || dir - lastDir < -180) && volume < 100){
      volume++;
      //wifiTransmission(volume);
      Serial.println(volume);
    }else if((dir - lastDir < 0 || dir - lastDir > 180) && volume > 0){
      volume--;
      //wifiTransmission(volume);
      Serial.println(volume);
    }

    lastDir = dir;
  }
}

void wifiReception(){
  // Réception du serveur → affichage
  if (client.available()) {
    String line = client.readStringUntil('\n');
    volume = line.toInt();
    lastSentValue = volume;
    Serial.println(line.toInt());
    line.trim();

    if (line.length() > 0) {
      Serial.print("Volume reçu : ");
      Serial.println(line);
    }
  }
}

void wifiTransmission(int value){
  // Envoi vers le serveur si valeur entrée dans Serial Monitor
  String val = String(value);
  val.trim();

  if (val.length() > 0) {
    client.println("SET:" + val);
  }

}


void writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readRegister(uint8_t deviceAddress, uint8_t registerAddress) {
  Wire.beginTransmission(deviceAddress);
  Wire.write(registerAddress);
  if (Wire.endTransmission(false) != 0) {
    Serial.println("Erreur I2C : impossible d'accéder au capteur !");
    return 0;
  }

  Wire.requestFrom(deviceAddress, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  } else {
    Serial.println("Erreur : aucune donnée reçue !");
    return 0;
  }
}