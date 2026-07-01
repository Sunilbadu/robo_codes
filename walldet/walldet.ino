#define leftmotor1 25
#define leftmotor2 26
#define rightmotor1 27
#define rightmotor2 14
#define ENA 33
#define ENB 32

#define LEFT_TRIG  22
#define LEFT_ECHO  21
#define FRONT_TRIG 19
#define FRONT_ECHO 18
#define RIGHT_TRIG 5
#define RIGHT_ECHO 17

#define MAX_DISTANCE 200  // max distance to report (cm)
#define TIMEOUT_US 30000  // ~200cm round trip, with margin

// Tuning
#define WALL_DIST      15   // desired distance from wall (cm)
#define FRONT_STOP     20   // stop/turn if front obstacle closer than this
#define BASE_SPEED     80
#define TURN_SPEED     70

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

void stopMotors() {
  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, LOW);
}

// Reads one ultrasonic sensor directly, no library.
// Returns distance in cm, or MAX_DISTANCE if no echo (out of range / bad wiring).
long readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, TIMEOUT_US);

  if (duration == 0) {
    return MAX_DISTANCE;
  }

  long distance = duration * 0.0343 / 2;  // speed of sound = 343 m/s
  return distance;
}

void setup() {
  pinMode(leftmotor1, OUTPUT);
  pinMode(leftmotor2, OUTPUT);
  pinMode(rightmotor1, OUTPUT);
  pinMode(rightmotor2, OUTPUT);
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(LEFT_TRIG, OUTPUT);
  pinMode(LEFT_ECHO, INPUT);
  pinMode(FRONT_TRIG, OUTPUT);
  pinMode(FRONT_ECHO, INPUT);
  pinMode(RIGHT_TRIG, OUTPUT);
  pinMode(RIGHT_ECHO, INPUT);

  Serial.begin(115200);
}

void loop() {

  long distLeft  = readDistanceCM(LEFT_TRIG, LEFT_ECHO);
  delay(30);
  long distFront = readDistanceCM(FRONT_TRIG, FRONT_ECHO);
  delay(30);
  long distRight = readDistanceCM(RIGHT_TRIG, RIGHT_ECHO);

  Serial.printf("L:%ld  F:%ld  R:%ld\n", distLeft, distFront, distRight);

  // No wall in front
  if (distFront > FRONT_STOP && distLeft > WALL_DIST && distRight > WALL_DIST) {
    // Go straight
    setMotors(BASE_SPEED, BASE_SPEED);
  }

  // Too close to left wall -> move right
  else if (distFront > FRONT_STOP && distLeft < WALL_DIST && distRight > WALL_DIST) {
    // Slight right adjustment
    setMotors(BASE_SPEED, BASE_SPEED / 2);
  }

  // Too close to right wall -> move left
  else if (distFront > FRONT_STOP && distLeft > WALL_DIST && distRight < WALL_DIST) {
    // Slight left adjustment
    setMotors(BASE_SPEED / 2, BASE_SPEED);
  }

  // Front blocked -> choose left if left side is more open
  else if (distFront < 25 && distLeft > distRight) {
    // Left turn
    setMotors(-TURN_SPEED, TURN_SPEED);
    delay(250);
  }

  // Front blocked -> choose right if right side is more open
  else if (distFront < 25 && distRight > distLeft) {
    // Right turn
    setMotors(TURN_SPEED, -TURN_SPEED);
    delay(250);
  }

  // Default
  else {
    setMotors(BASE_SPEED, BASE_SPEED);
  }

  delay(20);
}