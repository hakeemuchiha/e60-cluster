#include <FlexCAN_T4.h>

FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2;

static CAN_message_t clusterWake, clusterLights,mileageMsg,absConfig,fuelLevelMsg,absMsg, absMsg2, speedMsg,rpmMsg,initC;
uint32_t id = 0;
//SpeedMsg speed;
 
int fuel1 = 0x4F04;
int fuel2 = 0x2312;
 
int absCtr = 0;
int airBagCtr = 0;
int absCounter = 0; //for 0x19E

int mphCounter = 0;
word lastMPH = 0;
int secondCounter = 0;
bool counted = false;
word gasTest =0;
bool firstBoot = true;
bool increase = true;
bool speedBoot = true;
int lastVal = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial); 
  
  Can2.begin();
  Can2.setClock(CLK_60MHz);
  Can2.setBaudRate(100000);

  Can2.setMBFilter(REJECT_ALL);
  Can2.enableMBInterrupts();
//  Can2.setMBFilter(MB0, 0x800, 0x801, 0x803, 0x804, 0x805);
  Can2.enableFIFO();
  Can2.enableFIFOInterrupt();
  Can2.onReceive(FIFO, canSniff);
  Can2.mailboxStatus();
  

//  wakeCluster();
//  lightCluster();  

//00 D0 E0 F8 00 32 FE 91
id = 0x1B4;

  byte arr[] = {0x00,0xD0,0xE0,0xF8,0x00,0x32,0xFE,0x91};
  send_command(id, arr,initC);  
  Can2.write(initC);
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
  Can2.write(clusterWake);
}

//Turn on Backlight for the cluster
void lightCluster(){
  //Light the cluster
  id = 0x21A;
  byte arr[] = {0x04,0x22,0xF7};
  send_command(id, arr, clusterLights);
  Can2.write(clusterLights);
}
void getMileage(){
  //95 0A 01 2D 29 2F 9C 34
  id = 0x330;
  byte arr[] = {0xF4,0xFF,0x02,0x2D,0x29,0x2F,0x9C,0x34};
  send_command(id, arr, mileageMsg);
  Can2.write(mileageMsg);
}

void getSpeed(){
  //0x13, 0x4D, 0x46, 0x4D, 0x33, 0x4D, 0xD0, 0xFF 

  word MPH= mphCounter +lastMPH;

  //Set speed to global variable
  lastMPH=MPH;
  id = 0x1A6;
  if(speedBoot){
    byte arr[] = {lowByte(MPH),highByte(MPH),lowByte(MPH),highByte(MPH),lowByte(MPH),highByte(MPH),lowByte(mphCounter),highByte(mphCounter)};
    send_command(id, arr, speedMsg);  
    Can2.write(speedMsg);
    while(mphCounter <= 100){
      Serial.println("COunt >> ");
      Serial.println(MPH);
    }   
  } 
}
void sendRPM(){
  rpmMsg.id = 0x0AA; //From ECU
  rpmMsg.len =8;

    //If first boot
    if(firstBoot){
      rpmMsg.buf[0]= 0x6C;//0x5F;
      rpmMsg.buf[1]= 0x06;//0x59;
      rpmMsg.buf[2]= 0x28; //Throttle
      rpmMsg.buf[3]= 0xFE; //Throttle u
      rpmMsg.buf[4]= 0xD0; //RPM
      rpmMsg.buf[5]= 0X6B; //RPM
      rpmMsg.buf[6]= 0x94; 
      rpmMsg.buf[7]= 0x00;
      Can2.write(rpmMsg);
      firstBoot=false;
    }
}
void removeABS() {
    
    id = 0x19E;
  //00 E0 B3 FC F0 43 00 65
  absCounter++;
  byte arr[] = {0,0xE0,0xB3,0xfc,0xf0,0,absCounter,0x65};
  send_command(id, arr, absConfig);
  Can2.write(absConfig);
  Serial.print("ABS : ");
  Serial.print(absCounter);
}

//Remove the ABS Light
void sendABSCtr(){
  id = 0x0c0;
  //00 E0 B3 FC F0 43 00 65
  absCtr++;
  if (absCtr == 0x0f) absCtr = 0;
  byte val = 0xf0 | absCtr;
  byte arr[] = {val,0xff};
  send_command(id, arr, absConfig);
  Can2.write(absConfig);

}
void sendAirBagCtr() {
    
//    char data[2];
  airBagCtr++;
  if (airBagCtr == 0xff) airBagCtr = 0;
  id = 0x0D7;
  byte arr[] = {absCtr, 0xFF};
  send_command(id, arr, absMsg2);
  Can2.write(absMsg2);

}

//Send Fuel Level
void getFuelLevel(){
  //76 0F BE 1A 00
  Serial.print("VAL 1 :");  Serial.print(fuel1 / 256);
  Serial.print("VAL 2 :");  Serial.print(fuel1 & 0xff);
  Serial.print("VAL 3 :");  Serial.print(fuel2 / 256);
  Serial.print("VAL 4 :");  Serial.print(fuel2 & 0xff);

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
  fuelLevelMsg.buf[0]= fuel1/256; //0x5F;
  fuelLevelMsg.buf[1]= fuel1 & 0xff;//0x59;
  fuelLevelMsg.buf[2]= fuel2/256; //Throttle
  fuelLevelMsg.buf[3]= fuel2 & 0xff; //Throttle
  fuelLevelMsg.buf[4]= 0; //Throttle
  Can2.write(fuelLevelMsg);
  
}




void loop() {
  Can2.events();

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
    Can2.write(frame);
    wakeCluster();
    lightCluster();
    
//    getMileage();
//    sendRPM();
    if(firstBoot){
      getSpeed();
      firstBoot=false;
    }
    
    
//    sendABSCtr();
//    sendAirBagCtr();
//    removeABS();
//    getFuelLevel();
    
  }
  if ( millis() - u > 200 ) {
    u = millis();
    CAN_message_t frame;
    static uint32_t id = 0;
    id++;
    if ( id > 100 ) id = 1;
    frame.id = id;
    for ( uint8_t i = 0; i < 8; i++ ) {
      frame.buf[i] = id;
    }
    Can2.write(frame);
    sendABSCtr();
    sendAirBagCtr();
    removeABS();
    getFuelLevel();  
    
  }
  CAN_message_t msg;
  if ( Can2.read(msg) ) canSniff(msg); // polling
}
void canSniff(const CAN_message_t &msg) { // global callback
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}
