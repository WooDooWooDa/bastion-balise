#include <WiFi.h>
#include <HTTPClient.h>

#define LED_WIFI 17

DynamicJsonDocument json(1024);

const char* ssid = "Esp";
const char* password = "ganggangas";
const int timeout = 25;
int connectionAttempt = 0;

String serverName = "http://206.167.241.211:81";

unsigned long lastTime = 0;
unsigned long timerDelay = 20000;
bool isStartStopGameReady = false;
String response;

String httpGet(String);
void httpPost(String, String);

void WIFISetup() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while(WiFi.status() != WL_CONNECTED && connectionAttempt <= timeout) {
    delay(500);
    connectionAttempt++;
    Serial.print(".");
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.print("\nConnected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
    digitalWrite(LED_WIFI, HIGH);
  } else {
    Serial.println("\nCan't connect to WiFi...");
  }
}

void WIFILoop() {
  if ((millis() - lastTime) > timerDelay) {
    //check if game is running with this balise
    if(WiFi.status() == WL_CONNECTED) {
      char request[20];
      sprintf(request, "/activeGame/%d", BALISE_ID);
      response = httpGet(request);
      isStartStopGameReady = true;
      if (response == "startGame" && !gameIsRunning) {
        startGame();
      } else if (response == "stopGame" && gameIsRunning) {
        stopGame();
      }
    }
    
    lastTime = millis();
  }
}

String httpGet(String destination) {
  if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      
      String path = serverName + destination;
      Serial.print("Sending get to : ");
      Serial.println(path);
      http.begin(path.c_str());
      
      int httpResponseCode = http.GET();

      String payload = "";
      String jsonData = "";
      if (httpResponseCode > 0) {
        payload = http.getString();
        Serial.print("Payload : ");
        Serial.println(payload);
        deserializeJson(json, payload);
        Serial.print("Json Data : ");
        String jsonData = json["data"];
        payload = jsonData;
        Serial.println(jsonData);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      Serial.println();
      return payload;
  }
  Serial.println("WiFi Disconnected");
  return "";
}

void httpPost(String destination, String body) {
  if(WiFi.status() == WL_CONNECTED) {
      HTTPClient http;

      String path = serverName + destination + body;
      Serial.print("Sending post to : ");
      Serial.print(path);
      http.begin(path.c_str());
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");

      int httpResponseCode = http.POST(body);
      
      if (httpResponseCode>0) {
        String payload = http.getString();
        Serial.print("Payload : ");
        Serial.println(payload);
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
      Serial.println();
  } else {
    Serial.println("WiFi Disconnected");
  }
}
