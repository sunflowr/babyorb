#include <CapacitiveSensor.h>
#include <Wire.h>

// Set this line depending on if this arduino is master or slave (carrier of baby or parntner(s))
const byte isMaster = 0;

const byte TOUCH_SAMPLES = 30;
const byte TOUCH_THRESHOLD = 125;
int pot = 0;

// Touch state.
byte masterTouchState = 0;
byte slaveTouchState = 0;

// Baby size
byte babySize = 0;

int brightness = 0;
char fadeAmount = -5;

enum TouchStates
{
  TOUCH_START = 0,
  TOUCH_HOLD,
  TOUCH_RELEASE,
  TOUCH_NONE
};
byte touchState = TOUCH_NONE;


// Used for reading the touch.
CapacitiveSensor touch(4,2);


void setup() {
  if(isMaster)
  {
    // Master setup.
    // Join i2c bus (address optional for master).
    Wire.begin();
  }
  else
  { 
    // Slave setup.
    // Join i2c bus with address #8.
    Wire.begin(8);
  
    // Register events
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
  }

  pinMode(13, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(12, OUTPUT);

  // Start serial for output.
  Serial.begin(9600);
}

void loop()
{
  if(isMaster)
  {
    // Master code.
  
    // Read baby size.
    int a0 = analogRead(A0);
    babySize = map(a0, 0, 1023, 0, 255);

    // Read the touch value.
    int touchValue = touch.capacitiveSensor(TOUCH_SAMPLES);
    if(touchValue > TOUCH_THRESHOLD) {
      masterTouchState = HIGH;
    } else {
      masterTouchState = LOW;
    }
  
    // Send the master touch state to the slave.
    Wire.beginTransmission(8);
    Wire.write(masterTouchState);
    Wire.write(babySize);
    Wire.endTransmission();
  
    // Read the touch state from the slave
    // Request 1 byte (touch state) from slave device #8
    Wire.requestFrom(8, 1);
    while(Wire.available()) {
      slaveTouchState = Wire.read();
    }
  }
  else
  {
    // Slave code.
  
    // Read the touch value.
    int touchValue = touch.capacitiveSensor(TOUCH_SAMPLES);
    if(touchValue > TOUCH_THRESHOLD) {
      slaveTouchState = HIGH;
    } else {
      slaveTouchState = LOW;
    }
  }

  // Turn on a LED to show if it's working.
  //digitalWrite(13, ((masterTouchState == HIGH) || (slaveTouchState == HIGH)) ? HIGH : LOW);
  byte isOn = ((masterTouchState == HIGH) || (slaveTouchState == HIGH) || (brightness > 0)) ? HIGH : LOW;
  if((masterTouchState == HIGH) && (slaveTouchState == HIGH))
  {
    babySize = 255;
    brightness = 255;
  }
 
  byte led0 = isOn && (babySize > 0);
  byte led1 = isOn && (babySize > 36);
  byte led2 = isOn && (babySize > 72);
  byte led3 = isOn && (babySize > 109);
  byte led4 = isOn && (babySize > 145);
  byte led5 = isOn && (babySize > 182);
  byte led6 = isOn && (babySize > 218);

  analogWrite(3, led0 ? brightness : 0); //1
  analogWrite(6, led1 ? brightness : 0); //2
  analogWrite(5, led2 ? brightness : 0); //3
  analogWrite(11, led3 ? brightness : 0); //4
  analogWrite(10, led4 ? brightness : 0); //5
  analogWrite(9, led5 ? brightness : 0); //6
  digitalWrite(12, led6); //7

  switch(touchState)
  {
  case TOUCH_START:
    fadeAmount = 5;
    brightness = 0;
  case TOUCH_HOLD:
    brightness += fadeAmount;
    if(brightness < 0)
    {
      fadeAmount = 5;
      brightness = 0;
    }
    else if(brightness > 255)
    {
      fadeAmount = -5;
      brightness = 255;
    }
    if(isOn)
    {
      touchState = TOUCH_HOLD;
    }
    else
    {
      touchState = TOUCH_RELEASE;
    }
    break;
  case TOUCH_RELEASE:
    fadeAmount = -5;
  case TOUCH_NONE:
    brightness += fadeAmount;
    if(brightness < 0)
    {
      brightness = 0;
    }
    if(isOn)
    {
      touchState = TOUCH_START;
    }
    else
    {
      touchState = TOUCH_NONE;
    }
    break;
  }

  // Small delay.
  delay(20);
}

// Function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  masterTouchState = Wire.read();
  babySize = Wire.read();
}

// Function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent() {
  Wire.write(slaveTouchState);
}

