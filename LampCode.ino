
int redpin = 11;
int bluepin = 9;
int greenpin = 10;

int reddial = 1;
int greendial = 3;
int bluedial = 5;

int buttonpin = 8;

double valr;
double valg;
double valb;

int timeCounter = 0;
double timeChoice;

int buttonState = 0;
int buttonCounter = 0;
int lastButtonState = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(redpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(buttonpin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  off();
}

void off() {
  Serial.println("o");
  analogWrite(redpin, 0); 
  analogWrite(greenpin, 0); 
  analogWrite(bluepin, 0);     
  buttonState = digitalRead(buttonpin);
  if (buttonState != lastButtonState){
    if (buttonState == HIGH) {
      lastButtonState = buttonState;
      lightDials();
    }
  }
  delay(50);
}

void lightDials() {
  Serial.println("LD");
  valr = analogRead(reddial);
  valg = analogRead(greendial);
  valb = analogRead(bluedial);
  analogWrite(redpin, valr / 1024 * 254); 
  analogWrite(greenpin, valg / 1024 * 254); 
  analogWrite(bluepin, valb / 1024 * 254); 
  Serial.println(valr);
  Serial.println(valg);
  Serial.println(valb);
  if (buttonState != lastButtonState){
    if (buttonState == HIGH) {
      lastButtonState = buttonState;
      autoScroll();
    }
  }
  delay(10);
}

void autoScroll() {
  Serial.println("AS");
  timeChoice = 10 + 0.25 * analogRead(greendial);
  timeCounter++;
  if (timeCounter < 255){
    analogWrite(redpin, 255 - timeCounter);
    analogWrite(greenpin, timeCounter);
    analogWrite(bluepin, 0);
  }
  else if (timeCounter < 510){
    analogWrite(redpin, 0);
    analogWrite(greenpin, 510 - timeCounter);
    analogWrite(bluepin, timeCounter - 255);
  }
  else if (timeCounter < 765){
    analogWrite(redpin, timeCounter - 510);
    analogWrite(greenpin, 0);
    analogWrite(bluepin, 765 - timeCounter);
  }
  else if (timeCounter == 765){
    timeCounter = 0;
  }
  if (buttonState != lastButtonState){
    if (buttonState == HIGH) {
      lastButtonState = buttonState;
      off();
    }
  }
  delay(50);
}
