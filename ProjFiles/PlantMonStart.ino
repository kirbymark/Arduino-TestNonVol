#include <SPI.h>
#include <WiFiNINA.h>

char ap_ssid[] = "PM_SetupMode";
const IPAddress APIP(192, 168, 0, 34);

int status = WL_IDLE_STATUS;
WiFiServer server(80);
String HTTP_req;
boolean readingNetwork = false;
boolean readingPassword = false;
boolean readingdevicename = false;
String password = "";
String network = "";
String devicename = "";
boolean needCredentials = true;
boolean needWiFi = false;


void setup() {
  Serial.begin(9600);
  Serial.println("Access Point Web Server");
  
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
  
  Serial.print("Will create an access point named: ");
  Serial.println(ap_ssid);

  WiFi.config(APIP, APIP, APIP, IPAddress(255,255,255,0));
  if (WiFi.beginAP(ap_ssid) != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }
  else {
    Serial.println("AP Access is completed and Ready!");
  }

  delay(1000);
  server.begin();
  printAPStatus();  
  delay(1000);
}

void loop() {
  if (needCredentials) {
    getCredentials();
  }
  if (needWiFi) {
    getWiFi();
  }
}


void getCredentials() {
  WiFiClient client = server.available();
  if (client) {                            
    Serial.println("new client");           
    String currentLine = "";               
    while (client.connected()) {            
      if (client.available()) {            
        char c = client.read();           
        Serial.print(c);
        if (c=='?') readingNetwork = true;
        if (readingNetwork) {
          if (c=='!') {
            readingPassword = true;
            readingNetwork = false;
          }
          else if (c!='?') {
            network += c;
          }
        }
        if (readingPassword) {
          if (c==',') {
            readingdevicename = true;
            readingPassword = false; 
          }
          else if (c!='!') {
            password += c;
          }
        }
        if (readingdevicename) {
          if (c=='*') {
            Serial.println();
            Serial.print("Network Name: ");
            Serial.println(network);
            Serial.print("Password: ");
            Serial.println(password);
            Serial.print("Device Name: ");
            Serial.println(devicename);
            Serial.println();
            client.stop();
            WiFi.end();
            readingdevicename = false;
            needCredentials = false;
            needWiFi = true;  
          }
          else if (c!=',') {
            devicename += c;
          }
        }
        
        
        if (c == '\n') {   
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.println("<html>");
            client.println("<head>");
            client.println("<style type=\"text/css\"> body {font-family: sans-serif; font-size: 30px; margin:50px; padding:20px; line-height: 200% } </style>");
            client.println("<title>Plant Monitor Setup</title>");
            client.println("</head>");
            client.println("<body>");
            //client.println("<h1>Plant Monitor Setup</h1>");  
            //client.println("<h2>Wifi SSID WIFI CREDENTIALS</h2>");
            client.print("Wifi SSID / Network name: ");
            client.print("<input style=\"font-size:30px;\" id=\"network\"/><br>");
            client.print("Wifi Password: ");
            client.print("<input style=\"font-size:30px;\" id=\"password\"/><br>");
            //client.println("<h2>DEVICE DETAILS</h2>");
            client.print("Device Name: ");
            client.print("<input style=\"font-size:30px;\" id=\"devicename\"/><br>");
            client.print("<br>");
            
            client.print("<button style=\"font-size:48px;\" type=\"button\" onclick=\"SendText()\">Submit</button>");
            client.println("</body>");
            client.println("<script>");
            client.println("var network = document.querySelector('#network');");
            client.println("var password = document.querySelector('#password');");
            client.println("var devicename = document.querySelector('#devicename');");
            
            client.println("function SendText() {");
            client.println("nocache=\"&nocache=\" + Math.random() * 1000000;");
            client.println("var request =new XMLHttpRequest();");
            client.println("netText = \"&txt=\" + \"?\" + network.value + \"!\" + password.value + \",\" + devicename.value + \"*\" + \",&end=end\";");
          
            client.println("request.open(\"GET\", \"ajax_inputs\" + netText + nocache, true);");
            client.println("request.send(null)");
            client.println("network.value=''");
            client.println("password.value=''");
            client.println("devicename.value=''");
            client.println("}");
            client.println("</script>");
            client.println("</html>");
            client.println();

        

            break;
          }
          else { 
            currentLine = "";
          }
        }
        else if (c != '\r') { 
          currentLine += c;
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
    Serial.println();
  }
}

void getWiFi () {
  if (network == "" or password == "") {
        Serial.println("Invalid WiFi credentials");
        while (true);
      }
  if (devicename == "" ) {
      devicename = "NotProvided";
  }
  while (WiFi.status() != WL_CONNECTED) {
  
    int len = network.length() + 1;
    char ssid[len];
    network.toCharArray(ssid, len);
    
    int len2 = password.length() + 1;
    char pass[len2];
    password.toCharArray(pass, len2);
  
    Serial.print("Attempting to connect to SSID: ");  
    Serial.print(ssid);
    Serial.print(" with Password: ");  
    Serial.println(pass);

    WiFi.begin(ssid, pass);
    delay(10000);
    }

  Serial.println("WiFi connection successful");
  printWiFiStatus();
  needWiFi = false;
  
  delay(1000);
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");

  // print your MAC address:
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC address: ");
  printMacAddress(mac);


  // print the MAC address of the router you're attached to:
  byte bssid[6];
  WiFi.BSSID(bssid);
  Serial.print("BSSID: ");
  printMacAddress(bssid);

}

void printAPStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("signal strength (RSSI):");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("To connect, open a browser to http://");
  Serial.println(ip);
}

void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}