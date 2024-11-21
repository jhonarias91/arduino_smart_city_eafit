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

//End middle level

//Constants
const int ORIGINAL_GREEN1_TIME = 500;
const int ORIGINAL_GREEN2_TIME = 2000;

const int STATE_LIGHT1_GREEN_ON_START = 0;
const int STATE_LIGHT2_GREEN_ON = 4;
const int STATE_LIGHT_NIGHT_MODE = 8;
const int STATE_LIGHT_RED1_ON = 3;

const int LIGHT2_INCREASE_WHEN3_SENSORS = 2000;
const int LIGHT2_INCREASE_WHEN2_SENSORS = 1000;
const int ALL_SENSORS_ACTIVE = 3;
const int TWO_SENSORS_ACTIVE = 2;

unsigned long timeStamp = 0;
int state = 0;

long greenTime1 = ORIGINAL_GREEN1_TIME;
long yellowTime = 500;
long greenTime2 = ORIGINAL_GREEN2_TIME;
long redTime1 = 2000;
int blinks = 0;
long blinkTime = 100;

//Pedestrian 1 pulser control
const int PEDESTRIAN_CROSS_TIME = 12000;
int vP1 = 0;
bool p1IsCrossing = false;
unsigned long p1TimeStamp = 0;
bool pedestrian1WaitingForRedStatus = false;  //When the P1 = HIGH, need to wait until light is green again.

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
long unsigned lastPriorityTimeOnLight2;            //Help to check the last time on light2 to wait for some time to give some priority to it.
long unsigned priorityWaitingTimeOnLight2 = 1000;  //need to wait x ms until next priority

//This flag indicate when something want or is acting on the lights like waitihg for green on
//pedestrian crossing, waiting for green 2 on, light2 is priority, etc to avoid activating night mode ur others
bool isExternalRequestingLights = false;
const int lightSensorsCheckTime = 2000;  //Check the night sensor LDR every x time
unsigned long lightSensorTimeStamp = 0;  //The timer after the last ligh (night) sensor check
unsigned long nightTimeStampCheck;

bool nightMode = false;

LiquidCrystal_I2C display(0x27, 20, 4);

//CO2
float co2GreenTime2 = 10;

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

  display.init();
  display.backlight();
  timeStamp = millis();
  Serial.begin(9600);
  lightSensorTimeStamp = millis();
  light2IsPriority = false;
  nightMode = false;
  lastPriorityTimeOnLight2 = 0;
}

void loop() {
  Serial.println(getTimeForCO2Sensor());
  trafficLightFSM();
  setPedestrian1Pulser();
  //checkForCNYYLigh2Sensor();
  checkForLigh2ActiveSensors();
  checkNighMode();
  readSerial();
}
void readSerial() {
  static String received = "";  

  while (Serial.available() > 0) {
    char ch = Serial.read();  
    if (ch == '\n') {         
      int separatorIndex = received.indexOf(':');
      if (separatorIndex != -1) {
        String idStr = received.substring(0, separatorIndex);      // propiedad
        String valueStr = received.substring(separatorIndex + 1); // valor
        int value = valueStr.toInt();

        // Compara las propiedades y asigna valores
        if (idStr.equalsIgnoreCase("greenTime1")) {
          greenTime1 = value;
        } else if (idStr.equalsIgnoreCase("greenTime2")) {
          greenTime2 = value;
        } else if (idStr.equalsIgnoreCase("yellowTime")) {
          yellowTime = value;
        } else if (idStr.equalsIgnoreCase("co2GreenTime2")) {
          co2GreenTime2 = value;
        }else {
          // Si el idStr no coincide con ninguna propiedad
          Serial.println("Propiedad no reconocida: " + idStr);
        }

        // Muestra el dato recibido en el display
        display.setCursor(0, 3);
        display.print("                    ");  // Limpia la línea del display
        display.setCursor(0, 3);
        display.print(idStr + ":" + value);

        received = "";  // Limpia el buffer después de procesar
      } else {
        Serial.println("Formato incorrecto: " + received);
        received = "";  // Limpia el buffer si no tiene el formato esperado
      }
    } else {
      received += ch;  // Acumula los caracteres en el buffer
    }
  }
}


void checkNighMode() {
  nightTimeStampCheck = millis();
  if ((nightTimeStampCheck - lightSensorTimeStamp > lightSensorsCheckTime) && !isExternalRequestingLights) {
    Serial.println("Green2Time:");
    Serial.println(greenTime2);
    lightSensorTimeStamp = nightTimeStampCheck;

    //If nothing is trying to request alter over the lights, we can change to night mode
    vLDR2 = analogRead(LDR2);
    vLDR1 = analogRead(LDR1);

    //Enter night mode, check both sensors to resilience and avoid mistakes
    if (vLDR2 < 100 && vLDR1 < 100) {
      nightMode = true;
    } else {
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
  //When the infrared receive a 0 is because there is a car there.
  if (!p1IsCrossing && vCNY2 == LOW && !CNY2WaitinForGreenStatus) {
    CNY2WaitinForGreenStatus = true;
    isExternalRequestingLights = true;
  }

  if (CNY2WaitinForGreenStatus) {
    if (state == 0) {
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
  // totalSensors = 0 (3 sensors active), = 1 (2 sensors active), = 3 (0 active).
  int totalSensors = 3 - (vCNY4 + vCNY5 + vCNY6);
  //3 active sensors
  if ((totalSensors == ALL_SENSORS_ACTIVE || totalSensors == TWO_SENSORS_ACTIVE) && !light2IsPriority && !light2WaitingForGreenStatus
      && millis() - lastPriorityTimeOnLight2 > priorityWaitingTimeOnLight2) {
    //Active this flag until the light2 green is active again
    light2WaitingForGreenStatus = true;
    //Save the total sensors to then adjust the total time.
    lastLight2TotalSensors = totalSensors;
    isExternalRequestingLights = true;
  }

  if (light2WaitingForGreenStatus) {
    //Wait until traffi2 is on green (state = STATE_LIGHT2_GREEN_ON)
    if (state == STATE_LIGHT2_GREEN_ON) {
      light2PriorityTimeSTamp = millis();
      light2IsPriority = true;
      light2WaitingForGreenStatus = false;
      //lectura de sensor de co2 validar rangos
      greenTime2 += lightGreen2IncreaseWhenSensors;
      display.setCursor(0, 1);
      display.print("Prio ");
      display.print(lastLight2TotalSensors);
      display.print(" Sensors-C:");
      display.print(totalTimesWhenLight2Priority);
    }
  }

  //After the light 2 reach the green again decrease the total times
  if (light2IsPriority && (uint32_t)(millis() - light2PriorityTimeSTamp) > greenTime2) {
    //To ensure greenTime2 be active on this mode several times.
    totalTimesWhenLight2Priority--;
    light2PriorityTimeSTamp = millis();
    display.setCursor(17, 1);
    //display.print(" ");
    display.print(totalTimesWhenLight2Priority);
  }

  if (totalTimesWhenLight2Priority <= 0 && light2IsPriority) {
    totalTimesWhenLight2Priority = originalTotalTimesWhenLight2Priority;
    light2IsPriority = false;
    isExternalRequestingLights = false;
    display.setCursor(0, 1);
    display.print("                  ");
    lastPriorityTimeOnLight2 = millis();
  }
}

void trafficLightFSM() {

  switch (state) {
    case STATE_LIGHT1_GREEN_ON_START:

      digitalWrite(LG1, HIGH);
      digitalWrite(LR2, HIGH);
      digitalWrite(LY2, LOW);
      digitalWrite(LR1, LOW);
      digitalWrite(LY1, LOW);
      digitalWrite(LG2, LOW);
      if (millis() - timeStamp >= greenTime1) {
        state = 1;
        timeStamp = millis();
      }
      break;

    case 1:  //green blink

      if (millis() - timeStamp > blinkTime) {
        digitalWrite(LG1, !digitalRead(LG1));
        blinks++;
        timeStamp = millis();
      }
      if (blinks > 7) {
        blinks = 0;
        state = 2;
        timeStamp = millis();
      }
      break;
    case 2:  //Yellow1 On
      digitalWrite(LY1, HIGH);
      digitalWrite(LG1, LOW);
      if (millis() - timeStamp > yellowTime) {
        state = STATE_LIGHT_RED1_ON;  //check for 3
        timeStamp = millis();
      }
      break;
    case STATE_LIGHT_RED1_ON:
      digitalWrite(LR1, HIGH);
      digitalWrite(LY1, LOW);
      digitalWrite(LG1, LOW);
      digitalWrite(LY2, HIGH);
      digitalWrite(LG2, LOW);
      digitalWrite(LR2, LOW);

      if (millis() - timeStamp > yellowTime) {
        state = STATE_LIGHT2_GREEN_ON;
        timeStamp = millis();
      }
      break;
    case STATE_LIGHT2_GREEN_ON:  //Green2 on
      digitalWrite(LY2, LOW);
      digitalWrite(LG2, HIGH);
      if (millis() - timeStamp > greenTime2) {
        if (nightMode) {
          state = STATE_LIGHT_NIGHT_MODE;  //Slowly move to nightMode
        } else {
          state = 5;
        }
        timeStamp = millis();
      }
      break;
    case 5:  //Blink Green2
      if (millis() - timeStamp > blinkTime) {
        blinks++;
        timeStamp = millis();
        digitalWrite(LG2, !digitalRead(LG2));
      }

      if (blinks > 7) {
        blinks = 0;
        timeStamp = millis();
        state = 6;
      }
      break;
    case 6:
      digitalWrite(LG2, LOW);
      digitalWrite(LY2, HIGH);
      if (millis() - timeStamp > yellowTime) {
        state = 7;
        timeStamp = millis();
      }
      break;
    case 7:  //Red2 On
      digitalWrite(LR2, HIGH);
      digitalWrite(LY1, HIGH);

      digitalWrite(LR1, LOW);
      digitalWrite(LY2, LOW);
      if (millis() - timeStamp > yellowTime) {
        state = STATE_LIGHT1_GREEN_ON_START;
        timeStamp = millis();
      }
      break;
    case STATE_LIGHT_NIGHT_MODE:
      digitalWrite(LG2, HIGH);
      digitalWrite(LY2, LOW);
      digitalWrite(LR2, LOW);

      digitalWrite(LG1, LOW);
      digitalWrite(LY1, LOW);
      digitalWrite(LR1, HIGH);
      if (!nightMode) {
        state = STATE_LIGHT2_GREEN_ON;  //Send to and state similar to this to avoid chrashes
      }
  }
}

void setPedestrian1Pulser() {

  vP1 = digitalRead(P1);
  if (!p1IsCrossing) {
    if (vP1 == HIGH) {
      nightMode = false;
      pedestrian1WaitingForRedStatus = true;
      isExternalRequestingLights = true;
    }
  }

  if (pedestrian1WaitingForRedStatus && state == STATE_LIGHT1_GREEN_ON_START) {
    //Need to wait L1 to reach Red ON again -> status (0)
    p1TimeStamp = millis();
    p1IsCrossing = true;
    greenTime2 = PEDESTRIAN_CROSS_TIME;  //Increase the the other Light Green
    pedestrian1WaitingForRedStatus = false;
    display.setCursor(0, 0);
    display.print("Pedestrian1 crossing");
  }

  //After the time had been reach, we set the original time again
  if (p1IsCrossing && (millis() - p1TimeStamp > PEDESTRIAN_CROSS_TIME)) {
    p1IsCrossing = false;
    greenTime2 = ORIGINAL_GREEN2_TIME;
    isExternalRequestingLights = false;
    display.setCursor(0, 0);
    display.print("                    ");
  }
}
#define CO2 A2  // CO2 sensor connected in pin A3
const float DC_GAIN = 8.5;                                                               // define the DC gain of amplifier CO2 sensor
const float ZERO_POINT_VOLTAGE = 0.265;                                                  // define the output of the sensor in volts when the concentration of CO2 is 400PPM
const float REACTION_VOLTAGE = 0.059;                                                    // define the “voltage drop” of the sensor when move the sensor from air into 1000ppm CO2
const float CO2Curve[3] = {2.602, ZERO_POINT_VOLTAGE, (REACTION_VOLTAGE / (2.602 - 3))}; // Line curve with 2 points

// Variable definitions
float volts = 0;  // Variable to store current voltage from CO2 sensor
float co2 = 0;    // Variable to store CO2 value
int vCO2 = 0;
float dCO2 = 0;

int getTimeForCO2Sensor(){
  dCO2 = 0;
  volts = analogRead(CO2) * 5.0 / 1023.0;
  if (volts / DC_GAIN >= ZERO_POINT_VOLTAGE)
  {
    dCO2 = 400;
  }
  else
  {
    dCO2 = pow(10, ((volts / DC_GAIN) - CO2Curve[1]) / CO2Curve[2] + CO2Curve[0]);
  }
  return co2GreenTime2 * dCO2 / 10000;
}