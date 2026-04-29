//Conexion a WIFI (pendiente de terminar)
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";

WiFiClient   espClient;
PubSubClient client(espClient);


//Definir los pines de la ESP (pendiente) ------------------------------------
#define norter 5
#define nortea 5
#define nortev 5

#define surr 5
#define sura 5
#define surv 5

#define ester 5
#define estea 5
#define estev 5

#define oester 5
#define oestea 5
#define oestev 5


#define norte_ir_ade 5
#define norte_ir_atr 5

#define sur_ir_ade 5
#define sur_ir_atr 5

#define este_ir_ade 5
#define este_ir_atr 5

#define oeste_ir_ade 5
#define oeste_ir_atr 5

// Definiendo los semáforos --------------------------

int SEM_Norte[3] = {norter, nortea, nortev};

int SEM_Sur[3] = {surr, sura, surv};

int SEM_Este[3] = {ester, estea, estev};

int SEM_Oeste[3] = {oester, oestea, oestev};



// Conteo de carros - variables --------------------
int pasaron_norte = 0;
int pasaron_sur   = 0;
int pasaron_este  = 0;
int pasaron_oeste = 0;

bool este_atr_ante = 1;
bool oeste_atr_ante = 1;
bool norte_atr_ante = 1;
bool sur_atr_ante = 1;

long tiempo_conteo_este = 0;
long tiempo_conteo_oeste = 0;
long tiempo_conteo_norte = 0;
long tiempo_conteo_sur = 0;

bool flag_norte_c = 0;
bool flag_sur_c = 0;
bool flag_este_c = 0;
bool flag_oeste_c = 0;

// Hay carros esperando? variables-------------------
bool espera_norte_sur = false;
bool espera_este_oeste = false;

bool flag_norte = 0;
bool flag_sur = 0;
bool flag_este = 0;
bool flag_oeste = 0;

long tiempo_espera_este = 0;
long tiempo_espera_oeste = 0;
long tiempo_espera_norte = 0;
long tiempo_espera_sur = 0;

bool norte_ante = 0;
bool sur_ante = 0;
bool este_ante = 0;
bool oeste_ante = 0;
//------------------------------------------


//Funciones para saber si hay carros esperando, nos sirven las dos para las que se enfrentan.------------------------


bool hayAutos_n_s() {
//NORTE_______________
  if((norte_ante == HIGH) && (digitalRead(norte_ir_ade) == LOW)){
    tiempo_espera_norte = millis();
    flag_norte = 1;
  }
  if((norte_ante == LOW) && (digitalRead(norte_ir_ade) == HIGH)){
    flag_norte = 0;
  }
  if((millis() - tiempo_espera_norte > 3000) && (flag_norte == 1)){
    flag_norte = 0;
    return true;
  }

 //SUR_________________
  if((sur_ante == HIGH) && (digitalRead(sur_ir_ade) == LOW)){
    tiempo_espera_sur = millis();
    flag_sur = 1;
  }
  if((sur_ante == LOW) && (digitalRead(sur_ir_ade) == HIGH)){
    flag_sur = 0;
  }
  if((millis() - tiempo_espera_sur > 3000) && (flag_sur == 1)){
    flag_sur = 0;
    return true;
  }
  norte_ante = (digitalRead(norte_ir_ade));
  sur_ante   = (digitalRead(sur_ir_ade));
  return false;
}


bool hayAutos_e_o(){
  //ESTE__________

  if((este_ante == HIGH) && (digitalRead(este_ir_ade) == LOW)){
    tiempo_espera_este = millis();
    flag_este = 1;
  }
  if((este_ante == LOW) && (digitalRead(este_ir_ade) == HIGH)){
    flag_este = 0;
  }
  if((millis() - tiempo_espera_este > 3000) && (flag_este == 1)){
    flag_este = 0;
    return true;
  }

 //OESTE_________________
  if((oeste_ante == HIGH) && (digitalRead(oeste_ir_ade) == LOW)){
    tiempo_espera_oeste = millis();
    flag_oeste = 1;
  }
  if((oeste_ante == LOW) && (digitalRead(oeste_ir_ade) == HIGH)){
    flag_oeste = 0;
  }
  if((millis() - tiempo_espera_oeste > 3000) && (flag_oeste == 1)){
    flag_oeste = 0;
    return true;
  }
  este_ante  = (digitalRead(este_ir_ade));
  oeste_ante = (digitalRead(oeste_ir_ade));
  return false;
}


//funciones para contar los carros que van pasando en cada calle.---------------------------------------
//NORTE--------------------------------------------
void contar_autos_norte(){
  if ((norte_atr_ante == HIGH) && (digitalRead(norte_ir_atr) == LOW)) {
    tiempo_conteo_norte = millis();
    flag_norte_c = 1;
  }
  if((norte_atr_ante == LOW) && (digitalRead(norte_ir_atr) == HIGH)){
    flag_norte_c = 0;
  }
  if(((millis() - tiempo_conteo_norte) > 150) && (flag_norte_c == 1)){
    flag_norte_c = 0;
    pasaron_norte = pasaron_norte + 1;
  }
  norte_atr_ante = digitalRead(norte_ir_atr);
}

//-------------------------SUR--------------------------
void contar_autos_sur(){
  if ((sur_atr_ante == HIGH) && (digitalRead(sur_ir_atr) == LOW)) {
    tiempo_conteo_sur = millis();
    flag_sur_c = 1;
  }
  if((sur_atr_ante == LOW) && (digitalRead(sur_ir_atr) == HIGH)){
    flag_sur_c = 0;
  }
  if(((millis() - tiempo_conteo_sur) > 150) && (flag_sur_c == 1)){
    flag_sur_c = 0;
    pasaron_sur = pasaron_sur + 1;
  }
  sur_atr_ante = digitalRead(sur_ir_atr);
}

//-------------------------ESTE--------------------------
void contar_autos_este(){
  if ((este_atr_ante == HIGH) && (digitalRead(este_ir_atr) == LOW)) {
    tiempo_conteo_este = millis();
    flag_este_c = 1;
  }
  if((este_atr_ante == LOW) && (digitalRead(este_ir_atr) == HIGH)){
    flag_este_c = 0;
  }
  if(((millis() - tiempo_conteo_este) > 150) && (flag_este_c == 1)){
    flag_este_c = 0;
    pasaron_este = pasaron_este + 1;
  }
  este_atr_ante = digitalRead(este_ir_atr);
}

//-------------------------OESTE--------------------------
void contar_autos_oeste(){
  if ((oeste_atr_ante == HIGH) && (digitalRead(oeste_ir_atr) == LOW)) {
    tiempo_conteo_oeste = millis();
    flag_oeste_c = 1;
  }
  if((oeste_atr_ante == LOW) && (digitalRead(oeste_ir_atr) == HIGH)){
    flag_oeste_c = 0;
  }
  if(((millis() - tiempo_conteo_oeste) > 150) && (flag_oeste_c == 1)){
    flag_oeste_c = 0;
    pasaron_oeste = pasaron_oeste + 1;
  }
  oeste_atr_ante = digitalRead(oeste_ir_ade);
}



void tiempos(){
  
}




void setup() {
  Serial.begin(115200);

  // Salidas semáforos------------------------
  pinMode(norter, OUTPUT); pinMode(nortea, OUTPUT); pinMode(nortev, OUTPUT);
  pinMode(surr,   OUTPUT); pinMode(sura,   OUTPUT); pinMode(surv,   OUTPUT);
  pinMode(ester,  OUTPUT); pinMode(estea,  OUTPUT); pinMode(estev,  OUTPUT);
  pinMode(oester, OUTPUT); pinMode(oestea, OUTPUT); pinMode(oestev, OUTPUT);

  // Entradas sensores-------------------------
  pinMode(norte_ir_atr, INPUT); pinMode(norte_ir_ade, INPUT);
  pinMode(sur_ir_atr,   INPUT); pinMode(sur_ir_ade,   INPUT);
  pinMode(este_ir_atr,  INPUT); pinMode(este_ir_ade,  INPUT);
  pinMode(oeste_ir_atr, INPUT); pinMode(oeste_ir_ade, INPUT);






}

void loop() {

}
