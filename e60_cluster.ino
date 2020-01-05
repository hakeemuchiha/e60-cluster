// -------------------------------------------------------------
// CANtest for Teensy 4.0 using CAN2 and CAN2 bus
#include <FlexCAN_T4.h>

#define debug(msg) Serial.print("["); Serial.print(__FILE__); Serial.print("::"); Serial.print(__LINE__);  Serial.print("::"); Serial.print(msg); Serial.println("]");

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2;
static uint8_t hex[17] = "0123456789abcdef";


static CAN_message_t msg, data,info,speedo, fuel_info,temp,cel,cel2,abss;
int period = 100;
unsigned long time_now = 0;

  static unsigned long next = millis();


static void hexDump(uint8_t dumpLen, uint8_t *bytePtr)
{
  uint8_t working;
  while( dumpLen-- ) {
    working = *bytePtr++;
    Serial.write( hex[ working>>4 ] );
    Serial.write( hex[ working&15 ] );
  }
  Serial.write('\r');
  Serial.write('\n');
}


// -------------------------------------------------------------

void setup(void)
{
  Serial.begin(9600);
  while(!Serial); 
  Can1.begin();  
  Can1.setBaudRate(100000);   // I like to set the baud rates just to be on the safe side
  init_cluster();
  checkEngine();
  getRPM();
  clusterLights();
  getRangeFuelOdo();
  set_temp();
  hide_icons();
}

//Create function to pass CANBUS Commands
void send_command(uint32_t id, byte arr[],CAN_message_t &msg_name){  
//  char outbuf[8],msg_id[8];
  msg_name.id = id; //Set canbus id to the id passed in the function  
  msg_name.len = sizeof(arr)+1; //Set Size equal to the amount passed in
  for (int i = 0; i <= sizeof(arr); i++) {
    msg_name.buf[i] = arr[i];
  }
}

void init_cluster(){
//  time_now = millis();
//  if(millis() - next > 100){
    uint32_t id = 0x130;
    byte arr[] = {0x45,0x42,0x21,0x8F,0xFE};
    send_command(id, arr,data);
//    Serial.println(time_now);
//    Serial.println(period);
//    next += 100;
//  }  
}

void getRangeFuelOdo(){
  fuel_info.id = 0x366;
  fuel_info.len = 4;
  fuel_info.buf[0]= 0x78; //0x45;  //Key Status
  fuel_info.buf[1]= 0x50; // 0x40;  //Transponder Detected
  fuel_info.buf[2]= 0x14;  // 0x21;  //Terminal Status
  fuel_info.buf[3]= 0xFC;  // Steering lock?
}

void checkEngine(){
    //SPEEDO
      msg.id = 0x1A6;  
      msg.len =8;
      msg.buf[0]= 0x1F;  
      msg.buf[1]= 0x00;   
      msg.buf[2]= 0x1F;   
      msg.buf[3]= 0x00;
      msg.buf[4]= 0x1F;
      msg.buf[5]= 0x00;
      msg.buf[6]= 0x30;   
      msg.buf[7]= 0xF2;
    
     
}
void getRPM(){
  info.id = 0x0AA; //From ECU
  info.len =8;
  info.buf[0]= 0x6C;//0x5F;
  info.buf[1]= 0x06;//0x59;
  info.buf[2]= 0x28; //Throttle
  info.buf[3]= 0xFE; //Throttle
  info.buf[4]= 0xD0; //RPM
  info.buf[5]= 0X6B; //RPM
  info.buf[6]= 0x94; 
  info.buf[7]= 0x00;
}

void clusterLights(){
//  delay(100);
  uint32_t id = 0x21A;
  byte arr[] = {0xFF};
  send_command(id, arr, speedo);

  uint32_t id2 = 0x592;
    byte arr2[] = {0x40,0x71,0x01,30,0xFF,0xFF,0xFF,0xFF};
    byte arr3[] = {0x40,0x71,0x01,30,0xFF,0xFF,0xFF,0xFF};
    send_command(id2,arr, cel);
    send_command(id2,arr, cel2);
}

void hide_icons(){
  time_now = millis();
    uint32_t id = 0x592;
    byte arr[] = {0x40,0x71,0x01,30,0xFF,0xFF,0xFF,0xFF};
    byte arr2[] = {0x40,0x71,0x01,30,0xFF,0xFF,0xFF,0xFF};
    send_command(id,arr, cel);
    send_command(id,arr2, cel2);
   
  
//  send_command(id,arr2, cel);
}

void set_temp(){
  delay(0);
    temp.id = 0x0C0; //From ECU
    temp.len =2;
    temp.buf[0]= 0xF4;//0x5F;
    temp.buf[0]= 0xFF;//0x5F;

    uint32_t id = 0x19E;
    byte arr[] = {0x00,0xE0,0xB3,0xFC,0xF0,0x43,0x00,0x65};
    send_command(id,arr, abss);
  
}

// -------------------------------------------------------------
void loop(void)
{
  delay(100);
  Can1.write(msg);    //you could do it your way as well
  Can1.write(data);    //you could do it your way as well
  Can1.write(info);    //you could do it your way as well
  Can1.write(speedo);    //you could do it your way as well
  Can1.write(fuel_info);    //you could do it your way as well
  Can1.write(temp);    //you could do it your way as well
  Can1.write(cel);    //you could do it your way as well
  Can1.write(cel2);    //you could do it your way as well
  Can1.write(abss);    //you could do it your way as well
//  Serial.println("T4.0cantest - Repeat: Read bus2, Write bus1");
  CAN_message_t inMsg;
  if (Can1.read(inMsg)!=0)     // Changed this to if as as opposed to while - the way you had it just gets stuck since you haven't even sent a message yet 
  {
    Serial.print("W RD bus 2: "); hexDump(8, inMsg.buf);
  }

//  delay(100);
}
