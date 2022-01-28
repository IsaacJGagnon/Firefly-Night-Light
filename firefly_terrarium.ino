#include <Wire.h>
#include <RTClib.h>

/* Glower struct */
typedef struct {
  int  pin;
  int  brightness;
  int  wait;
  int  rate;
  bool fade;
} LED;

RTC_DS1307 rtc;
const int ledCount = 6;
const int seedPin  = A0;
bool reCal = false;
bool night = false;
int maxWait = 501;
const int minWaitMod = 101;

LED leds[ledCount];

//---- setup ----
void setup() {
  // put your setup code here, to run once:
  int pins[ledCount] = {3,5,6,9,10,11};

  Serial.begin(9600);
  while (!Serial);

  pinMode(seedPin, INPUT);
  randomSeed(analogRead(seedPin));
  
  for(int i = 0; i < ledCount; i++){
    leds[i].pin   = pins[i];
    leds[i].brightness  = 0;
    leds[i].wait  = random((maxWait - minWaitMod), maxWait);
    leds[i].rate = (i == 0) ? 5 : 15;
    leds[i].fade  = false;
    pinMode(leds[i].pin, OUTPUT);
  }

  if (!rtc.begin()){
    Serial.println("Couldn't find RTC");
    while(1);
  }

  //Uncomment, compile, and re-upload if timing gets weird
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  // put your main code here, to run repeatedly:
  timeCheck();
  if(night){
    reCal = true;
    for(int i = 0; i < ledCount; i++){
      if(i == 0){
        glowLED(leds[i]);
      } else if(leds[i].wait == 0){
        glowLED(leds[i]);
      } else {
        leds[i].wait--;
      }
    }
  }else if(reCal){
    reCalLeds();
    reCal = false;
  }

  delay(50);
}

int getMinutes(){
  return ((((rtc.now()).unixtime())%(86400L))/60);
}

void timeCheck(){
  int tm = getMinutes();
  if (tm >= 1140){
    maxWait = map(tm, 1140, 1440, 501, 101);
    night = true;
  } else {
    night = false;
  }
}

void reCalLeds(){
  for(int i = 0; i < ledCount; i++){
    leds[i].brightness  = 0;
    leds[i].wait  = random((maxWait - minWaitMod), maxWait);
    leds[i].fade  = false;
    analogWrite(leds[i].pin, leds[i].brightness);
  }
}

void glowLED(LED &led){
  if(led.fade){
    led.brightness -= led.rate;
  } else {
    led.brightness += led.rate;
  }

  analogWrite(led.pin, led.brightness);

  if(led.brightness == 255){
    led.fade = true;
  } else if (led.brightness == 0){
    led.fade = false;
    randomSeed(analogRead(seedPin));
    led.wait = random((maxWait - minWaitMod), maxWait);
  }
}
