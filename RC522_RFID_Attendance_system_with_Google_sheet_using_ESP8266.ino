#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"

//// MOSI D7 MISO D6 SCK D5 
bool flag1=false;
bool flag2=false;

int led1=D3;
int led2=D4;

#define SS_PIN 4  //D2
#define RST_PIN 5 //D1

byte readCard[4];
String tag_UID = "23 2A AE 14";  // Replace this with the UID of your tag!!!
String tag_UID2="33 CA DA 2A";
String tagId = "";
MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// Enter network credentials:
const char* ssid     = "theinfoflux";
const char* password = "12345678";

// Enter Google Script Deployment ID:
const char *GScriptId = "AKfycbyghQNNZBIBEvmLQmIBIIb9SlctvA71L-_iC97Uc_NnveH5baPEa5Rf1D0R0O1Vwyob";

// Enter command (insert_row or append_row) and your Google Sheets sheet name (default is Sheet1):
String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

// Google Sheets setup (do not edit)
const char* host = "script.google.com";
const int httpsPort = 443;

String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

// Declare variables that will be published to Google Sheets
String cardholder = "";
String id = "";
String Status ="";

void setup() {
  Serial.begin(115200);        
  Serial.println('\n');
  Serial.println("System initialized");
  SPI.begin(); // SPI bus
  mfrc522.PCD_Init(); // Initialise MFRC522
  // Connect to WiFi
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  digitalWrite(led1,LOW);
  digitalWrite(led2,LOW);
  WiFi.begin(ssid, password);             
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");
  
  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i=0; i<5; i++){ 
    int retval = client->connect(host, httpsPort);
    if (retval == 1){
       flag = true;
       Serial.println("Connected");
       break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag){
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    return;
  }
  delete client;    // delete HTTPSRedirect object
  client = nullptr; // delete HTTPSRedirect object


  
}


void loop() {
// Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  //Show UID on serial monitor
  Serial.println();
  Serial.print(" UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  Serial.println();
   tagId=content.substring(1);
  if( tagId==tag_UID)
  {
   digitalWrite(led1,HIGH);
    if( flag1== false)
    {
     flag1=true;
      cardholder="Salman";
      id="EE123";
      Status="in";
      updatesheet(cardholder,id, Status);
      tagId = "";
      delay(1000);
   
    }
 digitalWrite(led1,LOW);
  }

      if( tagId==tag_UID)
  {
    digitalWrite(led1,HIGH);
      if(flag1==true)
    {
      
     flag1=false;
      Status="";
      cardholder="Salman";
      id="EE123";
      Status="out";
             updatesheet(cardholder,id, Status);

      tagId = "";
      delay(1000);
      
    }
digitalWrite(led1,LOW);
  }
      
      if( tagId==tag_UID2)
  {
    if( flag2==false)
    {
      digitalWrite(led2,HIGH);
     flag2=true;
      cardholder="";
      id="";
      Status="";
      cardholder="Shahrukh";
      id="ME456";
      Status="in";
             updatesheet(cardholder,id, Status);

      tagId = "";
      delay(1000);
      digitalWrite(led2,LOW);
    }

  }

      if( tagId==tag_UID2)
  {
    digitalWrite(led2,HIGH);
             if( flag2 == true)
    {
      flag2=false;
       cardholder="Shahrukh";
      id="ME456";
      Status="out";
             updatesheet(cardholder,id, Status);

      tagId = "";
      delay(1000);
      digitalWrite(led2,LOW);
    }

  }

}

void updatesheet(String cardholder, String id, String Status)

{


  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){
    if (!client->connected()){
      client->connect(host, httpsPort);
    }
  }
  else{
    Serial.println("Error creating client object!");
  }
  
  // Create json object string to send to Google Sheets
  payload = payload_base + "\"" + cardholder + "," + id + "," + Status + "\"}";
  
  // Publish data to Google Sheets
  Serial.println("Publishing data...");
  Serial.println(payload);
  if(client->POST(url, host, payload)){ 
    // do stuff here if publish was successful
  }
  else{
    // do stuff here if publish was not successful
    Serial.println("Error while connecting");
  }

  // a delay of several seconds is required before publishing again    
  delay(5000);
  }
