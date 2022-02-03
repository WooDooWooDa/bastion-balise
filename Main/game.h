#define NO_TEAM 0
#define TEAM_GREEN 1
#define TEAM_TAN 2

#define LED_GREEN 23
#define LED_TAN 12

const String START_GAME_PACKET = "startGame";
const String STOP_GAME_PACKET = "stopGame";
const int POINT_PER_MIN = 1;
const int TIMER_TIME = 60 * 1000000; //1 minute

bool gameIsRunning = false;
bool isReadyToUpdate = false;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
volatile int count;

int currentTeam = NO_TEAM;
int pointTeamGreen = 0;
int pointTeamTan = 0;

void stopGame();
void addPoints();
void toggleTeam(int);
String buildUpdateMsg();
void lightupLed();

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  count++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void startGame() {
  stopGame();
  gameIsRunning = true;
  timer = timerBegin(0, 40, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, TIMER_TIME, true);
  timerAlarmEnable(timer);
  Serial.println("Game started");
}

void gameLoop() {
  buttonCheckAll();
  if (buttonState[0] == HOLD) {
    if (buttonState[1] == HOLD) {
      toggleTeam(TEAM_GREEN);
    } else if (buttonState[2] == HOLD) {
      toggleTeam(TEAM_TAN);
    }
  }
  
  if (count > 0) {                    //add points si timer est déclencher! (1minutes)
    portENTER_CRITICAL(&timerMux);
    count--;
    portEXIT_CRITICAL(&timerMux);

    addPoints();
    isReadyToUpdate = true;
  }
}

void addPoints() {
  Serial.print("Adding points to : ");
  Serial.println(currentTeam);
  if (currentTeam == TEAM_GREEN) {
    pointTeamGreen += POINT_PER_MIN;
  } else if (currentTeam == TEAM_TAN) {
    pointTeamTan += POINT_PER_MIN;
  }
  Serial.printf("Current points are ; green %d, tan %d ", pointTeamGreen, pointTeamTan);
  Serial.println();
}

void toggleTeam(int team) {
  Serial.print("Toggling team to : ");
  Serial.println(team);
  if (team == TEAM_GREEN) {
    currentTeam = TEAM_GREEN;
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_TAN, LOW);
  } else if (team == TEAM_TAN) {
    currentTeam = TEAM_TAN;
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_TAN, HIGH);
  }
  Serial.println();
}

void stopGame() {
  pointTeamGreen = 0;
  pointTeamTan = 0;
  currentTeam = NO_TEAM;
  isReadyToUpdate = false;
  gameIsRunning = false;
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_TAN, LOW);
  Serial.println("Game has been stopped...");
}

String buildUpdateMsg() {
  char msg[80];
  sprintf(msg, "update#%d?pointsGreen=%d&pointsTan=%d&batteryLevel=%d&currentTeam=%d", BALISE_ID, pointTeamGreen, pointTeamTan, batteryVoltage, currentTeam);
  isReadyToUpdate = false;
  return msg;
}
