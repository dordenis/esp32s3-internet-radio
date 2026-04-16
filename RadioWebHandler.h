#ifndef RADIO_WEB_HANDLER_H
#define RADIO_WEB_HANDLER_H

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "web_index.h" // Подключаем наш HTML файл

class RadioWebHandler {
private:
    AsyncWebServer server;
    AsyncWebSocket ws;

    std::function<void(int)> _onVolumeChange;
    std::function<void(int)> _onStationChange;
    std::function<void()> _onMuteToggle;
    std::function<String()> _getStatusJson;

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
            JsonDocument doc; // В V7 память управляется автоматически
            DeserializationError error = deserializeJson(doc, data, len);
            if (error) return;

            if (doc.containsKey("action")) {
                const char* action = doc["action"];
                if (strcmp(action, "setVol") == 0) _onVolumeChange(doc["value"]);
                else if (strcmp(action, "setStation") == 0) _onStationChange(doc["value"]);
                else if (strcmp(action, "toggleMute") == 0) _onMuteToggle();
            }
        }
    }

public:
    RadioWebHandler() : server(80), ws("/ws") {}

    void begin(
        std::function<void(int)> volCb, 
        std::function<void(int)> stCb, 
        std::function<void()> muteCb,
        std::function<String()> statusCb
    ) {
        _onVolumeChange = volCb;
        _onStationChange = stCb;
        _onMuteToggle = muteCb;
        _getStatusJson = statusCb;

        ws.onEvent([this](AsyncWebSocket *s, AsyncWebSocketClient *c, AwsEventType type, void *arg, uint8_t *data, size_t len) {
            if (type == WS_EVT_CONNECT) {
                c->text(_getStatusJson());
            } else if (type == WS_EVT_DATA) {
                handleWebSocketMessage(arg, data, len);
            }
        });

        server.addHandler(&ws);
        server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
            request->send_P(200, "text/html", index_html);
        });

        server.begin();
    }

    void broadcastStatus() {
        if (ws.count() > 0) ws.textAll(_getStatusJson());
    }
};

#endif