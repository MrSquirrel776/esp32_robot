#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "ESP32 LED Remote";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//LED
static const int LED_pin = 4;
bool LED_status = LOW;

void onEvent();
void handle_OnConnect();
void handle_F();
void handle_B();
void handle_NotFound();
String SendHTML(bool LED_status_html);

void createHtml() {
  const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html>
  <head>
    <title>ESP Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
    html {
      font-family: Arial, Helvetica, sans-serif;
      text-align: center;
    }
    h1 {
      font-size: 1.8rem;
      color: white;
    }
    h2{
      font-size: 1.5rem;
      font-weight: bold;
      color: #143642;
    }
    .topnav {
      overflow: hidden;
      background-color: #143642;
    }
    body {
      margin: 0;
    }
    .content {
      padding: 30px;
      max-width: 600px;
      margin: 0 auto;
    }
    .card {
      background-color: #F8F7F9;;
      box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
      padding-top:10px;
      padding-bottom:20px;
    }
    .button {
      padding: 15px 50px;
      font-size: 24px;
      text-align: center;
      outline: none;
      color: #fff;
      background-color: #0f8b8d;
      border: none;
      border-radius: 5px;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      -khtml-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
      -webkit-tap-highlight-color: rgba(0,0,0,0);
    }
    /*.button:hover {background-color: #0f8b8d}*/
    .button:active {
      background-color: #0f8b8d;
      box-shadow: 2 2px #CDCDCD;
      transform: translateY(2px);
    }
    .state {
      font-size: 1.5rem;
      color:#8c8c8c;
      font-weight: bold;
    }
    </style>
  <title>ESP Web Server</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  </head>
  <body>
    <div class="topnav">
      <h1>ESP WebSocket Server</h1>
    </div>
    <div class="content">
      <div class="card">
        <h2>Output - GPIO 2</h2>
        <p class="state">state: <span id="state">%STATE%</span></p>
        <p><button id="button" class="button">Toggle</button></p>
      </div>
    </div>
  <script>
    var gateway = `ws://${window.location.hostname}/ws`;
    var websocket;
    window.addEventListener('load', onLoad);
    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage; // <-- add this line
    }
    function onOpen(event) {
      console.log('Connection opened');
    }
    function onClose(event) {
      console.log('Connection closed');
      setTimeout(initWebSocket, 2000);
    }
    function onMessage(event) {
      var state;
      if (event.data == "1"){
        state = "ON";
      }
      else{
        state = "OFF";
      }
      document.getElementById('state').innerHTML = state;
    }
    function onLoad(event) {
      initWebSocket();
      initButton();
    }
    function initButton() {
      document.getElementById('button').addEventListener('click', toggle);
    }
    function toggle(){
      websocket.send('toggle');
    }
  </script>
  </body>
  </html>
  )rawliteral";
}

void notifyClients() {
  ws.textAll(String(LED_status));
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    if (strcmp((char*)data, "toggle") == 0) {
      LED_status = !LED_status;
      notifyClients();
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Websocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (LED_status){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  createHtml();

  Serial.begin(115200);
  pinMode(LED_pin, OUTPUT);
  digitalWrite(LED_pin, LOW);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.println("Connecting to WiFi..");
  }

  delay(100);
  
  server.on("/", handle_OnConnect);
  server.on("/F", handle_F);
  server.on("/B", handle_B);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // put your main code here, to run repeatedly:
  server.handleClient(); 

  //write LED's
  if(LED_status)
  {digitalWrite(LED_pin, HIGH);}
  else
  {digitalWrite(LED_pin, LOW);}
}

void handle_OnConnect() {
  LED_status = LOW;
  Serial.println("LED on");
  server.send(200, "text/html", SendHTML(false));
}

void handle_F() {
  LED_status = HIGH;
  Serial.println("LED on");
  server.send(200, "text/html", SendHTML(true));
}

void handle_B() {
  LED_status = LOW;
  Serial.println("LED off");
  server.send(200, "text/html", SendHTML(false));
}

void handle_NotFound(){
  server.send(404, "text/plain", "404 Not found");
}

String SendHTML(bool LED_status_html){
  String html ="<!doctype html> <html> <head> <meta charset=\"utf-8\"> <title>Controller</title> <style> body { -webkit-user-select: none; -ms-user-select: none; user-select: none; height: 100%; overflow-y: hidden; } .buttons { background: #FFC55A; width: 100px; height: 100px; border-radius: 15px; margin: 10px; text-align: center; } .button{ font-size: 70px; text-decoration: none; color: black; } .F { margin-left: 120px; } .L { display: inline-block; } .R { display: inline-block; margin-left: 110px; } .B { margin-left: 120px; } img.tilt-phone { position: fixed; display: block; width: 100%; height: 100%; object-fit: cover; top: 0; left: 0; right: 0; bottom: 0; z-index: 2; } @media (orientation: landscape) { img.tilt-phone { display: none; } } </style> </head> <body> <img src=\"rotate_phone.png\" class=\"tilt-phone\" alt=\"Please Turn \"></img> <div id=\"F\" class = \"buttons F\" onTouchStart=\"mDown(this)\" onTouchEnd=\"mUp(this)\"></div> <div id=\"L\" class = \"buttons L\" onTouchStart=\"mDown(this)\" onTouchEnd=\"mUp(this)\"></div> <div id=\"R\" class = \"buttons R\" onTouchStart=\"mDown(this)\" onTouchEnd=\"mUp(this)\"></div> <div id=\"B\" class = \"buttons B\" onTouchStart=\"mDown(this)\" onTouchEnd=\"mUp(this)\"></div> <script> var isHeld = false; function mDown(obj) { obj.style.backgroundColor = \"#123456\"; var btn = obj.id; if (!isHeld){ console.log(btn + \" opened\"); window.open(btn, \"_self\"); } isHeld = true; } function mUp(obj) { obj.style.backgroundColor = \"#FFC55A\"; isHeld = false; window.open(\"\", \"_self\"); } </script> </body> </html>";
  return html;
}