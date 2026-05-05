//Conexion a WIFI (pendiente de terminar)
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";

const char* mqtt_server = "broker.hivemq.com";

WiFiClient   espClient;
PubSubClient client(espClient);


//PAra los reportes -----------------
bool modo_manual = false; 
long ultimo_reporte = 0;



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
unsigned int pasaron_norte = 0;
unsigned int pasaron_sur = 0;
unsigned int pasaron_este = 0;
unsigned int pasaron_oeste = 0;

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
//------------------------NORTE-SUR-------------------------
bool hayAutos_n_s() {

  bool lectura_norte = digitalRead(norte_ir_ade);
  bool lectura_sur   = digitalRead(sur_ir_ade);
  //NORTE_______________
  if((norte_ante == HIGH) && (digitalRead(norte_ir_ade) == LOW)){
    tiempo_espera_norte = millis();
    flag_norte = 1;
  }
  if((norte_ante == LOW) && (digitalRead(norte_ir_ade) == HIGH)){
    flag_norte = 0;
  }
  if((millis() - tiempo_espera_norte > 2000) && (flag_norte == 1)){
    flag_norte = 0;
    norte_ante = lectura_norte; 
    sur_ante   = lectura_sur;
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
  if((millis() - tiempo_espera_sur > 2000) && (flag_sur == 1)){
    flag_sur = 0;
    sur_ante   = lectura_sur;
    norte_ante = lectura_norte; 
    return true;
  }
  norte_ante = lectura_norte;
  sur_ante   = lectura_sur;
  return false;
}

//-------------------------ESTE-OESTE--------------------------
bool hayAutos_e_o(){

  bool lectura_este = digitalRead(este_ir_ade);
  bool lectura_oeste   = digitalRead(oeste_ir_ade);

  //ESTE__________

  if((este_ante == HIGH) && (digitalRead(este_ir_ade) == LOW)){
    tiempo_espera_este = millis();
    flag_este = 1;
  }
  if((este_ante == LOW) && (digitalRead(este_ir_ade) == HIGH)){
    flag_este = 0;
  }
  if((millis() - tiempo_espera_este > 2000) && (flag_este == 1)){
    flag_este = 0;
    este_ante = lectura_este;
    oeste_ante = lectura_oeste; 
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
  if((millis() - tiempo_espera_oeste > 2000) && (flag_oeste == 1)){
    este_ante = lectura_este;
    oeste_ante = lectura_oeste; 
    flag_oeste = 0;
    return true;
  }
  este_ante = lectura_este;
  oeste_ante = lectura_oeste; 
  return false;
}


//funciones para contar los carros que van pasando en cada calle.---------------------------------------
//------------------------NORTE-------------------------
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
  oeste_atr_ante = digitalRead(oeste_ir_atr);
}



int tiempos(unsigned int car1, unsigned int car2, bool hayo){
  int cart = 0;
  int clo = 0;

  if(car1 == 0 && car2 == 0){
    return 1;
  }

  if(car1 > car2){
    cart = car1;
  }
  else if(car1 < car2 || car1 == car2){
    cart = car2;
  }

  
  if((hayo == false) && (cart <= 6)){
    clo = 3 + cart;
  }
  else if((hayo == false) && (cart > 6)){
    clo = 9;
  }


  if((hayo == true) && (cart <= 6)){
    clo = 3 + (cart / 2);
  }
  else if((hayo == true) && (cart > 6)){
    clo = 7;
  }

  return clo;
}


void actualizarSemaforo(int semaforo[], int r, int a, int v) {
    digitalWrite(semaforo[0], r);
    digitalWrite(semaforo[1], a);
    digitalWrite(semaforo[2], v);
}


void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

// --- 2. Recibir comandos desde Node-RED ---
void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";
  for (int i = 0; i < length; i++) {
    mensaje += (char)payload[i];
  }
  
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(mensaje);

  // Lógica para tomar control remoto
  if (String(topic) == "semaforo/control") {
    if (mensaje == "AUTO") {
      modo_manual = false;
      Serial.println("Regresando a modo automático");
    } 
    else if (mensaje == "MANUAL_NS") {
      modo_manual = true;
      actualizarSemaforo(SEM_Norte, 0, 0, 1); // Verde N-S
      actualizarSemaforo(SEM_Sur,   0, 0, 1); 
      actualizarSemaforo(SEM_Este,  1, 0, 0); // Rojo E-O
      actualizarSemaforo(SEM_Oeste, 1, 0, 0); 
    }
    else if (mensaje == "MANUAL_EO") {
      modo_manual = true;
      actualizarSemaforo(SEM_Norte, 1, 0, 0); // Rojo N-S
      actualizarSemaforo(SEM_Sur,   1, 0, 0); 
      actualizarSemaforo(SEM_Este,  0, 0, 1); // Verde E-O
      actualizarSemaforo(SEM_Oeste, 0, 0, 1); 
    }
  }
}

// --- 3. Mantener conexión MQTT viva ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    // Intentar conectar (usando un ID aleatorio)
    String clientId = "ESP32Client-" + String(random(0, 1000));
    if (client.connect(clientId.c_str())) {
      Serial.println("conectado");
      // Suscribirse al tópico de control en cuanto se conecta
      client.subscribe("semaforo/control");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}

// --- 4. Reportar datos a Node-RED ---
void reportar_datos() {
  if (client.connected()) {
    // Reportar conteo de autos 
    client.publish("semaforo/norte/conteo", String(pasaron_norte).c_str());
    client.publish("semaforo/sur/conteo", String(pasaron_sur).c_str());
    client.publish("semaforo/este/conteo", String(pasaron_este).c_str());
    client.publish("semaforo/oeste/conteo", String(pasaron_oeste).c_str());

    // Reportar si hay espera (1 = Sí, 0 = No)
    client.publish("semaforo/norte_sur/espera", espera_norte_sur ? "1" : "0");
    client.publish("semaforo/este_oeste/espera", espera_este_oeste ? "1" : "0");
  }
}


void setup() {
  Serial.begin(115200);

//WIFI-----------------------------
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

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

int estado = 1;
int anterior = 2;
int control = 0;
long tiempo_control_loop = 0;
int tie = 0;

unsigned int carros_antes_norte = 0;
unsigned int carros_antes_sur = 0;
unsigned int carros_antes_oeste = 0;
unsigned int carros_antes_este = 0;



void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if(millis() - ultimo_reporte > 5000) {
    reportar_datos();
    ultimo_reporte = millis();
  }

  espera_norte_sur = hayAutos_n_s();
  espera_este_oeste = hayAutos_e_o();  
  contar_autos_norte();
  contar_autos_sur();
  contar_autos_este();
  contar_autos_oeste();



//---------------------Norte-Sur-------------------------

  if (!modo_manual) {

    if(estado == 0){
      anterior = estado;

      if(control == 0){
        tiempo_control_loop = millis();
        control = 1;

        carros_antes_oeste = pasaron_oeste;
        carros_antes_este = pasaron_este;

        carros_antes_norte = pasaron_norte - carros_antes_norte;
        carros_antes_sur = pasaron_sur - carros_antes_sur;

        tie = (tiempos(carros_antes_norte, carros_antes_sur, espera_este_oeste) * 1000);
      }
      
      actualizarSemaforo(SEM_Norte, 0, 0, 1);
      actualizarSemaforo(SEM_Sur,  0, 0, 1); 
      actualizarSemaforo(SEM_Este, 1, 0, 0); 
      actualizarSemaforo(SEM_Oeste, 1, 0, 0);  


      if((millis() - tiempo_control_loop) >= tie){
          estado = 1;
          control = 0;
      }
    }


  //-------------------El intermedio-------------------------
    else if(estado == 1){

      actualizarSemaforo(SEM_Norte, LOW, HIGH, LOW);
      actualizarSemaforo(SEM_Sur, LOW, HIGH, LOW); 
      actualizarSemaforo(SEM_Este, LOW, HIGH, LOW); 
      actualizarSemaforo(SEM_Oeste, LOW, HIGH, LOW); 

      if(control == 0){
        tiempo_control_loop = millis();
        control = 1;
      }

      else if((millis() - tiempo_control_loop) > 2500){
        control = 0;
        if(anterior == 2){
          estado = 0;
        }
        else if(anterior == 0){
          estado = 2;
        }
      }
  }


  //-----------------------Oeste-Este--------------------------
    else if(estado == 2){

      if(control == 0){
        tiempo_control_loop = millis();
        control = 1;
        anterior = estado;

        carros_antes_norte = pasaron_norte;
        carros_antes_sur = pasaron_sur;

        carros_antes_este = pasaron_este - carros_antes_este;
        carros_antes_oeste = pasaron_oeste - carros_antes_oeste;

        tie = (tiempos(carros_antes_este, carros_antes_oeste, espera_norte_sur) * 1000);
      }
      

      actualizarSemaforo(SEM_Norte, 1, 0, 0);
      actualizarSemaforo(SEM_Sur,   1, 0, 0); 
      actualizarSemaforo(SEM_Este,  0, 0, 1); 
      actualizarSemaforo(SEM_Oeste, 0, 0, 1);  


      if((millis() - tiempo_control_loop) >= tie){
          estado = 1;
          control = 0;
      }
    }
  }
}


