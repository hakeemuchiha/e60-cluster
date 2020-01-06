#include <FlexCAN_T4.h>

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;

static CAN_message_t clusterWake, clusterLights,mileageMsg,absConfig,fuelLevelMsg,absMsg, absMsg2, speedMsg,rpmMsg;
uint32_t id = 0;
//SpeedMsg speed;
 
int fuel1 = 0x4F04;
int fuel2 = 0x2312;
 
int absCtr = 0;
int airBagCtr = 0;
int mphCounter = 0;
word lastMPH = 0;
int secondCounter = 0;
bool counted = false;
word gasTest =0;

void setup() {
  Serial.begin(115200);
  while(!Serial); 
  pinMode(2, OUTPUT); // for the transceiver enable pin
  Can1.begin();
  Can1.setBaudRate(100000);

  Can1.setMBFilter(REJECT_ALL);
  Can1.enableMBInterrupts();
  Can1.setMBFilter(MB0, 0x800, 0x801, 0x803, 0x804, 0x805);
  Can1.enableFIFO();
  Can1.enableFIFOInterrupt();
  Can1.onReceive(FIFO, canSniff);
  Can1.mailboxStatus();
  

//  wakeCluster();
//  lightCluster();  
}
void send_command(uint32_t id, byte arr[],CAN_message_t &msg_name){  
  
//  char outbuf[8],msg_id[8];
    msg_name.id = id; //Set canbus id to the id passed in the function  
    msg_name.len = sizeof(arr)+1; //Set Size equal to the amount passed in
    for (int i = 0; i <= sizeof(arr); i++) {
      msg_name.buf[i] = arr[i];
    }
}

//Code to Wake the cluster
void wakeCluster(){
  id = 0x130;
  byte arr[] = {0x45,0x42,0x21,0x8F,0xFE};
  send_command(id, arr,clusterWake);  
  Can1.write(clusterWake);
}

//Turn on Backlight for the cluster
void lightCluster(){
  //Light the cluster
  id = 0x21A;
  byte arr[] = {0x04,0x22,0xF7};
  send_command(id, arr, clusterLights);
  Can1.write(clusterLights);
}
void getMileage(){
  //95 0A 01 2D 29 2F 9C 34
  id = 0x330;
  byte arr[] = {0xF4,0xFF,0x02,0x2D,0x29,0x2F,0x9C,0x34};
  send_command(id, arr, mileageMsg);
  Can1.write(mileageMsg);
}

void getSpeed(){
  //0x13, 0x4D, 0x46, 0x4D, 0x33, 0x4D, 0xD0, 0xFF 

  int time_1A6=millis();
  word MPH_A=mphCounter;
  word MPH= MPH_A +lastMPH;
  
  //[time now] - [time last sent] ) / 50) * [Speed] ) + last reading
//  int MPH = ((((millis()-time_1A6)/50)*MPH_A)+lastMPH);

  //Set speed to global variable
  lastMPH=MPH;
  id = 0x1A6;
  byte arr[] = {lowByte(MPH),highByte(MPH),lowByte(MPH),highByte(MPH),lowByte(MPH),highByte(MPH),lowByte(mphCounter),highByte(mphCounter)};
//  byte arr[] = {0x13, 0x4D, 0x46, 0x4D, 0x33, 0x4D, lowByte(mphCounter),highByte(mphCounter)};
  send_command(id, arr, speedMsg);  
  mphCounter+=1;
  Can1.write(speedMsg);
  
//  Serial.print("Second: ");Serial.print(secondCounter);
//  Serial.print("Time: ");Serial.print(time_1A6);
//  Serial.print("Equation: ");Serial.print(MPH_2A);
//  Serial.print("Old Speed: ");Serial.print(MPH_A);
//  Serial.print("Speed: ");Serial.print(MPH_2A);
//  Serial.print("COUNTER: ");Serial.print(mphCounter);
}
void sendRPM(){
  rpmMsg.id = 0x0AA; //From ECU
  rpmMsg.len =8;
  rpmMsg.buf[0]= 0x6C;//0x5F;
  rpmMsg.buf[1]= 0x06;//0x59;
  rpmMsg.buf[2]= 0x28; //Throttle
  rpmMsg.buf[3]= 0xFE; //Throttle
  rpmMsg.buf[4]= 0xD0; //RPM
  rpmMsg.buf[5]= 0X6B; //RPM
  rpmMsg.buf[6]= 0x94; 
  rpmMsg.buf[7]= 0x00;
//  id = 0x0AA;
//  byte arr[] = {0x6C, 0x06, 0x28,0xFE,0xD0,0x6B,0x94,0x00};
//  send_command(id, arr, rpmMsg);
  Can1.write(rpmMsg);
}
void sendABSCtr() {
    
    char data[2];
    absCtr++;
    if (absCtr == 0x0f) absCtr = 0;
    id = 0x0C0;
    byte arr[] = {0xF0 | absCtr ,0xFF};
    send_command(id, arr, absMsg);
    Can1.write(absMsg);
    Serial.print("ABS : ");
    Serial.print(absCtr);
}
 
void sendAirBagCtr() {
    
//    char data[2];
    airBagCtr++;
    if (airBagCtr == 0xff) airBagCtr = 0;
    id = 0x0D7;
    byte arr[] = {absCtr ,0xFF};
    send_command(id, arr, absMsg2);
    Can1.write(absMsg2);
//8 1 3 C0 FF FF FF FF FF 
//      id = 0x592;
//      byte arr[] = {0x08, 0x01, 0x03, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//80  21  D5  7E  DE  F4  FD  B7
//      byte arr[] = {0x0, 0x13, 0x01, 0x31, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//Len = 8
      
//      byte arr2[] = {0x0, 0x13,0x01, 30, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
//      send_command(id, arr, absMsg);
//      send_command(id, arr2, absMsg);
//      Can1.write(absMsg);
//      Can1.write(absMsg2);
}

//Send Fuel Level
void getFuelLevel(){
  //76 0F BE 1A 00
  Serial.print("0 :");  Serial.print(fuel1 / 256);
  Serial.print(" 1 :");  Serial.print(fuel1 & 0xff);
  Serial.print(" 2 :");  Serial.print(fuel2 / 256);
  Serial.print(" 3 :");  Serial.print(fuel2 & 0xff);

//  data[0] = fuel1 / 256;
//    data[1] = fuel1 & 0xff;
//    data[2] = fuel2 / 256;
//    data[3] = fuel2 & 0xff;
//  id = 0x349;
//  byte arr[] = {fuel1/256,fuel1&0xff,fuel2/256,0};
//  send_command(id, arr, fuelLevelMsg);
//  gasTest+=0X76;  
  fuelLevelMsg.id = 0x349; //From ECU
  fuelLevelMsg.len =5;
  fuelLevelMsg.buf[0]= 0x76;//0x5F;
  fuelLevelMsg.buf[1]= 0x0F;//0x59;
  fuelLevelMsg.buf[2]= 0xBE; //Throttle
  fuelLevelMsg.buf[3]= 0x1A; //Throttle
  fuelLevelMsg.buf[4]= 0; //Throttle
  Can1.write(fuelLevelMsg);
  
}

//Remove the ABS Light
void removeABS(){
  id = 0x19E;
  //00 E0 B3 FC F0 43 00 65
  byte arr[] = {0x00,0xE0, 0xB3,0xFC, 0xF0, 0x43, 0x00, 0x65};
  send_command(id, arr, absConfig);
  
}

void loop() {
  Can1.events();

  //Custom Code
  
  static uint32_t t = millis();
  static uint32_t s = millis();
  static uint32_t u = millis();
//  static uint32_t s = millis();
//  int timeElapsed = millis() - t;
  if(millis() - s > 10){
    s = millis();
    sendRPM();
//    sendAirBagCtr();
  }
  if ( millis() - t > 100 ) {
    t = millis();
    CAN_message_t frame;
    static uint32_t id = 0;
    id++;
    if ( id > 100 ) id = 1;
    frame.id = id;
    for ( uint8_t i = 0; i < 8; i++ ) {
      frame.buf[i] = id;
    }
    Can1.write(frame);
    wakeCluster();
    lightCluster();
    
//    getMileage();
//    sendRPM();
//    getSpeed();
    
    sendABSCtr();
    sendAirBagCtr();
    removeABS();
//    getFuelLevel();
    
  }
  if ( millis() - u > 200 ) {
    u = millis();
    CAN_message_t frame3;
    static uint32_t id = 0;
    id++;
    if ( id > 200 ) id = 1;
    frame3.id = id;
    Can1.write(frame3);
//    sendABSCtr();
//    sendAirBagCtr();
//    removeABS();
//    getFuelLevel();  
    
  }
  CAN_message_t msg;
  if ( Can1.read(msg) ) canSniff(msg); // polling
}
void canSniff(const CAN_message_t &msg) { // global callback
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}
