///////////////////////////////////////////////////////////////////////////

// CAMERA 1

/////////////////////////////////////////////////////////////////////////////

// #include <WiFiNINA.h>
#include <IRremote.h>
// #include <WiFiClient.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <string.h>

///////////////////// variablen board ///////////////////////////////////
const int RECV_PIN = 7;
const int led = 8;
// const int buzzer = 4;
IRrecv irrecv(RECV_PIN);
decode_results results;
bool ledStatus = LOW;
bool camera_broke = false;

unsigned long lastSignalTime = 0;
const unsigned long signalTimeout1 = 3000;  // Tijdsperiode in milliseconden (hier 3 seconden)
const unsigned long signalTimeout2 = 3000;  // Tijdsperiode in milliseconden (hier 2 seconden)


//variablen WIFI
char ssid[] = "WOT";             //  your network SSID (name) between the " "
char pass[] = "enterthegame";      // your network password between the " "
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;      //connection status
// WiFiServer server(80);            //server socket
// WiFiClient client = server.available();

// MQTT Broker
const char *mqtt_broker = "192.168.50.97";
const int mqtt_port = 1883;

// Prop name
const String propName = "prop3";
const char *mqttLedStateTopic = "prop3/ledState";
const char *mqttGetLedStateTopic = "prop1/getLedState";
const char *mqttToggleLedTopic = "prop1/toggleLed";
const char *mqttPuzzleCompleteTopic = "prop1/puzzleComplete";
const char *mqttPuzzleCompleteMessage = "completed";

const char *mqttRestartArduinoTopic = "prop3/restartArduino";
const char *mqttRestartArduinoMessage = "restarted";

const char *mqttDeleteCameraTopic = "prop3/deleteCamera";
const char *mqttDeleteCameraMessage = "deleted";


WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void setupWifi()
{
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the Wi-Fi network");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT()
{
  mqttClient.setServer(mqtt_broker, mqtt_port);
  // mqttClient.setCallback(mqttCallback);

  while (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(propName.c_str()))
    {
      Serial.println("Connected!");
    }
    else
    {
      Serial.print("Failed with state ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }

  mqttClient.subscribe(mqttDeleteCameraTopic);
}


void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn();
  irrecv.blink13(true);
  pinMode(led, OUTPUT);
  // pinMode(buzzer, OUTPUT);
  while (!Serial);
  // enable_WiFi();
  // connect_WiFi();

  // server.begin();
  // printWifiStatus();
  setupWifi();
  setupMQTT();

}

void loop() {
  // client = server.available();
  mqttClient.loop();

  recieveIR();

  // if (client) {
  //   printWEB();
  // }

  digitalWrite(led, ledStatus);
}

void recieveIR() {
  unsigned long currentTime = millis();
  bool secondSignalReceived = false;
  bool thirdSignalReceived = false;

  while (millis() - currentTime < 500) {
  if (irrecv.decode(&results) && !camera_broke) {
    // IR-signaal ontvangen
    Serial.println("IR-signaal ontvangen!");
    // Stop de IR-bibliotheek opnieuw
    irrecv.disableIRIn();
    digitalWrite(led, HIGH);
    ledStatus = HIGH;
    // digitalWrite(buzzer, HIGH);
    delay(500);
    ledStatus = LOW;
    // digitalWrite(buzzer, LOW);
    digitalWrite(led, LOW);
    delay(2000);
    irrecv.resume();  // Ontvang het volgende IR-signaal
    lastSignalTime = currentTime;
    // Schakel de IR-bibliotheek weer in voor toekomstige signalen
    irrecv.enableIRIn();

    // Wacht tot het tweede signaal binnen de tijdslimiet wordt ontvangen
    while ((currentTime - lastSignalTime <= signalTimeout1) && (currentTime >= 1000)) {
      if (irrecv.decode(&results)) {
        // Tweede signaal ontvangen binnen de tijdslimiet
        Serial.println("Tweede signaal ontvangen!");
        // Stop de IR-bibliotheek opnieuw
        irrecv.disableIRIn();
        secondSignalReceived = true;
        digitalWrite(led, HIGH);
        ledStatus = HIGH;
        // digitalWrite(buzzer, HIGH);
        delay(500);
        // digitalWrite(buzzer, LOW);
        ledStatus = LOW;
        digitalWrite(led, LOW);
        delay(2000);
        irrecv.resume();  // Ontvang het volgende IR-signaal
        lastSignalTime = currentTime;
        // Schakel de IR-bibliotheek weer in voor toekomstige signalen
        irrecv.enableIRIn();

        // Wacht tot het derde signaal binnen de tijdslimiet wordt ontvangen
        while ((currentTime - lastSignalTime <= signalTimeout1) && (currentTime >= 1000)) {
          if (irrecv.decode(&results)) {
            // Derde signaal ontvangen binnen de tijdslimiet
            Serial.println("Derde signaal ontvangen!");
            thirdSignalReceived = true;
            break;
          }
          currentTime = millis();
        }
        break;
      }
      currentTime = millis();
    }

    // Schakel de IR-bibliotheek weer in voor toekomstige signalen
    irrecv.enableIRIn();

    if (secondSignalReceived && thirdSignalReceived) {
      // Tweede en derde signaal correct ontvangen
      // digitalWrite(buzzer, HIGH);
      delay(500);
      // digitalWrite(buzzer, LOW);
      Serial.println("Camera uitgeschakeld!");
      digitalWrite(led, HIGH);
      camera_broke = true;
      ledStatus = HIGH; // Zet de LED aan

      // Publish MQTT message here
      mqttClient.publish(mqttDeleteCameraTopic, mqttDeleteCameraMessage);

      // while (true) {
      //   // Hier blijft het programma wachten en zal het niet verder gaan
      // }
      // sendCameraStatusToServer(camera_broke); // Stuur de status naar de andere server
    } else {
      Serial.println("Tweede of derde signaal niet ontvangen binnen de tijdslimiet!");
    }
  }
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void enable_WiFi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }
}

void connect_WiFi() {
  // Stel het statische IP-adres, gateway, subnetmasker en DNS in:
  IPAddress ip(192, 168, 50, 121);  // Statisch IP-adres
  IPAddress gateway(192, 168, 50, 1);  // Gateway
  IPAddress subnet(255, 255, 255, 0);  // Subnetmasker
  IPAddress dns(192, 168, 50, 1);  // DNS-server (meestal hetzelfde als de gateway)

  // Configureer het WiFi-netwerk met het statische IP-adres en andere instellingen:
  WiFi.config(ip, gateway, subnet, dns);
  
  // Probeer verbinding te maken met het WiFi-netwerk:
  status = WiFi.begin(ssid, pass);

  // Wacht 10 seconden voor de verbinding:
  delay(10000);
}


// void printWEB() {
//   unsigned long currentTime = millis();
//   bool secondSignalReceived = false;
//   bool thirdSignalReceived = false;
  

//   // if (client) {
//     // Serial.println("new client");
//     // String currentLine = "";
//     while (client.connected()) {
//       if (client.available()) {
//         char c = client.read();
//         Serial.write(c);
//         if (c == '\n') {
//           if (currentLine.length() == 0) {
//             client.println("HTTP/1.1 200 OK");
//             client.println("Content-type:text/html");
//             client.println();
//             client.println("<html><body>");
            


//             // Controleer of de LED aan staat (pin 10)
//             if (camera_broke) {
//               ledStatus = HIGH;
//               // LED brandt, toon "camera kapot"
              
//               client.println("<div style='width:100px;height:100px;border:2px solid black;background-color:red;'>");
//               client.println("<p style='text-align:center;margin-top:40px;font-size:18px;'>Camera kapot</p>");
//             } else {
//               ledStatus = LOW;
//               // LED is uit, toon normaal camera vierkant
//               client.println("<div style='width:100px;height:100px;border:2px solid black;background-color:green;'>");
//             }
//             client.println("</div>");
//             client.println("</body></html>");
//             break;
//           } else {
//             currentLine = "";
//           }
//         } else if (c != '\r') {
//           currentLine += c;
//         }
//       }
//     }
//     client.stop();
//     Serial.println("client disconnected");
//   }
// }

void sendCameraStatusToServer(bool cameraBroke) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("192.168.0.103", 5000)) {  // Vervang dit door het juiste IP-adres en poortnummer van de Flask-server
      String status = cameraBroke ? "1" : "0"; // Stuur 1 als camera kapot is, 0 als het normaal werkt
      String request = "GET /update/" + status + " HTTP/1.1\r\n" +
                       "Host: 192.168.0.103:5000\r\n" +  // Vervang dit door het juiste IP-adres en poortnummer van de Flask-server
                       "Connection: close\r\n\r\n";
      client.print(request);
      client.stop();
    } else {
      Serial.println("Failed to connect to the Flask server");
    }
  } else {
    Serial.println("Not connected to WiFi");
  }
}

