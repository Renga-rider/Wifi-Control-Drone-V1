
#define BLYNK_PRINT Serial


#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define CPU_MHZ 80
#define CHANNEL_NUMBER 8  //set the number of chanels
#define CHANNEL_DEFAULT_VALUE 1000  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 0  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 5 //set PPM signal output pin on the arduino
#define DEBUGPIN 4
int fm=1;
volatile unsigned long next;
volatile unsigned int ppm_running=1;

int ppm[CHANNEL_NUMBER];

const byte captive_portal=0;


unsigned int alivecount=0;

unsigned long time_now = 0;
WidgetLCD lcd(V1);

char auth[] = "UHboDESufirGlMYyrMTfUIOc1sAawhat";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Renga";
char pass[] = "Renga_rider";

void inline ppmISR(void){
  static boolean state = true;

  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    next = next + (PULSE_LENGTH * CPU_MHZ);
    state = false;
    alivecount++;
  } 
  else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;
  
    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;// 
      next = next + ((FRAME_LENGTH - calc_rest) * CPU_MHZ);
      calc_rest = 0;
      digitalWrite(DEBUGPIN, !digitalRead(DEBUGPIN));
    }
    else{
      next = next + ((ppm[cur_chan_numb] - PULSE_LENGTH) * CPU_MHZ);
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }     
  }
  timer0_write(next);
}

void handleRoot() {
   if(ppm_running==0)
  {
    noInterrupts();
    timer0_isr_init();
    timer0_attachInterrupt(ppmISR);
    next=ESP.getCycleCount()+1000;
    timer0_write(next);
    for(int i=0; i<CHANNEL_NUMBER; i++){
      ppm[i]= CHANNEL_DEFAULT_VALUE;
    }
    ppm_running=1;
    interrupts();
  }
 }
 
BLYNK_WRITE(V2) {
  ppm[2] = param.asInt();

}
BLYNK_WRITE(V3) {
  ppm[3] = param.asInt();

}

BLYNK_WRITE(V6)
{
  ppm[1] = param.asInt(); // assigning incoming value from pin V6 to a variable
  ppm[0]=1500;
  
}
BLYNK_WRITE(V4)
{
  ppm[4] = param.asInt(); // assigning incoming value from pin V4 to a variable
  if(ppm[4]<=1250)
  {
    fm=1;
  }
  else if(ppm[4]>1250&&ppm[4]<=1500)
  {
    fm=2;
  }
  else if(ppm[4]>1500&&ppm[4]<=1750)
  {
    fm=3;
  }
  else if(ppm[4]>1750&&ppm[4]<=2000)
  {
    fm=4;
  }
  else
  {
    fm=1;
  }
}
BLYNK_WRITE(V5)
{
  ppm[7] = param.asInt(); // assigning incoming value from pin V5 to a variable
 
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  lcd.clear(); //Use it to clear the LCD Widget
  lcd.print(4, 0, "Drone"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  lcd.print(4, 1, "online");
  Blynk.begin(auth, ssid, pass);
  pinMode(sigPin,OUTPUT);
  digitalWrite(sigPin, !onState); //set the PPM signal pin to the default state (off)
  pinMode(DEBUGPIN,OUTPUT);
  digitalWrite(DEBUGPIN, !onState); //set the PPM signal pin to the default state (off)
  noInterrupts();
  timer0_detachInterrupt();
  ppm_running=0;
  noInterrupts();
  timer0_isr_init();
  timer0_attachInterrupt(ppmISR);
  next=ESP.getCycleCount()+1000;
  timer0_write(next);
  ppm[0]=1500;//yaw
  ppm[1]=1000;//throttle
  ppm[2]=1500;//roll
  ppm[3]=1500;//pitch
  ppm[4]=1000;//aux//flightmode
  ppm[5]=1000;//aux
  ppm[6]=1000;//aux
  ppm[7]=1000;//aux//arm disarm
  interrupts();
}

void loop()
{
  Blynk.run();
 
  if(fm==1)
  {
    lcd.print(0, 0, "ANGLE    ");
  }
  else if(fm==2)
  {
   lcd.print(0, 0, "HORIZON   ");
  }
   else if(fm==3)
  {
   lcd.print(0, 0, "HEADFREE  ");
  }
  else if(fm==4)
  {
   lcd.print(0, 0, "ACRO      "); 
  }
  if(ppm[7]>1500)
  {
    lcd.print(0, 1, "ARMED     ");
  }
  else
  {
     lcd.print(0, 1, "DISARMED   ");
  }
}
