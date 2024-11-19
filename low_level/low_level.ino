#include <LiquidCrystal_I2C.h>

#define LR1 22   // Red traffic light 1 connected in pin 22
#define LY1 23   // Yellow traffic light 1 connected in pin 23
#define LG1 24   // Green traffic light 1 connected in pin 24
#define LR2 25   // Red traffic light 2 connected in pin 25
#define LY2 26   // Yellow traffic light 2 connected in pin 26
#define LG2 27   // Green traffic light 2 connected in pin 27
#define P1 37
#define CNY2 34  // Infrared sensor 2 in traffic light 1 connected in pin 34 (middle)

//Constants
const int ORIGINAL_GREEN1_TIME = 500;

long timeStamp = 0;
int trafficLightState = 0;

long greenTime1 = ORIGINAL_GREEN1_TIME;
long yellowTime1 = 500;
long yellowTime2 = 500;
long greenTime2 = 2000;
long redTime1 = 2000;
int blinks = 0;
long blinkTime = 100;

//Pedestrian 1 pulser control

const int PEDESTRIAN_CROSS_TIME = 6000;
int vP1 = 0;
bool p1IsCrossing = false;
long p1TimeStamp = 0;
bool waitinForGreenStatus = false;  //When the P1 = HIGH, need to wait until light is green again.

//Infrared CNY2, when detect some car it increase the greentime, must that this is too short
int vCNY2 = 0;
bool ligh1DetectCar = false;
const int GREEN_TIME_WHEN_CAR_DETECTED = 3000;
bool ligh1CarIsCrossing = true;
bool CNY2WaitinForGreenStatus = false;  //When the P1 = HIGH, need to wait until light is green again.

LiquidCrystal_I2C display(0x27, 20, 4);

void setup() {

  // Output pin config
  pinMode(LR1, OUTPUT);  // Red traffic light 1 as Output
  pinMode(LY1, OUTPUT);  // Yellow traffic light 1 as Output
  pinMode(LG1, OUTPUT);  // Green traffic light 1 as Output
  pinMode(LR2, OUTPUT);  // Red traffic light 2 as Output
  pinMode(LY2, OUTPUT);  // Yellow traffic light 2 as Output
  pinMode(LG2, OUTPUT);  // Green traffic light 2 as Output
  pinMode(P1, INPUT);
    // Pedestrian Pulser 1

  //display.init();
  //display.backlight();
  timeStamp = millis();
  Serial.begin(9600);

}

void loop() {
  trafficLightFSM();
  setPedestrianPulser();
  checkForCNYYLigh2Sensor();
}

/*
Every CNY5EnabledTime we check if there is a car on CNY5 if that
is the case, we give higher priority to this ligh, reducing greenTime2, 
*/

void checkForCNYYLigh2Sensor() {
  vCNY2 = digitalRead(CNY2);
  Serial.println(vCNY2);
    //When the infrared receive a 0 is because there is a car there.
  if (!p1IsCrossing && vCNY2 == LOW && !CNY2WaitinForGreenStatus) {
      CNY2WaitinForGreenStatus = true;
  }

  if (CNY2WaitinForGreenStatus) {

    if (trafficLightState == 0) {

      p1TimeStamp = millis();
      ligh1CarIsCrossing = true;
      greenTime1 = GREEN_TIME_WHEN_CAR_DETECTED;
      CNY2WaitinForGreenStatus = false;   
    }
    
  }

    //After the time had been reach, we set the original time again
 if (ligh1CarIsCrossing && (millis() - p1TimeStamp > GREEN_TIME_WHEN_CAR_DETECTED)) {
       ligh1CarIsCrossing = false;
       greenTime1 = ORIGINAL_GREEN1_TIME;
    
  }
}

void trafficLightFSM() {

   switch (trafficLightState) {
      case 0 :

      digitalWrite(LG1, HIGH);
       digitalWrite(LR2, HIGH);
       digitalWrite(LY2, LOW);
       digitalWrite(LR1, LOW);
       digitalWrite(LY1, LOW);     
   if (millis() - timeStamp >= greenTime1) {
          trafficLightState = 1;
          timeStamp = millis();
    }
       break;

      case 1 :  //green blink

        if (millis() - timeStamp > blinkTime) {
                digitalWrite(LG1, !digitalRead(LG1));
                blinks++;
                timeStamp = millis();        
          }
        if (blinks > 7) {
           blinks = 0;
           trafficLightState = 2;
           timeStamp = millis();
        }
       break;

       case 2 :  //Yellow1 On
        digitalWrite(LY1, HIGH);
        digitalWrite(LG1, LOW);
        if (millis() - timeStamp > yellowTime1) {
           trafficLightState = 3;
           timeStamp = millis();
        }
        break;
        case 3 :  //RED1 ON
          digitalWrite(LY1, LOW);
          digitalWrite(LR1, HIGH);
          digitalWrite(LY2, HIGH);
          digitalWrite(LR2, LOW);
         if (millis() - timeStamp > redTime1) {
            trafficLightState = 4;
            timeStamp = millis();
          }
        break;

        case 4 : 
          digitalWrite(LR1, HIGH);
          digitalWrite(LY1, LOW);
          digitalWrite(LG1, LOW);
          digitalWrite(LY2, HIGH);
          digitalWrite(LG2, LOW);
          digitalWrite(LR2, LOW);

          if (millis() - timeStamp > yellowTime2) {
            trafficLightState = 5;
            timeStamp = millis();          
          }
        break;

        case 5 :  //Green2 on
          digitalWrite(LY2, LOW);
          digitalWrite(LG2, HIGH);
        if (millis() - timeStamp > greenTime2) {
           trafficLightState = 6;
           timeStamp = millis();
        }
        break;
        case 6 :  //Blink Green2    

          if (millis() - timeStamp > blinkTime) {
                blinks++;
                timeStamp = millis();
                digitalWrite(LG2, !digitalRead(LG2));
           }

          if (blinks > 7) {
            blinks = 0;
            timeStamp = millis();
            trafficLightState = 7;
          }
         break;
        case 7 :
          digitalWrite(LG2, LOW);
          digitalWrite(LY2, HIGH);
         if (millis() - timeStamp > yellowTime1) {
            trafficLightState = 8;
            timeStamp = millis();
          }
         break;
       case 8 :  //Red2 On and wait to red1 off
        digitalWrite(LY2, LOW);
        digitalWrite(LR2, HIGH);
        digitalWrite(LR1, LOW);
        digitalWrite(LY1, HIGH);
        if (millis() - timeStamp > yellowTime1 + redTime1) {
          trafficLightState = 0;
          timeStamp = millis();            
        }
        break;
  }
}

void setPedestrianPulser() {
   vP1 = digitalRead(P1);
   if (!p1IsCrossing) {
      if (vP1 == HIGH) {       
      waitinForGreenStatus = true;
      }
    }
    
    if (waitinForGreenStatus) {
     //Need to wait L1 to reach Green ON again -> status (0)
    if (trafficLightState == 0) {
           p1TimeStamp = millis();
           p1IsCrossing = true;
           greenTime1 = PEDESTRIAN_CROSS_TIME;  //From original redTime1 to PEDESTRIAN_CROSS_TIME
           waitinForGreenStatus = false;
      }
      }

    //After the time had been reach, we set the original time again

 if (p1IsCrossing && (millis() - p1TimeStamp > PEDESTRIAN_CROSS_TIME)) {
       p1IsCrossing = false;
       greenTime1 = ORIGINAL_GREEN1_TIME;
  }
}