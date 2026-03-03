# 🎨 Exemples de personnalisation

Ce fichier contient des exemples de code prêts à l'emploi pour personnaliser votre moniteur Bitaxe.

## 🎨 Changer le thème de couleur

### Thème "Matrix" (vert phosphorescent)

```cpp
// Remplacez dans src/main.cpp (lignes ~20-25)

#define COLOR_BG lv_color_hex(0x000000)          // Fond noir pur
#define COLOR_RED lv_color_hex(0x00FF00)         // Vert Matrix
#define COLOR_DARK_RED lv_color_hex(0x003300)    // Vert foncé
#define COLOR_CRIMSON lv_color_hex(0x00AA00)     // Vert moyen
#define COLOR_TEXT lv_color_hex(0x00FF00)        // Texte vert
#define COLOR_GRAY lv_color_hex(0x001100)        // Gris verdâtre
```

### Thème "Cyberpunk 2077" (rose et cyan)

```cpp
#define COLOR_BG lv_color_hex(0x0D0221)          // Violet très foncé
#define COLOR_RED lv_color_hex(0xFF006E)         // Rose vif
#define COLOR_DARK_RED lv_color_hex(0x8B0058)    // Rose foncé
#define COLOR_CRIMSON lv_color_hex(0xFF1493)     // Rose intense
#define COLOR_TEXT lv_color_hex(0x00F0FF)        // Cyan clair
#define COLOR_GRAY lv_color_hex(0x2A1B3D)        // Gris violet
```

### Thème "Bitcoin Orange"

```cpp
#define COLOR_BG lv_color_hex(0x000000)          // Fond noir
#define COLOR_RED lv_color_hex(0xF7931A)         // Orange Bitcoin
#define COLOR_DARK_RED lv_color_hex(0x8B4513)    // Orange foncé
#define COLOR_CRIMSON lv_color_hex(0xFF8C00)     // Orange vif
#define COLOR_TEXT lv_color_hex(0xFFFFFF)        // Blanc
#define COLOR_GRAY lv_color_hex(0x2F2F2F)        // Gris foncé
```

## ⚡ Ajouter un nouveau panneau

```cpp
// Ajouter après la création du right_panel dans create_futuristic_ui()

// ========== PANNEAU INFÉRIEUR - STATISTIQUES ==========
lv_obj_t *stats_panel = lv_obj_create(main_screen);
lv_obj_set_size(stats_panel, SCREEN_WIDTH - 40, 100);
lv_obj_align(stats_panel, LV_ALIGN_BOTTOM_MID, 0, -60);
lv_obj_set_style_bg_color(stats_panel, lv_color_hex(0x1A1A1A), 0);
lv_obj_set_style_radius(stats_panel, 10, 0);
create_glowing_border(stats_panel, COLOR_RED);

lv_obj_t *stats_title = lv_label_create(stats_panel);
lv_label_set_text(stats_title, "📊 STATISTIQUES 24H");
lv_obj_set_style_text_color(stats_title, COLOR_RED, 0);
lv_obj_set_style_text_font(stats_title, &lv_font_montserrat_16, 0);
lv_obj_align(stats_title, LV_ALIGN_TOP_MID, 0, 5);

lv_obj_t *label_shares = lv_label_create(stats_panel);
lv_label_set_text(label_shares, "Shares: 0 accepted / 0 rejected");
lv_obj_set_style_text_color(label_shares, COLOR_TEXT, 0);
lv_obj_align(label_shares, LV_ALIGN_TOP_LEFT, 10, 40);

lv_obj_t *label_uptime = lv_label_create(stats_panel);
lv_label_set_text(label_uptime, "Uptime: 0d 0h 0m");
lv_obj_set_style_text_color(label_uptime, COLOR_TEXT, 0);
lv_obj_align(label_uptime, LV_ALIGN_TOP_LEFT, 10, 70);
```

## 🔔 Ajouter des alertes sonores

```cpp
// Ajouter en haut du fichier avec les autres #define
#define BUZZER_PIN 17

// Dans setup()
pinMode(BUZZER_PIN, OUTPUT);

// Fonction pour jouer un son
void play_alert_sound(int duration_ms = 100) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration_ms);
    digitalWrite(BUZZER_PIN, LOW);
}

// Dans animate_new_block() - ajouter à la fin
play_alert_sound(200);  // Bip de 200ms pour nouveau bloc

// Dans check_miner_alerts() - ajouter quand alerte
if (active_miners < NUM_MINERS) {
    static unsigned long last_beep = 0;
    if (millis() - last_beep > 5000) {  // Bip toutes les 5 secondes
        play_alert_sound(100);
        last_beep = millis();
    }
}
```

## 📊 Ajouter un graphique

```cpp
// Ajouter en haut avec les variables globales
lv_obj_t *chart_hashrate;
lv_chart_series_t *chart_series;
float hashrate_history[50] = {0};
int history_index = 0;

// Dans create_futuristic_ui() - ajouter après center_panel
chart_hashrate = lv_chart_create(main_screen);
lv_obj_set_size(chart_hashrate, 300, 150);
lv_obj_align(chart_hashrate, LV_ALIGN_LEFT_MID, 20, -100);
lv_chart_set_type(chart_hashrate, LV_CHART_TYPE_LINE);
lv_chart_set_range(chart_hashrate, LV_CHART_AXIS_PRIMARY_Y, 0, 600);
lv_chart_set_point_count(chart_hashrate, 50);
lv_obj_set_style_bg_color(chart_hashrate, lv_color_hex(0x1A1A1A), 0);
create_glowing_border(chart_hashrate, COLOR_RED);

chart_series = lv_chart_add_series(chart_hashrate, COLOR_RED, LV_CHART_AXIS_PRIMARY_Y);

// Dans update_bitaxe_data() - après calcul du hashrate
hashrate_history[history_index] = total_hashrate;
history_index = (history_index + 1) % 50;

// Mettre à jour le graphique
for (int i = 0; i < 50; i++) {
    int idx = (history_index + i) % 50;
    lv_chart_set_next_value(chart_hashrate, chart_series, (int)hashrate_history[idx]);
}
lv_chart_refresh(chart_hashrate);
```

## 🌡️ Ajouter capteur de température DHT22

```cpp
// Ajouter dans platformio.ini
lib_deps = 
    adafruit/DHT sensor library@^1.4.4

// Ajouter en haut de main.cpp
#include <DHT.h>
#define DHT_PIN 35
#define DHT_TYPE DHT22
DHT dht(DHT_PIN, DHT_TYPE);

// Variables globales
float ambient_temp = 0;
float ambient_humidity = 0;
lv_obj_t *label_ambient;

// Dans setup()
dht.begin();

// Dans create_futuristic_ui() - ajouter un label
label_ambient = lv_label_create(left_panel);
lv_label_set_text(label_ambient, "Room: --°C --%%");
lv_obj_set_style_text_color(label_ambient, COLOR_TEXT, 0);
lv_obj_align(label_ambient, LV_ALIGN_TOP_LEFT, 10, 130);

// Nouvelle fonction
void update_ambient_sensor() {
    ambient_temp = dht.readTemperature();
    ambient_humidity = dht.readHumidity();
    
    if (!isnan(ambient_temp) && !isnan(ambient_humidity)) {
        char buf[32];
        sprintf(buf, "Room: %.1f°C %.0f%%", ambient_temp, ambient_humidity);
        lv_label_set_text(label_ambient, buf);
    }
}

// Dans loop() - appeler toutes les 10 secondes
static unsigned long last_ambient_update = 0;
if (millis() - last_ambient_update > 10000) {
    update_ambient_sensor();
    last_ambient_update = millis();
}
```

## 💰 Calculer et afficher la rentabilité

```cpp
// Ajouter avec les variables globales
float daily_revenue = 0;
float daily_cost = 0;
float daily_profit = 0;
lv_obj_t *label_profit;

// Paramètres (à ajuster)
float btc_per_th_per_day = 0.00000015;  // BTC/TH/jour
float electricity_cost_kwh = 0.15;       // €/kWh
float power_per_miner = 15.0;            // Watts par mineur

// Fonction de calcul
void calculate_profitability() {
    // Hashrate en TH/s
    float th_total = total_hashrate / 1000.0;
    
    // Revenus en BTC puis en USD
    float btc_per_day = th_total * btc_per_th_per_day;
    daily_revenue = btc_per_day * btc_price;
    
    // Coût électricité
    float power_total_kw = (power_per_miner * active_miners) / 1000.0;
    daily_cost = power_total_kw * 24 * electricity_cost_kwh;
    
    // Profit net
    daily_profit = daily_revenue - daily_cost;
}

// Dans create_futuristic_ui() - ajouter dans right_panel
label_profit = lv_label_create(right_panel);
lv_label_set_text(label_profit, "Profit: $--/day");
lv_obj_set_style_text_color(label_profit, COLOR_TEXT, 0);
lv_obj_align(label_profit, LV_ALIGN_TOP_LEFT, 10, 130);

// Dans update_mempool_data() - après avoir obtenu le prix
calculate_profitability();
char buf[32];
sprintf(buf, "Profit: $%.2f/day", daily_profit);
lv_label_set_text(label_profit, buf);

// Changer couleur selon profit
if (daily_profit > 0) {
    lv_obj_set_style_text_color(label_profit, lv_color_hex(0x00FF00), 0);
} else {
    lv_obj_set_style_text_color(label_profit, COLOR_RED, 0);
}
```

## 📡 Notifications Telegram

```cpp
// Ajouter dans platformio.ini
lib_deps = 
    witnessmenow/UniversalTelegramBot@^1.3.0

// Ajouter dans config.h
#define TELEGRAM_BOT_TOKEN "VOTRE_TOKEN_ICI"
#define TELEGRAM_CHAT_ID "VOTRE_CHAT_ID_ICI"

// Ajouter en haut de main.cpp
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

WiFiClientSecure secured_client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, secured_client);

// Dans setup()
secured_client.setInsecure();

// Fonction d'envoi
void send_telegram(String message) {
    if (WiFi.status() == WL_CONNECTED) {
        bot.sendMessage(TELEGRAM_CHAT_ID, message, "");
    }
}

// Dans animate_new_block()
send_telegram("🎉 Nouveau bloc trouvé! Block " + String(block_height));

// Dans check_miner_alerts() - quand alerte
if (active_miners < NUM_MINERS && !miner_alert) {
    String msg = "⚠️ Alerte: " + String(NUM_MINERS - active_miners) + " mineur(s) hors ligne!";
    send_telegram(msg);
}

// Quand température critique
if (avg_temperature > TEMP_CRITICAL_THRESHOLD) {
    static bool temp_alert_sent = false;
    if (!temp_alert_sent) {
        String msg = "🔥 Alerte température: " + String(avg_temperature) + "°C";
        send_telegram(msg);
        temp_alert_sent = true;
    }
} else {
    temp_alert_sent = false;
}
```

## 🕒 Changer le format de l'heure

```cpp
// Format 12h (AM/PM) au lieu de 24h
void update_center_display() {
    char buf[64];
    
    switch (center_view) {
        case 0: // Heure
            {
                time_t now = time(nullptr);
                struct tm timeinfo;
                localtime_r(&now, &timeinfo);
                
                int hour12 = timeinfo.tm_hour % 12;
                if (hour12 == 0) hour12 = 12;
                const char* ampm = (timeinfo.tm_hour >= 12) ? "PM" : "AM";
                
                sprintf(buf, "%02d:%02d:%02d %s", hour12, timeinfo.tm_min, timeinfo.tm_sec, ampm);
            }
            break;
        // ... reste du code
    }
    
    lv_label_set_text(center_label_big, buf);
}
```

## 🎨 Animation avancée pour nouveau bloc

```cpp
void animate_new_block() {
    // Animation plus complexe avec plusieurs éléments
    
    // 1. Flash complet de l'écran
    lv_obj_set_style_bg_color(main_screen, COLOR_RED, 0);
    delay(50);
    lv_obj_set_style_bg_color(main_screen, COLOR_BG, 0);
    
    // 2. Pulse sur les bordures
    for (int i = 0; i < 3; i++) {
        create_glowing_border(top_bar, COLOR_CRIMSON);
        create_glowing_border(center_panel, COLOR_CRIMSON);
        delay(100);
        create_glowing_border(top_bar, COLOR_RED);
        create_glowing_border(center_panel, COLOR_RED);
        delay(100);
    }
    
    // 3. Afficher temporairement un message
    lv_obj_t *msg = lv_label_create(main_screen);
    lv_label_set_text(msg, "⚡ NEW BLOCK MINED! ⚡");
    lv_obj_set_style_text_color(msg, COLOR_RED, 0);
    lv_obj_set_style_text_font(msg, &lv_font_montserrat_32, 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, 0);
    
    // Animation fade out
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, msg);
    lv_anim_set_time(&a, 2000);
    lv_anim_set_values(&a, LV_OPA_100, LV_OPA_0);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_style_opa);
    lv_anim_set_ready_cb(&a, [](lv_anim_t * a){
        lv_obj_del((lv_obj_t*)a->var);
    });
    lv_anim_start(&a);
    
    Serial.println("⚡ NOUVEAU BLOC MINÉ! ⚡");
}
```

## 🌐 Ajouter une page web de stats

```cpp
// Ajouter dans platformio.ini
lib_deps = 
    me-no-dev/ESPAsyncWebServer@^1.2.3

// Ajouter en haut de main.cpp
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

// Fonction pour créer le JSON des stats
String get_stats_json() {
    StaticJsonDocument<512> doc;
    doc["hashrate"] = total_hashrate;
    doc["temperature"] = avg_temperature;
    doc["miners_active"] = active_miners;
    doc["miners_total"] = NUM_MINERS;
    doc["btc_price"] = btc_price;
    doc["block_height"] = block_height;
    doc["fees"] = avg_fees;
    
    String output;
    serializeJson(doc, output);
    return output;
}

// Dans setup() - après setup_wifi()
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><head><title>Bitaxe Monitor</title></head>";
    html += "<body style='background:#0A0A0A;color:#FF0000;font-family:monospace;'>";
    html += "<h1>⚡ BITAXE MONITOR ⚡</h1>";
    html += "<p>Hashrate: " + String(total_hashrate) + " GH/s</p>";
    html += "<p>Temperature: " + String(avg_temperature) + " °C</p>";
    html += "<p>Active miners: " + String(active_miners) + "/" + String(NUM_MINERS) + "</p>";
    html += "<p>BTC Price: $" + String(btc_price) + "</p>";
    html += "<p><a href='/api/stats'>JSON API</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
});

server.on("/api/stats", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", get_stats_json());
});

server.begin();
Serial.println("Serveur web démarré sur http://" + WiFi.localIP().toString());
```

---

**Amusez-vous à personnaliser! 🎨⚡**
