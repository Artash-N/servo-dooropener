#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

Servo myservo;
const int servoPin = 18;

// ====== Wi-Fi AP settings ======
const char* ssid     = "SecretDoor-ESP32";   // Wi-Fi name
const char* password = "OpenSesame";        // At least 8 chars

WebServer server(80);

const int HOME_ANGLE    = 170;   // default servo position (door "closed")
const int TRIGGER_ANGLE = 5;    // angle when door is "open"

// Fancy HTML main page
const char MAIN_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Secret Door Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
    :root {
      --bg: #050816;
      --card-bg: rgba(15, 23, 42, 0.95);
      --accent: #38bdf8;
      --accent-soft: rgba(56, 189, 248, 0.2);
      --text-main: #e5e7eb;
      --text-muted: #9ca3af;
      --danger: #f97373;
    }
    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }
    body {
      min-height: 100vh;
      background: radial-gradient(circle at top, #1e293b 0, #020617 45%, #000 100%);
      display: flex;
      align-items: center;
      justify-content: center;
      color: var(--text-main);
      padding: 16px;
    }
    .glow-orb {
      position: fixed;
      width: 450px;
      height: 450px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(56, 189, 248, 0.35), transparent 60%);
      top: -120px;
      right: -80px;
      filter: blur(2px);
      z-index: -1;
    }
    .panel {
      max-width: 420px;
      width: 100%;
      background: var(--card-bg);
      border-radius: 24px;
      padding: 26px 22px 22px;
      box-shadow:
        0 0 0 1px rgba(148, 163, 184, 0.3),
        0 40px 80px rgba(15, 23, 42, 0.9);
      position: relative;
      overflow: hidden;
    }
    .panel::before {
      content: "";
      position: absolute;
      inset: 0;
      background: linear-gradient(135deg, rgba(56,189,248,0.16), transparent 50%);
      opacity: 0.75;
      pointer-events: none;
    }
    .panel-inner {
      position: relative;
      z-index: 1;
    }
    .chip {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 4px 10px;
      border-radius: 999px;
      font-size: 11px;
      text-transform: uppercase;
      letter-spacing: 0.08em;
      border: 1px solid rgba(148, 163, 184, 0.4);
      background: rgba(15, 23, 42, 0.8);
      margin-bottom: 16px;
      color: var(--text-muted);
    }
    .chip-dot {
      width: 7px;
      height: 7px;
      border-radius: 999px;
      background: #22c55e;
      box-shadow: 0 0 10px rgba(34, 197, 94, 0.8);
    }
    h1 {
      font-size: 24px;
      margin-bottom: 6px;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    h1 span.icon {
      font-size: 26px;
    }
    .subtitle {
      font-size: 13px;
      color: var(--text-muted);
      margin-bottom: 18px;
    }
    .info-row {
      display: flex;
      justify-content: space-between;
      font-size: 12px;
      color: var(--text-muted);
      margin-bottom: 12px;
    }
    .info-row span.label {
      text-transform: uppercase;
      letter-spacing: 0.08em;
      font-size: 11px;
    }
    .info-row span.value {
      font-weight: 500;
      color: var(--text-main);
    }
    .status-pill {
      margin: 14px 0 20px;
      padding: 10px 12px;
      border-radius: 12px;
      background: linear-gradient(90deg, rgba(56,189,248,0.18), rgba(15,23,42,0.9));
      border: 1px solid rgba(56,189,248,0.5);
      font-size: 12px;
      display: flex;
      align-items: center;
      gap: 8px;
    }
    .status-dot {
      width: 9px;
      height: 9px;
      border-radius: 999px;
      border: 2px solid rgba(15, 23, 42, 1);
      background: #22c55e;
      box-shadow: 0 0 18px rgba(34, 197, 94, 0.9);
    }
    .status-text-main {
      font-weight: 500;
    }
    .status-text-sub {
      opacity: 0.8;
    }

    .button-wrap {
      margin-top: 12px;
      margin-bottom: 8px;
    }
    button.open-btn {
      all: unset;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      gap: 10px;
      padding: 12px 26px;
      border-radius: 999px;
      background: radial-gradient(circle at top left, #4ade80, #22c55e);
      color: #022c22;
      font-weight: 600;
      font-size: 16px;
      box-shadow:
        0 0 0 1px rgba(21, 128, 61, 0.6),
        0 24px 40px rgba(22, 163, 74, 0.7);
      position: relative;
      overflow: hidden;
      min-width: 210px;
    }
    button.open-btn::before {
      content: "";
      position: absolute;
      inset: 0;
      background: radial-gradient(circle at 0 0, rgba(255,255,255,0.45), transparent 55%);
      opacity: 0.9;
      pointer-events: none;
    }
    button.open-btn:active {
      transform: translateY(1px) scale(0.99);
      box-shadow:
        0 0 0 1px rgba(21, 128, 61, 0.7),
        0 16px 30px rgba(22, 163, 74, 0.7);
    }
    .btn-icon {
      background: rgba(16, 185, 129, 0.8);
      width: 26px;
      height: 26px;
      border-radius: 999px;
      display: inline-flex;
      align-items: center;
      justify-content: center;
      font-size: 15px;
      box-shadow: 0 0 14px rgba(45, 212, 191, 0.8);
    }
    .footer {
      margin-top: 18px;
      font-size: 11px;
      color: var(--text-muted);
      opacity: 0.8;
    }
    .footer span {
      opacity: 0.9;
    }
    .highlight {
      color: var(--accent);
      font-weight: 500;
    }
    @media (max-width: 480px) {
      h1 { font-size: 22px; }
      button.open-btn { width: 100%; }
    }
  </style>
</head>
<body>
  <div class="glow-orb"></div>
  <div class="panel">
    <div class="panel-inner">
      <div class="chip">
        <div class="chip-dot"></div>
        <span>Inside Access • Online</span>
      </div>
      <h1>
        <span class="icon">🚪</span>
        Secret Door Console
      </h1>
      <p class="subtitle">
        Tap once to let yourself out. The servo will pull to open, hold for 4&nbsp;seconds, then return to locked position.
      </p>

      <div class="info-row">
        <div>
          <span class="label">Mode</span><br />
          <span class="value">Inside-Only Release</span>
        </div>
        <div style="text-align:right;">
          <span class="label">Servo Profile</span><br />
          <span class="value">170° ⟷ 30°</span>
        </div>
      </div>

      <div class="status-pill">
        <div class="status-dot"></div>
        <div>
          <div class="status-text-main">Door is currently <strong>closed</strong> (servo at 170°)</div>
          <div class="status-text-sub">Press “Open Door” to pulse the mechanism.</div>
        </div>
      </div>

      <form action="/move" method="GET" class="button-wrap">
        <button type="submit" class="open-btn">
          <span class="btn-icon">⇄</span>
          <span>Open Door</span>
        </button>
      </form>

      <div class="footer">
        <span>Project: <span class="highlight">Personal Auto-Door</span> · Powered by ESP32</span>
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";

// Fancy "movement complete" page
const char DONE_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Door Pulse Complete</title>
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <style>
    body {
      min-height: 100vh;
      margin: 0;
      padding: 0;
      display: flex;
      align-items: center;
      justify-content: center;
      font-family: system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
      background: radial-gradient(circle at top, #1e293b 0, #020617 45%, #000 100%);
      color: #e5e7eb;
    }
    .card {
      background: rgba(15, 23, 42, 0.96);
      border-radius: 24px;
      padding: 26px 22px 18px;
      width: 90%;
      max-width: 380px;
      box-shadow:
        0 0 0 1px rgba(148, 163, 184, 0.3),
        0 32px 70px rgba(15, 23, 42, 0.95);
      text-align: center;
      position: relative;
      overflow: hidden;
    }
    .card::before {
      content: "";
      position: absolute;
      inset: -40%;
      background: conic-gradient(
        from 0deg,
        rgba(56,189,248,0.0),
        rgba(56,189,248,0.65),
        rgba(56,189,248,0.0)
      );
      opacity: 0.35;
      filter: blur(18px);
      z-index: -1;
    }
    .icon {
      font-size: 38px;
      margin-bottom: 8px;
    }
    h2 {
      margin: 4px 0 6px;
      font-size: 22px;
    }
    p {
      font-size: 13px;
      color: #9ca3af;
      margin-bottom: 18px;
    }
    .pill {
      display: inline-flex;
      align-items: center;
      gap: 8px;
      padding: 6px 11px;
      border-radius: 999px;
      border: 1px solid rgba(56,189,248,0.6);
      background: rgba(15,23,42,0.9);
      font-size: 11px;
      margin-bottom: 18px;
    }
    .pill-dot {
      width: 8px;
      height: 8px;
      border-radius: 999px;
      background: #22c55e;
      box-shadow: 0 0 16px rgba(34, 197, 94, 0.9);
    }
    a.btn {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      padding: 9px 22px;
      border-radius: 999px;
      border: 1px solid rgba(148, 163, 184, 0.7);
      color: #e5e7eb;
      font-size: 14px;
      text-decoration: none;
      gap: 6px;
    }
    a.btn span.arrow {
      font-size: 15px;
    }
    .hint {
      margin-top: 12px;
      font-size: 11px;
      color: #6b7280;
    }
  </style>
</head>
<body>
  <div class="card">
    <div class="icon">✅</div>
    <h2>Door Pulse Complete</h2>
    <p>
      The servo moved from <strong>170° → 30° → 170°</strong>.<br/>
      Your door should now be back in the normal closed position.
    </p>
    <div class="pill">
      <div class="pill-dot"></div>
      <span>Inside access cycle finished safely.</span>
    </div>
    <a href="/" class="btn">
      <span class="arrow">↩</span>
      <span>Back to Control Panel</span>
    </a>
    <div class="hint">
      You can close this tab once you’re out.
    </div>
  </div>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", MAIN_page);
}

void handleMove() {
  Serial.println("Button clicked: 170° → 30° (4s) → 170°");

  // Move to "open" angle and hold 4 seconds
  myservo.write(TRIGGER_ANGLE);
  delay(2000);

  // Move back to "home" angle
  myservo.write(HOME_ANGLE);

  // Send fancy completion page
  server.send(200, "text/html", DONE_page);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  myservo.attach(servoPin, 500, 2400);

  // Start servo at 170° (door closed)
  myservo.write(HOME_ANGLE);
  delay(500);

  Serial.println();
  Serial.println("Starting Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Connect to WiFi: "); Serial.println(ssid);
  Serial.print("Password: "); Serial.println(password);
  Serial.print("Open browser at: http://"); Serial.println(IP);

  server.on("/", handleRoot);
  server.on("/move", handleMove);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
