#include <NewPing.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;

// ─── PIN DEFINITIONS ───────────────────────────────────────────────────────
#define left_ir    15
#define center_ir  4
#define right_ir   17

#define leftmotor1  26
#define leftmotor2  27
#define rightmotor1 33
#define rightmotor2 32

#define ENA 14
#define ENB 25

// Ultrasonic pins
#define LEFT_TRIG  16
#define LEFT_ECHO  18
#define FRONT_TRIG 19
#define FRONT_ECHO 21
#define RIGHT_TRIG 22
#define RIGHT_ECHO 23

#define MAX_DISTANCE 200

// ─── TUNING ────────────────────────────────────────────────────────────────
#define BASE_SPEED   80
#define TURN_SPEED   60

#define SIDE_CLOSE   13 // too close to a side wall -> steer away
#define SIDE_LOST    20   // both sides closer than this + no line = enter wall mode
#define FRONT_OPEN   18   // front clear enough to just go straight
#define FRONT_BLOCK  40// front obstacle -> must turn

// ─── ULTRASONIC ────────────────────────────────────────────────────────────
NewPing sonarLeft(LEFT_TRIG, LEFT_ECHO, MAX_DISTANCE);
NewPing sonarFront(FRONT_TRIG, FRONT_ECHO, MAX_DISTANCE);
NewPing sonarRight(RIGHT_TRIG, RIGHT_ECHO, MAX_DISTANCE);

// ─── STATE MACHINE ─────────────────────────────────────────────────────────
enum RobotMode { LINE_FOLLOW, WALL_FOLLOW };
RobotMode mode = LINE_FOLLOW;

bool manualMode = false;   // false = automatic (default)

int leftMotorSpeed = 90;
int rightMotorSpeed = 90;

char command;

// ─── MOTOR FUNCTIONS ──────────────────────────────────────────────────────

void forward() {
  analogWrite(ENA, leftMotorSpeed);
  analogWrite(ENB, rightMotorSpeed);
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}
void Mforward() {
  analogWrite(ENA, 125);
  analogWrite(ENB, 125);
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}

void Sforward() {
  analogWrite(ENA, leftMotorSpeed - 10);
  analogWrite(ENB, rightMotorSpeed - 10);
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}

void back() {
  analogWrite(ENA, leftMotorSpeed - 20);
  analogWrite(ENB, rightMotorSpeed - 20);
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, HIGH);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, HIGH);
}

void forwardLeft() {
  analogWrite(ENA, leftMotorSpeed * 2 / 10);   
  analogWrite(ENB, rightMotorSpeed +25);         
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}

void forwardRight() {
  analogWrite(ENA, leftMotorSpeed +25);           
  analogWrite(ENB, rightMotorSpeed * 2 / 10); 
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}

void backLeft() {
  analogWrite(ENA, (leftMotorSpeed - 20) * 2 / 10);
  analogWrite(ENB, rightMotorSpeed - 20);
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, HIGH);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, HIGH);
}

void backRight() {
  analogWrite(ENA, leftMotorSpeed - 20);
  analogWrite(ENB, (rightMotorSpeed - 20) * 2 / 10);
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, HIGH);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, HIGH);
}

void turnLeft() {
  analogWrite(ENA, leftMotorSpeed);
  analogWrite(ENB, rightMotorSpeed);
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}

void turnRight() {
  analogWrite(ENA, leftMotorSpeed);
  analogWrite(ENB, rightMotorSpeed);
  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, LOW);
}

void stopRobot() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, LOW);
}

// Signed speed helper used by wall-follow (positive = forward, negative = reverse)
void setMotors(int leftSpeed, int rightSpeed) {
  if (leftSpeed >= 0) {
    digitalWrite(leftmotor1, HIGH);
    digitalWrite(leftmotor2, LOW);
  } else {
    digitalWrite(leftmotor1, LOW);
    digitalWrite(leftmotor2, HIGH);
    leftSpeed = -leftSpeed;
  }

  if (rightSpeed >= 0) {
    digitalWrite(rightmotor1, HIGH);
    digitalWrite(rightmotor2, LOW);
  } else {
    digitalWrite(rightmotor1, LOW);
    digitalWrite(rightmotor2, HIGH);
    rightSpeed = -rightSpeed;
  }

  analogWrite(ENA, constrain(leftSpeed, 0, 255));
  analogWrite(ENB, constrain(rightSpeed, 0, 255));
}

// Returns distance in cm, or MAX_DISTANCE if out of range (no echo)
long readDistance(NewPing &sonar) {
  unsigned int cm = sonar.ping_cm();
  if (cm == 0) return MAX_DISTANCE;
  return cm;
}

// ─── BLUETOOTH / MANUAL CONTROL ────────────────────────────────────────────
void bluetoothControl() {
  if (!SerialBT.available()) return;

  char command = SerialBT.read();

  // Enable manual mode
  if (command == '1') {
    manualMode = true;
    stopRobot();
    Serial.println("Manual Mode ON");
    return;
  }

  // Disable manual mode (back to automatic)
  if (command == '2') {
    manualMode = false;
    stopRobot();
    Serial.println("Manual Mode OFF");
    return;
  }

  // Ignore driving commands unless manual mode is enabled
  if (!manualMode) return;

  switch (command) {
    case 'U': Mforward(); break;
    case 'D': back(); break;
    case 'R': turnLeft(); break;
    case 'L': turnRight(); break;
    case 'S': stopRobot(); break;
     case 'F': forwardLeft(); break;   // Up + Left
  case 'T': forwardRight(); break;  // Up + Right
  case 'G': backLeft(); break;      // Down + Left
  case 'H': backRight(); break;     // Down + Right
  }
}
// ─── LINE FOLLOW ────────────────────────────────────────────────────────────
void lineFollow() {
  int l = digitalRead(left_ir);
  int c = digitalRead(center_ir);
  int r = digitalRead(right_ir);

  // ── transition check: only ping the sonars when the line is actually gone ──
  bool lineLost = ((l == 0) && (c == 0) && (r == 0) ||(l == 1) && (c == 1) && (r == 1) ) ;
  if (lineLost) {
    long distLeft  = readDistance(sonarLeft);
    delay(15);
    long distRight = readDistance(sonarRight);

    if (distLeft < SIDE_LOST && distRight < SIDE_LOST) {
      mode = WALL_FOLLOW;
      Serial.println("-> Switching to WALL_FOLLOW");
      return;
    }
  }

  // ── line-follow steering (unchanged) ──
  if ((l == 1) && (c == 0) && (r == 1))
    forward();
  else if ((l == 1) && (c == 0) && (r == 0))
    turnRight();
  else if ((l == 0) && (c == 0) && (r == 1))
    turnLeft();
  else if ((l == 1) && (c == 1) && (r == 0))
    turnRight();
  else if ((l == 0) && (c == 1) && (r == 1))
    turnLeft();
  else if ((l == 1) && (c == 1) && (r == 1))
    back();
  else if ((l == 1) && (c == 0) && (r == 1))
    forward();
  else if ((l == 0) && (c == 0) && (r == 0))
    Sforward();   // no line, no walls close yet -> creep forward and keep looking
  else
    stopRobot();
}

// ─── WALL FOLLOW ────────────────────────────────────────────────────────────
void wallFollow() {
  long distLeft  = readDistance(sonarLeft);
  delay(15);
  long distFront = readDistance(sonarFront);
  delay(15);
  long distRight = readDistance(sonarRight);

  Serial.print("L: ");
  Serial.print(distLeft);
  Serial.print(" cm\tF: ");
  Serial.print(distFront);
  Serial.print(" cm\tR: ");
  Serial.print(distRight);
  Serial.print(" cm\t-> ");



  // No wall in front, both sides fine -> go straight
  if (distFront > FRONT_OPEN && distLeft > SIDE_CLOSE && distRight > SIDE_CLOSE) {
    setMotors(BASE_SPEED + 20 , BASE_SPEED + 20 );
  }
  // Too close to left wall -> steer right
  else if (distFront > FRONT_OPEN && distLeft < SIDE_CLOSE && distRight > SIDE_CLOSE) {
    setMotors(TURN_SPEED -8 , -TURN_SPEED);
  }
  // Too close to right wall -> steer left
  else if (distFront > FRONT_OPEN && distLeft > SIDE_CLOSE && distRight < SIDE_CLOSE) {
    setMotors(-TURN_SPEED, TURN_SPEED - 8);
  }
  // Front blocked -> turn toward the more open side
  else if (distFront < FRONT_BLOCK && distLeft > distRight) {
    setMotors(-TURN_SPEED, TURN_SPEED +8 );
    delay(150);
  }
  else if (distFront < FRONT_BLOCK && distRight > distLeft) {
    setMotors(TURN_SPEED + 8, -TURN_SPEED);
    delay(150);
  }
  else {
    setMotors(BASE_SPEED, BASE_SPEED);
  }

  delay(20);
}

// ─── SETUP ────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);

  pinMode(left_ir, INPUT);
  pinMode(center_ir, INPUT);
  pinMode(right_ir, INPUT);

  pinMode(leftmotor1, OUTPUT);
  pinMode(leftmotor2, OUTPUT);
  pinMode(rightmotor1, OUTPUT);
  pinMode(rightmotor2, OUTPUT);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

   SerialBT.begin("Sudip");
  // Serial.println("Bluetooth Started");
  // Serial.println("Boot Mode: LINE_FOLLOW");
}

// ─── LOOP ─────────────────────────────────────────────────────────────────
void loop() {
  bluetoothControl();

  // If manual mode is ON, don't run automatic code
  if (manualMode)
    return;

  // Automatic mode
  if (mode == LINE_FOLLOW)
    lineFollow();
  else
    wallFollow();
}