#include <CurieBLE.h>
#include "CurieIMU.h"
BLEPeripheral blePeripheral;  // BLE Peripheral Device 
BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service


/*set BLE characteristic*/
  BLEUnsignedCharCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin2 = 5; // pin to use for the LED++++++++++++++------------------------------------------------------------------------------------------------

int ax, ay, az;         // accelerometer values
int gx, gy, gz;         // gyrometer values

int axBrightness = 0;
const int axLed = 9;
int pitch=0;
int initialax, initialay, initialaz;

const int buttonPin = 2;
const int ledPin = 13;
int buttonState=0;

int inputValue = 0;

boolean override = false;
boolean first = true;

void setup()
{
 // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // set advertised local name and service UUID:
  blePeripheral.setLocalName("UD"); //Local name
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());

  // add service and characteristic:
   blePeripheral.addAttribute(ledService);
   blePeripheral.addAttribute(switchCharacteristic);

   // set the initial value for the characeristic:
    switchCharacteristic.setValue(0);

   // begin advertising BLE service:
   blePeripheral.begin();

  pinMode(axLed, OUTPUT); 
  Serial.begin(9600); // initialize Serial communication
  CurieIMU.begin();
  delay(5000); // Allow the user to have backpack set down
  CurieIMU.autoCalibrateGyroOffset();
  CurieIMU.autoCalibrateAccelerometerOffset(X_AXIS, 0);
  CurieIMU.autoCalibrateAccelerometerOffset(Y_AXIS, 0);
  CurieIMU.autoCalibrateAccelerometerOffset(Z_AXIS, 0);

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
 }

//Creating console output for debugggig
String jsonEncodeValue(String key, float keyVal){
  return "\"" + key + "\":" + String(keyVal) + "";
}

String assembleJson(String keysAndVals){
  return "{" + keysAndVals + "}";
}


void loop() 
{
  // listen for BLE peripherals to connect:
 BLECentral central = blePeripheral.central();

 // if a central is connected to peripheral:
  if (central)
  {
   // while the central is still connected to peripheral:
    while (central.connected())
    {
    
      // read the state of the pushbutton value:
      buttonState = digitalRead(buttonPin);

      // check if the pushbutton is pressed. 
      if (buttonState == HIGH) {
        //switch state to either being overriden or not
        digitalWrite(ledPin, HIGH);
        override=switchOverride(override);
        delay(100);
      } else {
        digitalWrite(ledPin, LOW);
      }

      //Check if a value was input via BLE
      if (switchCharacteristic.written())
        {
            inputValue =  switchCharacteristic.value();
            Serial.println(inputValue);
        }

      //check if alarm is enabled
      if(override==false and inputValue==1)
      {

        digitalWrite(ledPin2, HIGH); 
        CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);
        // display JSON formatted output of accelerometer and gyrometer values
        String keyVals = jsonEncodeValue("ax", ax) + ",";
        keyVals += jsonEncodeValue("ay", ay) + ",";
        keyVals += jsonEncodeValue("az", az) + ",";
        keyVals += jsonEncodeValue("gx", gx) + ",";
        keyVals += jsonEncodeValue("gy", gy) + ",";
        keyVals += jsonEncodeValue("gz", gz);
        if(Serial){
          Serial.println(keyVals);
        }
        delay(100);

        //check if this is first time to create base values to compare to
        if(first) {
          CurieIMU.readMotionSensor(ax, ay, az, gx, gy, gz);
          initialax = ax;
          initialay=ay;
          initialaz=az;
        }
        first=false;

        //checks if the accleromter has been moved enough to trigger alarm
        if(outOfRange(initialax,ax) or outOfRange(initialay,ay) or outOfRange(initialay,ay))
        {
          axBrightness = 50;
          pitch = 50; 
        }
         
        else
        {
          axBrightness=0;
          pitch =0;
        }
        analogWrite(axLed,axBrightness);
        tone(3,pitch,10); //plays the alarm sound
      }
       
      else 
      {                              
        digitalWrite(ledPin2, LOW);         
      }
        
     }
    
    
  }
}

//checks if current is more than 200 away from initial
boolean outOfRange(int intial, int current) {
  if((current> (intial+200)) or (current < (intial - 200)))
    return true;
  return false;}

//switches override to other boolean value
boolean switchOverride(boolean state){
  if(state==true)
      return false;
  return true;
}




