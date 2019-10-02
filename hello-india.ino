/*
 * Author: Louis Moreau: https://github.com/luisomoreau
 * Author: Guillaume Simenel: https://github.com/gsimenel
 * Date: 2017/03/03
 * Description:
 * This arduino example will show you how to send a Sigfox message
 * using the wisol module and the arduino UNO (https://yadom.fr/carte-breakout-sfm10r1.html)
*/

// include the SoftwareSerial library so you can use its functions:
#include <SoftwareSerial.h>

#define rxPin 5
#define txPin 4

// set up a new serial port
SoftwareSerial Sigfox =  SoftwareSerial(rxPin, txPin);

//Set to 0 if you don't need to see the messages in the console
#define DEBUG 1

//Message buffer
uint8_t msg[12];
uint8_t answer[8];

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  if(DEBUG){
    Serial.begin(9600);
  }

  // open Wisol communication
   // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  Sigfox.begin(9600);
  delay(100);
  
  getID();
  getPAC();
  sendCommand("AT$IF=865200000", "UL Frequency set: ");
  sendCommand("AT$DR=866300000", "DL Frequency set: ");
  sendCommand("ATS410=1" ,"Public key: ");
  sendCommand("ATS302=15" ,"Set output power to max: ");
  sendCommand("AT$WR", "Save config: ");
  sendCommand("AT$T?", "Get module temperature: ");
}

// the loop function runs over and over again forever
void loop() {
  msg[0]=0x00;
  msg[1]=0xFF;
  msg[2]=0xEE;
  sendMessage(msg, 3);
  delay(6000);
  
  msg[0]=0xBA;
  msg[1]=0xBE;
  sendMessageDL(msg, 2);
  delay(6000);
  // In the ETSI zone, due to the reglementation, an object cannot emit more than 1% of the time hourly
  // So, 1 hour = 3600 sec
  // 1% of 3600 sec = 36 sec
  // A Sigfox message takes 6 seconds to emit
  // 36 sec / 6 sec = 6 messages per hours -> 1 every 10 minutes
  
}

void blink(){
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);    
}

//Get Sigfox ID
String getID(){
  return sendCommand("AT$I=10", "Sigfox Device ID:");
}

//Get PAC number
String getPAC(){
  return sendCommand("AT$I=11", "Sigfox Device PAC:");
}

//Send Sigfox Message
String sendCommand(String cmd, String display){
  Sigfox.print(cmd);
  Sigfox.print("\r");

  return displayStatus(display + cmd);
}

//Send Sigfox Message
void sendMessage(uint8_t msg[], int size){
  
  Sigfox.print("AT$SF=");
  
  for(int i= 0;i<size;i++){
    
    if (msg[i] < 16){
      Sigfox.write('0');
    }
    Sigfox.print(String(msg[i], HEX));
    if(DEBUG){
      if (msg[i] < 16){
        Serial.write('0');
      }
      Serial.print(String(msg[i], HEX));
    }
  }
  Serial.println();
  Sigfox.print("\r");

  displayStatus("UL Status \t");
}

//Send Sigfox Message with Downlink
String sendMessageDL(uint8_t msg[], int size){

  Sigfox.print("AT$SF=");
  
  for(int i= 0;i<size;i++){
    if (msg[i] < 16){
      Sigfox.write('0');
    }
    Sigfox.print(String(msg[i], HEX));
    if(DEBUG){
      if (msg[i] < 16){
        Serial.write('0');
      }
      Serial.print(String(msg[i], HEX));
    }
  }
  Serial.println();
  Sigfox.print(",1\r");
  if ( displayStatus("DL Status") == "OK") {
    // Wait for 20s before the opening of the DL window. 
    delay(20000);
    return displayStatus("Returned");
  }
}

String displayStatus(String message) {
  String status = "";
  char output;
  
  while (!Sigfox.available()){
     blink();
  }
  while(Sigfox.available()){
    output = (char)Sigfox.read();
    status += output;
    delay(10);
  }
  if(DEBUG){
    Serial.println(message);
    Serial.println(status);
  }
  return status;
}
