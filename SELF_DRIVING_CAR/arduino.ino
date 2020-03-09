char data;
const int EnableL = 5;
const int HighL = 6;       // LEFT SIDE MOTOR
const int LowL =7;

const int EnableR = 10;
const int HighR = 8;       //RIGHT SIDE MOTOR
const int LowR =9;

void setup() {
  Serial.begin(9600);
}

void Forward()
{
  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  analogWrite(EnableL,255);

  digitalWrite(HighR, HIGH);
  digitalWrite(LowR, LOW);
  analogWrite(EnableR,255);
  
}

void sideChange()
{
  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);
  delay(800);

  analogWrite(EnableL, 250);
  analogWrite(EnableR, 250);    //forward
  delay(1400);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);
  delay(800);

  digitalWrite(HighL, HIGH);
  digitalWrite(LowL, LOW);
  digitalWrite(HighR, LOW);   //   left
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  delay(1100);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);
  delay(800);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);   // forward
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  delay(1300);

  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);
  delay(800);

  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);    //Right
  digitalWrite(LowR, HIGH);
  analogWrite(EnableL, 255);
  analogWrite(EnableR, 255);
  delay(1100);


  analogWrite(EnableL, 0);
  analogWrite(EnableR, 0);
  delay(1400);



  digitalWrite(HighL, LOW);
  digitalWrite(LowL, HIGH);
  digitalWrite(HighR, LOW);  // forward
  digitalWrite(LowL, HIGH);
  analogWrite(EnableL, 150);
  analogWrite(EnableR, 150);
  delay(700);
}

void loop() 
{ 
    if(Serial.available()!=0)
    {
        data = Serial.read();
    }

    if(data=='1')
     {
         
         sideChange();
         Serial.println(data);
         
       
       
     }
     else{
       Forward();
       Serial.println(" car is moving forward");
     }

     

    
     
}
