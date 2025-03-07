#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <cstring> // For strcmp

// Access Point credentials
const char* apSSID = "ESP8266_Stoplight";
const char* apPassword = "12345678";

// GPIO pin assignments
const int RED_PIN = D1;
const int YELLOW_PIN = D2;
const int GREEN_PIN = D3;

// Web server on port 80
ESP8266WebServer server(80);

// Stoplight states
const char* lights[] = {"green", "yellow", "red"};
int currentIndex = 0;
bool automaticRunning = false;
// System starts off
bool systemOn = false;  

// Auto mode per-light intervals (in milliseconds)
// Default: 1000 ms (1 sec) for each light.
unsigned long autoIntervalRed    = 1000;
unsigned long autoIntervalYellow = 1000;
unsigned long autoIntervalGreen  = 1000;

// For non-blocking delay
unsigned long previousMillis = 0;

void updateLights() {
  digitalWrite(RED_PIN,    (strcmp(lights[currentIndex], "red")    == 0 && systemOn) ? HIGH : LOW);
  digitalWrite(YELLOW_PIN, (strcmp(lights[currentIndex], "yellow") == 0 && systemOn) ? HIGH : LOW);
  digitalWrite(GREEN_PIN,  (strcmp(lights[currentIndex], "green")  == 0 && systemOn) ? HIGH : LOW);
}

unsigned long getCurrentInterval() {
  if (strcmp(lights[currentIndex], "red") == 0)
    return autoIntervalRed;
  else if (strcmp(lights[currentIndex], "yellow") == 0)
    return autoIntervalYellow;
  else
    return autoIntervalGreen;
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Stoplight Control</title>
  <style>
      body {
          text-align: center;
          font-family: Arial, sans-serif;
          margin: 20px;
          background-color: #f0f0f0;
      }
      .status-bar {
          padding: 10px;
          color: white;
          margin-bottom: 10px;
          border-radius: 5px;
          transition: background-color 0.3s ease, opacity 0.3s ease;
      }
      .off    { background: gray; }
      .manual { background: blue; }
      .auto   { background: green; }
      button {
          padding: 10px 15px;
          margin: 5px;
          border: none;
          border-radius: 5px;
          font-size: 16px;
          cursor: pointer;
          transition: background-color 0.3s ease, opacity 0.3s ease;
      }
      .disabled {
          opacity: 0.5;
          pointer-events: none;
      }
      .section {
          margin-top: 20px;
          padding: 10px;
          border: 1px solid #ccc;
          border-radius: 5px;
          background: white;
      }
      .selected-red    { background: red;    color: white; }
      .selected-yellow { background: yellow; color: black; }
      .selected-green  { background: green;  color: white; }
      /* Notification styles */
      #notification {
          position: fixed;
          top: 10px;
          right: 10px;
          background: rgba(0,0,0,0.7);
          color: #fff;
          padding: 10px;
          border-radius: 5px;
          display: none;
          opacity: 0;
          transition: opacity 0.3s ease;
          z-index: 1000;
      }
      /* Interval controls */
      .interval-row {
          display: block;
          margin: 10px auto;
          width: 250px;
          text-align: left;
      }
      .interval-label {
          display: inline-block;
          width: 70px;
          font-weight: bold;
      }
      .interval-buttons {
          display: inline-flex;
          align-items: center;
      }
      button.arrow {
          width: 35px;
          height: 35px;
          padding: 0;
          font-size: 18px;
          margin: 0 5px;
          border: 1px solid #ccc;
          background: #ddd;
          border-radius: 5px;
      }
      input.interval {
          width: 35px;
          height: 35px;
          text-align: center;
          font-size: 16px;
          border: 1px solid #ccc;
          border-radius: 5px;
          background: #fff;
      }
      .unit {
          margin-left: 8px;
          font-size: 14px;
      }
  </style>
  <script>
      var lastManualUpdateTime = 0;

      function increment(id) {
          var field = document.getElementById(id);
          var current = parseFloat(field.value);
          if (isNaN(current)) current = 1;
          current += 1;
          field.value = current.toFixed(0);
          lastManualUpdateTime = Date.now();
      }
      function decrement(id) {
          var field = document.getElementById(id);
          var current = parseFloat(field.value);
          if (isNaN(current)) current = 1;
          if (current > 1) {
              current -= 1;
              field.value = current.toFixed(0);
              lastManualUpdateTime = Date.now();
          }
      }
      function showNotification(message) {
          var notif = document.getElementById("notification");
          notif.innerText = message;
          notif.style.display = "block";
          notif.style.opacity = 1;
          setTimeout(function() {
              notif.style.opacity = 0;
              setTimeout(function() { notif.style.display = "none"; }, 300);
          }, 3000);
      }
      function sendRequest(path) {
          fetch(path)
              .then(response => response.text())
              .then(text => {
                  showNotification(text);
                  updateStatus();
              });
      }
      function setCycleIntervals() {
          var redVal    = document.getElementById("interval-red").value;
          var yellowVal = document.getElementById("interval-yellow").value;
          var greenVal  = document.getElementById("interval-green").value;
          fetch('/set_cycle_intervals?red=' + redVal + '&yellow=' + yellowVal + '&green=' + greenVal)
              .then(response => response.text())
              .then(text => {
                  showNotification(text);
                  // Delay updating fields so manual values aren't overwritten
                  setTimeout(updateStatus, 1000);
              });
      }
      function updateStatus() {
          fetch("/get_status")
              .then(res => res.json())
              .then(data => {
                  const isOn = (data.power === "On");
                  document.getElementById("status-bar").className = "status-bar " +
                      (isOn ? (data.state === "Auto Mode" ? "auto" : "manual") : "off");
                  document.getElementById("status-bar").innerText = isOn ? data.state : "System Off";

                  // Only update interval fields if no manual change in the last 3 seconds
                  if (Date.now() - lastManualUpdateTime > 3000) {
                      document.getElementById("interval-red").value    = (data.autoIntervalRed / 1000).toFixed(0);
                      document.getElementById("interval-yellow").value = (data.autoIntervalYellow / 1000).toFixed(0);
                      document.getElementById("interval-green").value  = (data.autoIntervalGreen / 1000).toFixed(0);
                  }

                  if (!isOn) {
                      document.getElementById("on-btn").disabled = false;
                      document.getElementById("on-btn").classList.remove("disabled");
                      document.getElementById("off-btn").disabled = true;
                      document.getElementById("off-btn").classList.add("disabled");
                      document.getElementById("btn-red").disabled = true;
                      document.getElementById("btn-red").classList.add("disabled");
                      document.getElementById("btn-yellow").disabled = true;
                      document.getElementById("btn-yellow").classList.add("disabled");
                      document.getElementById("btn-green").disabled = true;
                      document.getElementById("btn-green").classList.add("disabled");
                      document.getElementById("start-auto").disabled = true;
                      document.getElementById("start-auto").classList.add("disabled");
                      document.getElementById("stop-auto").disabled = true;
                      document.getElementById("stop-auto").classList.add("disabled");
                  } else {
                      document.getElementById("on-btn").disabled = true;
                      document.getElementById("on-btn").classList.add("disabled");
                      document.getElementById("off-btn").disabled = false;
                      document.getElementById("off-btn").classList.remove("disabled");
                      if (data.state === "Auto Mode") {
                          document.getElementById("btn-red").disabled = true;
                          document.getElementById("btn-red").classList.add("disabled");
                          document.getElementById("btn-yellow").disabled = true;
                          document.getElementById("btn-yellow").classList.add("disabled");
                          document.getElementById("btn-green").disabled = true;
                          document.getElementById("btn-green").classList.add("disabled");
                          document.getElementById("start-auto").disabled = true;
                          document.getElementById("start-auto").classList.add("disabled");
                          document.getElementById("stop-auto").disabled = false;
                          document.getElementById("stop-auto").classList.remove("disabled");
                      } else {
                          document.getElementById("btn-red").disabled = false;
                          document.getElementById("btn-red").classList.remove("disabled");
                          document.getElementById("btn-yellow").disabled = false;
                          document.getElementById("btn-yellow").classList.remove("disabled");
                          document.getElementById("btn-green").disabled = false;
                          document.getElementById("btn-green").classList.remove("disabled");
                          document.getElementById("start-auto").disabled = false;
                          document.getElementById("start-auto").classList.remove("disabled");
                          document.getElementById("stop-auto").disabled = true;
                          document.getElementById("stop-auto").classList.add("disabled");
                      }
                  }

                  // Highlight selected manual button
                  ["btn-red", "btn-yellow", "btn-green"].forEach(function(id) {
                      document.getElementById(id).classList.remove("selected-red", "selected-yellow", "selected-green");
                  });
                  if(isOn && data.state === "Manual Mode") {
                      if (data.light === "red") {
                          document.getElementById("btn-red").classList.add("selected-red");
                      } else if (data.light === "yellow") {
                          document.getElementById("btn-yellow").classList.add("selected-yellow");
                      } else if (data.light === "green") {
                          document.getElementById("btn-green").classList.add("selected-green");
                      }
                  }

                  // Show/hide Start/Stop Auto
                  if (isOn) {
                      if (data.state === "Manual Mode") {
                          document.getElementById("start-auto").style.display = "inline-block";
                          document.getElementById("stop-auto").style.display = "none";
                      } else {
                          document.getElementById("start-auto").style.display = "none";
                          document.getElementById("stop-auto").style.display = "inline-block";
                      }
                  } else {
                      document.getElementById("start-auto").style.display = "none";
                      document.getElementById("stop-auto").style.display = "none";
                  }

                  // Minimal Hardware Status
                  document.getElementById("hardware-status").innerHTML =
                      "<h2>Hardware Status</h2>" +
                      "<p>Stations Connected: " + data.stations + "</p>" +
                      "<p>Uptime: " + data.uptime + " seconds</p>";
              });
      }
      setInterval(updateStatus, 5000);
  </script>
</head>
<body onload="updateStatus()">
  <h1>Stoplight Control</h1>
  <div id="notification"></div>
  <p id="status-bar" class="status-bar off">System Off</p>
  <button id="on-btn"  onclick="sendRequest('/turn_on')">Turn On</button>
  <button id="off-btn" onclick="sendRequest('/turn_off')">Turn Off</button>

  <div class="section">
      <h2>Manual Control</h2>
      <p>Select a light to turn on manually.</p>
      <button id="btn-red"    onclick="sendRequest('/manual_cycle?light=red')">Red</button>
      <button id="btn-yellow" onclick="sendRequest('/manual_cycle?light=yellow')">Yellow</button>
      <button id="btn-green"  onclick="sendRequest('/manual_cycle?light=green')">Green</button>
  </div>

  <div class="section">
      <h2>Automatic Mode</h2>
      <p>Start or stop automatic light cycling.</p>
      <button id="start-auto" onclick="sendRequest('/start_auto')">Start Auto</button>
      <button id="stop-auto"  onclick="sendRequest('/stop_auto')" style="display:none;">Stop Auto</button>
      <br><br>
      <h3>Customize Cycle Intervals</h3>
      <p>Adjust each lights interval (in 1 second increments, minimum 1 second):</p>

      <div class="interval-row">
          <span class="interval-label">Red:</span>
          <div class="interval-buttons">
              <button class="arrow" onclick="decrement('interval-red')">-</button>
              <input type="text" id="interval-red" class="interval" value="1" readonly>
              <button class="arrow" onclick="increment('interval-red')">+</button>
              <span class="unit">seconds</span>
          </div>
      </div>

      <div class="interval-row">
          <span class="interval-label">Yellow:</span>
          <div class="interval-buttons">
              <button class="arrow" onclick="decrement('interval-yellow')">-</button>
              <input type="text" id="interval-yellow" class="interval" value="1" readonly>
              <button class="arrow" onclick="increment('interval-yellow')">+</button>
              <span class="unit">seconds</span>
          </div>
      </div>

      <div class="interval-row">
          <span class="interval-label">Green:</span>
          <div class="interval-buttons">
              <button class="arrow" onclick="decrement('interval-green')">-</button>
              <input type="text" id="interval-green" class="interval" value="1" readonly>
              <button class="arrow" onclick="increment('interval-green')">+</button>
              <span class="unit">seconds</span>
          </div>
      </div>

      <br>
      <button id="set-intervals-btn" onclick="setCycleIntervals()">Set Intervals</button>
  </div>

  <div class="section" id="hardware-status">
      <!-- Minimal hardware status info will be updated here -->
  </div>
</body>
</html>
)rawliteral");
}

void setup() {
  Serial.begin(115200);
  pinMode(RED_PIN,    OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  pinMode(GREEN_PIN,  OUTPUT);
  updateLights(); // System starts off

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);
  WiFi.setSleep(false);

  // Serve main page
  server.on("/", handleRoot);

  // Return system/hardware status
  server.on("/get_status", []() {
      String powerStatus = systemOn ? "On" : "Off";
      String stateStatus = automaticRunning ? "Auto Mode" : (systemOn ? "Manual Mode" : "Off");
      String jsonResponse = "{";
      jsonResponse += "\"power\": \"" + powerStatus + "\", ";
      jsonResponse += "\"state\": \"" + stateStatus + "\", ";
      jsonResponse += "\"light\": \"" + String(lights[currentIndex]) + "\", ";
      jsonResponse += "\"stations\": " + String(WiFi.softAPgetStationNum()) + ", ";
      jsonResponse += "\"uptime\": " + String(millis()/1000);
      jsonResponse += ", \"autoIntervalRed\": " + String(autoIntervalRed);
      jsonResponse += ", \"autoIntervalYellow\": " + String(autoIntervalYellow);
      jsonResponse += ", \"autoIntervalGreen\": " + String(autoIntervalGreen);
      jsonResponse += "}";
      server.send(200, "application/json", jsonResponse);
  });

  // Turn system on/off
  server.on("/turn_on", []() {
      systemOn = true;
      updateLights();
      server.send(200, "text/plain", "System On");
  });
  server.on("/turn_off", []() {
      systemOn = false;
      automaticRunning = false;
      updateLights();
      server.send(200, "text/plain", "System Off");
  });

  // Manual cycle
  server.on("/manual_cycle", []() {
      String light = server.arg("light");
      if (systemOn) {
          if (light == "red")    currentIndex = 2;
          else if (light == "yellow") currentIndex = 1;
          else if (light == "green")  currentIndex = 0;
          updateLights();
      }
      server.send(200, "text/plain", lights[currentIndex]);
  });

  // Auto mode start/stop
  server.on("/start_auto", []() {
      automaticRunning = true;
      server.send(200, "text/plain", "Auto Mode On");
  });
  server.on("/stop_auto", []() {
      automaticRunning = false;
      updateLights();
      server.send(200, "text/plain", "Auto Mode Off");
  });

  // Set cycle intervals in seconds
  server.on("/set_cycle_intervals", []() {
      float redSec    = atof(server.arg("red").c_str());
      float yellowSec = atof(server.arg("yellow").c_str());
      float greenSec  = atof(server.arg("green").c_str());

      // Enforce minimum of 1 second
      if (redSec    < 1.0) redSec    = 1.0;
      if (yellowSec < 1.0) yellowSec = 1.0;
      if (greenSec  < 1.0) greenSec  = 1.0;

      autoIntervalRed    = (unsigned long)(redSec    * 1000.0);
      autoIntervalYellow = (unsigned long)(yellowSec * 1000.0);
      autoIntervalGreen  = (unsigned long)(greenSec  * 1000.0);

      // Respond in seconds, not ms
      server.send(200, "text/plain",
          "Cycle intervals updated: Red=" + String((int)redSec) +
          " s, Yellow=" + String((int)yellowSec) +
          " s, Green=" + String((int)greenSec) + " s"
      );
  });

  server.begin();
}

void loop() {
  server.handleClient();
  if (automaticRunning && systemOn) {
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= getCurrentInterval()) {
          previousMillis = currentMillis;
          currentIndex = (currentIndex + 1) % 3;
          updateLights();
      }
  }
}
