#define LED1 5
#define LED2 6
#define LED3 7
#define LED4 8

#define SENT1 1
#define SENT2 2
#define SENT3 3
#define SENT4 4

#define SEND1 11
#define SEND2 12
#define SEND3 13
#define SEND4 14


void setup() {
    
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);

  pinMode(SENT1, INPUT);
  pinMode(SENT2, INPUT);
  pinMode(SENT3, INPUT);
  pinMode(SENT4, INPUT);

  pinMode(SEND1, INPUT);
  pinMode(SEND2, INPUT);
  pinMode(SEND3, INPUT);
  pinMode(SEND4, INPUT);

Serial.begin(9600);

}

int contador1(bool control1){

  bool ante1 = digitalRead(SENT1);
  int cont1 = 0;

  if ((digitalRead(SENT1) == HIGH) && (ante1 == LOW)){
    cont1 = cont1 + 1;
    return cont1
  }
  if(control1 == 1){
    cont1 = 0;
    control1 = 0;
  }
}

int contador2(bool control2){

  bool ante2 = digitalRead(SENT2);
  int cont2 = 0;

  if ((digitalRead(SENT2) == HIGH) && (ante2 == LOW)){
    cont2 = cont2 + 1;
    return cont2
  }
  if(control2 == 1){
    cont2 = 0;
    control2 = 0;
  }
}

int contador3(bool control3){

  bool ante3 = digitalRead(SENT3);
  int cont3 = 0;

  if ((digitalRead(SENT3) == HIGH) && (ante3 == LOW)){
    cont3 = cont3 + 1;
    return cont3
  }
  if(control3 == 1){
    cont3 = 0;
    control3 = 0;
  }
}

int contador4(bool control4){

  bool ante4 = digitalRead(SENT4);
  int cont4 = 0;

  if ((digitalRead(SENT4) == HIGH) && (ante4 == LOW)){
    cont4 = cont4 + 1;
    return cont4
  }
  if(control4 == 1){
    cont4 = 0;
    control4 = 0;
  }
}






void loop() {

}
