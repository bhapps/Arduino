/*
  
 On most Arduino, the PWM pins are identified with a "~" sign, like ~3, ~5, ~6, ~9, ~10 and ~11 

 We use the same library SoftwareSerial.h as for the HC-06 module:

 SoftwareSerial hc05(Rx,Tx) to define the pins of the serial port
   hc05.begin() to define the baudrate (value should be the same as your module)
   hc05.available() to test if data are available in the buffer of the module
   hc05.read() to read data one byte at a time
   hc05.print() to send a string in ASCII form
   hc05.write() to send data one byte at a time


*/
#include <SoftwareSerial.h> 
#include <Servo.h>

//byte/hex codes
int byteAsCarriageReturn = 10;
int byteAsNewLine = 13;
int byteAsSpace = 32;
int byteAsHash = 35;

//bluetooth
int bluetoothTx = 5;
int bluetoothRx = 6;
SoftwareSerial bluetoothModuleHC05(bluetoothRx, bluetoothTx); // RX | TX
int bluetoothModuleHC05Verified = 0;
int bluetoothModuleHC05DataReceived = 0;
int bluetoothModuleHC05Connected = false;
const long bluetoothModuleHC05BaudRate = 38400;
char bluetoothModuleHC05ConnectedFeed = ' ';
String incomingBluetoothModuleHC05ConnectedFeed = "";
String incomingBluetoothModuleHC05Data;
char bluetoothModuleHC05IncomingValue = 0;
int sendDataEvery5SecondToConnectedDevice = 0;

//received operation codes
String incomingBluetoothModuleHC05ConnectedToDevice = "connected";
String incomingBluetoothModuleHC05DisconnectedFromDevice = "disconnected";

String incomingBluetoothModuleHC05LEDOn = "turn_led_on";
String incomingBluetoothModuleHC05LEDOff = "turn_led_off";

//delays
int minimumDelay  = 10;
int defaultDelay  = 100;
int fastestDelay  = 100;
int fasterDelay  = 250;
int fastDelay  = 500;
int quickDelay  = 1000;
int smallDelay  = 2500;
int meduimDelay = 5000;
int longDelay   = 10000;
int minuteDelay   = 60000;

//task timers
int processEvery1SecondTimer = 0;
int processEvery2SecondTimer = 0;
int processEvery5SecondTimer = 0;
int processEvery10SecondTimer = 0;
int processEvery30SecondTimer = 0;
int processEvery60SecondTimer = 0;

//error status values
boolean deviceHasErrorsAndNeedsResart = false;

//start up checks
boolean startUpState = false;
int startUpStateCheckCount = 0;

//errorLEDPin
const int errorLEDPin = 9;
boolean errorLEDState = false;
int errorLEDBrightness = 0;
int errorLEDFadeAmount = 5;
int errorLEDMaxBrightness = 255;

//powerLEDPin
const int powerLEDPin = 10;
boolean powerLEDState = false;
int powerLEDBrightness = 0;
int powerLEDFadeAmount = 5;
int powerLEDMaxBrightness = 255;

//receiveInboundLEDPin
const int receiveInboundLEDPin = 11;
boolean receiveInboundLEDState = false;
int receiveInboundLEDBrightness = 0;
int receiveInboundLEDFadeAmount = 5;
int receiveInboundLEDMaxBrightness = 50;
byte receiveInboundLEDFlickerArray[] = {180, 30, 89, 23, 255, 200, 90, 150, 60, 230, 180, 45, 90};
boolean receiveInboundLEDActivityState = false;

//sendOutboundLEDPin
const int sendOutboundLEDPin = 12;
boolean sendOutboundLEDState = false;
int sendOutboundLEDBrightness = 0;
int sendOutboundLEDFadeAmount = 5;
int sendOutboundLEDMaxBrightness = 50;
byte sendOutboundLEDFlickerArray[] = {180, 30, 89, 23, 255, 200, 90, 150, 60, 230, 180, 45, 90};
boolean sendOutboundLEDActivityState = false;

//activityLEDPin
const int activityLEDPin = 13;
boolean activityLEDState = false;
int activityLEDBrightness = 0;
int activityLEDFadeAmount = 5;
int activityLEDMaxBrightness = 50;
byte activityLEDFlickerArray[] = {180, 30, 89, 23, 255, 200, 90, 150, 60, 230, 180, 45, 90};
boolean activityLEDActivityState = false;

//rotatePin
Servo servoMotor;
const int servoMotorPin = 8;
boolean servoMotorPinEnabled = true;
int servoMotorPinCurrentAngle = 0;
int servoMotorPinStartPosition = 0;
int servoMotorPinEndPosition = 360;
int servoMotorRotationSpeed = 50;  

void setup() {
  Serial.begin(9600);
  bluetoothModuleHC05.begin(9600);
  //servoMotor.attach(servoMotorPin);
  //servoMotor.write(servoMotorPinCurrentAngle);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(powerLEDPin, OUTPUT);
  pinMode(errorLEDPin, OUTPUT);
  pinMode(receiveInboundLEDPin, OUTPUT);
  pinMode(sendOutboundLEDPin, OUTPUT);
  pinMode(activityLEDPin, OUTPUT);  
}

void loop() {
 
  digitalWrite(LED_BUILTIN, LOW);
 
  if(!deviceHasErrorsAndNeedsResart){
    if(!startUpState){
      if(startUpStateCheckCount > 5){
        deviceHasErrorsAndNeedsResart = true;
      }else{
        checkStatesAtStartUp();
      }
    }else{
      processBluetooth();     
      processTasks();
      setPowerLEDBrightness(powerLEDMaxBrightness);
      //rotateServoMotorAngle(); 
    }
  }else{
    enterErrorMode();
  }

  delay(defaultDelay);
}

void enterErrorMode() {
  
  analogWrite(powerLEDPin, LOW);
  analogWrite(receiveInboundLEDPin, LOW);
  analogWrite(sendOutboundLEDPin, LOW);
  analogWrite(activityLEDPin, LOW);
 
  analogWrite(errorLEDPin, HIGH);
  delay(quickDelay);
  analogWrite(errorLEDPin, LOW);
  delay(quickDelay);
  analogWrite(errorLEDPin, HIGH);
  delay(quickDelay);
  analogWrite(errorLEDPin, LOW);
}

void checkStatesAtStartUp() {
  
  setPowerLEDOn(true);
  setErrorLEDOn(true);
  setReceiveInboundLEDOn(true);
  setSendOutboundLEDOn(true);
  setActivityLEDOn(true);
  delay(quickDelay);
  
  startUpStateCheckCount = startUpStateCheckCount + 1;
  if(!powerLEDState){
    powerLEDState = true;
    analogWrite(powerLEDPin, HIGH);
  }

  if(!errorLEDState){
    errorLEDState = true;
    setErrorLEDOn(false);
  }

  if(!receiveInboundLEDState){
    receiveInboundLEDState = true;
    setReceiveInboundLEDOn(false);
  }

  if(!sendOutboundLEDState){
    sendOutboundLEDState = true;
    setSendOutboundLEDOn(false);
  }

  if(!activityLEDState){
    activityLEDState = true;
    setActivityLEDOn(false);
  }

  if(powerLEDState && errorLEDState && receiveInboundLEDState && sendOutboundLEDState && activityLEDState){
    startUpState = true;
  }
}

void processTasks() {
  processEvery1Second();
  processEvery2Second();
  processEvery5Second();
  processEvery10Second();
  processEvery30Second();
  processEvery60Second();
}

void processEvery1Second() {
  
}

void processEvery2Second() {
  processEvery2SecondTimer = processEvery2SecondTimer + 1;
  if(processEvery2SecondTimer == 20){
      processEvery2SecondTimer = 0;
      //do task every 2 seconds
  }
}

void processEvery5Second() {
  processEvery5SecondTimer = processEvery5SecondTimer + 1;
  if(processEvery5SecondTimer == 50){
      processEvery5SecondTimer = 0;
      //do task every 5 seconds
      //doActivityLEDProcessing();
  }
}

void processEvery10Second() {
  processEvery10SecondTimer = processEvery10SecondTimer + 1;
  if(processEvery10SecondTimer == 100){
      processEvery10SecondTimer = 0;
      //do task every 10 seconds
  }
}

void processEvery30Second() {
  processEvery30SecondTimer = processEvery30SecondTimer + 1;
  if(processEvery30SecondTimer == 300){
      processEvery30SecondTimer = 0;
      //do task every 30 seconds
  }
}

void processEvery60Second() {
  processEvery60SecondTimer = processEvery60SecondTimer + 1;
  if(processEvery60SecondTimer == 600){
      processEvery60SecondTimer = 0;
      //do task every 60 seconds
  }
}

//bluetooth
void processBluetooth() {
   processSendingDataEvery5SecondToConnectedDevice();
   if (bluetoothModuleHC05.available() > 0) {

    bluetoothModuleHC05Connected = true;
    boolean finishedReceiving = false;

    while(bluetoothModuleHC05.available()) {
      bluetoothModuleHC05DataReceived = bluetoothModuleHC05.read();
      if(String(bluetoothModuleHC05DataReceived) == String(byteAsNewLine) ||
         String(bluetoothModuleHC05DataReceived) == String(byteAsCarriageReturn) ||
         String(bluetoothModuleHC05DataReceived) == String(byteAsSpace)
      ){
        //String(NL) received
        //String(CR) received
        //String(SP) received
      }else if(String(bluetoothModuleHC05DataReceived) == String(byteAsHash)){
       //String(END) received
       finishedReceiving = true;
      }else{
       //valid data
       incomingBluetoothModuleHC05Data += char(bluetoothModuleHC05DataReceived);
       //Serial.print("android data received: ");Serial.print(char(bluetoothModuleHC05DataReceived));
       //Serial.print("\n");
      }
            
    }

    if(incomingBluetoothModuleHC05Data.length() > 0){
      if(String(incomingBluetoothModuleHC05Data) == incomingBluetoothModuleHC05LEDOn) {
        setReceiveInboundLEDOn(true);
      }else if(String(incomingBluetoothModuleHC05Data) == incomingBluetoothModuleHC05LEDOff) {
        setReceiveInboundLEDOn(false);
      }else if(String(incomingBluetoothModuleHC05Data) == incomingBluetoothModuleHC05ConnectedToDevice) {
        setActivityLEDOn(true);
      }else if(String(incomingBluetoothModuleHC05Data) == incomingBluetoothModuleHC05DisconnectedFromDevice) {
        setActivityLEDOn(false);
      }      
      incomingBluetoothModuleHC05Data = "";
    }
    
   }
 
}

void processSendingDataEvery5SecondToConnectedDevice() {
  sendDataEvery5SecondToConnectedDevice = sendDataEvery5SecondToConnectedDevice + 1;
  if(sendDataEvery5SecondToConnectedDevice == 50){
      sendDataEvery5SecondToConnectedDevice = 0;
      //do task appx every 5 seconds
      bluetoothModuleHC05.println("devices_connected");
      delay(minimumDelay);
  }
}

//PowerLED
void setPowerLEDOn(boolean turnOn) {
    if(turnOn){
      digitalWrite(powerLEDPin, HIGH);
    }else{
      digitalWrite(powerLEDPin, LOW);
    }
}

void doPowerLEDProcessing() {
    analogWrite(powerLEDPin, powerLEDBrightness);
    powerLEDBrightness = powerLEDBrightness + powerLEDFadeAmount;
    if (powerLEDBrightness <= 0 || powerLEDBrightness >= powerLEDMaxBrightness) {
      powerLEDFadeAmount = -powerLEDFadeAmount;
    }
}

void setPowerLEDBrightness(int _powerLEDBrightness) {
    analogWrite(powerLEDPin, _powerLEDBrightness);
}

void resetPowerLED() {
    digitalWrite(powerLEDPin, LOW);
    delay(fastestDelay);
    digitalWrite(powerLEDPin, HIGH);
}
//PowerLED

//ErrorLED
void setErrorLEDOn(boolean turnOn) {
    if(turnOn){
      digitalWrite(errorLEDPin, HIGH);
    }else{
      digitalWrite(errorLEDPin, LOW);
    }
}

void doErrorLEDProcessing() {
    analogWrite(errorLEDPin, errorLEDBrightness);
    errorLEDBrightness = errorLEDBrightness + errorLEDFadeAmount;
    if (errorLEDBrightness <= 0 || errorLEDBrightness >= errorLEDMaxBrightness) {
      errorLEDFadeAmount = -errorLEDFadeAmount;
    }
}

void setErrorLEDBrightness(int _errorLEDBrightness) {
    analogWrite(errorLEDPin, _errorLEDBrightness);
}

void resetErrorLED() {
    digitalWrite(errorLEDPin, LOW);
    delay(fastestDelay);
    digitalWrite(errorLEDPin, HIGH);
}
//ErrorLED

//ReceiveInboundLED
void doReceiveInboundLEDProcessing() {
    if(!receiveInboundLEDActivityState) {
      receiveInboundLEDActivityState = true;
      interrupts();
      int randNumber = random(10, 12);
      for(int receiveInboundLEDFlickerArrayIndex=0; receiveInboundLEDFlickerArrayIndex,randNumber; receiveInboundLEDFlickerArrayIndex++)
      {
        analogWrite(receiveInboundLEDPin, receiveInboundLEDFlickerArray[receiveInboundLEDFlickerArrayIndex]);
        delay(fastestDelay);
      }
      setReceiveInboundLEDOn(false);
      delay(smallDelay);
      receiveInboundLEDActivityState = false;     
    }
}

void setReceiveInboundLEDBrightness(int _receiveInboundLEDBrightness) {
    analogWrite(receiveInboundLEDPin, _receiveInboundLEDBrightness);
}

void setReceiveInboundLEDOn(boolean turnOn) {
    if(turnOn){
      digitalWrite(receiveInboundLEDPin, HIGH);
    }else{
      digitalWrite(receiveInboundLEDPin, LOW);
    }
}

void resetReceiveInboundLED() {
    digitalWrite(receiveInboundLEDPin, LOW);
    delay(fastestDelay);
    digitalWrite(receiveInboundLEDPin, HIGH);
}
//ReceiveInboundLED

//sendOutboundLED
void doSendOutboundLEDProcessing() {
    if(!sendOutboundLEDActivityState) {
      sendOutboundLEDActivityState = true;
      interrupts();
      int randNumber = random(10, 12);
      for(int sendOutboundLEDFlickerArrayIndex=0; sendOutboundLEDFlickerArrayIndex,randNumber; sendOutboundLEDFlickerArrayIndex++)
      {
        analogWrite(sendOutboundLEDPin, sendOutboundLEDFlickerArray[sendOutboundLEDFlickerArrayIndex]);
        delay(fastestDelay);
      }
      setSendOutboundLEDOn(false);
      delay(smallDelay);
      sendOutboundLEDActivityState = false;     
    }
}

void setSendOutboundLEDBrightness(int _sendOutboundLEDBrightness) {
    analogWrite(sendOutboundLEDPin, _sendOutboundLEDBrightness);
}

void setSendOutboundLEDOn(boolean turnOn) {
    if(turnOn){
      digitalWrite(sendOutboundLEDPin, HIGH);
    }else{
      digitalWrite(sendOutboundLEDPin, LOW);
    }
}

void resetSendOutboundLED() {
    digitalWrite(sendOutboundLEDPin, LOW);
    delay(fastestDelay);
    digitalWrite(sendOutboundLEDPin, HIGH);
}
//sendOutboundLED

//ActivityLED
void doActivityLEDProcessing() {
    if(!activityLEDActivityState) {
      activityLEDActivityState = true;
      int randNumber = random(10, 12);
      for(int activityLEDFlickerArrayIndex=0; activityLEDFlickerArrayIndex,randNumber; activityLEDFlickerArrayIndex++)
      {
        analogWrite(activityLEDPin, activityLEDFlickerArray[activityLEDFlickerArrayIndex]);
        delay(fastestDelay);
      }
      setActivityLEDOn(false);
      delay(smallDelay);
      activityLEDActivityState = false;     
    }
}

void setActivityLEDBrightness(int _activityLEDBrightness) {
    analogWrite(activityLEDPin, _activityLEDBrightness);
}

void setActivityLEDOn(boolean turnOn) {
    if(turnOn){
      digitalWrite(activityLEDPin, HIGH);
    }else{
      digitalWrite(activityLEDPin, LOW);
    }
}

void resetActivityLED() {
    digitalWrite(activityLEDPin, LOW);
    delay(fastestDelay);
    digitalWrite(activityLEDPin, HIGH);
}
//ActivityLED

//ServoMotor
void rotateServoMotorAngle() {
    // scan from 0 to 180 degrees
    for(servoMotorPinCurrentAngle = 10; servoMotorPinCurrentAngle < 180; servoMotorPinCurrentAngle++)  
    {                                  
      servoMotor.write(servoMotorPinCurrentAngle);               
      delay(minimumDelay);                   
    } 
    // now scan back from 180 to 0 degrees
    for(servoMotorPinCurrentAngle = 180; servoMotorPinCurrentAngle > 10; servoMotorPinCurrentAngle--)    
    {                                
      servoMotor.write(servoMotorPinCurrentAngle);           
      delay(minimumDelay);       
    } 
}
//ServoMotor
