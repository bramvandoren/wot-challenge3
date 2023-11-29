///////////////////////////////////////////////////////////////////////////

// CAMERA 3

/////////////////////////////////////////////////////////////////////////////

#include <WiFi.h>
#include <PubSubClient.h>
#include <IRremote.h>

// Define your WiFi credentials
const char *ssid = "WOT";
const char *password = "enterthegame";

// Define MQTT broker information
const char *mqtt_server = "192.168.50.97";
const int mqtt_port = 1883;
const char *mqtt_topic = "camera/status";

// Define variables for board
const int RECV_PIN = 7;
const int led = 8;
IRrecv irrecv(RECV_PIN);
decode_results results;

unsigned long lastSignalTime = 0;
const unsigned long signalTimeout1 = 3000;
const unsigned long signalTimeout2 = 3000;

bool ledStatus = LOW;
bool camera_broke = false;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(9600);

  irrecv.enableIRIn();
  irrecv.blink13(true);
  pinMode(led, OUTPUT);

  while (!Serial);

  connectWiFi();

  client.setServer(mqtt_server, mqtt_port);

}

void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();
  recieveIR();
  digitalWrite(led, ledStatus);
}

void recieveIR() {
  unsigned long currentTime = millis();
  bool secondSignalReceived = false;
  bool thirdSignalReceived = false;
  bool fourthSignalReceived = false;

  while (millis() - currentTime < 500) {
    if (irrecv.decode(&results) && !camera_broke) {
      // IR-signaal ontvangen
      Serial.println("IR-signaal ontvangen!");
      irrecv.disableIRIn();
      digitalWrite(led, HIGH);
      ledStatus = HIGH;
      delay(500);
      ledStatus = LOW;
      digitalWrite(led, LOW);
      delay(1000);
      irrecv.resume();
      lastSignalTime = millis();
      irrecv.enableIRIn();

      // Wacht tot het tweede signaal binnen de tijdslimiet wordt ontvangen
      while ((millis() - lastSignalTime <= signalTimeout1) && (millis() >= 1000)) {
        if (irrecv.decode(&results)) {
          Serial.println("Tweede signaal ontvangen!");
          irrecv.disableIRIn();
          secondSignalReceived = true;
          digitalWrite(led, HIGH);
          ledStatus = HIGH;
          delay(500);
          ledStatus = LOW;
          digitalWrite(led, LOW);
          delay(1000);
          irrecv.resume();
          lastSignalTime = millis();
          irrecv.enableIRIn();

          // Wacht tot het derde signaal binnen de tijdslimiet wordt ontvangen
          while ((millis() - lastSignalTime <= signalTimeout1) && (millis() >= 1000)) {
            if (irrecv.decode(&results)) {
              Serial.println("Derde signaal ontvangen!");
              irrecv.disableIRIn();
              thirdSignalReceived = true;
              digitalWrite(led, HIGH);
              ledStatus = HIGH;
              delay(500);
              ledStatus = LOW;
              digitalWrite(led, LOW);
              delay(1000);
              irrecv.resume();
              lastSignalTime = millis();
              irrecv.enableIRIn();

              // Wacht tot het vierde signaal binnen de tijdslimiet wordt ontvangen
              while ((millis() - lastSignalTime <= signalTimeout1) && (millis() >= 1000)) {
                if (irrecv.decode(&results)) {
                  Serial.println("Vierde signaal ontvangen!");
                  fourthSignalReceived = true;
                  break;
                }
              }
              break; // Voeg deze break toe om de derde while-lus te verlaten na ontvangst van het derde signaal
            }
          }
          break; // Voeg deze break toe om de tweede while-lus te verlaten na ontvangst van het tweede signaal
        }
      }

      // Schakel de IR-bibliotheek weer in voor toekomstige signalen
      irrecv.enableIRIn();

      if (secondSignalReceived && thirdSignalReceived && fourthSignalReceived) {
        Serial.println("Alle signalen correct ontvangen!");
        digitalWrite(led, HIGH);
        camera_broke = true;
        ledStatus = HIGH;
        sendCameraStatusToMQTT(camera_broke);
        break;  // Voeg een break toe om de buitenste while-lus te verlaten na succesvolle ontvangst van signalen
      } else {
        Serial.println("Niet alle signalen correct ontvangen!");
        // Doe hier wat je moet doen wanneer niet alle signalen correct zijn ontvangen
      }
    }
  }
}



void connectWiFi() {
  Serial.println("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }
  Serial.println("Connected to WiFi");
}

// void printWifiStatus() {
//   // print the SSID of the network you're attached to:
//   Serial.print("SSID: ");
//   Serial.println(WiFi.SSID());

//   // print your board's IP address:
//   IPAddress ip = WiFi.localIP();
//   Serial.print("IP Address: ");
//   Serial.println(ip);

//   // print the received signal strength:
//   long rssi = WiFi.RSSI();
//   Serial.print("signal strength (RSSI):");
//   Serial.print(rssi);
//   Serial.println(" dBm");

//   Serial.print("To see this page in action, open a browser to http://");
//   Serial.println(ip);
// }

// void enable_WiFi() {
//   // check for the WiFi module:
//   if (WiFi.status() == WL_NO_MODULE) {
//     Serial.println("Communication with WiFi module failed!");
//     // don't continue
//     while (true);
//   }

//   String fv = WiFi.firmwareVersion();
//   if (fv < "1.0.0") {
//     Serial.println("Please upgrade the firmware");
//   }
// }

// void connect_WiFi() {
//   // Stel het statische IP-adres, gateway, subnetmasker en DNS in:
//   IPAddress ip(192, 168, 50, 121);  // Statisch IP-adres
//   IPAddress gateway(192, 168, 50, 1);  // Gateway
//   IPAddress subnet(255, 255, 255, 0);  // Subnetmasker
//   IPAddress dns(192, 168, 50, 1);  // DNS-server (meestal hetzelfde als de gateway)

//   // Configureer het WiFi-netwerk met het statische IP-adres en andere instellingen:
//   WiFi.config(ip, gateway, subnet, dns);
  
//   // Probeer verbinding te maken met het WiFi-netwerk:
//   status = WiFi.begin(ssid, pass);

//   // Wacht 10 seconden voor de verbinding:
//   delay(10000);
// }

void reconnectMQTT() {
  while (!client.connected()) {
    // Serial.println("Connecting to MQTT...");
    if (client.connect("CameraClient")) {
      // Serial.println("Connected to MQTT");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void sendCameraStatusToMQTT(bool cameraBroke) {
  if (client.connected()) {
    String status = cameraBroke ? "Camera kapot" : "Camera aan";
    client.publish(mqtt_topic, status.c_str());
    Serial.print("Camera Status published: ");
    Serial.println(status);
  } else {
    Serial.println("Not connected to MQTT");
  }
}



// void printWEB() {
//   unsigned long currentTime = millis();
//   bool secondSignalReceived = false;
//   bool thirdSignalReceived = false;
  

//   if (client) {
//     Serial.println("new client");
//     String currentLine = "";
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

// void sendCameraStatusToServer(bool cameraBroke) {
//   if (WiFi.status() == WL_CONNECTED) {
//     WiFiClient client;
//     if (client.connect("192.168.0.103", 5000)) {  // Vervang dit door het juiste IP-adres en poortnummer van de Flask-server
//       String status = cameraBroke ? "1" : "0"; // Stuur 1 als camera kapot is, 0 als het normaal werkt
//       String request = "GET /update/" + status + " HTTP/1.1\r\n" +
//                        "Host: 192.168.0.103:5000\r\n" +  // Vervang dit door het juiste IP-adres en poortnummer van de Flask-server
//                        "Connection: close\r\n\r\n";
//       client.print(request);
//       client.stop();
//     } else {
//       Serial.println("Failed to connect to the Flask server");
//     }
//   } else {
//     Serial.println("Not connected to WiFi");
//   }
// }

