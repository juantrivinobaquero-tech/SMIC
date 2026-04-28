/*
 * ============================================================
 *   PROYECTO SMIC - Semáforo Inteligente Controlado
 *   Escuela Colombiana de Ingeniería Julio Garavito
 *   Autor: Juan José Triviño Baquero
 * ============================================================
 *
 *  PINOUT ESP32-S3
 * ---------------------------------------------------------------
 *  SEMÁFOROS (3 pines cada uno: Rojo, Amarillo, Verde)
 *    Semáforo 1 (Norte) : pines 1, 2, 3
 *    Semáforo 2 (Sur)   : pines 4, 5, 6
 *    Semáforo 3 (Este)  : pines 7, 8, 9
 *    Semáforo 4 (Oeste) : pines 11, 12, 13
 *
 *  SENSORES IR (entrada/salida por calle)
 *    Sensor entrada  calle 1 (Norte): pin 15
 *    Sensor salida   calle 1 (Norte): pin 16
 *    Sensor entrada  calle 2 (Sur)  : pin 17
 *    Sensor salida   calle 2 (Sur)  : pin 18
 *    Sensor entrada  calle 3 (Este) : pin 35
 *    Sensor salida   calle 3 (Este) : pin 36
 *    Sensor entrada  calle 4 (Oeste): pin 37
 *    Sensor salida   calle 4 (Oeste): pin 38
 *
 *  Nota: Los sensores IR de corte dan LOW cuando detectan un carro
 *        (el haz se interrumpe). Se usan transistores para amplificar.
 * ---------------------------------------------------------------
 */

#include <WiFi.h>
#include <PubSubClient.h>

// ── WiFi ────────────────────────────────────────────────────────
const char* ssid     = "sc-17f0";
const char* password = "Q5DN24V4FLYG";

// ── MQTT ─────────────────────────────────────────────────────────
const char* mqtt_server = "broker.hivemq.com";
// Tópicos de publicación (reportes)
#define TOPIC_CONTEO    "smic/conteo"      // carros que pasaron (JSON)
#define TOPIC_ESPERA    "smic/espera"      // carros esperando   (JSON)
#define TOPIC_ESTADO    "smic/estado"      // fase activa actual
// Tópicos de suscripción (control remoto NODE-RED)
#define TOPIC_OVERRIDE  "smic/override"    // forzar fase: "1","2","3","4","auto"
#define TOPIC_TIEMPOS   "smic/tiempos"     // JSON con tiempos personalizados

WiFiClient   espClient;
PubSubClient client(espClient);

// ── PINES SEMÁFOROS ──────────────────────────────────────────────
// Cada semáforo: {Rojo, Amarillo, Verde}
const int SEM[4][3] = {
  {1,  2,  3 },   // 0 = Norte
  {4,  5,  6 },   // 1 = Sur
  {7,  8,  9 },   // 2 = Este
  {11, 12, 13}    // 3 = Oeste
};

// ── PINES SENSORES IR ────────────────────────────────────────────
// {sensor_entrada, sensor_salida}
// LOW = carro detectado (haz cortado)
const int SNS_ENT[4] = {15, 17, 35, 37};
const int SNS_SAL[4] = {16, 18, 36, 38};

// ── ESTADOS DE SEMÁFORO ──────────────────────────────────────────
#define ROJO    0
#define AMARILLO 1
#define VERDE   2

// ── TIEMPOS BASE (ms) ────────────────────────────────────────────
// Se pueden modificar desde NODE-RED vía MQTT
unsigned long T_VERDE    = 5000;
unsigned long T_AMARILLO = 2000;
unsigned long T_ROJO_MIN = 3000;   // tiempo mínimo de rojo garantizado

// ── LÓGICA DE FASES ──────────────────────────────────────────────
// Un cruce de 4 calles con movimiento recto:
//   Fase 1: Norte-Sur en verde  |  Este-Oeste en rojo
//   Fase 2: Amarillo N-S        |  rojo E-O
//   Fase 3: Este-Oeste en verde |  Norte-Sur en rojo
//   Fase 4: Amarillo E-O        |  rojo N-S
//
//  Índice de fase: 0..3

int  faseActual   = 0;
bool modoOverride = false;   // true = control manual desde NODE-RED
int  faseOverride = 0;

unsigned long tiempoFase = 0;  // marca de inicio de la fase actual

// ── CONTADORES ───────────────────────────────────────────────────
volatile int  carrosPasaron[4] = {0, 0, 0, 0};  // conteo acumulado
volatile int  carrosEspera[4]  = {0, 0, 0, 0};  // cola actual

// ── DEBOUNCE SENSORES ─────────────────────────────────────────────
unsigned long lastTrigENT[4] = {0,0,0,0};
unsigned long lastTrigSAL[4] = {0,0,0,0};
bool estadoAnteriorENT[4]    = {true,true,true,true};  // HIGH = sin carro
bool estadoAnteriorSAL[4]    = {true,true,true,true};
#define DEBOUNCE_MS 80

// ── REPORTE MQTT (cada N ms) ──────────────────────────────────────
unsigned long ultimoReporte = 0;
#define INTERVALO_REPORTE 3000

// ─────────────────────────────────────────────────────────────────
//  FUNCIONES DE SEMÁFORO
// ─────────────────────────────────────────────────────────────────

void setSemaforo(int idx, int estado) {
  // Apaga los 3 colores y enciende el pedido
  for (int c = 0; c < 3; c++) digitalWrite(SEM[idx][c], LOW);
  digitalWrite(SEM[idx][estado], HIGH);
}

// Aplica la fase actual a los 4 semáforos
// Fases: 0=Verde N-S / Rojo E-O   1=Amarillo N-S / Rojo E-O
//        2=Verde E-O / Rojo N-S   3=Amarillo E-O / Rojo N-S
void aplicarFase(int fase) {
  switch (fase) {
    case 0:
      setSemaforo(0, VERDE);    // Norte
      setSemaforo(1, VERDE);    // Sur
      setSemaforo(2, ROJO);     // Este
      setSemaforo(3, ROJO);     // Oeste
      break;
    case 1:
      setSemaforo(0, AMARILLO);
      setSemaforo(1, AMARILLO);
      setSemaforo(2, ROJO);
      setSemaforo(3, ROJO);
      break;
    case 2:
      setSemaforo(0, ROJO);
      setSemaforo(1, ROJO);
      setSemaforo(2, VERDE);
      setSemaforo(3, VERDE);
      break;
    case 3:
      setSemaforo(0, ROJO);
      setSemaforo(1, ROJO);
      setSemaforo(2, AMARILLO);
      setSemaforo(3, AMARILLO);
      break;
  }
}

// Duración de cada fase según tráfico
unsigned long duracionFase(int fase) {
  if (fase == 1 || fase == 3) return T_AMARILLO;  // fases amarillo: fijas

  // Para fases verdes: ajusta tiempo según cola en las calles activas
  // Fase 0 → N(0) y S(1) activos; Fase 2 → E(2) y O(3) activos
  int a = (fase == 0) ? 0 : 2;
  int b = a + 1;
  int colaActiva = carrosEspera[a] + carrosEspera[b];

  // +1 segundo por cada 2 carros en espera, máximo +10 s
  long extra = (colaActiva / 2) * 1000L;
  if (extra > 10000) extra = 10000;
  return T_VERDE + extra;
}

// ─────────────────────────────────────────────────────────────────
//  SENSORES IR  (polling con debounce, sin interrupciones de HW
//                para no saturar el scheduler de MQTT)
// ─────────────────────────────────────────────────────────────────

void leerSensores() {
  unsigned long ahora = millis();
  for (int i = 0; i < 4; i++) {

    // ── Sensor de entrada (carro llega a esperar) ─────────────────
    bool entActual = digitalRead(SNS_ENT[i]);  // LOW = carro
    if (!entActual && estadoAnteriorENT[i] && (ahora - lastTrigENT[i] > DEBOUNCE_MS)) {
      // Flanco descendente → carro entrando
      carrosEspera[i]++;
      lastTrigENT[i] = ahora;
    }
    estadoAnteriorENT[i] = entActual;

    // ── Sensor de salida (carro cruzó) ────────────────────────────
    bool salActual = digitalRead(SNS_SAL[i]);
    if (!salActual && estadoAnteriorSAL[i] && (ahora - lastTrigSAL[i] > DEBOUNCE_MS)) {
      carrosPasaron[i]++;
      if (carrosEspera[i] > 0) carrosEspera[i]--;
      lastTrigSAL[i] = ahora;
    }
    estadoAnteriorSAL[i] = salActual;
  }
}

// ─────────────────────────────────────────────────────────────────
//  MQTT  – callback (mensajes entrantes desde NODE-RED)
// ─────────────────────────────────────────────────────────────────

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) msg += (char)payload[i];

  // ── Control de override ───────────────────────────────────────
  if (String(topic) == TOPIC_OVERRIDE) {
    if (msg == "auto") {
      modoOverride = false;
      Serial.println("[MQTT] Modo automático restaurado");
    } else {
      int f = msg.toInt();
      if (f >= 1 && f <= 4) {
        faseOverride = f - 1;  // NODE-RED envía 1-4, internamente 0-3
        modoOverride  = true;
        aplicarFase(faseOverride);
        tiempoFase = millis();
        Serial.print("[MQTT] Override fase: "); Serial.println(f);
      }
    }
  }

  // ── Ajuste de tiempos ─────────────────────────────────────────
  // Formato esperado: "verde=6000,amarillo=2500"
  if (String(topic) == TOPIC_TIEMPOS) {
    int iv = msg.indexOf("verde=");
    int ia = msg.indexOf("amarillo=");
    if (iv != -1) T_VERDE    = msg.substring(iv + 6).toInt();
    if (ia != -1) T_AMARILLO = msg.substring(ia + 9).toInt();
    Serial.print("[MQTT] Tiempos → verde="); Serial.print(T_VERDE);
    Serial.print(" ms, amarillo="); Serial.println(T_AMARILLO);
  }
}

// ─────────────────────────────────────────────────────────────────
//  WiFi + MQTT
// ─────────────────────────────────────────────────────────────────

void setup_wifi() {
  Serial.print("Conectando a WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.print("\nIP: "); Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando MQTT...");
    if (client.connect("ESP32_SMIC")) {
      Serial.println(" OK");
      client.subscribe(TOPIC_OVERRIDE);
      client.subscribe(TOPIC_TIEMPOS);
    } else {
      Serial.print(" fallo, rc="); Serial.println(client.state());
      delay(2000);
    }
  }
}

// ─────────────────────────────────────────────────────────────────
//  REPORTE MQTT
// ─────────────────────────────────────────────────────────────────

void publicarReporte() {
  // Conteo de carros que pasaron
  char buf[128];
  snprintf(buf, sizeof(buf),
    "{\"N\":%d,\"S\":%d,\"E\":%d,\"O\":%d}",
    carrosPasaron[0], carrosPasaron[1], carrosPasaron[2], carrosPasaron[3]);
  client.publish(TOPIC_CONTEO, buf);

  // Carros en espera
  snprintf(buf, sizeof(buf),
    "{\"N\":%d,\"S\":%d,\"E\":%d,\"O\":%d}",
    carrosEspera[0], carrosEspera[1], carrosEspera[2], carrosEspera[3]);
  client.publish(TOPIC_ESPERA, buf);

  // Fase activa
  const char* nombres[] = {"Verde N-S","Amarillo N-S","Verde E-O","Amarillo E-O"};
  snprintf(buf, sizeof(buf),
    "{\"fase\":%d,\"desc\":\"%s\",\"override\":%s}",
    faseActual + 1, nombres[faseActual], modoOverride ? "true" : "false");
  client.publish(TOPIC_ESTADO, buf);
}

// ─────────────────────────────────────────────────────────────────
//  SETUP
// ─────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  // Pines semáforos como salida
  for (int i = 0; i < 4; i++)
    for (int c = 0; c < 3; c++)
      pinMode(SEM[i][c], OUTPUT);

  // Pines sensores como entrada con pull-up interno
  for (int i = 0; i < 4; i++) {
    pinMode(SNS_ENT[i], INPUT_PULLUP);
    pinMode(SNS_SAL[i], INPUT_PULLUP);
  }

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Estado inicial: todos en rojo
  for (int i = 0; i < 4; i++) setSemaforo(i, ROJO);
  delay(1000);

  // Arrancar en fase 0
  aplicarFase(0);
  tiempoFase = millis();
}

// ─────────────────────────────────────────────────────────────────
//  LOOP
// ─────────────────────────────────────────────────────────────────

void loop() {
  // ── Mantener conexión MQTT ────────────────────────────────────
  if (!client.connected()) reconnect();
  client.loop();

  // ── Leer sensores IR ─────────────────────────────────────────
  leerSensores();

  // ── Máquina de estados de semáforo (solo en modo auto) ────────
  if (!modoOverride) {
    unsigned long ahora   = millis();
    unsigned long duracion = duracionFase(faseActual);

    if (ahora - tiempoFase >= duracion) {
      faseActual = (faseActual + 1) % 4;
      aplicarFase(faseActual);
      tiempoFase = ahora;
      Serial.print("[FASE] → "); Serial.println(faseActual + 1);
    }
  }

  // ── Reporte periódico MQTT ────────────────────────────────────
  if (millis() - ultimoReporte >= INTERVALO_REPORTE) {
    publicarReporte();
    ultimoReporte = millis();
  }
}
