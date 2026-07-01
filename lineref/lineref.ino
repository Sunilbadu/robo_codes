#define left_ir    34
#define center_ir  4
#define right_ir   15

#define leftmotor1  25
#define leftmotor2  26
#define rightmotor1 27
#define rightmotor2 14

#define ENA 33
#define ENB 32

int leftMotorSpeed = 72;
int rightMotorSpeed = 72;


void forward() {
  analogWrite(ENA, leftMotorSpeed);
  analogWrite(ENB, rightMotorSpeed);

  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}


void turnLeft() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 180);

  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, HIGH);
  digitalWrite(rightmotor2, LOW);
}

void back(){
  analogWrite(ENA, leftMotorSpeed);
  analogWrite(ENB, rightMotorSpeed);

  digitalWrite(leftmotor1, LOW);
  digitalWrite(leftmotor2, HIGH);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, HIGH);
}

void turnRight() {
  analogWrite(ENA, 180);
  analogWrite(ENB,0);

  digitalWrite(leftmotor1, HIGH);
  digitalWrite(leftmotor2, LOW);
  digitalWrite(rightmotor1, LOW);
  digitalWrite(rightmotor2, LOW);
}


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
}

void loop(){
   lineFollow();
}

void lineFollow(){
  int l = digitalRead(left_ir);
  int c = digitalRead(center_ir);
  int r = digitalRead(right_ir);
if( (l==1) && (c==0 )&&(r==1)){
    forward();
  }
  else if((l == 1) && (c == 0) && (r == 0)){
    // delay(200);
    turnRight();

  }
  else if ((l == 0) && (c == 0) && (r == 1)){
  //delay(200);
    turnLeft();
   
}
else if ((l == 1) && (c == 1) && (r == 0)){
// delay(200);
    turnRight();
}

  else if ((l == 0) && (c == 1) && (r == 1)){
    // delay(200);
    turnLeft();
}
else if((l==0 )&&(c==0)&&(r==0)){
  back();
}
else if((l==1 )&&(c==1)&&(r==1)){
 back();
}

}
