#define RELEASE 0
#define CLICK   1
#define DBCLICK 2
#define HOLD    3

#define PINHOLD 37
#define PINTAN 38
#define PINGREEN 39

int bounceTime = 50;
int holdTime = 250;
int doubleTime = 500;

int buttonState[5];
int lastReading[5];
int hold[5];
int single[5];

long onTime[5];
long lastSwitchTime[5];

int checkButton(int, int);
int onRelease(int);

void setupButton() {
  pinMode(PINTAN, INPUT);
  pinMode(PINGREEN, INPUT);
  pinMode(PINHOLD, INPUT);
}

void buttonCheckAll() {
  buttonState[0] = checkButton(PINHOLD, 0);
  buttonState[1] = checkButton(PINGREEN, 1);
  buttonState[2] = checkButton(PINTAN, 2);
}

//=====================================//

int checkButton(int buttonPin, int index) {
  int event = 0;
  int reading = digitalRead(buttonPin);
  
  //first pressed
  if (reading == HIGH && lastReading[index] == LOW) {
    onTime[index] = millis();
  }

  //held
  if (reading == HIGH && lastReading[index] == HIGH) {
    if ((millis() - onTime[index]) > holdTime) {
      event = HOLD;
      hold[index] = 1;
    }
  }

  //released
  if (reading == LOW && lastReading[index] == HIGH) {
    if (((millis() - onTime[index]) > bounceTime) && hold[index] != 1) {
      event = onRelease(index);
    }
    if (hold[index] == 1) {
      hold[index] = 0;
      Serial.println("hold release");
    }   
  }
   
  lastReading[index] = reading;
  if (single[index] == 1 && (millis() - lastSwitchTime[index]) > doubleTime) {
    event = CLICK;
    single[index] = 0;
  }
  
  return event;
}

int onRelease(int index) {  
  if ((millis() - lastSwitchTime[index]) >= doubleTime) {
    single[index] = 1;
    lastSwitchTime[index] = millis();
    return 0;
  }  

  if ((millis() - lastSwitchTime[index]) < doubleTime) {
    single[index] = 0;
    lastSwitchTime[index] = millis();
    return DBCLICK;
  }  
}
