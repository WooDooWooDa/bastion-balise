const int buttonGreen = 12;


const int RELEASE = 0;
const int CLICK = 1;
const int DBCLICK = 2;
const int HOLD = 3;

int bounceTime = 50;
int holdTime = 250;
int doubleTime = 500;

int buttonState[5];
int lastReading[5];
int hold[5];
int single[5];

long onTime[5];
long lastSwitchTime[5];

void setup() {
  pinMode(buttonGreen, INPUT);
  Serial.begin(9600);
}

void loop() {
  buttonState[0] = checkButton(buttonGreen, 0);
  if (buttonState[0] == HOLD) {
    Serial.println("hold");
  } else if (buttonState[0] == CLICK) {
    Serial.println("clicked ");
  }
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
