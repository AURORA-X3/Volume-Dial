#include <M5Unified.h>
#include <WiFi.h>

#ifndef BOARD_SDIO_ESP_HOSTED_CLK
  #define SDIO2_CLK GPIO_NUM_12
  #define SDIO2_CMD GPIO_NUM_13
  #define SDIO2_D0  GPIO_NUM_11
  #define SDIO2_D1  GPIO_NUM_10
  #define SDIO2_D2  GPIO_NUM_9
  #define SDIO2_D3  GPIO_NUM_8
  #define SDIO2_RST GPIO_NUM_15
#endif

// WiFi & serveur
const char* WIFI_SSID = "ESP32_VOLUME_AP";
const char* WIFI_PASSWORD = "12345678";
const char* SERVER_IP = "192.168.4.1";
const int SERVER_PORT = 4210;

WiFiClient client;
int volumeValue = 0;       // valeur actuelle
int lastVolumeValue = 0;   // dernière valeur affichée

// Timing
static const uint32_t kReconnectIntervalMs = 2000;
static const uint32_t kReadIntervalMs = 50;
uint32_t lastReconnectMs = 0;
uint32_t lastReadMs = 0;

// Barre de volume
const int barX = 40;
const int barY = 100;
const int barWidth = 240;
const int barHeight = 30;

// ------------------ Affichage ------------------

// Affichage initial complet
void drawVolumeBar(int val) {
    auto gfx = M5.Display;
    gfx.fillScreen(BLACK);

    gfx.setTextSize(3);
    gfx.setTextColor(WHITE, BLACK);
    gfx.setCursor(20, 30);
    gfx.printf("Volume : %d", val);

    gfx.drawRect(barX, barY, barWidth, barHeight, WHITE);

    int fillWidth = map(val, 0, 100, 0, barWidth - 2);
    gfx.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, GREEN);

    lastVolumeValue = val;
}

// Mise à jour partielle
void updateVolumeBar(int val) {
    if (val == lastVolumeValue) return;

    auto gfx = M5.Display;

    // Efface la barre précédente
    gfx.fillRect(barX + 1, barY + 1, barWidth - 2, barHeight - 2, BLACK);
    gfx.drawRect(barX, barY, barWidth, barHeight, WHITE);

    // Remplissage
    int fillWidth = map(val, 0, 100, 0, barWidth - 2);
    if (fillWidth > 0) gfx.fillRect(barX + 1, barY + 1, fillWidth, barHeight - 2, GREEN);

    // Efface texte précédent
    gfx.fillRect(20, 30, 220, 30, BLACK);
    gfx.setTextSize(3);
    gfx.setTextColor(WHITE, BLACK);
    gfx.setCursor(20, 30);
    gfx.printf("Volume : %d", val);

    lastVolumeValue = val;
}

// ------------------ WiFi ------------------
bool connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        M5.update();
        delay(200);
    }
    return WiFi.status() == WL_CONNECTED;
}

// ------------------ Setup ------------------
void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    Serial.begin(115200);

    M5.Display.setRotation(1);
    M5.Display.setBrightness(200);
    M5.Display.fillScreen(BLACK);

    #ifdef BOARD_SDIO_ESP_HOSTED_CLK
    WiFi.setPins(BOARD_SDIO_ESP_HOSTED_CLK, BOARD_SDIO_ESP_HOSTED_CMD,
                 BOARD_SDIO_ESP_HOSTED_D0, BOARD_SDIO_ESP_HOSTED_D1,
                 BOARD_SDIO_ESP_HOSTED_D2, BOARD_SDIO_ESP_HOSTED_D3,
                 BOARD_SDIO_ESP_HOSTED_RESET);
    #else
    WiFi.setPins(SDIO2_CLK, SDIO2_CMD, SDIO2_D0, SDIO2_D1, SDIO2_D2, SDIO2_D3, SDIO2_RST);
    #endif

    M5.Display.setTextSize(3);
    M5.Display.setTextColor(WHITE);
    M5.Display.setCursor(10, 10);
    M5.Display.println("Connexion WiFi...");

    if (!connectWiFi()) {
        M5.Display.println("Erreur WiFi !");
        while (true) { M5.update(); delay(1000); }
    }

    M5.Display.println("WiFi OK !");
    drawVolumeBar(0); // initialisation
}

// ------------------ Loop ------------------
void loop() {
    M5.update();
    uint32_t now = millis();

    // Reconnexion TCP si nécessaire
    if (!client.connected() && now - lastReconnectMs >= kReconnectIntervalMs) {
        lastReconnectMs = now;
        if (client.connect(SERVER_IP, SERVER_PORT, 2000)) {
            Serial.println("TCP connecté");
        } else {
            Serial.println("Erreur TCP");
        }
    }

    // Lecture TCP
    if (client.connected() && client.available() && now - lastReadMs >= kReadIntervalMs) {
        lastReadMs = now;
        String line = client.readStringUntil('\n');
        line.trim();

        // Filtrage : ignorer vide ou non numérique
        if (line.length() == 0) return;
        if (line.startsWith("SET:")) line = line.substring(4);
        line.trim();
        if (line.length() == 0) return;
        for (uint8_t i = 0; i < line.length(); i++) {
            if (!isDigit(line[i])) return; // ignore invalide
        }

        int val = line.toInt();
        if (val >= 0 && val <= 100 && val != volumeValue) {
            volumeValue = val;
            updateVolumeBar(volumeValue);
        }
    }

    delay(10);
}
