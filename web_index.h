#ifndef WEB_INDEX_H
#define WEB_INDEX_H

#include <Arduino.h>

// Мы будем использовать const char* с макросом PROGMEM, чтобы интерфейс хранился в Flash-памяти и не занимал драгоценную оперативку (RAM).
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Radio Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <meta charset="utf-8">
  <style>
    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; text-align: center; background: #1a1a1a; color: #e0e0e0; margin: 0; padding: 20px; }
    .card { background: #2d2d2d; padding: 30px; border-radius: 20px; display: inline-block; box-shadow: 0 4px 15px rgba(0,0,0,0.5); width: 90%; max-width: 400px; }
    h2 { color: #00adb5; margin-bottom: 20px; }
    
    /* Стилизация ползунка */
    input[type=range] { width: 100%; height: 10px; border-radius: 5px; background: #393e46; outline: none; margin: 20px 0; -webkit-appearance: none; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 20px; height: 20px; background: #00adb5; border-radius: 50%; cursor: pointer; }

    button { width: 100%; padding: 12px; margin: 8px 0; border-radius: 8px; border: none; background: #393e46; color: white; font-size: 16px; cursor: pointer; transition: 0.3s; }
    button:hover { background: #4e545c; }
    button.active { background: #00adb5; color: #fff; font-weight: bold; box-shadow: 0 0 10px rgba(0, 173, 181, 0.5); }
    
    .mute-btn { background: #0f969c; font-weight: bold; }
    .mute-btn.active-mute { background: #e94560; }
    
    .vol-label { font-size: 28px; font-weight: bold; color: #00adb5; }
    .station-list { margin-top: 20px; border-top: 1px solid #393e46; padding-top: 10px; }
  </style>
</head>
<body>
  <div class="card">
    <h2>Radio S3</h2>
    <p>Громкость: <span id="vVal" class="vol-label">--</span></p>
    <input type="range" id="volSlider" min="0" max="21" oninput="sendVol(this.value)">
    <button id="muteBtn" class="mute-btn" onclick="sendMute()">MUTE: OFF</button>
    <div id="stations" class="station-list"></div>
  </div>

<script>
  let gateway = `ws://${window.location.hostname}/ws`;
  let websocket;

  function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onmessage = (event) => {
      const data = JSON.parse(event.data);
      
      // Громкость
      document.getElementById('volSlider').value = data.vol;
      document.getElementById('vVal').innerText = data.vol;
      
      // Mute
      const mBtn = document.getElementById('muteBtn');
      mBtn.innerText = data.mute ? "MUTE: ON" : "MUTE: OFF";
      data.mute ? mBtn.classList.add('active-mute') : mBtn.classList.remove('active-mute');

      // Список станций
      const container = document.getElementById('stations');
      container.innerHTML = ''; 
      data.list.forEach((name, index) => {
        let btn = document.createElement('button');
        btn.innerText = name;
        if(index === data.cur) btn.className = 'active';
        btn.onclick = () => websocket.send(JSON.stringify({action: "setStation", value: index}));
        container.appendChild(btn);
      });
    };
    websocket.onclose = () => setTimeout(initWebSocket, 2000);
  }

  function sendVol(v) { 
    websocket.send(JSON.stringify({action: "setVol", value: parseInt(v)})); 
  }
  function sendMute() { 
    websocket.send(JSON.stringify({action: "toggleMute"})); 
  }

  window.onload = initWebSocket;
</script>
</body></html>
)rawliteral";

#endif