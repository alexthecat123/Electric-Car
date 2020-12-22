#include <Adafruit_NeoPixel.h>

#define ledPin 6
#define ledCount 239

Adafruit_NeoPixel strip(ledCount, ledPin, NEO_GRB + NEO_KHZ800);

int colorIndex = 0;
bool stripOn = true;
bool flashMod = false;
bool marquisMod = false;
bool breathingMod = false;
bool signalMod = false;
int marquisLEDs = 1;
int cycleDelay = 100;
bool driving = false;

float timeToFlash = 0;
bool flashOn = false;
int bright = 10;
bool breatheDir = 0;
bool offWait = 0;
int offTime = 0;
int ledPos = 0;
int ledSpacing = 0;
bool marquisRun = 1;
int marquisDelay = 0;
int a = 0;
long pixelHue = 0;
int pixel = 0;
long nextPixelHue = 0;
int oldRight = 0;
int oldLeft = 0;
int signalCount = 0;
bool doSignals = 1;
int oldR = 0;
int oldG = 0;
int oldB = 0;
bool changeState = false;
bool firstTime = true;
bool exitMod = false;


String data = "";

void setup() {
  strip.begin();
  strip.setBrightness(255);
  Serial.begin(115200);
  Serial.setTimeout(1000);
}

void loop() {
  if(Serial.available() == 0){
   ledController();
  }
  if(Serial.available() > 0){
    data = Serial.readStringUntil('\r');
    delay(200);
    data = Serial.readStringUntil('\r');
    //Serial.println(data);
    if(data.equals("stripPower")){
      //Serial.println("Power");
      stripOn = !stripOn;
      if(stripOn == true){
        exitMod = true;
      }
      firstTime = true;
    }
    else if(data.equals("incColor")){
      //Serial.println("Inc");
      if(colorIndex < 9){
        colorIndex += 1;
      }
      else{
        colorIndex = 0;
      }
      if(colorIndex == 8){
        strip.setBrightness(255);
        cycleDelay = 80;
      }
      if(colorIndex == 9){
        strip.setBrightness(255);
        for(int i = 0; i < ledCount; i++){
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        cycleDelay = 60;
      }
    }
    else if(data.equals("decColor")){
      //Serial.println("Dec");
      if(colorIndex > 0){
        colorIndex -= 1;
      }
      else{
        colorIndex = 9;
        strip.setBrightness(255);
        for(int i = 0; i < ledCount; i++){
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        cycleDelay = 60;
      }
      if(colorIndex == 8){
        strip.setBrightness(255);
        cycleDelay = 80;
      }
    }
    else if(data.equals("flashMod")){
      cycleDelay = 40;
      flashMod = !flashMod;
      marquisMod = 0;
      breathingMod = 0;
      signalMod = 0;
      exitMod = true;
    }
    else if(data.equals("marquisMod")){
      cycleDelay = 40;
      marquisLEDs = ledCount/6;
      marquisMod = !marquisMod;
      flashMod = 0;
      breathingMod = 0;
      signalMod = 0;
      exitMod = true;
    }
    else if(data.equals("breathingMod")){
      cycleDelay = 60;
      breathingMod = !breathingMod;
      flashMod = 0;
      marquisMod = 0;
      signalMod = 0;
      exitMod = true;
    }
    else if(data.equals("marquisLEDsDown")){
      if(marquisLEDs > 1){
        marquisLEDs -= 1;
      }
    }
    else if(data.equals("marquisLEDsUp")){
      if(marquisLEDs < ledCount/2){
        marquisLEDs += 1;
      }
    }
    else if(data.equals("cycleDelayDown")){
      if(cycleDelay != 0){
        cycleDelay -= 20;
        //Serial.println(cycleDelay);
      }
    }
    else if(data.equals("cycleDelayUp")){
      if(cycleDelay != 100){
        cycleDelay += 20;
        //Serial.println(cycleDelay);
      }
    }
    data = "";
  }


}



void setStripColor(int r, int g, int b){
  if(stripOn == false && firstTime == true){
    //Serial.println("Here");
    marquisMod = 0;
    breathingMod = 0;
    flashMod = 0;
    colorIndex = 0;
    for(int i = 0; i < ledCount; i++){
        strip.setPixelColor(i, 0, 0, 0);
    }
    firstTime = false;
    changeState = true;
  }
  else if(flashMod == 0 && marquisMod == 0 && breathingMod == 0 && stripOn == true){ // && signalMod == 0
    strip.setBrightness(255);
    for(int i = 0; i < ledCount; i++){
      strip.setPixelColor(i, 0, 0, 0);
    }
    for(int i = 0; i < ledCount; i+=2){
      strip.setPixelColor(i, r, g, b);
    }
    if(oldR != r or oldG != g or oldB != b or exitMod == true){
      changeState = true;
      exitMod = false;
    }
    else{
      changeState = false;
    }
    oldR = r;
    oldG = g;
    oldB = b;
  }
  else if(flashMod == 1){
    strip.setBrightness(255);
    if(r != oldR or g != oldG or b != oldB){
      for(int i = 0; i < ledCount; i+=2){
        strip.setPixelColor(i, r, g, b);
      }
      changeState = true;
    }
    if(cycleDelay == 0){
      cycleDelay = 20;
    }
    if(timeToFlash >= cycleDelay * 10000){
      //Serial.println("Toggle: " + String(flashOn));
      if(flashOn == true){
        for(int i = 0; i < ledCount; i+=2){
          strip.setPixelColor(i, r, g, b);
        }
        changeState = true;
      }
      else if(flashOn == false){
        for(int i = 0; i < ledCount; i++){
          strip.setPixelColor(i, 0, 0, 0);
        }
        changeState = true;
      }
      flashOn = !flashOn;
      timeToFlash = 0;
    }
    timeToFlash += 0.5;
    oldR = r;
    oldG = g;
    oldB = b;
  }


    else if(marquisMod == 1){
      changeState = false;
      strip.setBrightness(255);

      ledSpacing = (ledCount/marquisLEDs) - 1;
      //Serial.println(ledSpacing);

      //Serial.println("Run");
      if(marquisRun == 1){
        for(int i = 0; i < ledCount; i++){
          strip.setPixelColor(i, 0, 0, 0);
        }
        for(int c = a; c < ledCount; c+= ledSpacing){
          strip.setPixelColor(c, r, g, b);
          //Serial.println(c);
        }
        strip.show();
        a += 1;
        if(a >= ledSpacing){
           a = 0;
        }
      }


        marquisRun = 0;
        //Serial.println("Stop");
        //Serial.println(marquisDelay);
        if(marquisDelay >= max(cycleDelay, 10) * 80){
          marquisRun = 1;
          marquisDelay = 0;
        }
        marquisDelay += 1;
    }
      //Serial.println("Done");






  else if(breathingMod == 1){
    for(int i = 0; i < ledCount; i+=2){
      strip.setPixelColor(i, r, g, b);
    }
    if(bright <= 0 or bright >= 245){
      //Serial.println("Changing directions!");
      breatheDir = !breatheDir;
      if(bright <= 0){
        bright = 1;
        offWait = 1;
      }
    }
    if(offWait == 1){
      strip.setBrightness(0);
      if(offTime >= map(cycleDelay, 0, 100, 10, 80)){
        offWait = 0;
        offTime = 0;
      }
      offTime += 1;
    }
    //Serial.println(bright);
    strip.setBrightness(max(bright, 0));
    changeState = true;
    if((breatheDir == 0) and (offWait == 0)){
      bright -= map(cycleDelay, 0, 100, 5, 1);
    }
    if((breatheDir == 1) and (offWait == 0)){
      bright += map(cycleDelay, 0, 100, 5, 1);
    }
  }

  if(changeState == true){
    strip.show();
  }
  changeState = false;
}

void ledController(){
  switch (colorIndex){
    case 0:
      setStripColor(255, 0, 0);
      break;
    case 1:
      setStripColor(255, 34, 0);
      break;
    case 2:
      setStripColor(255, 170, 0);
      break;
    case 3:
      setStripColor(0, 255, 0);
      break;
    case 4:
      setStripColor(0, 0, 255);
      break;
    case 5:
      setStripColor(255, 51, 119);
      break;
    case 6:
      setStripColor(128, 0, 128);.

      break;
    case 7:
      setStripColor(255, 255, 255);
      break;
    case 8:
      //Serial.println("Case 8");
      if(stripOn == true){
        uint32_t packedRGB = strip.gamma32(strip.ColorHSV(pixelHue));
        int r =  packedRGB & 255;
        int g = (packedRGB >> 8) & 255;
        int b = (packedRGB >> 16) & 255;
        setStripColor(r, g, b);
        pixelHue += map(cycleDelay, 0, 100, 512, 2);
        break;
      }
      if(stripOn == false){
        for(int i = 0; i < ledCount; i++){
          strip.setPixelColor(i, 0, 0, 0);
        }
        strip.show();
        colorIndex = 0;
      }


    case int(9):
      break;
  }
  if(colorIndex == 9 && stripOn == true){
    //Serial.println("Case 9");
    for(int pixel = 0; pixel < ledCount; pixel+=2){
      int nextPixelHue = pixelHue + (pixel * 65536L / ledCount);
      uint32_t packedRGB = strip.gamma32(strip.ColorHSV(nextPixelHue));
      int r =  packedRGB & 255;
      int g = (packedRGB >> 8) & 255;
      int b = (packedRGB >> 16) & 255;
      //Serial.println(String(r) + ", " + String(g) + ", " + String(b));
      strip.setPixelColor(pixel, r, g, b);
    }
    strip.show();
    pixelHue += map(cycleDelay, 0, 100, 512, 32);
    }
  if(stripOn == false){
    for(int i = 0; i < ledCount; i++){
      strip.setPixelColor(i, 0, 0, 0);
    }
    strip.show();
    colorIndex = 0;
  }
}
