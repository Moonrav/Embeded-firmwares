#include <ESP8266WiFi.h>
#include <Wire.h> // Library for I2C communication
#include "RTClib.h"
#include "HX711.h"     // this library warking fine
 
RTC_DS3231 RTC;                                 // Setup an instance of DS3231 naming it RTC

//---------------Load cell config--------------//
const int HX711_dout = 2;             //data pin connected to mcu GPIO
const int HX711_sck = 0;             //clock pin connected to mcu GPIO
HX711 LoadCell; // setup hx711 named as scale

int t =0;

int LT_SW1 = 10;
int LT_SW1_STATE=0;
///////Motot m1/////////////
int   M1_Fwd = 12;
int   M1_Rev = 13;
///////Motot m1/////////////
int   M3 = 16;
//////////// Motor m2//////////////////////
int M2_Fwd = 14;                          // foreord pin connected to in1 of L298N module
int M2_rev = 15;                          // reverce pin connected to in2 of L298N module

 uint32_t Tc;             //current rtc time
 uint32_t T_set1 =172500;  
 uint32_t T_set2 =172700;

float calibration=-1.0;
float W=0;
float W_set = 200;
float W_temp;
void setup () 
{
  Serial.begin(9600);                             // Set serial port speed
  LoadCell.begin(HX711_dout, HX711_sck);
  LoadCell.tare();
  


  
  RTC.begin();                              // Init RTC
  DateTime now = RTC.now();
  DateTime compiled = DateTime(__DATE__, __TIME__);
  if (now.unixtime() < compiled.unixtime()) {
    Serial.println("RTC is older than compile time! Updating");
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  // sets the pins as outputs:
  pinMode(M2_Fwd, OUTPUT);
  pinMode(M2_rev, OUTPUT);
  pinMode(M1_Fwd, OUTPUT);
  pinMode(M1_Rev, OUTPUT);
  pinMode(M3, OUTPUT);
  pinMode(LT_SW1, INPUT);
  
}
 
void loop () { 

  checkCurrentTime();
  delay(50);
  measurement();
  delay(1000);
  
  if((Tc>=T_set1 && Tc<=T_set1+30)||(Tc>=T_set2 && Tc<=T_set2+30))
  {
    
    digitalWrite(M3,HIGH);
    Serial.println("spreader ON");
    while(W<=W_set){
      Serial.println("checked weight");
      if(W_temp<=30)
      {
        
        measurement();
        Serial.println(W_set);
        Serial.println(W);
        Serial.println(W_temp);
        W=W+W_temp;
        
            
        run_dumper();
        delay(1000);
        
        
      }
    }
    digitalWrite(M3,LOW);
    Serial.println("spreader OFF");
   
  
  
  }
}

void checkCurrentTime()
  {
   Serial.println("rtc function called");
  DateTime now = RTC.now();
  delay(300);

  Tc = 10000*now.hour()+100*now.minute()+now.second();
  Serial.println(Tc);

  
  }

 void measurement()
 {
  Serial.println("load cell called:");
    if (LoadCell.is_ready()) 
    {
      float   i=0.0;
      LoadCell.tare();
      delay(300);
      //i = LoadCell.read();
      i = LoadCell.get_value();
      i=i*calibration;
      Serial.println(i);
      //long i = LoadCell.get_units(10);
      delay(200);
      //i = LoadCell.read();
      i = LoadCell.get_value();
      delay(50);
      Serial.print("Load_cell output val: ");
      Serial.println(i);
      W_temp=i;
      LoadCell.tare();
    }
    else{
      Serial.println("HX711 NOT FOUND");
    }

 }

void run_dumper(){
   digitalWrite(M1_Fwd,HIGH);       //start dumpper motor
      Serial.println("Dumpper motor on,in fwd dir");
  delay(500);
  digitalWrite(M1_Fwd,LOW);
      Serial.println("Dumpper motor.fwd off");
  while(W_temp!=30 || W_temp<30 ){
    Serial.println("waiting for 30 gms food dispence");
    if(W_temp>=30){
      break;
    }
  }
  if(W_temp>=30)
      {
    digitalWrite(M1_Rev,HIGH);       //start dumpper motor rev   
    Serial.println("Dumpper motor on,in rev dir");
      while(digitalRead(LT_SW1)!=HIGH){
        LT_SW1_STATE=digitalRead(LT_SW1); 
        delay(10);
        Serial.println("waiting for sw to high");         
      } 
      if(LT_SW1_STATE==HIGH){
       digitalWrite(M1_Rev,LOW);
        Serial.println("Dumpper motor.rev off");
        run_flapper();      
      }
        
    }
    }  
  
 
void run_flapper()
{    
    Serial.println("Flapper function called"); 
    digitalWrite(M2_Fwd,HIGH);    //weigh feeder motor forward
    delay(10000);
    digitalWrite(M2_Fwd,LOW);     // stop the motor
    digitalWrite(M2_rev,HIGH);    //weigh feeder motor reverse
    delay(10000);
    digitalWrite(M2_rev,LOW);     //stop the motor
}
