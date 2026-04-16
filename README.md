
# 📻 ESP32-S3 WebSocket Internet Radio

Современное интернет-радио на базе **ESP32-S3 (Waveshare S3 Zero)** с использованием протокола **WebSockets** для мгновенного управления через веб-интерфейс и аппаратным энкодером.

## ✨ Умный энкодер:
* Вращение: Регулировка громкости.
* Нажатие + вращение: Переключение станций.
* Двойной клик: Режим Mute.

---

## 🛠 Технологический стек
* **Hardware:** ESP32-S3 (Waveshare S3 Zero), I2S DAC (например, PCM5102 или встроенный в Waveshare модуль), Роторный энкодер.
* **Libraries:**
    * `ESPAsyncWebServer` — асинхронный сервер.
    * `ArduinoJson 7` — работа с JSON данными.
    * `ESP32-AudioI2S` — декодирование аудиопотока.
    * `AiEsp32RotaryEncoder` — работа с энкодером.

---

## 📂 Структура проекта
* `main.ino` — Основной файл: инициализация периферии, прерывания энкодера и логика управления аудио.
* `RadioWebHandler.h` — Класс управления веб-сервером и WebSocket-соединениями.
* `web_index.h` — Интерфейс управления (HTML/CSS/JS), упакованный в Flash-память.

---

## 🔌 Схема подключения (Waveshare S3 Zero)

| Компонент | Пин ESP32-S3 | Описание |
| :--- | :--- | :--- |
| **I2S BCLK** | 11 | Bit Clock |
| **I2S LRC** | 10 | Left/Right Clock |
| **I2S DOUT** | 12 | Data Out |
| **SD_MODE** | 9 | Mute Control (Amplifier) |
| **ENC_CLK** | 6 | Rotary Encoder CLK |
| **ENC_DT** | 5 | Rotary Encoder DT |
| **ENC_SW** | 7 | Rotary Encoder Switch |

---

## 🚀 Быстрый старт

1.  Установите библиотеки через **PlatformIO** (рекомендуется) или Arduino IDE:
    ```ini
    lib_deps =
        https://github.com/me-no-dev/ESPAsyncWebServer
        bblanchon/ArduinoJson @ ^7.0.0
        esphome/ESP32-AudioI2S
        igorantolic/Ai Esp32 Rotary Encoder
    ```
2.  В файле `RadioRadio.ino` укажите данные вашей Wi-Fi сети:
    ```cpp
    const char* ssid = "YOUR_SSID";
    const char* password = "YOUR_PASSWORD";
    ```
3.  Прошейте вашу ESP32-S3.
4.  Найдите IP-адрес устройства в Serial-мониторе и откройте его в браузере.

---

## 📸 Интерфейс
Веб-панель выполнена в темных тонах (Dark Mode) и полностью адаптивна для мобильных устройств. Она позволяет:
* Видеть текущую играющую станцию.
* Плавно регулировать громкость ползунком.
* Мгновенно переключать стримы из списка.
* Видеть изменения, сделанные другими пользователями или физическим энкодером, в режиме реального времени.
