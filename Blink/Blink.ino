#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h> 
#include <ESP8266mDNS.h> // Brought this back for drift.local support!

const char* ssid = "Drift_Proto_Car";
const char* password = "password123";

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); 

const int motorPin = D2;

// Light, fast HTML page with JavaScript to send WebSocket signals instantly
String getHTML() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<style>html { font-family: Arial; text-align: center; background: #222; color: white;}";
  html += ".button { background-color: #4CAF50; border: none; color: white; padding: 30px; font-size: 30px;";
  html += "margin: 40px auto; cursor: pointer; border-radius: 15px; display: block; width: 80%; user-select: none;}";
  html += ".button:active { background-color: #3e8e41; }</style></head>";
  
  html += "<body><h1>Real-Time Drift Control</h1>";
  html += "<button class=\"button\" id=\"actionBtn\" onmousedown=\"sendData(1)\" onmouseup=\"sendData(0)\" ontouchstart=\"sendData(1)\" ontouchend=\"sendData(0)\">HOLD TO SPIN</button>";
  
  html += "<script>";
  // This line dynamically finds the correct IP/Domain regardless of how you connected
  html += "var connection = new WebSocket('ws://' + location.hostname + ':81/');";
  html += "connection.onopen = function () { console.log('Connected!'); };";
  html += "function sendData(state) { connection.send(state); }";
  html += "</script></body></html>";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    if (payload[0] == '1') {
      digitalWrite(motorPin, HIGH); // Instant ON
    } else if (payload[0] == '0') {
      digitalWrite(motorPin, LOW);  // Instant OFF
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // START MDNS RESPONDER SO DRIFT.LOCAL WORKS AGAIN!
  if (MDNS.begin("drift")) {
    Serial.println("mDNS responder started!");
  }

  server.on("/", handleRoot);
  server.begin();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  Serial.println("Universal Real-time engine ready!");
}

void loop() {
  MDNS.update(); // Keeps the drift.local identity alive
  server.handleClient();
  webSocket.loop(); 
}
