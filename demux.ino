int data1 = 3;
int data2 = 4;
int data3 = 5;
int data4 = 6;
int clk = 13;
int i;

void clkpulse(int dur) {
  for (int i=0;i<dur;i++){
    digitalWrite(clk,HIGH);
    delayMicroseconds(100);
    digitalWrite(clk,LOW);
    delayMicroseconds(100);
  }
}

void setup() {
  pinMode(data1,OUTPUT);
  pinMode(data2,OUTPUT);
  pinMode(data3,OUTPUT);
  pinMode(data4,OUTPUT);

  Serial.begin(9600);
}

void loop() {
  
  //set S0 to high
  Serial.println("S0");
  digitalWrite(data1,LOW);
  digitalWrite(data2,LOW);
  digitalWrite(data3,LOW);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

  //set S1 to high
  Serial.println("S1");
  digitalWrite(data1,HIGH);
  digitalWrite(data2,LOW);
  digitalWrite(data3,LOW);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

  //set S2 to high
  Serial.println("S2");
  digitalWrite(data1,LOW);
  digitalWrite(data2,HIGH);
  digitalWrite(data3,LOW);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

    //set S3 to high
  Serial.println("S3");
  digitalWrite(data1,HIGH);
  digitalWrite(data2,HIGH);
  digitalWrite(data3,LOW);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

    //set S4 to high
  Serial.println("S4");
  digitalWrite(data1,LOW);
  digitalWrite(data2,LOW);
  digitalWrite(data3,HIGH);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

    //set S5 to high
  Serial.println("S5");
  digitalWrite(data1,HIGH);
  digitalWrite(data2,LOW);
  digitalWrite(data3,HIGH);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

    //set S6 to high
  Serial.println("S6");
  digitalWrite(data1,LOW);
  digitalWrite(data2,HIGH);
  digitalWrite(data3,HIGH);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

    //set S7 to high
  Serial.println("S7");
  digitalWrite(data1,HIGH);
  digitalWrite(data2,HIGH);
  digitalWrite(data3,HIGH);
  digitalWrite(data4,LOW);
  clkpulse(10000); //waits 2 secs

}
