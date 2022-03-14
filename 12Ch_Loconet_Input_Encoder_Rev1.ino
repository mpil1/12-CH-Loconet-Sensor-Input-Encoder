/***********************************************
*12 Channel Locnet Sensor Input Encoder
*Rev 1.0, 
*March 13, 2022
*Michael Pilyih
*Copyright (C) 2022 Michael Pilyih
*************************************************
*************************************************
*Loconet.h Header:
*Copyright (C) 2009 to 2013 Alex Shepherd
*Portions Copyright (C) Digitrax Inc.
*Portions Copyright (C) Uhlenbrock Elektronik Gmb
*************************************************

Description:
-The 12 Input Loconet Ecoder will monitor 12 inputs and report the on / off state of each input
 to Loconet in the form of a Loconet Sensor Request.   
-Device will read a Start Address at startup from address 0x0 of EEPROM, assign that address to Input 1, then
  assign addresses to inputs 2 through 12 in sequece from there incrementing one address per input.
-Device is assigned a Start Address via Loconet as described below.  If a Start Address of 0 (or less) is 
  read from EEPROM at startup, the device's inputs will default to 1 though 12 in sequence.
-Serial Debug mode thru Arduino USB port = 115200 baud

Normal Run Mode: 
 -Red Loconet Status LED on, Green Program Mode LED off.  Red LED blinks when traffic on Loconet
 -Device moniotors the voltage at the inputs with respect to device ground:  
    -A voltage >= 5V-18VDC will cause the device to send a "Sensor nnnn ON" message over Loconet.  (nnnm being a number 
       from 1 to 4095, the maximum address allowed by Loconet)
    -A voltage of 0VDC will cause the device to sent a "Sensor nnnn OFF" message over Loconet.     
 
Program Mode:
  1) Press and hold Program Button for approx 5 Seconds.  When Red Loconet Status LED goes out and Green Program
     Mode LED lights, immediately let go of button.  Exit Program Mode at any time by pressing the Program button again. 
  2.)After approx 3 seconds the Green Program LED will flash slowly. Using a DCC throttle connected to loconet, select a 
      Switch # corresponding to the Sensor Address you want the device to start at and issue a THROWN command.
      Once the device receives the Switch Thrown command the device will respond by quickly
       flashing the Program Mode and Loconet Status LED's.  It will assgin the number you picked to input 1 and automatically
       assign sequential address to the remaining 11 inputs, in increments of 1. 
  3.)When the Green Program Mode LED goes out and the Red Loconet Status LED turns on, the device is now ready to
    use with the new address range assigned
     
     
Various Notes: 

//Loconet Turnout Direction parameter Transmitted: 1= Closed, 0 = Thrown
//--Command station sends a value of "32" for close, 0 for thrown

*/


#include <LocoNet.h>
#include <EEPROM.h>

#define PROGButton  A5  //Connects input to GND when closed
#define ledLocStat A4   //Annode to ouput pin, cathode to GND
#define ledProg A3      //Annode to ouput pin, cathode to GND

float rev = 1;

//Sensor Addresses
const int sensor[] = {2, 3, 4, 5, 7, 9, 10, 11, 12, 13, A1, A0}; //sensor input pin numbers
int sensorArrayLen = (sizeof(sensor)/sizeof(sensor[0]));
int sensorOn[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int startAddress;
int address[12];
int addressArrayLen = (sizeof(address)/sizeof(address[0]));

int sensorCount;
bool stillPressed = 0;

int progMode = 0;         //0= Run Mode, 1=Address Program Mode

unsigned long buttonPressMillis = 0;
unsigned long progDelay = 5000;
unsigned long flashRate[4]={750, 300, 100, 50};
unsigned long flashMillis;
bool flashState = 1;
int x;    //generic counter
int y;    //use to count eeprom address
int z;    //counter to quick flash led's in prog mode
bool z1;   //led toggle state for quick flash in prog mode
int dccPacketNum; //use to count and ignore second packet sent by cmd station
//****Digitrax Command Station sends two switch packets at once


lnMsg *LnPacket;      //need this before setup() to be used by Loconet.h


void setup() {
//Initialize Debug Serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Serial Debug Started");
  Serial.println("12 Input Loconet Encoder");
  Serial.print("Firmware Rev ");
  Serial.println(rev);
  Serial.println("(c) 2022 Mike Pilyih");
  Serial.println();

 //INPUT Pin Setup
  pinMode(PROGButton, INPUT_PULLUP);
  for (int i = 0; i < sensorArrayLen; i ++){
    pinMode(sensor[i], INPUT);  
  }

 //OUTPUT Pin Setup
  pinMode(ledLocStat, OUTPUT);
  pinMode(ledProg, OUTPUT);

     
 //Read Start Address Value from EEPROM, if value is zero default address to output number 1-12
    y = 0;
    EEPROM.get(y, startAddress);
    if (startAddress<= 0) startAddress = 1;
    for (sensorCount = 0; sensorCount < addressArrayLen; sensorCount++){
       address[sensorCount] = startAddress + sensorCount;
       Serial.print("Sensor ");
       Serial.print(sensorCount + 1);
       Serial.print(" Address= ");
       Serial.println(address[sensorCount]);
    }
  
  //initialize Loconet, default TX pin = 6, RX pin = 8
  LocoNet.init();
  Serial.println("Loconet Connection Initialized");
  digitalWrite(ledLocStat, 1);

  Serial.println("Input Encoder Ready!");
  
 



}




void loop() {


  addressProgramMode();
  updateSensors();  

  //Check for and process Loconet incoming message
   LnPacket = LocoNet.receive();
   if(LnPacket) {
    digitalWrite(ledLocStat, 0);
    delay(30);
    digitalWrite(ledLocStat, 1);
    //LocoNet.processSwitchSensorMessage(LnPacket);
   }
  

   

}

//******Loconet Switch Request Handler*************

void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction ) {

 if(progMode == 1) {  
   if (Output != 0){
        return;
      }
      startAddress = Address;
      EEPROM.put(0, startAddress );
      Serial.print("Start address saved as: ");
      Serial.println(startAddress);
      progMode = 2;
      x = 1;
      z1 = 0;
      for (z = 0; z <= 10; z++){
         digitalWrite(ledProg, z1);
         digitalWrite(ledLocStat, !z1);
         z1 = !z1;
         delay(100);
      }
 }   
}

//********* Other Functions*********************


void updateSensors() {
   //Check for and process Loconet incoming message
   LnPacket = LocoNet.receive();
   if(LnPacket) {
    digitalWrite(ledLocStat, 0);
    delay(30);
    digitalWrite(ledLocStat, 1);
    LocoNet.processSwitchSensorMessage(LnPacket);
   }

   for (sensorCount = 0; sensorCount < sensorArrayLen; sensorCount ++){
      if(digitalRead(sensor[sensorCount]) == 1 && !sensorOn[sensorCount]) {
         LocoNet.reportSensor(address[sensorCount], 1);
         sensorOn[sensorCount] = 1;
         Serial.print("Sensor ");
         Serial.print(address[sensorCount]);
         Serial.println(" ON");
         Serial.println();
         delay(20);
      }

      if(digitalRead(sensor[sensorCount]) == 0 && sensorOn[sensorCount]) {
        LocoNet.reportSensor(address[sensorCount], 0);
         sensorOn[sensorCount] = 0;
         Serial.print("Sensor ");
         Serial.print(address[sensorCount]);
         Serial.println(" OFF");
         Serial.println();
         delay(20);
      }
   }      

}

void addressProgramMode(){
  if(!digitalRead(PROGButton) == 1){
     buttonPressMillis = millis();
     while(!digitalRead(PROGButton) == 1){
       if ((millis()-buttonPressMillis) >= progDelay){
         Serial.println("Entering Address Program Mode");
         //Do Address Programming Routine within this IF statement
         digitalWrite(ledLocStat, 0);
         digitalWrite(ledProg, 1);
         delay(3000); 
         progMode = 1;   //entering program mode, waiting for address from Loconet
         x = 0;     //use this for both flash rate and address counter
         y = 0;
         
         while(!digitalRead(PROGButton) == 0){
            if (x == 1){
               Serial.println("We are here");
              break;
            }
            
            if((millis()-flashMillis) > flashRate[1]){
            flashState = !flashState; 
            Serial.println(flashState);
            digitalWrite(ledProg, flashState);   
            Serial.println("Waiting for Start Address from LocoNet.");
            flashMillis = millis();
            }
            
            
            LnPacket = LocoNet.receive();
            if(LnPacket) {
               LocoNet.processSwitchSensorMessage(LnPacket);       //this line will call notifySwitchRequest()
            }

            if (progMode == 2){
               for (y = 0; y < addressArrayLen; y ++){
               address[y] = startAddress + y; 
               }   
            }
         }
               
         digitalWrite(ledProg, 0);
         digitalWrite(ledLocStat, 1);
         progMode = 0;     //leaivng program mode
         Serial.println("Leaving Address Program Mode");
         delay(1000);
       }
     }
   }
}
