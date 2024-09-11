// For GSM Module
#include <SoftwareSerial.h>

// Library for Servo Motor
#include <Servo.h>

// for keypad
#include <Keypad.h>

//library for loadcell
#include <HX711_ADC.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

// Library for i2c display
// for i2c display
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

//library for temperature sensor
#include <OneWire.h>
#include <DallasTemperature.h>


/***** Servo Motor *****/

// Define the servo objects for each pin
Servo servo1;          //used to measure sugar amount
Servo servo2;          //used to measure sugar amount
Servo servo3;          //used to measure Anchor amount
Servo servo4;          //used to measure Anchor amount
Servo servo5;          //used to measure Tea leaf amount
Servo servo6;          //used to measure Tea leaf amount
Servo servo7;          //Used in tea leaf filter system
Servo servo8;   

// Define the angles for each movement
const int angle0 = 0;
const int angle90 = 90;
const int angle180 = 180;
const int angle270 = 270;

/***********/
/*****relay****/
const int pump_relay_1 = 53; //r1 // the Arduino pin, which connects to the IN pin of relay
const int pump_relay_2 = 51; //r2
const int pumpmotor_relay = 47; //r3 //tea to cup
const int h_pump_relay_3 = 49; //r4 //heating element to mixer container
const int mixermotor_relay = 43;//r6 
const int h_coil_relay =23 ;// coil relay r7
/*****level sensor*********/
const int levelsensor_pin = A0;
const int numReadings = 100;
int sensorReadings[numReadings];
int currentReading = 0;
long totalSensorValue = 0;
int averageSensorValue = 0;

/***** LCD Display *****/
// Set the LCD address and dimensions (change these to match your setup)
const int lcdColumns = 16;
const int lcdRows = 2;
const int lcdAddress = 0x27;

LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);
/************/

/***** Keypad ****/

// Define the keypad layout
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};
byte rowPins[ROWS] = { 39, 37, 35, 33 };  // Connect to the row pinouts of the keypad
byte colPins[COLS] = { 31, 29, 27, 25 };  // Connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/************/
/******temporature sensor****/
const int SENSOR_PIN = 4; // Arduino pin connected to DS18B20 sensor's DQ pin

OneWire oneWire(SENSOR_PIN);         // setup a oneWire instance
DallasTemperature tempSensor(&oneWire); // pass oneWire to DallasTemperature library

float temp;    // temperature in Celsius

/*************/
/********IR sensor*******/
const int irSensorPin = 52;  // Pin for the IR sensor


/*** GSM Module ********/
SoftwareSerial mySerial(18, 19);  // RX, TX
// conect RX to TX and Tx to RX

/***********/


/***  Load Cell ********/
int count = 0;
//pins:
const int HX711_dout = 24; //mcu > HX711 dout pin
const int HX711_sck = 22; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);

const int calVal_eepromAdress = 0;
unsigned long t = 0;
/************/

/*** Other variables ******/
const int numOfTeaTypes = 3;

const char* TeaTypes[numOfTeaTypes] = {
  "1. Normal",
  "2. Strong",
  "3. Sugarless"
};

int selectedTeaType = 0;

int TeaQuantity;

void setup() {
  /*****relay****/
  pinMode(pump_relay_1, OUTPUT);
  pinMode(pump_relay_2, OUTPUT);
  pinMode(h_pump_relay_3, OUTPUT);
  pinMode(pumpmotor_relay, OUTPUT);
  pinMode(mixermotor_relay, OUTPUT);
  pinMode(h_coil_relay, OUTPUT);
  /****levelsensor****/
  pinMode(levelsensor_pin, INPUT);  

  
  Serial.begin(9600);
  /*** i2c Display *******/
  // Initialize the I2C bus
  Wire.begin();

  // Initialize the LCD
  lcd.begin(lcdColumns, lcdRows);
  lcd.setBacklight(HIGH);  // Set backlight to a higher level
  /************/
   
  pinMode(irSensorPin, INPUT);  // Pin mode for IR sensor

  // Attach the servo objects to their respective pins
  servo1.attach(45);
  servo2.attach(44);
  servo3.attach(43);
  servo4.attach(42);
  servo5.attach(41);
  servo6.attach(40);
  servo7.attach(39);
  servo8.attach(38);
  servo1.write(90);
  servo2.write(0);
  servo3.write(90);
  servo4.write(0);
  servo5.write(90);
  servo6.write(0);
  servo7.write(90);
  servo8.write(0);
  
  Serial.begin(9600);
  delay(2000);
  Serial.println();
  Serial.println("Starting...");
  Serial.println("SIM900A Ready");
  tempSensor.begin();    // initialize the sensor

  LoadCell.begin();
  //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
  float calibrationValue; // calibration value (see example file "Calibration.ino")
  calibrationValue = 417.73; //  to set the calibration value in the sketch
  #if defined(ESP8266)|| defined(ESP32)
  #endif
  unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    while (1);
  }
  else {
    LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
    Serial.println("Startup is complete");
  }

}

void loop() {
  int TeaQuantity = 0;

  Serial.begin(9600);

  // print 'starting machine' on display
  lcd.clear();
  lcd.print("Starting");
  lcd.setCursor(0, 1);
  lcd.print("Machine.......");
  delay(2000);

  lcd.clear();  // clear lcd
  lcd.print("Avilable Tea");
  lcd.setCursor(0, 1);
  lcd.print("Types : ");
  delay(2000);

  // Print tea types in lcd types
  for (int i = 0; i < numOfTeaTypes; i++) {
    lcd.clear();  // Clear the LCD display
    lcd.print(TeaTypes[i]);
    delay(2000);  // Delay to display each topping for 2 seconds
  }

  lcd.clear();  // Clear the LCD display
  lcd.print("Select Tea");
  lcd.setCursor(0, 1);
  lcd.print("Type : ");

  char key;
  while (true) {
    key = keypad.getKey();  // get keyboard input

    if (key == '1') {
      lcd.clear();
      lcd.print("You select");
      lcd.setCursor(0, 1);
      lcd.print(TeaTypes[key - '1']);
      selectedTeaType = 1;
      delay(2000);
      break;
    } else if (key == '2') {
      lcd.clear();
      lcd.print("You select");
      lcd.setCursor(0, 1);
      lcd.print(TeaTypes[key - '1']);
      selectedTeaType = 2;
      delay(2000);
      break;
    } else if (key == '3') {
      lcd.clear();
      lcd.print("You select");
      lcd.setCursor(0, 1);
      lcd.print(TeaTypes[key - '1']);
      selectedTeaType = 3;
      delay(2000);
      break;
    }

    if (key == 'A' || key == 'B' || key == 'C' || key == 'D' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0') {
      lcd.clear();
      lcd.print("Invalid");
      lcd.setCursor(0, 1);
      lcd.print("Selection!");
      delay(1500);

      lcd.clear();
      lcd.print("Please Select");
      lcd.setCursor(0, 1);
      lcd.print("Option 1 / 2 / 3");
      delay(1500);
    }
  }

  lcd.clear();
  lcd.print("Enter Tea");
  lcd.setCursor(0, 1);
  lcd.print("Quantity : ");

  while (true) {
    char qty = ' ';

    qty = keypad.getKey();

    if (qty == '1' || qty == '2' || qty == '3' || qty == '4' || qty == '5') {
      switch (qty) {
        case '1':
          TeaQuantity = 1;
          break;
        case '2':
          TeaQuantity = 2;
          break;
        case '3':
          TeaQuantity = 3;
          break;
        case '4':
          TeaQuantity = 4;
          break;
        case '5':
          TeaQuantity = 5;
          break;
      }
      lcd.clear();
      lcd.print("Quantity ");
      lcd.setCursor(0, 1);
      lcd.print(qty);
      delay(2000);
      break;
    } else if (qty == '6' || qty == '7' || qty == '8' || qty == '9' || qty == '0' || qty == 'A' || qty == 'B' || qty == 'C' || qty == 'D') {
      lcd.clear();
      lcd.print("Invalid");
      lcd.setCursor(0, 1);
      lcd.print("Quantity!");
      delay(1500);

      lcd.clear();
      lcd.print("Select");
      lcd.setCursor(0, 1);
      lcd.print("Less than 5");
    }


    /*if (TeaQuantity > 0) {
      break;
    }*/
  }

  lcd.clear();
  lcd.print("Type: " + String(TeaTypes[selectedTeaType - 1]));
  lcd.setCursor(0, 1);
  lcd.print("Quantity: " + String(TeaQuantity));
  delay(2000);

  lcd.clear();
  lcd.print("A: Continue");
  lcd.setCursor(0, 1);
  lcd.print("B: Reset");

  char ConfirmationKey;
  while (true) {
    ConfirmationKey = keypad.getKey();

    if (ConfirmationKey == 'A') {
      lcd.clear();
      lcd.print("Tea Making");
      lcd.setCursor(0, 1);
      lcd.print("Started...");
      delay(2000);
      break;
    } else if (ConfirmationKey == 'B') {
      lcd.clear();
      lcd.print("Resetting");
      lcd.setCursor(0, 1);
      lcd.print("Machine");
      delay(2000);
      break;
    } else if (ConfirmationKey == '1' || ConfirmationKey == '2' || ConfirmationKey == '3' || ConfirmationKey == '4' || ConfirmationKey == '5' || ConfirmationKey == '6' || ConfirmationKey == '7' || ConfirmationKey == '8' || ConfirmationKey == '9' || ConfirmationKey == '0' || ConfirmationKey == 'C' || ConfirmationKey == 'D') {
      lcd.clear();
      lcd.print("Invalid");
      lcd.setCursor(0, 1);
      lcd.print("Selection!");
      delay(1000);

      lcd.print("A: Continue");
      lcd.setCursor(0, 1);
      lcd.print("B: Reset");
    }
  }
  //level check
 for(int i=0;i<TeaQuantity;i++){
    int waterLevel_1 = measureWaterLevel();  // Measure the water level
    Serial.print(waterLevel_1);
    if(waterLevel_1<100) {
      Serial.println("Turn on the motor");  // Water level is below or equal to 400, turn on the motor
      digitalWrite(pump_relay_1, HIGH);
      delay(75000);
      digitalWrite(pump_relay_1, LOW);  
    }

    int waterLevel_2 = measureWaterLevel();  // Measure the water level
    Serial.print(waterLevel_2);
    if (waterLevel_2 > 300) {
       Serial.println("Start the progress");  // Water level is above 400, start the progress
       // Perform other actions related to starting the progress
       digitalWrite(pump_relay_2, HIGH); // turn on solinoid valve relay 5 seconds
       delay(5000);
       digitalWrite(pump_relay_2, LOW);  // turn off solinoid valve relay 5 seconds
       delay(5000);
       // Perform other actions related to turning on the motor
        
       //selection for the tea
       Serial.println("tea type");
     
       if(selectedTeaType==1){
           selection_1_measuring(); 
       } 
       if(selectedTeaType==2){
          selection_2_measuring();
       }
       if(selectedTeaType==3){
           selection_3_measuring();
  
       }
       float temperature = tempmeasurment();
    digitalWrite(h_coil_relay, HIGH); // turn on coil
    
   while(temperature<70){
        temperature = tempmeasurment();
      }
  if (temperature>70){
        Serial.print("turn off the rellay");
        digitalWrite(h_coil_relay, LOW);
      }
      //pass the hot water to the mixer container
    digitalWrite(h_pump_relay_3, HIGH); // turn on pump 5 seconds
    delay(1000);
    digitalWrite(h_pump_relay_3, LOW);  // turn off pump 5 seconds
    delay(1000);
    //mix the tea
    digitalWrite(mixermotor_relay, HIGH); // turn on mixer motor 5 seconds
    delay(3000);
    digitalWrite(mixermotor_relay, LOW);
    delay(1000); 
       turnServo_8(servo8, angle90, 1);
       turnServo_7(servo7, angle180, 1);
       delay(2000);  
       turnServo_8(servo8, angle0, 1);
       delay(1000);
    
       lcd.clear();
      lcd.print("place the cup");
      lcd.setCursor(0, 1);
      lcd.print("in the position");
      delay(1500);
       if (digitalRead(irSensorPin) == LOW) {
          Serial.println("Object detected");
          Serial.println("pump motor is working");
          digitalWrite(pumpmotor_relay, HIGH);
          delay(1000);  // Duration of pump operation
          digitalWrite(pumpmotor_relay, LOW);
          delay(1000);
         }

  
  } 
  
  delay(1000);
 lcd.clear();
      lcd.print("Thank You");
      lcd.setCursor(0, 1);
      lcd.print("Come Again");
      delay(1500);
 }
 float x=weight_of_waste();
    if(x>500){
    Serial.println("dispose the weight");
    sendSMS("+94777602179", "dispose the weight");  // Send SMS
    }

}



bool validQuantitySelection(char key) {
  if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5') {
    return true;
  } else if (key == '6' || key == '7' || key == '8' || key == '9' || key == '0' || key == 'A' || key == 'B' || key == 'C' || key == 'D') {
    return false;
  }
}
/**** Functions for loade cells ****/


/**** GSM Module ****/
void sendSMS(String recipient, String message) {
  mySerial.println("AT+CMGF=1");
  delay(1000);

  mySerial.println("AT+CMGS=\"" + recipient + "\"");
  delay(1000);

  mySerial.println(message);
  delay(1000);

  mySerial.write(26);
  delay(5000);

  Serial.println("SMS Sent successfully!");
}

/***********/


/**** Servo Motor *****/
// Function to turn the servo by the given angle for the specified number of times
void turnServo(Servo servo, int angle, int numOfTimes) {
  for (int i = 0; i < numOfTimes; i++) {
    servo.write(angle);
    delay(2000);
    servo.write(0);
    delay(1000);
  }
}

void turnServo_8(Servo servo, int angle, int numOfTimes) {
  for (int i = 0; i < numOfTimes; i++) {
    servo.write(angle);
    delay(1000);
  }
}
void turnServo_7(Servo servo, int angle, int numOfTimes) {
  for (int i = 0; i < numOfTimes; i++) {
    servo.write(angle);
    delay(1000);
    servo.write(90);
    delay(2000);
  }
}
void selection_1_measuring(){
 for(int i=0;i<3;i++){
    //suger                                      //Strong tea
      turnServo(servo1, angle90, 1);
      turnServo(servo2, angle90, 1);
    }
    //tea
    for(int i=0;i<2;i++){
      turnServo(servo3, angle90, 1);
      turnServo(servo4, angle90, 1);
    }
    //milkpowder
   for(int i=0;i<2;i++){
      turnServo(servo5, angle90, 1);
      turnServo(servo6, angle90, 1);
    }
}
void selection_2_measuring(){
   //suger
   for(int i=0;i<2;i++){
      turnServo(servo1, angle90, 1);
      turnServo(servo2, angle90, 1);
    }
    //tea
    for(int i=0;i<1;i++){
      turnServo(servo3, angle90, 1);
      turnServo(servo4, angle90, 1);
    }
    //milkpowder
   for(int i=0;i<3;i++){
      turnServo(servo5, angle90, 1);
      turnServo(servo6, angle90, 1);
    }

}
void selection_3_measuring(){
  //suger
  for(int i=0;i<3;i++){
      turnServo(servo1, angle90, 1);
      turnServo(servo2, angle90, 1);
    }
    //tea
    for(int i=0;i<3;i++){
      turnServo(servo3, angle90, 1);
      turnServo(servo4, angle90, 1);
    }

}
/***water level measurment****/

int measureWaterLevel() {
  totalSensorValue = 0;

  // Read the sensor value multiple times and calculate the average
  for (int i = 0; i < numReadings; i++) {
    sensorReadings[currentReading] = analogRead(levelsensor_pin);
    totalSensorValue += sensorReadings[currentReading];
    currentReading = (currentReading + 1) % numReadings;
    delay(2);
  }

  averageSensorValue = totalSensorValue / numReadings;

  //Serial.print("Average sensor value = ");
  //Serial.println(averageSensorValue);

  return averageSensorValue;
}

/***********/
/******calculate the weight of the weight*****/
float weight_of_waste(){
  float actual_weight_value=0;
   while (count < 100){
    
    static boolean newDataReady = 0;
    const int serialPrintInterval = 0; //increase value to slow down serial print activity

    // check for new data/start next conversion:
    if (LoadCell.update()) newDataReady = true;

    // get smoothed value from the dataset:
    if (newDataReady) {
      if (millis() > t + serialPrintInterval) {
        float weight_val = LoadCell.getData();
        Serial.print("Load_cell actual weight value : ");
        Serial.println(weight_val);
        newDataReady = 0;
        t = millis();
        if (count >= 50){
          actual_weight_value = actual_weight_value + weight_val;
        }
        count = count + 1;
      }
    }
  }

  actual_weight_value = actual_weight_value/50;
  count = 0;

  Serial.print("The actual weighgt is : ");
  Serial.println(actual_weight_value);



  return actual_weight_value;

}
/*************/
float tempmeasurment(){
  tempSensor.requestTemperatures();             // send the command to get temperatures
  temp = tempSensor.getTempCByIndex(0);  // read temperature in Celsius
  /*Serial.print("\nTemperature: ");
  Serial.print(temp);    // print the temperature in Celsius
  Serial.print("°C");
  Serial.print("  ~  ");        // separator between Celsius and Fahrenheit
  Serial.print(tempFahrenheit); // print the temperature in Fahrenheit
  Serial.println("°F");*/

  delay(500);
  return temp;
}