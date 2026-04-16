#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include <Preferences.h>
#include <AiEsp32RotaryEncoder.h>
#include "RadioWebHandler.h"

// --- НАСТРОЙКИ ПИНОВ ---
#define I2S_DOUT    12
#define I2S_BCLK    11
#define I2S_LRC     10
#define SD_MODE     9

#define ENC_CLK     6
#define ENC_DT      5
#define ENC_SW      7
#define VCC_PIN     -1
#define ENC_STEPS   4

// --- ОБЪЕКТЫ ---
AiEsp32RotaryEncoder encoder = AiEsp32RotaryEncoder(ENC_DT, ENC_CLK, ENC_SW, VCC_PIN, ENC_STEPS);
Audio audio;
Preferences preferences;
RadioWebHandler webHandler;

// --- ДАННЫЕ ---
const char* ssid = "****";
const char* password = "******";

struct Station {
  const char* name;
  const char* url;
};

Station stationList[] = {
    {"Наше радио", "http://nashe3.hostingradio.ru/nashe-128.mp3.m3u"},
    {"Искатель", "https://iskatel.hostingradio.ru:8015/iskatel-128.mp3"},
    {"Русские песни", "http://listen.rusongs.ru/ru-mp3-128"},
    {"Рок-Атака", "https://listen7.myradio24.com/rockataka"},
    {"Angels Radio", "http://channel.angelsradio.ru:9000/angels"},
    {"Русский рок 24", "https://stream.vyshka24.ru/russianrok24"}
};
const int stationsCount = sizeof(stationList) / sizeof(Station);

int currentStationIdx = 0;
bool isMuted = false;
long lastEncoderValue = 0;
unsigned long lastClickTime = 0;
const unsigned long doubleClickDelay = 400;

// --- ЛОГИКА ---

String getStatusPayload() {
    JsonDocument doc;
    doc["vol"] = audio.getVolume();
    doc["cur"] = currentStationIdx;
    doc["mute"] = isMuted;
    JsonArray list = doc["list"].to<JsonArray>();
    for(int i=0; i<stationsCount; i++) list.add(stationList[i].name);
    String output;
    serializeJson(doc, output);
    return output;
}

void setVolume(int v) {
    int vol = constrain(v, 0, 21);
    audio.setVolume(vol);
    preferences.putInt("volume", vol);
    webHandler.broadcastStatus(); 
    Serial.printf("Громкость: %d\n", vol);
}

void setStation(int index) {
    if (index >= 0 && index < stationsCount) {
        currentStationIdx = index;
        audio.connecttohost(stationList[currentStationIdx].url);
        preferences.putInt("station", currentStationIdx);
        webHandler.broadcastStatus();
        Serial.printf("Станция: %s\n", stationList[index].name);
    }
}

void setMute(bool mute) {
    isMuted = mute;
    digitalWrite(SD_MODE, isMuted ? LOW : HIGH);
    webHandler.broadcastStatus();
}

// --- ПРЕРЫВАНИЯ И ЗАДАЧИ ---

void IRAM_ATTR readEncoderISR() {
    encoder.readEncoder_ISR();
}

void handle_encoder() {
    long currentEncoderValue = encoder.readEncoder();
    int delta = currentEncoderValue - lastEncoderValue;

    if (delta != 0) {
        lastEncoderValue = currentEncoderValue;
        if (digitalRead(ENC_SW) == LOW) {
            int dir = (delta > 0) ? 1 : -1;
            int next = (currentStationIdx + dir) % stationsCount;
            if (next < 0) next = stationsCount - 1;
            setStation(next);
        } else {
            setVolume(audio.getVolume() + delta);
        }
    }

    if (encoder.isEncoderButtonClicked()) {
        unsigned long now = millis();
        if (now - lastClickTime < doubleClickDelay) setMute(!isMuted);
        lastClickTime = now;
    }
}

void audio_task(void *pvParameters) {
    while (true) {
        audio.loop();
        vTaskDelay(1); 
    }
}

void setup() {
    Serial.begin(115200);

    pinMode(ENC_CLK, INPUT_PULLUP);
    pinMode(ENC_DT, INPUT_PULLUP);
    pinMode(ENC_SW, INPUT_PULLUP);
    pinMode(SD_MODE, OUTPUT);
    digitalWrite(SD_MODE, HIGH);

    encoder.begin();
    encoder.setup(readEncoderISR);
    encoder.setAcceleration(0);

    preferences.begin("radio_pref", false);
    int savedVol = preferences.getInt("volume", 5);
    currentStationIdx = preferences.getInt("station", 0);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.println("\nWiFi Connected: " + WiFi.localIP().toString());

    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(savedVol);
    audio.connecttohost(stationList[currentStationIdx].url);

    // Инициализация веб-сервера через нашу библиотеку
    webHandler.begin(
        [](int v) { setVolume(v); },
        [](int s) { setStation(s); },
        []() { setMute(!isMuted); },
        []() { return getStatusPayload(); }
    );

    xTaskCreatePinnedToCore(audio_task, "AudioTask", 10000, NULL, 3, NULL, 1);
}

void loop() {
    handle_encoder();
    vTaskDelay(2);
}