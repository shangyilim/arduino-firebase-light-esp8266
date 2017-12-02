#include <ESP8266HTTPClient.h>

#include <Firebase.h>
#include <FirebaseArduino.h>



#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

IPAddress    apIP(42, 42, 42, 42);  // Defining a static IP address: local & gateway
                                    // Default IP in AP mode is 192.168.4.1

/* This are the WiFi access point settings. Update them to your likin */
const char *ssid = "ESP8266-1";
const char *password = "password";
int apModePin = D0;
int wifiModePin = D1;
int ledPin = D2;

int isAPMode = 1;
// Define a web server at port 80 for HTTP
ESP8266WebServer server(80);

bool ledState = false;

void handleRoot() {
  digitalWrite (LED_BUILTIN, 1); //turn the built in LED on pin DO of NodeMCU on
  char html[1000];
// Build an HTML page to display on the web-server root address
  snprintf ( html, 1000,

"<html>\
  <head>\
    <title>ESP8266 WiFi Network</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 1.5em; Color: #000000; }\
      h1 { Color: #AA0000; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 Wi-Fi Access Point and Web Server Demo</h1>\
    <form method='GET' action='/ssid_login'>\
    SSID: <input type='text' id='ssid' name='ssid' /> <br/>\
    Password: <input type='text' id='ssid_password' name='ssid_password' />\
    <input type='submit' value='Submit'>\
    </form>\
  </body>\
</html>"
  );
  server.send ( 200, "text/html", html );
  digitalWrite ( LED_BUILTIN, 1 );
}

void handleNotFound() {
  digitalWrite ( LED_BUILTIN, 0 );
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send ( 404, "text/plain", message );
  digitalWrite ( LED_BUILTIN, 1 ); //turn the built in LED on pin DO of NodeMCU off
}

void setup() {
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode ( apModePin, OUTPUT );
  pinMode ( wifiModePin, OUTPUT );
  
  pinMode ( ledPin, OUTPUT );
  

  if( isAPMode == 1){
    startAP();
  }
  
  delay(1000);
 
}

void loginSsid(){
  Serial.println("should start logging in ssid");
  String ssid = server.arg("ssid");
  String password = server.arg("ssid_password");
  String detail = "ssid:"+ssid + "pswd:"+password;
  Serial.println(detail);
  Serial.println("disconnecting AP, connecting to network");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  WiFi.begin(ssid.c_str(), password.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  isAPMode = 0;
  
  startFirebaseListening();
  
}

void startAP(){
   Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring access point...");

  //set-up the custom IP address
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));   // subnet FF FF FF 00  
  
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
 
  server.on ( "/", handleRoot );
  server.on ( "/ssid_login", loginSsid);
  server.on ( "/led=0", handleRoot);
  server.on ( "/inline", []() {
    server.send ( 200, "text/plain", "this works as well" );
  } );
  server.onNotFound ( handleNotFound );
  
  server.begin();
  Serial.println("HTTP server started");
}

void startFirebaseListening(){
   Firebase.begin("home-sensor-70625.firebaseio.com");
  Firebase.stream("/light");  
  
}
void statusLights(){
  if(isAPMode == 1){
    digitalWrite(apModePin, HIGH);
    digitalWrite(wifiModePin, LOW);
  }
  else{
    digitalWrite(wifiModePin, HIGH);
    digitalWrite(apModePin, LOW);
  }
}
void loop() {
  server.handleClient();
  statusLights();
  if(isAPMode == 0){
    if (Firebase.failed()) {
      Serial.println("streaming error");
      Serial.println(Firebase.error());
    }
    
    if (Firebase.available()) {
       FirebaseObject event = Firebase.readEvent();
        String eventType = event.getString("type");
     eventType.toLowerCase();
     
     Serial.print("event: ");
     Serial.println(eventType);
     if (eventType == "put") {
      String data = event.getString("data");
       Serial.print("data: ");
       Serial.println(data);

       if(data == "true"){
        digitalWrite(ledPin, HIGH);
       }
       else{
        digitalWrite(ledPin, LOW);
       }
       
     }
    }
  }
  
}
