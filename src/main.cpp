#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "ESP32 LED Remote";  // Enter SSID here
const char* password = "12345678";  //Enter Password here

IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80);

//LED
static const int LED_pin = 4;
bool LED_status = LOW;

void handle_OnConnect();
void handle_F();
void handle_B();
void handle_NotFound();
String SendHTML(bool LED_status_html);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_pin, OUTPUT);

  WiFi.softAP(ssid);
  WiFi.softAPConfig(local_ip, gateway, subnet);
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