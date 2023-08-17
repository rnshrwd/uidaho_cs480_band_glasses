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

int redPin = 4;
int greenPin = 3;
int bluePin = 5;
int pin_reset = 6;
int pin_cs = 8;
int pin_interrupt = 2;
int idVal;
int startId;

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

void setup() {
  DDRC = 0x00;
  Serial.begin(9600);
  SoftPWMBegin();
  SoftPWMSet(redPin, 0);
  SoftPWMSet(greenPin, 0);
  pinMode(bluePin, OUTPUT);
  SoftPWMSetFadeTime(redPin, 10, 10);
  SoftPWMSetFadeTime(greenPin, 10, 10);
  
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

int redPin = 4;
int greenPin = 3;
int bluePin = 5;
int pin_reset = 6;
int pin_cs = 8;
int pin_interrupt = 2;
int idVal;
int startId;

Mrf24j mrf(pin_reset, pin_cs, pin_interrupt);

void setup() {
  DDRC = 0x00;
  Serial.begin(9600);
  SoftPWMBegin();
  SoftPWMSet(redPin, 0);
  SoftPWMSet(greenPin, 0);
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
  digitalWrite(A0, HIGH);
  digitalWrite(A1, HIGH);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, HIGH);
  digitalWrite(A4, HIGH);
  digitalWrite(A5, HIGH);
  
  mrf.reset();
  mrf.init();
  mrf.set_pan(2015);
  mrf.set_channel(0x0C);
  mrf.address16_write(0x4202);
  mrf.set_promiscuous(true);
  mrf.set_bufferPHY(true);
  
  attachInterrupt(0, interrupt_routine, CHANGE);
  interrupts();
  idVal = digitalRead(22);
  //digitalRead(PB1); 
  //PORTC & 0b111111111;
  //PC0 & 0b111111111;
  //idVal = 95;
  //Redundant but leaving for now
  startId = idVal;
}

void interrupt_routine()
{
  mrf.interrupt_handler();
}

void loop()
{
  Serial.println(idVal);
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

  
  int red = (colorVal & 0b11100000);
  //The colors must now be converted to appropriate rgb colors
  if ((colorVal - 224) == 0)
  {
    red = 255;
  }
  else
  {
    red = 0;
  }

  int green = (colorVal & 0b00011100);
  //green = (green >> 2);
  if((colorVal - 28) == 0)
  {
    green = 255;
  }
  else
  {
    green = 0;
  }


  //blue is already at the value needed
  int blue = (colorVal & 0b00000011);
  if ((colorVal - 3) == 0)
  {
    blue = 255;
  }
  else
  {
    blue = 0;
  }



  SoftPWMSet(redPin, red);
  SoftPWMSet(greenPin, green);
  analogWrite(bluePin, blue);
}


