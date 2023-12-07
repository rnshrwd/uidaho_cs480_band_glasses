#include <SoftPWM_timer.h>
#include <SoftPWM.h>

#include "mrf24j.h"
#include <SPI.h>

/*
* uiGC0 
* Goofy Controller 0
* common cathode LEDs +
* Mrf24j40
* 
* Benjamin Jeffery
* University of Idaho
* 10/09/2015
* 
*/

int redPin = 3;
int greenPin = 4;
int bluePin = 5;
int pin_reset = 6;
int pin_cs = 8;
int pin_interrupt = 2;
int idVal;
int startId;
int red = 0;
int green = 0;
int blue = 0;

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

void setup() {
  DDRC = 0x00;
  Serial.begin(9600);
  SoftPWMBegin();
  SoftPWMSet(redPin, 0);
  SoftPWMSet(greenPin, 0);
  //pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  SoftPWMSetFadeTime(redPin, 10, 10);
  SoftPWMSetFadeTime(greenPin, 10, 10);
  
  // The following pins are connected to the rotary or DIP switch that sets
  // the channel. They are all inputs, but we write to them to enable the
  // internal pullup resistor.

  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  DDRB = (DDRB | 0b00000010);
  pinMode(A6, INPUT);
  pinMode(A7, INPUT);
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
  digitalWrite(A6, HIGH);
  PORTB = (PORTB | 0b00000010);
  digitalWrite(A7, HIGH);
  
  mrf.reset();
  mrf.init();
  mrf.set_pan(2015);
  //mrf.set_channel(0x0C);
  mrf.address16_write(0x4202);
  mrf.set_promiscuous(true);
  mrf.set_bufferPHY(true);
  
  attachInterrupt(0, interrupt_routine, CHANGE);
  interrupts();
  //Calculating the ID value by shifting bits
  int dipSwitch7And8Val = ((~PINB & 0b00000010) << 5);
  int signalToRead = ((~PINB & 0b00000100) >> 2);
  if(signalToRead == 0) {
    mrf.set_channel(0x0C);
  }
  else  {
    mrf.set_channel(0x0D);
  }
  idVal = dipSwitch7And8Val + (~PINC & 0b00111111);
  //Redundant but leaving for now
  startId = idVal;
}

void interrupt_routine()
{
  mrf.interrupt_handler();
}

void loop()
{
  //Serial.println(idVal);
    mrf.check_flags(&handle_rx, &handle_tx);
}

void handle_rx()
{
  //setColor function changed to use less data for more numbers of glasses.
  setColor(mrf.get_rxinfo()->rx_data[startId]);
  mrf.rx_flush();
}

void handle_tx()
{
}

void setColor(int colorVal)
{
  
  //To make this work with a single integer we use AND statements and bit shifting.
  //Given this byte: 11111111
  //rrrgggbb 
  //r = red color values: g = green color values: b = blue color values

  Serial.println(colorVal);
  if(colorVal == 255) {
    return;
  }

  red = colorVal & 0b11100000;
  red = (red >> 5);
  red = red * 36;

  green = colorVal & 0b00011100;
  green = (green >> 2);
  green = green *36;

  blue = colorVal & 0b00000011;
  blue = blue * 85;
  

  // color selection based on case statement
  int pickcolor = colorVal;

  //Set brightness of colors, values 1-255. (where 1 is the lowest, and 255 is the highest) 
  int brightness = 10;
 /*
 switch (pickcolor) { 
    
 case 224: //224 = Red
    red = brightness;
    break;
     
 case 28: // 28 = Green
    green = brightness;
    break; 

 case 3: // 3 = Blue
    blue = brightness;
    break;
      
 case 252: // 224+28 = 252 Yellow
    red = brightness; 
    green = brightness;
    break; 
  
  case 227: // 224+3 = 227 purple/magenta
    red = brightness;
    blue = brightness;

  case 31:  // 28+3 = 31 cyan
    blue = brightness;
    green = brightness;
          
 default: 
    red = 0; 
    green = 0;
    blue = 0;   
    break; 
  } 
  */


  SoftPWMSet(redPin, red);
  SoftPWMSet(greenPin, green);
  analogWrite(bluePin, blue);
}
