# ⚙️ Configurations avancées - Bitaxe Monitor

## 🚀 Optimisations de performances

### Augmenter la fréquence du CPU

Dans `platformio.ini`, ajoutez:
```ini
build_flags = 
    -DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_240=1
    -DCONFIG_FREERTOS_HZ=1000
```

### Optimiser LVGL pour plus de fluidité

Dans `include/lv_conf.h`:
```c
#define LV_MEM_SIZE (256U * 1024U)  // Augmenter à 256KB
#define LV_DISP_DEF_REFR_PERIOD 16  // 60 FPS
#define LV_INDEV_DEF_READ_PERIOD 10 // Réactivité tactile
```

### Activer le cache SPI

Dans `src/main.cpp`:
```cpp
#define SPI_FREQUENCY 80000000  // 80 MHz au lieu de 40 MHz
```

## 🔌 API Bitaxe - Endpoints complets

### Informations système
```
GET http://[IP]/api/system/info
```

Réponse complète:
```json
{
  "power": 15.2,
  "voltage": 1200,
  "current": 2100,
  "temp": 65.0,
  "vrTemp": 55.0,
  "hashRate": 500.5,
  "bestDiff": "1234567890",
  "bestSessionDiff": "987654321",
  "freeHeap": 128000,
  "coreVoltage": 1200,
  "hostname": "bitaxe-ABC123",
  "ssid": "MonWiFi",
  "wifiStatus": "Connected!",
  "sharesAccepted": 1234,
  "sharesRejected": 5,
  "uptimeSeconds": 86400,
  "flipscreen": 0,
  "stratumURL": "public-pool.io",
  "stratumPort": 21496,
  "stratumUser": "bc1q...",
  "version": "v2.0.0",
  "boardVersion": "203",
  "asicCount": 1,
  "smallCoreCount": 224,
  "asicFrequency": 485,
  "asicModel": "BM1397",
  "fanSpeed": 3000,
  "fanPerc": 75
}
```

### Contrôler le Bitaxe

#### Redémarrer
```bash
curl -X POST http://[IP]/api/system/restart
```

#### Changer la fréquence
```bash
curl -X POST http://[IP]/api/system \
  -H "Content-Type: application/json" \
  -d '{"frequency": 525}'
```

#### Contrôler le ventilateur
```bash
curl -X POST http://[IP]/api/system \
  -H "Content-Type: application/json" \
  -d '{"fanspeed": 80}'  # 0-100%
```

## 📡 Ajout de fonctionnalités

### 1. Notification push (Telegram)

Ajoutez dans `platformio.ini`:
```ini
lib_deps = 
    witnessmenow/UniversalTelegramBot@^1.3.0
```

Dans `src/main.cpp`:
```cpp
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define BOT_TOKEN "VOTRE_TOKEN_BOT"
#define CHAT_ID "VOTRE_CHAT_ID"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

void send_telegram_alert(String message) {
    secured_client.setInsecure();
    bot.sendMessage(CHAT_ID, message, "");
}

// Dans check_miner_alerts():
if (active_miners < NUM_MINERS) {
    send_telegram_alert("⚠️ Alerte! Mineur hors ligne");
}
```

### 2. Logs sur carte SD

Ajoutez un lecteur microSD sur ESP32:
```cpp
#include <SD.h>
#include <SPI.h>

#define SD_CS 5

void setup_sd() {
    if (!SD.begin(SD_CS)) {
        Serial.println("Erreur carte SD");
        return;
    }
    Serial.println("Carte SD OK");
}

void log_to_sd(String data) {
    File file = SD.open("/bitaxe_log.txt", FILE_APPEND);
    if (file) {
        file.println(data);
        file.close();
    }
}
```

### 3. Mode nuit automatique

```cpp
// Ajouter dans les variables globales
bool night_mode = false;
int night_start_hour = 22;  // 22h
int night_end_hour = 7;     // 7h

void check_night_mode() {
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    int hour = timeinfo.tm_hour;
    
    bool should_be_night = (hour >= night_start_hour || hour < night_end_hour);
    
    if (should_be_night && !night_mode) {
        // Activer mode nuit
        night_mode = true;
        lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);
        lv_obj_set_style_opa(main_screen, LV_OPA_50, 0);  // 50% opacité
        pinMode(TFT_BL, OUTPUT);
        analogWrite(TFT_BL, 50);  // Luminosité 20%
    } else if (!should_be_night && night_mode) {
        // Désactiver mode nuit
        night_mode = false;
        lv_obj_set_style_bg_color(main_screen, COLOR_BG, 0);
        lv_obj_set_style_opa(main_screen, LV_OPA_100, 0);
        analogWrite(TFT_BL, 255);  // Luminosité 100%
    }
}

// Appeler dans loop() toutes les minutes
```

### 4. Affichage de graphiques historiques

```cpp
#include <vector>

// Structures pour historique
std::vector<float> hashrate_history;
std::vector<float> temp_history;
const int MAX_HISTORY = 100;

void add_to_history(std::vector<float>& history, float value) {
    history.push_back(value);
    if (history.size() > MAX_HISTORY) {
        history.erase(history.begin());
    }
}

void create_chart() {
    lv_obj_t *chart = lv_chart_create(main_screen);
    lv_obj_set_size(chart, 400, 200);
    lv_obj_align(chart, LV_ALIGN_CENTER, 0, 0);
    
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 600);
    
    lv_chart_series_t *ser = lv_chart_add_series(chart, COLOR_RED, LV_CHART_AXIS_PRIMARY_Y);
    
    for (float value : hashrate_history) {
        lv_chart_set_next_value(chart, ser, (int)value);
    }
}
```

### 5. OTA (Over-The-Air) Updates

```cpp
#include <ArduinoOTA.h>

void setup_ota() {
    ArduinoOTA.setHostname("bitaxe-monitor");
    ArduinoOTA.setPassword("admin"); // Changez le mot de passe!
    
    ArduinoOTA.onStart([]() {
        Serial.println("Début mise à jour OTA");
    });
    
    ArduinoOTA.onEnd([]() {
        Serial.println("Fin mise à jour OTA");
    });
    
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Erreur OTA [%u]: ", error);
    });
    
    ArduinoOTA.begin();
}

// Dans loop()
void loop() {
    ArduinoOTA.handle();
    // ... reste du code
}
```

Uploader via OTA:
```bash
pio run -t upload --upload-port 192.168.1.150
```

## 🎨 Thèmes personnalisés

### Thème Matrix (vert sur noir)
```cpp
#define COLOR_BG lv_color_hex(0x000000)
#define COLOR_PRIMARY lv_color_hex(0x00FF00)
#define COLOR_DARK lv_color_hex(0x003300)
```

### Thème Cyberpunk (cyan et magenta)
```cpp
#define COLOR_BG lv_color_hex(0x0D0221)
#define COLOR_PRIMARY lv_color_hex(0xFF006E)
#define COLOR_SECONDARY lv_color_hex(0x00F0FF)
```

### Thème Bitcoin (orange)
```cpp
#define COLOR_BG lv_color_hex(0x000000)
#define COLOR_PRIMARY lv_color_hex(0xF7931A)
#define COLOR_DARK lv_color_hex(0x4A2511)
```

## 📊 Monitoring avancé

### Calculer le revenu estimé

```cpp
float btc_per_th_per_day = 0.00000015;  // À ajuster selon la difficulté
float electricity_cost_kwh = 0.15;      // €/kWh
float power_w = 15.2 * NUM_MINERS;      // Watts total

float calculate_daily_revenue() {
    float gh_to_th = total_hashrate / 1000.0;
    float btc_per_day = gh_to_th * btc_per_th_per_day;
    float revenue_usd = btc_per_day * btc_price;
    float cost_usd = (power_w * 24 / 1000.0) * electricity_cost_kwh;
    return revenue_usd - cost_usd;
}
```

### Pool mining stats

```cpp
void get_pool_stats() {
    HTTPClient http;
    String url = "https://public-pool.io/api/client/";
    url += WALLET_ADDRESS;
    
    http.begin(url);
    if (http.GET() == HTTP_CODE_OK) {
        String payload = http.getString();
        // Parser les stats de la pool
    }
    http.end();
}
```

## 🔒 Sécurité

### 1. Protéger avec un mot de passe

```cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
const char* www_username = "admin";
const char* www_password = "bitaxe2024";

void setup_web_auth() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        if(!request->authenticate(www_username, www_password)){
            return request->requestAuthentication();
        }
        request->send(200, "text/plain", "Dashboard protégé");
    });
    server.begin();
}
```

### 2. Chiffrer les credentials WiFi

```cpp
#include <Preferences.h>

Preferences preferences;

void save_encrypted_wifi(String ssid, String password) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("pass", password);
    preferences.end();
}
```

## 🌡️ Capteurs additionnels

### DHT22 (température/humidité ambiante)

```cpp
#include <DHT.h>

#define DHT_PIN 4
#define DHT_TYPE DHT22

DHT dht(DHT_PIN, DHT_TYPE);

void read_ambient_sensor() {
    float temp = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    char buf[32];
    sprintf(buf, "Amb: %.1f°C %.0f%%", temp, humidity);
    // Afficher sur l'écran
}
```

## 🎯 Optimisations mémoire

### Libérer de la mémoire
```cpp
// Dans platformio.ini
build_flags = 
    -DCORE_DEBUG_LEVEL=0  # Désactiver debug
    -DCONFIG_ARDUHAL_LOG_COLORS=0
```

### Monitorer la mémoire
```cpp
void print_memory_info() {
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.printf("PSRAM free: %d bytes\n", ESP.getFreePsram());
}
```

## 📱 Interface web complémentaire

```cpp
AsyncWebServer server(80);

void setup_web_interface() {
    server.on("/api/stats", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = "{";
        json += "\"hashrate\":" + String(total_hashrate) + ",";
        json += "\"temp\":" + String(avg_temperature) + ",";
        json += "\"miners\":" + String(active_miners);
        json += "}";
        request->send(200, "application/json", json);
    });
    
    server.begin();
}
```

Accéder via: `http://[IP_ESP32]/api/stats`

---

**Bon hacking! 🔥**
