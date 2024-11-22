#include <LiquidCrystal_I2C.h>

#define LR1 22   // Red traffic light 1 connected in pin 22
#define LY1 23   // Yellow traffic light 1 connected in pin 23
#define LG1 24   // Green traffic light 1 connected in pin 24
#define LR2 25   // Red traffic light 2 connected in pin 25
#define LY2 26   // Yellow traffic light 2 connected in pin 26
#define LG2 27   // Green traffic light 2 connected in pin 27
#define P1 37    //Crosswalk pulser L1
#define P2 36    //Crosswalk pulser L2
#define CNY2 34  // Infrared sensor 2 in traffic light 1 connected in pin 34 (middle)

//Middle level
#define CNY1 35  // Infrared sensor 1 in traffic light 1 connected in pin 35
#define CNY3 33  // Infrared sensor 3 in traffic light 1 connected in pin 33
#define CNY4 32  // Infrared sensor 4 in traffic light 2 connected in pin 32
#define CNY5 31  // Infrared sensor 5 in traffic light 2 connected in pin 31
#define CNY6 30  // Infrared sensor 6 in traffic light 2 connected in pin 30
#define LDR1 A0  // LDR Light sensor from traffic light 1 connected in pin A0
#define LDR2 A1  // LDR Light sensor from traffic light 2 connected in pin A1
#define CO2 A2   // CO2 sensor connected in pin A3

//End middle level

//Constants
const char* ID = "id23";  //ID to identify the device TODO: Change before deploy

// States of the finite state machine (FSM)
const int STATE_LIGHT1_GREEN_ON_START = 0;  // Traffic light 1 green light on at start and light 2 red on
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

int originalGreen2Time = 4000;  //No as a contast to be able to change using serial
int originalGreen1Time = 2000;

unsigned long timeStamp = 0;
int state = 0;

long greenTime1 = originalGreen1Time;
long yellowTime = 500;
long greenTime2 = originalGreen2Time;
int blinks = 0;
long blinkTime = 100;
long totalBlinksInOut = 6;

//Pedestrian 1 pulser control
unsigned long pedestrian1CrossTime = 10000;
int vP1 = 0;
bool p1IsCrossing = false;
unsigned long p1TimeStamp = 0;
bool pedestrian1WaitingForPriority = false;  //When the P1 = HIGH, need to wait until light is red again.
unsigned int originalPedestrianReduceGreenTime1 = 3000; 
unsigned int pedestrianReduceGreenTime1 = originalPedestrianReduceGreenTime1; //Reduce the green time when pedestrian is crossing
int previousStatePedestrian1 = -1;           // Previous state to verify when a complete cycle was completed
unsigned long lastGreenTime1 = greenTime1; 

//Pedestrian 2 pulser control
unsigned long pedestrian2CrossTime = 4000;
int vP2 = 0;
bool p2IsCrossing = false;
unsigned long p2TimeStamp = 0;
bool pedestrian2WaitingForPriority = false;  //When the P2 = HIGH, need to wait until light is red again.
unsigned long originalPedestrianReduceGreenTime2 = 5000; 
unsigned long pedestrianReduceGreenTime2 = originalPedestrianReduceGreenTime2; //Reduce the green time when pedestrian is crossing
int previousStatePedestrian2 = -1;           // Previous state to verify when a complete cycle was completed
unsigned long lastGreenTime2 = greenTime2; 

//Infrared CNY2, when detect some car it increase the greentime, must that this is too short
bool ligh1DetectCar = false;
int greenLight1TimeWhenCar = 3000;
bool ligh1CarIsCrossing = true;
int previousState = -1;                 // To ligh1 previous state
int previousStateLight2 = -1;           // To ligh2 previous state

//Middle level
//Additional time when 3 sensors are active.
int lightGreen2IncreaseWhenSensors = 2000;
int totalTimesWhenLight2Priority = 3;  //This will be the amount of sensors

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
unsigned long lastSensorTimeChecked;   //To verify the sensors every x time
unsigned long lastSensorTimeChecked2;  //To verify the sensors every x time
bool nightMode = false;

//### Sensors on Light2 ###
int vCNY1 = 0;
int vCNY2 = 0;
int vCNY3 = 0;

bool light1IsPriority = false;
bool light1WaitingForGreenStatus = false;
int lastLight1TotalSensors = 0;
int totalTimesWhenLight1Priority = 3;

long unsigned lastPriorityTimeOnLight1 = 0;        // Last time Light 1 had priority
long unsigned priorityWaitingTimeOnLight1 = 3000;  // Time to wait before giving priority again
long unsigned light1PriorityTimeStamp = 0;

int lightGreen1IncreaseWhenSensors = 2000;  // Additional green time when sensors are active

//#### End sensors light 2 ###

int unsigned displayRefreshTime = 1000;
int unsigned displayRefreshTimeStamp;
int unsigned displayRefreshTimeAfterNotification = 4000;  //Time to wait until a new notification was show

LiquidCrystal_I2C display(0x27, 20, 4);

//CO2
int vCO2 = 0;
const float DC_GAIN = 8.5;                                                                  // define the DC gain of amplifier CO2 sensor
const float ZERO_POINT_VOLTAGE = 0.265;                                                     // define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float REACTION_VOLTAGE = 0.059;                                                       // define the “voltage drop” of the sensor when move the sensor from air into 1000ppm CO2
const float CO2Curve[3] = { 2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3)) };  // Line curve with 2 points

float volts = 0;  // Variable to store current voltage from CO2 sensor
float co2 = 0;    // Variable to store CO2 value
float dCO2 = 0;
unsigned long co2GreenTime2 = 20000;
//End CO2

//Sending to server
unsigned long timeToSendData = 1000*60;
unsigned long timeToSendDataTimeStamp = 0;
//

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
  pinMode(P2, INPUT);    // Pedestrian Pulser 1

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
  Serial.begin(115200);
  lightSensorTimeStamp = currTime;
  light2IsPriority = false;
  nightMode = false;
  lastPriorityTimeOnLight2 = 0;
  lastSensorTimeChecked = currTime;
  displayRefreshTimeStamp = currTime;
  timeToSendDataTimeStamp = currTime;
}

void loop() {
  trafficLightFSM();
  setPedestrian1Pulser();
  checkForLigh1ActiveSensors();
  checkForLigh2ActiveSensors();
  checkNighMode();
  readSerial();
  showData();
  sendDataToServer();
}

void sendDataToServer() {
  unsigned long currTime = millis();
  if (currTime - timeToSendDataTimeStamp > timeToSendData) {
    timeToSendDataTimeStamp = currTime;  
    
    sendVariableToSerial("greenTime1", greenTime1);
    sendVariableToSerial("greenTime2", greenTime2);
    sendVariableToSerial("yellowTime", yellowTime);
    sendVariableToSerial("blinkTime", blinkTime);
    sendVariableToSerial("totalBlinksInOut", totalBlinksInOut);
    sendVariableToSerial("pedestrianReduceGreenTime1", pedestrianReduceGreenTime1);
    sendVariableToSerial("lightGreen2IncreaseWhenSensors", lightGreen2IncreaseWhenSensors);
    sendVariableToSerial("priorityWaitingTimeOnLight2", priorityWaitingTimeOnLight2);
    sendVariableToSerial("priorityWaitingTimeOnLight1", priorityWaitingTimeOnLight1);
    sendVariableToSerial("lightGreen1IncreaseWhenSensors", lightGreen1IncreaseWhenSensors);
    sendVariableToSerial("displayRefreshTimeAfterNotification", displayRefreshTimeAfterNotification);
    sendVariableToSerial("greenLight1TimeWhenCar", greenLight1TimeWhenCar);
    sendVariableToSerial("pedestrian1CrossTime", pedestrian1CrossTime);
    sendVariableToSerial("co2GreenTime2", co2GreenTime2);
  }
}

void sendVariableToSerial(const char* key, int value) {
    char buffer[50]; // Ensure the buffer is large enough
    snprintf(buffer, sizeof(buffer), "%s_%s:%d", ID, key, value); // Use snprintf for safety
    Serial.println(buffer); // Directly send the char buffer
}

void sendNotificationToSerial(const char* value) {
    char buffer[50]; // Ensure the buffer is large enough
    snprintf(buffer, sizeof(buffer), "%s_%s", ID, value); // Use snprintf for safety
    Serial.println(buffer); // Directly send the char buffer
}

void showData() {
  int timeMark = millis();
  if (timeMark - displayRefreshTimeStamp > displayRefreshTime) {
    displayRefreshTimeStamp = timeMark;
    //Restablish the time to the original in case a notification was sended
    displayRefreshTime = 1000;
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
          originalGreen1Time = greenTime1;
        } else if (idStr.equalsIgnoreCase("greenTime2")) {
          greenTime2 = value;
          originalGreen2Time = greenTime2;
        } else if (idStr.equalsIgnoreCase("yellowTime")) {
          yellowTime = value;
        } else if (idStr.equalsIgnoreCase("blinkTime")) {
          blinkTime = value;
        } else if (idStr.equalsIgnoreCase("totalBlinksInOut")) {
          totalBlinksInOut = value;
        } else if (idStr.equalsIgnoreCase("pedestrianReduceGreenTime1")) {
          pedestrianReduceGreenTime1 = value;
        } else if (idStr.equalsIgnoreCase("lightGreen2IncreaseWhenSensors")) {
          lightGreen2IncreaseWhenSensors = value;
        } else if (idStr.equalsIgnoreCase("priorityWaitingTimeOnLight2")) {
          priorityWaitingTimeOnLight2 = value;
        } else if (idStr.equalsIgnoreCase("priorityWaitingTimeOnLight1")) {
          priorityWaitingTimeOnLight1 = value;
        } else if (idStr.equalsIgnoreCase("lightGreen1IncreaseWhenSensors")) {
          lightGreen1IncreaseWhenSensors = value;
        } else if (idStr.equalsIgnoreCase("displayRefreshTimeAfterNotification")) {
          displayRefreshTimeAfterNotification = value;
        } else if (idStr.equalsIgnoreCase("greenLight1TimeWhenCar")) {
          greenLight1TimeWhenCar = value;
        } else if (idStr.equalsIgnoreCase("pedestrian1CrossTime")) {
          pedestrian1CrossTime = value;          
        } else if (idStr.equalsIgnoreCase("pedestrian2CrossTime")) {
          pedestrian2CrossTime = value;
        }else if (idStr.equalsIgnoreCase("pedestrianReduceGreenTime1")) {
          pedestrianReduceGreenTime1 = value;
          originalPedestrianReduceGreenTime1 = value;
        }else if (idStr.equalsIgnoreCase("pedestrianReduceGreenTime2")) {
          pedestrianReduceGreenTime2 = value;
          originalPedestrianReduceGreenTime2 = value;
        }        
        else if (idStr.equalsIgnoreCase("co2GreenTime2")) {
          co2GreenTime2 = value;
        }
        // Show the value
        display.setCursor(0, 3);
        display.print("                    ");  // Clean the display
        display.setCursor(0, 3);
        display.print(idStr + ":" + value);
        setNotificationWaitingTime();
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
void setNotificationWaitingTime() {
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
      sendVariableToSerial("nightMode", 1);
    } else {      
      nightMode = false;
      sendVariableToSerial("nightMode", 0);
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
      //With this we can increase the green reduce time when the sensors are active
      pedestrianReduceGreenTime1 = pedestrianReduceGreenTime1 + lightGreen1IncreaseWhenSensors;
      sendNotificationToSerial("Semaforo 1 Preparando  prioridad");
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

      sendNotificationToSerial("Semaforo 1 con prioridad");
      display.setCursor(0, 1);
      display.print("Prio ");
      display.print(lastLight1TotalSensors);
      display.print(" Sensors-C:");
      display.print(totalTimesWhenLight1Priority);
      setNotificationWaitingTime();
    }
  }

  // Decrease the priority cycle count after the green phase
  if (light1IsPriority && previousState == STATE_LIGHT1_GREEN_ON_START && state != STATE_LIGHT1_GREEN_ON_START) {
    totalTimesWhenLight1Priority--;
    if (totalTimesWhenLight1Priority <= 0) {
      light1IsPriority = false;
      isExternalRequestingLights = false;
      lastPriorityTimeOnLight1 = currTime;
      greenTime1 = originalGreen1Time;
      pedestrianReduceGreenTime1 = originalPedestrianReduceGreenTime1;
     sendNotificationToSerial("Semaforo 1 fin prioridad");
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
      sendNotificationToSerial("Semaforo 2 Preparando prioridad");   
    }
  }

  if (light2WaitingForGreenStatus) {
    if (state == STATE_LIGHT2_GREEN_ON) {
      light2PriorityTimeSTamp = currTime;
      light2IsPriority = true;
      light2WaitingForGreenStatus = false;
      originalGreen2Time = greenTime2;
      greenTime2 = greenTime2 + lightGreen2IncreaseWhenSensors + getTimeForCO2Sensor();
      //With this we can increase the green reduce time when the sensors are active
      pedestrianReduceGreenTime2 = pedestrianReduceGreenTime2 + lightGreen2IncreaseWhenSensors + getTimeForCO2Sensor();
      sendNotificationToSerial("Semaforo 2 con prioridad");
      display.setCursor(0, 1);
      display.print("Prio ");
      display.print(lastLight2TotalSensors);
      display.print(" Sensors-C:");
      display.print(totalTimesWhenLight2Priority);
      setNotificationWaitingTime();
    }
  }
  //Check if the light change
  bool checkStateChange = previousStateLight2 == STATE_LIGHT2_GREEN_ON && state != STATE_LIGHT2_GREEN_ON;
  if (light2IsPriority && checkStateChange) {
    totalTimesWhenLight2Priority--;

    if (totalTimesWhenLight2Priority <= 0) {

      light2IsPriority = false;
      isExternalRequestingLights = false;
      lastPriorityTimeOnLight2 = currTime;
      greenTime2 = originalGreen2Time;
      pedestrianReduceGreenTime2 = originalPedestrianReduceGreenTime2;
      sendNotificationToSerial("Semaforo 2 fin prioridad");
    }
  }
  previousStateLight2 = state;  //Update the previous state with current
}

void setPedestrian1Pulser() {
  vP1 = digitalRead(P1);
  if (!p1IsCrossing && !p2IsCrossing) {  //Not need to add isExternalRequestingLights because we want to switch fast to this when requested.
    if (vP1 == HIGH) {
      pedestrian1WaitingForPriority = true;
      isExternalRequestingLights = true;
      //remaining time
      unsigned long remainingGreenTime1 = greenTime1 - (millis() - timeStamp);
      if (remainingGreenTime1 > pedestrianReduceGreenTime1) {
        //Save the current time for the green
        lastGreenTime1 = greenTime1;
        //Decrease the time for the pedestrian to cross
        greenTime1 = pedestrianReduceGreenTime1;
      }
    }
  }

  if (pedestrian1WaitingForPriority && state == STATE_LIGHT1_GREEN_ON_START) {
    //Need to wait L1 to reach Red ON again -> status (0)
    p1TimeStamp = millis();
    p1IsCrossing = true;
    originalGreen2Time = greenTime2;
    if (!light1IsPriority){
      greenTime2 = greenTime2 + pedestrian1CrossTime;  //Increase the other Green to have more RED in here
    }else{
      greenTime2 = greenTime2 + pedestrian1CrossTime/2;  //Increase just the half
    }
    sendNotificationToSerial("Peaton 1 cruzando");
    pedestrian1WaitingForPriority = false;
    display.setCursor(0, 0);   
    setNotificationWaitingTime();
  }

  //After the time had been reach, we set the original time again
  if (p1IsCrossing && (millis() - p1TimeStamp > (greenTime2 + blinkTime * blinks + yellowTime))) {
    p1IsCrossing = false;
    greenTime2 = originalGreen2Time;
    //Set the greentime1 to the original value after the pedestrian has crossed
    greenTime1 = lastGreenTime1;
    isExternalRequestingLights = false;
    sendNotificationToSerial("Gracias por cuidarte!");
  }
}

void setPedestrian2Pulser() {

  vP2 = digitalRead(P2);
  if (!p1IsCrossing && !p2IsCrossing) {  //Not need to add isExternalRequestingLights because we want to switch fast to this when requested.
    if (vP2 == HIGH) {
      pedestrian2WaitingForPriority = true;
      isExternalRequestingLights = true;
      //If the remaining time for the light 2 is greather than the pedestrianReduceGreenTime2, we reduce the waiting time.
      //TODO: Check the sensors and the CO2 emmiter. this for high level
      unsigned long remainingGreenTime2 = greenTime2 - (millis() - timeStamp);
      if (remainingGreenTime2 > pedestrianReduceGreenTime2) {       
        lastGreenTime2 = greenTime2;
        greenTime2 = pedestrianReduceGreenTime2;
      }
    }
  }
  //Wait until light 2 is on green on
  if (pedestrian2WaitingForPriority && state == STATE_LIGHT2_GREEN_ON) {    
    p2TimeStamp = millis();
    p2IsCrossing = true;
    originalGreen1Time = greenTime1; //Set this in case current state is for sensors activate
    
    if (!light2IsPriority){
      greenTime1 = greenTime1 + pedestrian2CrossTime;  //Increase the other Green to have more RED in here
    }
    pedestrian2WaitingForPriority = false; 
    display.setCursor(0, 0);
    display.print("Pedestrian2 crossing");    
    setNotificationWaitingTime();
    sendNotificationToSerial("Peaton 2 cruzando");
  }

  //After the time had been reach, we set the original time again
  if (p2IsCrossing && previousStatePedestrian2 == STATE_LIGHT1_GREEN_ON_START && state != STATE_LIGHT1_GREEN_ON_START) {
    p2IsCrossing = false;    
    greenTime2 = 
    //Set the greentime1 to the original value after the pedestrian has crossed
    greenTime1 = originalGreen1Time;
    isExternalRequestingLights = false;
    sendNotificationToSerial("Gracias por cuidarte!");
  }
  previousStatePedestrian2 = state;
}

void readAllData() {
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
  display.print("LDR1          CNY123");
  display.setCursor(0, 1);
  display.print("LDR2               ");
  display.setCursor(0, 2);
  display.print("CO2           CNY456");
  display.setCursor(0, 3);
  display.print("L1   L2            ");
  display.setCursor(5, 0);
  display.print(vLDR1);
  display.setCursor(5, 1);
  display.print(vLDR2);
  display.setCursor(4, 2);
  display.print(vCO2);
  display.setCursor(8, 2);
  display.print(getTimeForCO2Sensor());
  showGreenTimes();
  display.setCursor(17, 1);
  display.print(1 * vCNY1);
  display.setCursor(18, 1);
  display.print(1 * vCNY2);
  display.setCursor(19, 1);
  display.print(1 * vCNY3);
  display.setCursor(17, 3);
  display.print(1 * vCNY4);
  display.setCursor(18, 3);
  display.print(1 * vCNY5);
  display.setCursor(19, 3);
  display.print(1 * vCNY6);
}

void showGreenTimes() {

  // Calcula el tiempo total para cada greenTime
  unsigned long totalTime1 = greenTime1 + (blinkTime * blinks) + 999;
  unsigned long totalTime2 = greenTime2 + (blinkTime * blinks) + 999;

  // Calcula el tiempo transcurrido desde el inicio del período verde
  unsigned long elapsedTime = millis() - timeStamp;

  // Calcula el tiempo restante en milisegundos para cada greenTime
  long remainingTimeMillis1 = (long)totalTime1 - (long)elapsedTime;
  long remainingTimeMillis2 = (long)totalTime2 - (long)elapsedTime;

  // Asegúrate de que el tiempo restante no sea negativo
  if (remainingTimeMillis1 < 0) remainingTimeMillis1 = 0;
  if (remainingTimeMillis2 < 0) remainingTimeMillis2 = 0;

  // Redondea hacia arriba al segundo más cercano
  unsigned long remainingGreenTime1 = (remainingTimeMillis1) / 1000;
  unsigned long remainingGreenTime2 = (remainingTimeMillis2) / 1000;

  if (state == STATE_LIGHT1_GREEN_ON_START) {
    display.setCursor(3, 3);
    display.print(remainingGreenTime1);
  } else if (state == STATE_LIGHT1_GREEN_BLINK) {
    display.setCursor(3, 3);
    display.print("0");
  } else {
    display.setCursor(3, 3);
    display.print("  ");
  }

  if (state == STATE_LIGHT2_GREEN_ON) {
    display.setCursor(8, 3);
    display.print(remainingGreenTime2);
  } else if (state == STATE_LIGHT2_GREEN_BLINK) {
    display.setCursor(8, 3);
    display.print("0");
  } else {
    display.setCursor(8, 3);
    display.print("  ");
  }

  if (state == STATE_LIGHT_NIGHT_MODE) {
    display.setCursor(8, 3);
    display.print("NIGHT");
  }
}

int getTimeForCO2Sensor() {
  dCO2 = 0;
  volts = analogRead(CO2) * 5.0 / 1023.0;
  if (volts / DC_GAIN >= ZERO_POINT_VOLTAGE) {
    dCO2 = 400;
  } else {
    dCO2 = pow(10, ((volts / DC_GAIN) - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
  }

  float time = (co2GreenTime2 * dCO2 / 10000);
  return (int)(fmod(time, co2GreenTime2));
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
        sendVariableToSerial("state", state);
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
        sendVariableToSerial("state", state);;
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
        sendVariableToSerial("state", state);
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
        sendVariableToSerial("state", state);
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
        sendVariableToSerial("state", state);
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
        sendVariableToSerial("state", state);
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
        sendVariableToSerial("state", state);
      }
      break;

    case STATE_LIGHT_NIGHT_MODE:
      // Alternate blinking for night mode
      digitalWrite(LG2, HIGH);
      digitalWrite(LR1, HIGH);

      if (!nightMode) {
        state = STATE_LIGHT2_GREEN_ON;  // Return to normal operation
        sendVariableToSerial("state", state);
      }
      break;
  }
}
