#include <LiquidCrystal_I2C.h>

#define LR1 22   // Red traffic light 1 connected in pin 22
#define LY1 23   // Yellow traffic light 1 connected in pin 23
#define LG1 24   // Green traffic light 1 connected in pin 24
#define LR2 25   // Red traffic light 2 connected in pin 25
#define LY2 26   // Yellow traffic light 2 connected in pin 26
#define LG2 27   // Green traffic light 2 connected in pin 27
#define P1 37    //Crosswalk pulser
#define CNY2 34  // Infrared sensor 2 in traffic light 1 connected in pin 34 (middle)

//Middle level 
#define CNY1 35 // Infrared sensor 1 in traffic light 1 connected in pin 35
#define CNY3 33 // Infrared sensor 3 in traffic light 1 connected in pin 33
#define CNY4 32 // Infrared sensor 4 in traffic light 2 connected in pin 32
#define CNY5 31 // Infrared sensor 5 in traffic light 2 connected in pin 31
#define CNY6 30 // Infrared sensor 6 in traffic light 2 connected in pin 30
#define LDR1 A0 // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 A1 // LDR Light sensor from traffic light 2 connected in pin A1

//End middle level

//Constants
const int ORIGINAL_GREEN1_TIME = 500;
const int ORIGINAL_GREEN2_TIME = 2000;

const int LIGHT2_GREEN_ON=5;
const int LIGHT_NIGHT_MODE=9;
const int LIGHT2_INCREASE_WHEN3_SENSORS = 2000;
const int LIGHT2_INCREASE_WHEN2_SENSORS = 1000;
const int ALL_SENSORS_ACTIVE = 0;
const int TWO_SENSORS_ACTIVE = 1;

long timeStamp = 0;
int trafficLightState = 0;

long greenTime1 = ORIGINAL_GREEN1_TIME;
long yellowTime1 = 500;
long yellowTime2 = 500;
long greenTime2 = ORIGINAL_GREEN2_TIME;
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

//Middle level
int lightGreen1TimeWhen3Sensors = 4000;
//Additional time when 3 sensors are active.
int lightGreen2IncreaseWhenSensors = 2000;
//This original is used to allow change using external like Serial.read
const int originalTotalTimesWhenLight2Priority = 3;
int totalTimesWhenLight2Priority = originalTotalTimesWhenLight2Priority;

int vCNY4 = 0;
int vCNY5 = 0;
int vCNY6 = 0;
int vLDR1 = 0;
int vLDR2 = 0;

bool light2WaitingForGreenStatus = false;  //When the P1 = HIGH, need to wait until light is green again.
long light2PriorityTimeSTamp = 0;
bool light2IsPriority = false;
int lastLight2TotalSensors = 0;
//This flag indicate when something want or is acting on the lights like waitihg for green on
//pedestrian crossing, waiting for green 2 on, light2 is priority, etc to avoid activating night mode ur others
bool isExternalRequestingLights = false;
const int lightSensorsCheckTime = 100; //Check the night sensor LDR every x time
const int lightSensorTimeStamp = 0; //The timer after the last ligh (night) sensor check

bool nightMode = false;

LiquidCrystal_I2C display(0x27, 20, 4);

void setup() {

  // Output pin config
  pinMode(LR1, OUTPUT);  // Red traffic light 1 as Output
  pinMode(LY1, OUTPUT);  // Yellow traffic light 1 as Output
  pinMode(LG1, OUTPUT);  // Green traffic light 1 as Output
  pinMode(LR2, OUTPUT);  // Red traffic light 2 as Output
  pinMode(LY2, OUTPUT);  // Yellow traffic light 2 as Output
  pinMode(LG2, OUTPUT);  // Green traffic light 2 as Output
  pinMode(P1, INPUT);    // Pedestrian Pulser 1

  pinMode(CNY1, INPUT); 
  pinMode(CNY2, INPUT); 
  pinMode(CNY3, INPUT); 
  pinMode(CNY4, INPUT); 
  pinMode(CNY5, INPUT); 
  pinMode(CNY6, INPUT); 
  pinMode(LDR1, INPUT); 
  pinMode(LDR2, INPUT); 
    
  //display.init();
  //display.backlight();
  timeStamp = millis();
  Serial.begin(9600);
  lightSensorTimeStamp = millis();
}

void loop() {
  trafficLightFSM();
  setPedestrianPulser();
  checkForCNYYLigh2Sensor();
  checkForLigh2ActiveSensors();
  if (millis() - lightSensorTimeStamp > lightSensorsCheckTime){
      checkNighMode();
      lightSensorTimeStamp = millis();
  }
  
}

void checkNighMode(){
  //If nothing is trying to request alter over the lights, we can change to night mode
  if (!isExternalRequestingLights){
    vLDR2 = digitalRead(LDR2);
    vLDR1 = digitalRead(LDR1);

    //Enter night mode, check both sensors to resilience and avoid mistakes
    if(vLDR2 < 100 && vLDR1 < 100){
      nightMode = true;
    }else{
      nightMode = false;
    }
  }  
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
      isExternalRequestingLights = true;
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
       isExternalRequestingLights = false;
  }
}

/*
Middle: Detect the total of infrared on the light 2 are active.
CNY4, CNY5, CNY6
Then give priority with higher time for several times for this light
*/
void checkForLigh2ActiveSensors() {  

  vCNY4 = digitalRead(CNY4);
  vCNY5 = digitalRead(CNY5);
  vCNY6 = digitalRead(CNY6);

  // When the infrared sensor detects 0, it means a car is present.
  // totalSensors = 0 (3 sensors active), totalSensors = 1 (2 sensors active).
  int totalSensors = vCNY4 + vCNY5 + vCNY6;

  //3 active sensors
  if ((totalSensors == ALL_SENSORS_ACTIVE ||  totalSensors == TWO_SENSORS_ACTIVE) && !light2IsPriority && !light2WaitingForGreenStatus) {      
      //Active this flag until the light2 green is active again
      light2WaitingForGreenStatus = true; 
      //Save the total sensors to then adjust the total time.   
      lastLight2TotalSensors = totalSensors;
      isExternalRequestingLights = true;
  }

  if (light2WaitingForGreenStatus) {
    //Wait until traffi2 is on green (state = LIGHT2_GREEN_ON)
    if (trafficLightState == LIGHT2_GREEN_ON) {
      light2PriorityTimeSTamp = millis();
      light2IsPriority = true;
      light2WaitingForGreenStatus = false;
      greenTime2 += lightGreen2IncreaseWhenSensors;      
    }    
  }

  //After the light 2 reach the green again decrease the total times
 if (light2IsPriority && (uint32_t)(millis() - light2PriorityTimeSTamp) > greenTime2) {
      //To ensure greenTime2 increase several times.
      
      totalTimesWhenLight2Priority--;
      light2PriorityTimeSTamp = millis();
  }

  if (totalTimesWhenLight2Priority <=  && light2IsPriority){
    totalTimesWhenLight2Priority = originalTotalTimesWhenLight2Priority;
    light2IsPriority = false;
    isExternalRequestingLights = false;
  }
}


void trafficLightFSM() {

if (nightMode && trafficLightState != LIGHT_NIGHT_MODE){
  trafficLightState = LIGHT_NIGHT_MODE;
}

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
        case LIGHT2_GREEN_ON :  //Green2 on
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
        case LIGHT_NIGHT_MODE:
          digitalWrite(LG2, HIGH);
          digitalWrite(LY2, LOW);
          digitalWrite(LR2, LOW);

          digitalWrite(LG1, LOW);
          digitalWrite(LY1, LOW);
          digitalWrite(LR1, HIGH);
  }
}

void setPedestrianPulser() {
   vP1 = digitalRead(P1);
   if (!p1IsCrossing) {
      if (vP1 == HIGH) {       
        nightMode = false;
        waitinForGreenStatus = true;
        isExternalRequestingLights = true;
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
       isExternalRequestingLights = false;
  }
}