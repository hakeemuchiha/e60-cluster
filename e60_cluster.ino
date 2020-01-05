// -------------------------------------------------------------
// CANtest for Teensy 4.0 using CAN2 and CAN2 bus
#include <FlexCAN_T4.h>

#define debug(msg) Serial.print("["); Serial.print(__FILE__); Serial.print("::"); Serial.print(__LINE__);  Serial.print("::"); Serial.print(msg); Serial.println("]");

FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> Can1;
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2;

static CAN_message_t msg, data,info,speedo, fuel_info,temp;
int period = 100;
unsigned long time_now = 0;

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
  uint32_t id = 0x130;
  byte arr[] = {0x45,0x42,0x69,0x8F,0xE2};
  send_command(id, arr,data);
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
  time_now = millis();

  while(millis() < time_now + period){
    //wait approx. [period] ms

    //SPEEDO
//    msg.id = 0x1A6;  
//    msg.len =8;
//    msg.buf[0]= 0x13;  
//    msg.buf[1]= 0x4D;   
//    msg.buf[2]= 0x46;   
//    msg.buf[3]= 0x4D;
//    msg.buf[4]= 0x33;
//    msg.buf[5]= 0x4D;
//    msg.buf[6]= 0xD0;   
//    msg.buf[7]= 0xFF; 
  }
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
  speedo.id = 0x21A; //From ECU

  //Lights cluster with headlights
  //speedo.len =1;

  //Lights cluster without headlights (parking lights)
  speedo.len =1;
  speedo.buf[0]= 0xFF;//0x5F;
}

void set_temp(){
//  delay(100);
  time_now = millis();

  while(millis() < time_now + period){
    temp.id = 0x0C0; //From ECU
    temp.len =2;
    temp.buf[0]= 0xF4;//0x5F;
    temp.buf[0]= 0xFF;//0x5F;
    if(millis() < time_now + period){
     Serial.println("Print");
    }
    else{
      Serial.println("NO");
    }
  }
}

// -------------------------------------------------------------
void loop(void)
{
  delay(100);
  Can1.write(msg);    //you could do it your way as well
  Can1.write(data);    //you could do it your way as well
//  Can1.write(info);    //you could do it your way as well
  Can1.write(speedo);    //you could do it your way as well
  Can1.write(fuel_info);    //you could do it your way as well
  //Can1.write(temp);    //you could do it your way as well
//  Serial.println("T4.0cantest - Repeat: Read bus2, Write bus1");

//  delay(100);
}
