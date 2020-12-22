/*******************************************************************************
 * This file is part of PsxNewLib.                                             *
 *                                                                             *
 * Copyright (C) 2019-2020 by SukkoPera <software@sukkology.net>               *
 *                                                                             *
 * PsxNewLib is free software: you can redistribute it and/or                  *
 * modify it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * PsxNewLib is distributed in the hope that it will be useful,                *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with PsxNewLib. If not, see http://www.gnu.org/licenses.              *
 *******************************************************************************
 *
 * This sketch will dump to serial whatever is done on a PSX controller. It is
 * an excellent way to test that all buttons/sticks are read correctly.
 *
 * It's missing support for analog buttons, that will come in the future.
 *
 * This example drives the controller by bitbanging the protocol, there is
 * another similar one using the hardware SPI support.
 */

#include <DigitalIO.h>
#include <PsxControllerBitBang.h>

#include <avr/pgmspace.h>


#include <Servo.h>


Servo leftWheel;
Servo rightWheel;



typedef const __FlashStringHelper * FlashStr;
typedef const byte* PGM_BYTES_P;
#define PSTR_TO_F(s) reinterpret_cast<const __FlashStringHelper *> (s)

// These can be changed freely when using the bitbanged protocol
const byte PIN_PS2_ATT = 10;
const byte PIN_PS2_CMD = 11;
const byte PIN_PS2_DAT = 12;
const byte PIN_PS2_CLK = 13;

const byte PIN_BUTTONPRESS = A0;
const byte PIN_HAVECONTROLLER = A1;

const char buttonSelectName[] PROGMEM = "Select";
const char buttonL3Name[] PROGMEM = "L3";
const char buttonR3Name[] PROGMEM = "R3";
const char buttonStartName[] PROGMEM = "Start";
const char buttonUpName[] PROGMEM = "Up";
const char buttonRightName[] PROGMEM = "Right";
const char buttonDownName[] PROGMEM = "Down";
const char buttonLeftName[] PROGMEM = "Left";
const char buttonL2Name[] PROGMEM = "L2";
const char buttonR2Name[] PROGMEM = "R2";
const char buttonL1Name[] PROGMEM = "L1";
const char buttonR1Name[] PROGMEM = "R1";
const char buttonTriangleName[] PROGMEM = "Triangle";
const char buttonCircleName[] PROGMEM = "Circle";
const char buttonCrossName[] PROGMEM = "Cross";
const char buttonSquareName[] PROGMEM = "Square";

const char* const psxButtonNames[PSX_BUTTONS_NO] PROGMEM = {
  buttonSelectName,
  buttonL3Name,
  buttonR3Name,
  buttonStartName,
  buttonUpName,
  buttonRightName,
  buttonDownName,
  buttonLeftName,
  buttonL2Name,
  buttonR2Name,
  buttonL1Name,
  buttonR1Name,
  buttonTriangleName,
  buttonCircleName,
  buttonCrossName,
  buttonSquareName
};


bool driving = false; //is the car driving or sitting still?

#define CHANNEL_NUMBER 12  //set the number of chanNels
#define CHANNEL_DEFAULT_VALUE 1500  //set the default servo value
#define FRAME_LENGTH 22500  //set the PPM frame length in microseconds (1ms = 1000µs)
#define PULSE_LENGTH 300  //set the pulse length
#define onState 1  //set polarity of the pulses: 1 is positive, 0 is negative
#define sigPin 6  //set PPM signal output pin on the arduino

int ppm[CHANNEL_NUMBER];



byte psxButtonToIndex (PsxButtons psxButtons) {
  byte i;

  for (i = 0; i < PSX_BUTTONS_NO; ++i) {
    if (psxButtons & 0x01) {
      break;
    }

    psxButtons >>= 1U;
  }

  return i;
}

FlashStr getButtonName (PsxButtons psxButton) {
  FlashStr ret = F("");

  byte b = psxButtonToIndex (psxButton);
  if (b < PSX_BUTTONS_NO) {
    PGM_BYTES_P bName = reinterpret_cast<PGM_BYTES_P> (pgm_read_ptr (&(psxButtonNames[b])));
    ret = PSTR_TO_F (bName);
  }

  return ret;
}

void dumpButtons (PsxButtons psxButtons) {
  static PsxButtons lastB = 0;

  if (psxButtons != lastB) {
    lastB = psxButtons;     // Save it before we alter it
    //Serial.println(lastB);
    if(driving == false){ //if we aren't driving (for safety purposes) and a button is pressed that changes the behavior of the lights, send a command to the other arduino that controls them
      if(lastB == 32){
        Serial.print("incColor\r");
        delay(100);
        Serial.print("incColor\r");
      }
      else if(lastB == 128){
        Serial.print("decColor\r");
        delay(100);
        Serial.print("decColor\r");
      }
      else if(lastB == 8){
        Serial.print("stripPower\r");
        delay(100);
        Serial.print("stripPower\r");
      }
      else if(lastB == 4096){
        Serial.print("flashMod\r");
        delay(100);
        Serial.print("flashMod\r");
      }
      else if(lastB == 32768){
        Serial.print("marquisMod\r");
        delay(100);
        Serial.print("marquisMod\r");
      }
      else if(lastB == 8192){
        Serial.print("breathingMod\r");
        delay(100);
        Serial.print("breathingMod\r");
      }
      else if(lastB == 256){
        Serial.print("marquisLEDsDown\r");
        delay(100);
        Serial.print("marquisLEDsDown\r");
      }
      else if(lastB == 512){
        Serial.print("marquisLEDsUp\r");
        delay(100);
        Serial.print("marquisLEDsUp\r");
      }
      else if(lastB == 2048){
        Serial.print("cycleDelayDown\r");
        delay(100);
        Serial.print("cycleDelayDown\r");
      }
      else if(lastB == 1024){
        Serial.print("cycleDelayUp\r");
        delay(100);
        Serial.print("cycleDelayUp\r");
      }
    }



    //Serial.println(colorIndex);
    //Serial.print (F("Pressed: "));
    for (byte i = 0; i < PSX_BUTTONS_NO; ++i) {
      byte b = psxButtonToIndex (psxButtons);
      if (b < PSX_BUTTONS_NO) {
        PGM_BYTES_P bName = reinterpret_cast<PGM_BYTES_P> (pgm_read_ptr (&(psxButtonNames[b])));
        //Serial.print (PSTR_TO_F (bName));
      }

      psxButtons &= ~(1 << b);

      if (psxButtons != 0) {
        //Serial.print (F(", "));
      }
    }

    //Serial.println ();
  }
}

void dumpAnalog (const char *str, const byte x, const byte y) {
  Serial.print (str);
  Serial.print (F(" analog: x = "));
  Serial.print (x);
  Serial.print (F(", y = "));
  Serial.println (y);
}



const char ctrlTypeUnknown[] PROGMEM = "Unknown";
const char ctrlTypeDualShock[] PROGMEM = "Dual Shock";
const char ctrlTypeDsWireless[] PROGMEM = "Dual Shock Wireless";
const char ctrlTypeGuitHero[] PROGMEM = "Guitar Hero";
const char ctrlTypeOutOfBounds[] PROGMEM = "(Out of bounds)";

const char* const controllerTypeStrings[PSCTRL_MAX + 1] PROGMEM = {
  ctrlTypeUnknown,
  ctrlTypeDualShock,
  ctrlTypeDsWireless,
  ctrlTypeGuitHero,
  ctrlTypeOutOfBounds
};







PsxControllerBitBang<PIN_PS2_ATT, PIN_PS2_CMD, PIN_PS2_DAT, PIN_PS2_CLK> psx;

boolean haveController = false;

int leftPWM = 1500;
int rightPWM = 1500;

void setup () {
  leftWheel.attach(5);
  rightWheel.attach(3);
  leftWheel.writeMicroseconds(leftPWM);
  rightWheel.writeMicroseconds(rightPWM); //make sure that neither wheel is moving

  fastPinMode (PIN_BUTTONPRESS, OUTPUT);
  fastPinMode (PIN_HAVECONTROLLER, OUTPUT);

  delay (300);

  //initiallize default ppm values
  for(int i=0; i<CHANNEL_NUMBER; i++){
      ppm[i]= CHANNEL_DEFAULT_VALUE;
  }

  pinMode(sigPin, OUTPUT);
  digitalWrite(sigPin, !onState);  //set the PPM signal pin to the default state (off)

  cli();
  TCCR1A = 0; // set entire TCCR1 register to 0
  TCCR1B = 0;

  OCR1A = 100;  // compare match register, change this
  TCCR1B |= (1 << WGM12);  // turn on CTC mode
  TCCR1B |= (1 << CS11);  // 8 prescaler: 0,5 microseconds at 16mhz
  TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
  sei();

  Serial.begin (115200);
  //Serial.println (F("Ready!"));
}

void loop () {
  static byte slx, sly, srx, sry;

  fastDigitalWrite (PIN_HAVECONTROLLER, haveController);


  if (!haveController) {
    leftWheel.writeMicroseconds(1500);
    rightWheel.writeMicroseconds(1500); //if we lose connection to the controller, tell both wheels to stop
    if (psx.begin ()) {
      //Serial.println (F("Controller found!"));
      delay (300);
      if (!psx.enterConfigMode ()) {
        //Serial.println (F("Cannot enter config mode"));
      } else {
        PsxControllerType ctype = psx.getControllerType ();
        PGM_BYTES_P cname = reinterpret_cast<PGM_BYTES_P> (pgm_read_ptr (&(controllerTypeStrings[ctype < PSCTRL_MAX ? static_cast<byte> (ctype) : PSCTRL_MAX])));
        //Serial.print (F("Controller Type is: "));
        //Serial.println (PSTR_TO_F (cname));

        if (!psx.enableAnalogSticks ()) {
          //Serial.println (F("Cannot enable analog sticks"));
        }

        //~ if (!psx.setAnalogMode (false)) {
          //~ Serial.println (F("Cannot disable analog mode"));
        //~ }
        //~ delay (10);

        if (!psx.enableAnalogButtons ()) {
          //Serial.println (F("Cannot enable analog buttons"));
        }

        if (!psx.exitConfigMode ()) {
          //Serial.println (F("Cannot exit config mode"));
        }
      }

      haveController = true;
    }
  } else {
    if (!psx.read ()) {
      leftWheel.writeMicroseconds(1500);
      rightWheel.writeMicroseconds(1500) //if we lose connection to the controller, tell both wheels to stop
      //Serial.println (F("Controller lost :("));
      haveController = false;
    } else {
      fastDigitalWrite (PIN_BUTTONPRESS, !!psx.getButtonWord ());
      dumpButtons (psx.getButtonWord ());

      byte lx, ly;
      psx.getLeftAnalog (lx, ly);
      leftPWM = map(ly, 0, 255, 2000, 1000);
      if(leftPWM > 1350 && leftPWM < 1600){
        leftPWM = 1500;
      }
      leftWheel.writeMicroseconds(leftPWM); //read the left analog stick and write the corresponding PWM value to the left wheel
      if (lx != slx || ly != sly) {
        //dumpAnalog ("Left", lx, ly);
        slx = lx;
        sly = ly;
      }

      byte rx, ry;
      psx.getRightAnalog (rx, ry);
      rightPWM = map(ry, 0, 255, 2000, 1000);
      if(rightPWM > 1350 && rightPWM < 1600){
        rightPWM = 1500;
      }
      //Serial.println(rightPWM);
      rightWheel.writeMicroseconds(rightPWM); //read the right analog stick and write the corresponding PWM value to the right wheel
      if (rx != srx || ry != sry) {
        //dumpAnalog ("Right", rx, ry);
        srx = rx;
        sry = ry;
      }
      if(leftPWM != 1500 || rightPWM != 1500){
        driving = true;
      }
      else{
        driving = false; //a check to see if we're driving or sitting still (used to determine if we can change the lights or not)
      }
      //Serial.println(String(leftPWM) + ", " + String(rightPWM));
    }
  }


  delay (1000 / 60);
}


/*ISR(TIMER1_COMPA_vect){  //leave this alone
  static boolean state = true;

  TCNT1 = 0;

  if (state) {  //start pulse
    digitalWrite(sigPin, onState);
    OCR1A = PULSE_LENGTH * 2;
    state = false;
  } else{  //end pulse and calculate when to start the next pulse
    static byte cur_chan_numb;
    static unsigned int calc_rest;

    digitalWrite(sigPin, !onState);
    state = true;

    if(cur_chan_numb >= CHANNEL_NUMBER){
      cur_chan_numb = 0;
      calc_rest = calc_rest + PULSE_LENGTH;//
      OCR1A = (FRAME_LENGTH - calc_rest) * 2;
      calc_rest = 0;
    }
    else{
      OCR1A = (ppm[cur_chan_numb] - PULSE_LENGTH) * 2;
      calc_rest = calc_rest + ppm[cur_chan_numb];
      cur_chan_numb++;
    }
  }
}*/
