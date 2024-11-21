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
#define CNY1 35  // Infrared sensor 1 in traffic light 1 connected in pin 35
#define CNY3 33  // Infrared sensor 3 in traffic light 1 connected in pin 33
#define CNY4 32  // Infrared sensor 4 in traffic light 2 connected in pin 32
#define CNY5 31  // Infrared sensor 5 in traffic light 2 connected in pin 31
#define CNY6 30  // Infrared sensor 6 in traffic light 2 connected in pin 30
#define LDR1 A0  // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 A1  // LDR Light sensor from traffic light 2 connected in pin A1
#define CO2 A2  // CO2 sensor connected in pin A3

//End middle level

//Constants

// States of the finite state machine (FSM)
const int STATE_LIGHT1_GREEN_ON_START = 0;  // Traffic light 1 green light on at start
const int STATE_LIGHT1_GREEN_BLINK = 1;     // Traffic light 1 green light blinking
const int STATE_LIGHT1_YELLOW_ON = 2;       // Traffic light 1 yellow light on
const int STATE_LIGHT_RED1_ON = 3;          // Traffic light 1 red light on
const int STATE_LIGHT2_GREEN_ON = 4;        // Traffic light 2 green light on
const int STATE_LIGHT2_GREEN_BLINK = 5;     // Traffic light 2 green light blinking
const int STATE_LIGHT2_YELLOW_ON = 6;       // Traffic light 2 yellow light on
const int STATE_LIGHT_NIGHT_MODE = 7;       // Night mode where light 2 has the priority

const int LIGHT2_INCREASE_WHEN3_SENSORS = 2000;
const int LIGHT2_INCREASE_WHEN2_SENSORS = 1000;
const int ALL_SENSORS_ACTIVE = 3;
const int TWO_SENSORS_ACTIVE = 2;
const int LAST_SENSOR_TIME_CHECKER = 10;

int originalGreen2Time = 2000;  //No as a contast to be able to change using serial
int originalGreen1Time =2000;

unsigned long timeStamp = 0;
int state = 0;

long greenTime1 = originalGreen1Time;
long yellowTime = 500;
long greenTime2 = originalGreen2Time;
int blinks = 0;
long blinkTime = 100;
long totalBlinksInOut = 6;
//Pedestrian 1 pulser control
const int PEDESTRIAN_CROSS_TIME = 10000;
int vP1 = 0;
int vP2 = 0;
bool p1IsCrossing = false;
unsigned long p1TimeStamp = 0;
bool pedestrian1WaitingForRedStatus = false;  //When the P1 = HIGH, need to wait until light is green again.
unsigned int pedestrianReduceGreenTime1 = 3000;

//Infrared CNY2, when detect some car it increase the greentime, must that this is too short
bool ligh1DetectCar = false;
int greenLight1TimeWhenCar = 3000;
bool ligh1CarIsCrossing = true;
bool CNY2WaitinForGreenStatus = false;  //When the P1 = HIGH, need to wait until light is green again.
int previousState = -1;                 // To ligh1 previous state
int previousStateLight2 = -1;                 // To ligh2 previous state

//Middle level
int lightGreen1TimeWhen3Sensors = 4000;
//Additional time when 3 sensors are active.
int lightGreen2IncreaseWhenSensors = 2000;
//This original is used to allow change using external like Serial.read
const int originalTotalTimesWhenLight2Priority = 3;  //Total times the light is set with the priority time.
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
long unsigned lastPriorityTimeOnLight2;            //Help to check the last time on light2 to wait for some time to give some priority to it.
long unsigned priorityWaitingTimeOnLight2 = 7000;  //need to wait x ms until next priority checker

//This flag indicate when something want or is acting on the lights like waitihg for green on
//pedestrian crossing, waiting for green 2 on, light2 is priority, etc to avoid activating night mode ur others
bool isExternalRequestingLights = false;
const int lightSensorsCheckTime = 2000;  //Check the night sensor LDR every x time
unsigned long lightSensorTimeStamp = 0;  //The timer after the last ligh (night) sensor check
unsigned long nightTimeStampCheck;
unsigned long lastSensorTimeChecked;  //To verify the sensors every x time
unsigned long lastSensorTimeChecked2;  //To verify the sensors every x time
bool nightMode = false;

//### Sensors on Light2 ###
int vCNY1 = 0;
int vCNY2 = 0;
int vCNY3 = 0;

bool light1IsPriority = false;
bool light1WaitingForGreenStatus = false;
int lastLight1TotalSensors = 0;
const int originalTotalTimesWhenLight1Priority = 3;  // Total cycles with increased green time
int totalTimesWhenLight1Priority = originalTotalTimesWhenLight1Priority;

long unsigned lastPriorityTimeOnLight1 = 0;        // Last time Light 1 had priority
long unsigned priorityWaitingTimeOnLight1 = 3000;  // Time to wait before giving priority again
long unsigned light1PriorityTimeStamp = 0;

int lightGreen1IncreaseWhenSensors = 2000;  // Additional green time when sensors are active

//#### End sensors light 2 ###

int unsigned originalDisplayRefreshTime = 1000;
int unsigned displayRefreshTime = originalDisplayRefreshTime;
int unsigned displayRefreshTimeStamp ;
int unsigned displayRefreshTimeAfterNotification = 4000; //Time to wait until a new notification was show

LiquidCrystal_I2C display(0x27, 20, 4);

//CO2
int vCO2 = 0;
const float DC_GAIN = 8.5;                                                               // define the DC gain of amplifier CO2 sensor
const float ZERO_POINT_VOLTAGE = 0.265;                                                  // define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float REACTION_VOLTAGE = 0.059;                                                    // define the “voltage drop” of the sensor when move the sensor from air into 1000ppm CO2
const float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))}; // Line curve with 2 points

float volts = 0;  // Variable to store current voltage from CO2 sensor
float co2 = 0;    // Variable to store CO2 value
float dCO2 = 0;
//End CO2

void setup() {
  long unsigned currTime = millis();
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
  pinMode(CO2, INPUT);

  display.init();
  display.backlight();
  timeStamp = currTime;
  Serial.begin(9600);
  lightSensorTimeStamp = currTime;
  light2IsPriority = false;
  nightMode = false;
  lastPriorityTimeOnLight2 = 0;
  lastSensorTimeChecked = currTime;
  displayRefreshTimeStamp = currTime;
}

void loop() {
  trafficLightFSM();
  setPedestrian1Pulser();
  checkForLigh1ActiveSensors();
  checkForLigh2ActiveSensors();
  checkNighMode();
  readSerial();
  showData();
}

void showData(){
  int timeMark = millis();
  if (timeMark - displayRefreshTimeStamp > displayRefreshTime){
    displayRefreshTimeStamp = timeMark;
    //Restablish the time to the original in case a notification was sended
    displayRefreshTime = originalDisplayRefreshTime;
    showDataInDisplay();
  }
}

void readSerial() {
  static String received = "";

  while (Serial.available() > 0) {
    char ch = Serial.read();  //Serial just allow us to read one char
    if (ch == '\n') {
      int separatorIndex = received.indexOf(':');
      if (separatorIndex != -1) {
        String idStr = received.substring(0, separatorIndex);      //key
        String valueStr = received.substring(separatorIndex + 1);  // value
        int value = valueStr.toInt();

        // Compare the keys and assign the values
        if (idStr.equalsIgnoreCase("greenTime1")) {
          greenTime1 = value;
          originalGreen1Time = greenTime1;  //useful when we change the greenTime for some request and then assign the same value
        } else if (idStr.equalsIgnoreCase("greenTime2")) {
          greenTime2 = value;
          originalGreen2Time = greenTime2;
        } else if (idStr.equalsIgnoreCase("yellowTime")) {
          yellowTime = value;
        }
        // Show the value
        display.setCursor(0, 3);
        display.print("                    ");  // Clean the display
        display.setCursor(0, 3);
        display.print(idStr + ":" + value);
        notificate();
        received = "";  // Clean the buffer to receive again a new word
      } else {
        //Invalid format
        received = "";
      }
    } else {
      received += ch;  // Save values on the tmp buffer
    }
  }
}

/*
After each notification on the display, we wait this time
*/
void notificate(){
  displayRefreshTimeStamp = millis();  
  displayRefreshTime = displayRefreshTimeAfterNotification;
}

void checkNighMode() {
  nightTimeStampCheck = millis();
  if ((nightTimeStampCheck - lightSensorTimeStamp > lightSensorsCheckTime) && !isExternalRequestingLights) {
    lightSensorTimeStamp = nightTimeStampCheck;

    //If nothing is trying to request alter over the lights, we can change to night mode
    vLDR2 = analogRead(LDR2);
    vLDR1 = analogRead(LDR1);
    //Enter night mode, check both sensors to resilience and avoid mistakes
    if (vLDR2 < 60 && vLDR1 < 60) {
      nightMode = true;
    } else {
      nightMode = false;
    }
  }
}

void checkForLigh1ActiveSensors() {
  unsigned long currTime = millis();

  if (currTime - lastSensorTimeChecked > LAST_SENSOR_TIME_CHECKER && !isExternalRequestingLights) {
    lastSensorTimeChecked = currTime;

    vCNY1 = digitalRead(CNY1);
    vCNY2 = digitalRead(CNY2);
    vCNY3 = digitalRead(CNY3);

    // Calculate the total number of active sensors (0 = active, 1 = inactive)
    int totalSensors = 3 - (vCNY1 + vCNY2 + vCNY3);

    // Check if 2 or 3 sensors are active and priority is not already set
    if (totalSensors > 0 && !light1IsPriority && !light1WaitingForGreenStatus && currTime - lastPriorityTimeOnLight1 > priorityWaitingTimeOnLight1) {

      nightMode = false;
      light1WaitingForGreenStatus = true;
      lastLight1TotalSensors = totalSensors;
      isExternalRequestingLights = true;
      totalTimesWhenLight1Priority = totalSensors;
    }
  }

  if (light1WaitingForGreenStatus) {
    // Wait until Light 1 turns green
    if (state == STATE_LIGHT1_GREEN_ON_START) {
      light1PriorityTimeStamp = currTime;
      light1IsPriority = true;
      light1WaitingForGreenStatus = false;
      originalGreen1Time = greenTime1;
      greenTime1 += lightGreen1IncreaseWhenSensors;

      display.setCursor(0, 1);
      display.print("Prio ");
      display.print(lastLight1TotalSensors);
      display.print(" Sensors-C:");
      display.print(totalTimesWhenLight1Priority);
      notificate();
    }
  }

  // Decrease the priority cycle count after the green phase
  //  if (!p1IsCrossing && light1IsPriority && ((currTime - light1PriorityTimeStamp) > (greenTime1 + yellowTime * 2 + greenTime1 + (blinkTime * totalBlinksInOut)))) {
  if (light1IsPriority && previousState == STATE_LIGHT1_GREEN_ON_START && state != STATE_LIGHT1_GREEN_ON_START) {
    totalTimesWhenLight1Priority--;
    if (totalTimesWhenLight1Priority <= 0) {
      // Reset priority flags and times
      totalTimesWhenLight1Priority = originalTotalTimesWhenLight1Priority;
      light1IsPriority = false;
      isExternalRequestingLights = false;
      lastPriorityTimeOnLight1 = currTime;
      greenTime1 = originalGreen1Time;
      //display.setCursor(0, 1); todo: check if needed with the new refresh
      //display.print("                    ");  // Clear the display line      
    }
  }
  previousState = state;
}

void checkForLigh2ActiveSensors() {

  unsigned long currTime = millis();

  if (currTime - lastSensorTimeChecked > LAST_SENSOR_TIME_CHECKER && !isExternalRequestingLights) {
    lastSensorTimeChecked = currTime;
    vCNY4 = digitalRead(CNY4);
    vCNY5 = digitalRead(CNY5);
    vCNY6 = digitalRead(CNY6);

    int totalSensors = 3 - (vCNY4 + vCNY5 + vCNY6);
    

    if (totalSensors > 0 && !light2IsPriority && !light2WaitingForGreenStatus
        && currTime - lastPriorityTimeOnLight2 > priorityWaitingTimeOnLight2) {

      light2WaitingForGreenStatus = true;
      lastLight2TotalSensors = totalSensors;
      isExternalRequestingLights = true;
      totalTimesWhenLight2Priority = totalSensors;
    }
  }

  if (light2WaitingForGreenStatus) {
    if (state == STATE_LIGHT2_GREEN_ON) {
      light2PriorityTimeSTamp = currTime;
      light2IsPriority = true;
      light2WaitingForGreenStatus = false;
      originalGreen2Time = greenTime2;
      greenTime2 = greenTime2 + lightGreen2IncreaseWhenSensors;
      display.setCursor(0, 1);
      display.print("Prio ");
      display.print(lastLight2TotalSensors);
      display.print(" Sensors-C:");
      display.print(totalTimesWhenLight2Priority);
      notificate();
    }
  }
  //Check if the light change
  bool checkStateChange = previousStateLight2 == STATE_LIGHT2_GREEN_ON && state != STATE_LIGHT2_GREEN_ON;
  if (light2IsPriority && checkStateChange) {
    totalTimesWhenLight2Priority--;

    if (totalTimesWhenLight2Priority <= 0) {
      totalTimesWhenLight2Priority = originalTotalTimesWhenLight2Priority;
      light2IsPriority = false;
      isExternalRequestingLights = false;
      lastPriorityTimeOnLight2 = currTime;
      greenTime2 = originalGreen2Time;
      //display.setCursor(0, 1); todo: check if needed
      //display.print("                  ");
    }
  }
  previousStateLight2 = state;//Update the previous state with current
}

void setPedestrian1Pulser() {

  vP1 = digitalRead(P1);
  if (!p1IsCrossing) { //Not need to add isExternalRequestingLights because we want to switch fast to this when requested.
    if (vP1 == HIGH) {
      pedestrian1WaitingForRedStatus = true;
      isExternalRequestingLights = true;

      //remaining time
      unsigned long remainingGreenTime1 = greenTime1 - (millis() - timeStamp);
      if (remainingGreenTime1 > pedestrianReduceGreenTime1) {
        //Decrease the time for the pedestrian to cross
        greenTime1 = pedestrianReduceGreenTime1;
      }
    }
  }

  if (pedestrian1WaitingForRedStatus && state == STATE_LIGHT1_GREEN_ON_START) {
    //Need to wait L1 to reach Red ON again -> status (0)
    p1TimeStamp = millis();
    p1IsCrossing = true;
    originalGreen2Time = greenTime2;
    greenTime2 = greenTime2 + PEDESTRIAN_CROSS_TIME;  //Increase the the other Light Green
    pedestrian1WaitingForRedStatus = false;
    display.setCursor(0, 0);
    display.print("Pedestrian1 crossing");
    notificate();
  }

  //After the time had been reach, we set the original time again
  if (p1IsCrossing && (millis() - p1TimeStamp > (greenTime2 + blinkTime * blinks + yellowTime))) {
    p1IsCrossing = false;
    greenTime2 = originalGreen2Time;
    //Set the greentime1 to the original value after the pedestrian has crossed
    greenTime1 = originalGreen1Time;

    isExternalRequestingLights = false;
    //display.setCursor(0, 0); todo: check if needed
    //display.print("                    ");
  }
}

void readAllData()
{
  vLDR1 = analogRead(LDR1);
  vLDR2 = analogRead(LDR2);
  vCO2 = analogRead(CO2);
  dCO2 = 0;
  volts = analogRead(CO2) * 5.0 / 1023.0;
  if (volts / DC_GAIN >= ZERO_POINT_VOLTAGE) {
    dCO2 = 400;
  } else {
    dCO2 = pow(10, ((volts / DC_GAIN) - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
  }
  
  vCNY1 = digitalRead(CNY1);
  vCNY2 = digitalRead(CNY2);
  vCNY3 = digitalRead(CNY3);
  vCNY4 = digitalRead(CNY4);
  vCNY5 = digitalRead(CNY5);
  vCNY6 = digitalRead(CNY6);
}
 
void showDataInDisplay() {
  readAllData();
  display.setCursor(0, 0);
  display.print("LDR1       CNY 1 2 3");
  display.setCursor(0, 1);
  display.print("LDR2               ");
  display.setCursor(0, 2);
  display.print("CO2        CNY 4 5 6");
  display.setCursor(0, 3);
  display.print("L1   L2            ");
  display.setCursor(5, 0); display.print(vLDR1);
  display.setCursor(5, 1); display.print(vLDR2);
  display.setCursor(5, 2); display.print(vCO2);
  showGreenTimes();
  display.setCursor(15, 1); display.print(1 * vCNY1);
  display.setCursor(17, 1); display.print(1 * vCNY2);
  display.setCursor(19, 1); display.print(1 * vCNY3);
  display.setCursor(15, 3); display.print(1 * vCNY4);
  display.setCursor(17, 3); display.print(1 * vCNY5);
  display.setCursor(19, 3); display.print(1 * vCNY6);
}

void showGreenTimes(){
  
  unsigned long remainingGreenTime1 = (((greenTime1 + (blinkTime * blinks)) - (millis() - timeStamp)) + 999)/1000; //Sum 999 to round up
  unsigned long remainingGreenTime2 = (((greenTime2 + (blinkTime * blinks)) - (millis() - timeStamp)) + 999)/1000;
  if (remainingGreenTime1 < 0 || remainingGreenTime1 > 999 ) remainingGreenTime1 = 0; // to avoid negative val
  if (remainingGreenTime2 < 0 || remainingGreenTime2 > 999) remainingGreenTime2 = 0; 

  if (state == STATE_LIGHT1_GREEN_ON_START || state == STATE_LIGHT1_GREEN_BLINK){
      display.setCursor(3, 3); display.print(remainingGreenTime1);
  }else{
    display.setCursor(3, 3); display.print("  ");
  }

  if (state == STATE_LIGHT2_GREEN_ON || state == STATE_LIGHT2_GREEN_BLINK){
      display.setCursor(8, 3); display.print(remainingGreenTime2);
  }else{
    display.setCursor(8, 3); display.print("  ");
  }

}
 

void trafficLightFSM() {
  switch (state) {
    case STATE_LIGHT1_GREEN_ON_START:
      // Turn on green light for traffic light 1 and red for traffic light 2
      digitalWrite(LG1, HIGH);
      digitalWrite(LR2, HIGH);
      digitalWrite(LY2, LOW);
      digitalWrite(LR1, LOW);
      digitalWrite(LY1, LOW);
      digitalWrite(LG2, LOW);

      // Transition to blinking green light after greenTime1
      if (millis() - timeStamp >= greenTime1) {
        state = STATE_LIGHT1_GREEN_BLINK;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT1_GREEN_BLINK:
      // Blink green light for traffic light 1
      if (millis() - timeStamp > blinkTime) {
        digitalWrite(LG1, !digitalRead(LG1));
        blinks++;
        timeStamp = millis();
      }

      // Transition to yellow light after total blinks
      if (blinks > totalBlinksInOut) {
        blinks = 0;
        state = STATE_LIGHT1_YELLOW_ON;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT1_YELLOW_ON:
      // Turn on yellow light for traffic light 1
      digitalWrite(LY1, HIGH);
      digitalWrite(LG1, LOW);

      // Transition to red light for traffic light 1
      if (millis() - timeStamp > yellowTime) {
        state = STATE_LIGHT_RED1_ON;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT_RED1_ON:
      // Turn on red light for traffic light 1 and yellow for traffic light 2
      digitalWrite(LR1, HIGH);
      digitalWrite(LY1, LOW);
      digitalWrite(LY2, HIGH);
      digitalWrite(LG2, LOW);
      digitalWrite(LR2, LOW);

      // Transition to green light for traffic light 2
      if (millis() - timeStamp > yellowTime) {
        state = STATE_LIGHT2_GREEN_ON;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT2_GREEN_ON:
      // Turn on green light for traffic light 2
      digitalWrite(LY2, LOW);
      digitalWrite(LG2, HIGH);

      // Transition to blinking green light or night mode
      if (millis() - timeStamp > greenTime2) {
        state = nightMode ? STATE_LIGHT_NIGHT_MODE : STATE_LIGHT2_GREEN_BLINK;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT2_GREEN_BLINK:
      // Blink green light for traffic light 2
      if (millis() - timeStamp > blinkTime) {
        blinks++;
        timeStamp = millis();
        digitalWrite(LG2, !digitalRead(LG2));
      }

      // Transition to yellow light after total blinks
      if (blinks > totalBlinksInOut) {
        blinks = 0;
        state = STATE_LIGHT2_YELLOW_ON;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT2_YELLOW_ON:
      // Turn on yellow light for traffic light 2
      digitalWrite(LG2, LOW);
      digitalWrite(LY2, HIGH);

      // Transition directly to the start of the cycle
      if (millis() - timeStamp > yellowTime) {
        state = STATE_LIGHT1_GREEN_ON_START;
        timeStamp = millis();
      }
      break;

    case STATE_LIGHT_NIGHT_MODE:
      // Alternate blinking for night mode
      digitalWrite(LG2, HIGH);
      digitalWrite(LR1, HIGH);

      if (!nightMode) {
        state = STATE_LIGHT2_GREEN_ON; // Return to normal operation
      }
      break;
  }
}


