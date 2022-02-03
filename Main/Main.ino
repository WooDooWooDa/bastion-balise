#include <ArduinoJson.h>
#include "heltec.h"
#include "bluetoothServer.h"
#include "SPI.h"
#include "batteryReader.h"
#include "button.h"
#include "game.h"
#include "wifiUnit.h"

#define BAND        915E6 
#define NODE_ID     1
const String ACK_PACKET = "ack";

String rssi = "RSSI --";
String packSize = "--";
String packet;
unsigned int packetCounter;
int resendCounter = 0;

bool receiveflag = false;
bool waitingForAck = false;
bool resendflag=false;
bool deepsleepflag=false;
bool updatePacketReady = false;

long lastSendTime = 0;        // last send time
int resendInterval = 2000;          // resendInterval between resends (20sec)
uint64_t chipid;
String lastMsg;
String updatePacket;

void interrupt_GPIO0()
{
  delay(10);
  if(digitalRead(0)== 0) {
     if(digitalRead(LED)==LOW) {
      resendflag=true;
    } else {
      deepsleepflag=true;
    }
  }
}

void setup() {
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_TAN, OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  Heltec.begin(true /*DisplayEnable Enable*/, true /*LoRa Enable*/, true /*Serial Enable*/, true /*LoRa use PABOOST*/, BAND /*LoRa RF working band*/);
  delay(300);
  Heltec.display -> clear();

  WIFISetup();
  bleServerStart();
  setupBatteryReading();
  setupButton();
  
  chipid=ESP.getEfuseMac();
  Serial.printf("ESP32ChipID=%04X \n",(uint16_t)(chipid>>32));

  attachInterrupt(0,interrupt_GPIO0,FALLING);
  //define on receive method
  LoRa.onReceive(onReceive);
  //put lora on receive state
  //send("initialize");
  LoRa.receive();
}

void loop() {
  if(receiveflag) {           //apres avoir recu, rentre ici
    displayReceive();
    delay(100);
    receiveflag = false;
    
    sendAck(); //send le ACK apres avoir receive dequoi
  }
  
  WIFILoop();

  if (isStartStopGameReady) {
    sendLora(response);
    isStartStopGameReady = false;
  }

  if (updatePacketReady) {
    send(updatePacket);
    updatePacketReady = false;
  }
  
  bleServerLoop();
  readBatteryVoltage();
  
  if (gameIsRunning) {
    gameLoop();
    if (isReadyToUpdate) {
      String msg = buildUpdateMsg();
      send(msg);
    }
  }
  
  if(deepsleepflag) {
    sleep();
  }
  if(resendflag) {
   resendflag=false;
   send("try hello " + (String)(packetCounter++));
   displaySendLoRa();
  }

  if (waitingForAck) {        //waiting for ACK
    if (lastSendTime == resendInterval) {
      resendCounter += 1;
      send(lastMsg);
      displayResend();
    }
    lastSendTime++;
  }
}

void displayResend() {
  Heltec.display -> drawString(0, 40, "Resent Packet " + (String)(packetCounter) + " X" + (String)(resendCounter));
  Heltec.display -> drawString(0, 50, "Packet: " + (String)(lastMsg));
  Heltec.display -> display();
  delay(100);
  Heltec.display -> clear();
}

void displaySendLoRa() {
  Heltec.display -> drawString(0, 0, "Sent packet on LoRa: " + (String)(packetCounter-1));
  Heltec.display -> drawString(0, 10, "Packet: " + (String)(lastMsg));
  Heltec.display -> display();
  delay(100);
  Heltec.display -> clear();
}

void displaySendWiFi(String msg) {
  Heltec.display -> drawString(0, 0, "Sent packet on WiFi: " + (String)(packetCounter-1));
  Heltec.display -> drawString(0, 10, "Packet: " + msg);
  Heltec.display -> display();
  delay(100);
  Heltec.display -> clear();
}

void displayReceive() {
  Heltec.display -> drawString(0, 0, "Received Size  " + packSize + " packages:");
  Heltec.display -> drawString(0, 10, packet);
  Heltec.display -> drawString(0, 20, "With RSSI: " + rssi + "db");
  Heltec.display -> display();
  delay(100);
  Heltec.display -> clear();
}

void sendAck() {
  resendCounter = 0;
  LoRa.beginPacket();
  LoRa.print(ACK_PACKET);
  LoRa.endPacket();
  delay(50);
  LoRa.receive();
}

void send(String msg) {
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Msg sent on WiFi...");
    httpPost("/updateFromBalise/", msg.substring(msg.indexOf('#') + 1));
    //displaySendWiFi(msg);
    return;
  }

  Serial.print("WiFi Disconnected... Sending msg on LoRa...\nMSG sent : ");
  sendLora(msg);
}

void sendLora(String msg) {
  Serial.println("Msg sent on Lora...");
  lastMsg = msg;
  digitalWrite(25, LOW);
  LoRa.beginPacket();
  LoRa.print(msg);
  LoRa.endPacket();
  delay(50);
  LoRa.receive();
  
  lastSendTime = 0;
  waitingForAck = true;
  displaySendLoRa();
}

void onReceive(int packetSize) { //LoRa receiver interrupt service (method) 
  if (packetSize == 0) return;

  packet = "";
  packSize = String(packetSize, DEC);

  while (LoRa.available()) {
    packet += (char) LoRa.read();
  }
  rssi = String(LoRa.packetRssi(), DEC);   

  Serial.print("\nPACKET : ");
  Serial.println(packet); 

  if (packet == ACK_PACKET && waitingForAck) {
    digitalWrite(25,HIGH); //WHITE LED ON BOARD
    Serial.println("ACK Received");
    Heltec.display -> clear();
    waitingForAck = false;
    lastSendTime = 0;
    return;
  }

  if (packet == START_GAME_PACKET && !gameIsRunning) {
    startGame();
  } else if (packet == STOP_GAME_PACKET && gameIsRunning) {
    stopGame();
  } else if (packet.startsWith("update")) {
    updatePacket = packet;
    updatePacketReady = true;
  }
  
  receiveflag = true;
}

void sleep() { //Sleep method
  LoRa.end();
  LoRa.sleep();
  delay(100);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(14,INPUT);
  pinMode(15,INPUT);
  pinMode(16,INPUT);
  pinMode(17,INPUT);
  pinMode(18,INPUT);
  pinMode(19,INPUT);
  pinMode(26,INPUT);
  pinMode(27,INPUT);
  digitalWrite(Vext,HIGH);
  delay(2);
  esp_deep_sleep_start();
}
