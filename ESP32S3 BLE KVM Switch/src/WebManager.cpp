#include "WebManager.h"
#include "Config.h"
#include "Controller.h"
#include "WiFiManager.h"
#include <WebServer.h>

namespace {

WebServer server(80);

const char PAGE_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html lang="zh">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>蓝牙唤醒器</title>
<style>
body{font-family:system-ui,-apple-system,sans-serif;max-width:420px;margin:0 auto;padding:16px;background:#111;color:#eee}
h1{font-size:1.3rem;text-align:center;margin-bottom:20px}
.btn{display:block;width:100%;padding:18px;margin:10px 0;font-size:1.1rem;font-weight:bold;border:0;border-radius:12px;cursor:pointer;color:#fff}
.win{background:#2563eb}
.mac{background:#16a34a}
.sleep{background:#dc2626}
.pair{background:transparent;border:1px solid #6b7280;color:#d1d5db}
.group{font-size:.8rem;color:#9ca3af;margin:16px 0 2px}
.btn:active{opacity:.7}
fieldset{border:1px solid #333;border-radius:12px;padding:14px;margin-top:24px}
legend{color:#9ca3af;font-size:.85rem;padding:0 6px}
input{width:100%;box-sizing:border-box;padding:11px;margin:7px 0;border:1px solid #333;border-radius:8px;background:#222;color:#eee;font-size:1rem}
input[type=submit]{background:#f59e0b;color:#111;border:0;font-weight:bold;cursor:pointer}
#status{font-size:.85rem;color:#9ca3af;margin-top:14px;text-align:center}
</style>
</head>
<body>
<h1>双设备蓝牙唤醒器</h1>

<div class="group">唤醒</div>
<button class="btn win" onclick="wake(0)">唤醒 Windows</button>
<button class="btn mac" onclick="wake(1)">唤醒 Mac</button>

<div class="group">睡眠</div>
<button class="btn sleep" onclick="sleep(0)">睡眠 Windows</button>
<button class="btn sleep" onclick="sleep(1)">睡眠 Mac</button>

<div class="group">配对</div>
<button class="btn pair" onclick="pair(0)">配对 Windows</button>
<button class="btn pair" onclick="pair(1)">配对 Mac</button>

<fieldset>
<legend>Wi-Fi 配网</legend>
<form method="post" action="/api/config">
<input name="ssid" placeholder="Wi-Fi 名称 (SSID)" required>
<input name="pass" type="password" placeholder="Wi-Fi 密码（留空=开放网络）">
<input type="submit" value="保存并连接">
</form>
</fieldset>

<div id="status">正在读取状态…</div>

<script>
function act(path,t){
  fetch(path+'?target='+t).then(r=>r.text()).then(s=>{document.getElementById('status').textContent=s});
}
function wake(t){ act('/api/wake',t); }
function sleep(t){ act('/api/sleep',t); }
function pair(t){ act('/api/pair',t); }
fetch('/api/status').then(r=>r.text()).then(s=>{document.getElementById('status').textContent=s});
</script>
</body>
</html>
)rawliteral";

} // namespace

void WebManager::begin() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/wake", HTTP_GET, handleWake);
  server.on("/api/sleep", HTTP_GET, handleSleep);
  server.on("/api/pair", HTTP_GET, handlePair);
  server.on("/api/config", HTTP_POST, handleConfig);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.onNotFound(handleRoot);
  server.begin();
  Serial.println("[Web] HTTP server started on port 80");
}

void WebManager::loop() {
  server.handleClient();
}

void WebManager::handleRoot() {
  server.send(200, "text/html", PAGE_HTML);
}

void WebManager::handleWake() {
  int target = server.arg("target").toInt();
  if (target < 0 || target >= NUM_DEVICE_SLOTS) {
    server.send(400, "text/plain", "bad target");
    return;
  }
  Controller::requestWake(target == 0 ? Target::WINDOWS : Target::MAC);
  server.send(200, "text/plain", "ok");
}

void WebManager::handlePair() {
  int target = server.arg("target").toInt();
  if (target < 0 || target >= NUM_DEVICE_SLOTS) {
    server.send(400, "text/plain", "bad target");
    return;
  }
  Controller::requestPair(target == 0 ? Target::WINDOWS : Target::MAC);
  server.send(200, "text/plain", "ok");
}

void WebManager::handleSleep() {
  int target = server.arg("target").toInt();
  if (target < 0 || target >= NUM_DEVICE_SLOTS) {
    server.send(400, "text/plain", "bad target");
    return;
  }
  Controller::requestSleep(target == 0 ? Target::WINDOWS : Target::MAC);
  server.send(200, "text/plain", "ok");
}

void WebManager::handleConfig() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");
  if (ssid.length() == 0) {
    server.send(400, "text/plain", "ssid required");
    return;
  }
  // Respond first, then reconnect (the AP may disappear during reconnect).
  server.send(200, "text/plain", "已保存，正在连接…");
  WiFiManager::saveCredentials(ssid, pass);
}

void WebManager::handleStatus() {
  String s = "slot=" + String(Controller::currentSlot() + 1) + " | " +
             WiFiManager::statusString();
  server.send(200, "text/plain", s);
}
