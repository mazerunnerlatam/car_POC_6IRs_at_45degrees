/*
  ==================================================================================
  Código de Control para Robot Seguidor de Línea - 6 SENSORES IR
  Plataforma: Arduino Mega
  Versión: 4.9 (Velocidades de giro suave corregidas)

  Descripción:
  Se revierte al modelo de control discreto (paso a paso) que funciona bien en
  líneas rectas. Se han ajustado las velocidades de giro suave para asegurar
  que ningún motor reciba un valor de PWM por debajo de su umbral mínimo de
  movimiento, garantizando giros más fluidos.
  ==================================================================================
*/

// ===== SECCIÓN 1: PARÁMETROS DE AJUSTE =====

// --- Velocidades de Maniobra ---
const int VELOCIDAD_BASE = 80;
const int VELOCIDAD_GIRO_FUERTE = 84;
const int VELOCIDAD_BUSQUEDA = 80; // Velocidad para buscar la línea cuando se pierde

// --- Velocidades para Giros Suaves (NUEVO) ---
const int VELOCIDAD_GIRO_SUAVE_MAX = 84; // Velocidad de la rueda exterior en un giro suave
const int VELOCIDAD_GIRO_SUAVE_MIN = 80; // Velocidad de la rueda interior (nunca por debajo del mínimo)

// --- Tiempos de Maniobra (en milisegundos) - REDUCIDOS PARA MAYOR AGILIDAD ---
const int TIEMPO_MANIOBRA = 100;
const int PAUSA_ENTRE_ACCIONES = 100;
const int TIEMPO_BUSQUEDA = 120; // Pulso corto hacia adelante para buscar
//--- Tiempos de Maniobra (en milisegundos) ---
// const int TIEMPO_MANIOBRA = 200;
// const int PAUSA_ENTRE_ACCIONES = 300;
// const int TIEMPO_BUSQUEDA = 150; // Pulso corto hacia adelante para buscar

// ===== SECCIÓN 2: DEFINICIONES DE HARDWARE Y VARIABLES GLOBALES (MODIFICADO) =====

// --- Pines de Sensores (Adaptado para 6 IRs de A0 a A5, IZQUIERDA -> DERECHA) ---
#define IR1_IZQ_EXT A0 // Sensor extremo izquierdo
#define IR2_IZQ_MED A1 // Sensor medio izquierdo
#define IR3_IZQ_CEN A2 // Sensor centro-izquierdo
#define IR4_DER_CEN A3 // Sensor centro-derecho
#define IR5_DER_MED A4 // Sensor medio derecho
#define IR6_DER_EXT A5 // Sensor extremo derecho

// --- Pines de Motores (Sin cambios) ---
#define MOTOR_IZQ_IN1 24
#define MOTOR_IZQ_IN2 25
#define MOTOR_IZQ_ENA 2
#define MOTOR_DER_IN3 26
#define MOTOR_DER_IN4 27
#define MOTOR_DER_ENB 3
const int STATUS_LED_PIN = 13;

// --- Variables Globales (Sin cambios) ---
int lineaPerdidaContador = 0;
const int MAX_INTENTOS_BUSQUEDA = 7;

// ===== SECCIÓN 3: FUNCIONES DE ACCIÓN DISCRETA (MODIFICADAS) =====

void accion_Detener() {
  digitalWrite(MOTOR_IZQ_IN1, LOW);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  analogWrite(MOTOR_IZQ_ENA, 0);
  digitalWrite(MOTOR_DER_IN3, LOW);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_DER_ENB, 0);
}

void accion_AvanzarRecto() {
  Serial.println("-> Accion: Avanzar Recto");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_BASE);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_BASE);
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroSuaveDerecha() {
  Serial.println("-> Accion: Giro Suave Derecha");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_GIRO_SUAVE_MAX);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_GIRO_SUAVE_MIN);
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroSuaveIzquierda() {
  Serial.println("-> Accion: Giro Suave Izquierda");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_GIRO_SUAVE_MIN);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_GIRO_SUAVE_MAX);
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroFuerteDerecha() {
  Serial.println("-> Accion: GIRO FUERTE DERECHA");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, LOW);
  digitalWrite(MOTOR_DER_IN4, HIGH);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_GIRO_FUERTE);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_GIRO_FUERTE);
  delay(TIEMPO_MANIOBRA);
}

void accion_GiroFuerteIzquierda() {
  Serial.println("-> Accion: GIRO FUERTE IZQUIERDA");
  digitalWrite(MOTOR_IZQ_IN1, LOW);
  digitalWrite(MOTOR_IZQ_IN2, HIGH);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_GIRO_FUERTE);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_GIRO_FUERTE);
  delay(TIEMPO_MANIOBRA);
}

void accion_BuscarAdelante() {
  Serial.print("-> Accion: Buscando linea... (Intento ");
  Serial.print(lineaPerdidaContador);
  Serial.println(")");
  digitalWrite(MOTOR_IZQ_IN1, HIGH);
  digitalWrite(MOTOR_IZQ_IN2, LOW);
  digitalWrite(MOTOR_DER_IN3, HIGH);
  digitalWrite(MOTOR_DER_IN4, LOW);
  analogWrite(MOTOR_IZQ_ENA, VELOCIDAD_BUSQUEDA);
  analogWrite(MOTOR_DER_ENB, VELOCIDAD_BUSQUEDA);
  delay(TIEMPO_BUSQUEDA);
}

void startupLEDPattern() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED_PIN, HIGH);
    delay(250);
    digitalWrite(STATUS_LED_PIN, LOW);
    delay(250);
  }
  digitalWrite(STATUS_LED_PIN, HIGH); delay(1250);
  digitalWrite(STATUS_LED_PIN, LOW); delay(1250);
}

// ===== SECCIÓN 4: SETUP Y LOOP PRINCIPAL (MODIFICADO) =====

void setup() {
  Serial.begin(9600);
  // Configurar pines de los 6 sensores como ENTRADA
  pinMode(IR1_IZQ_EXT, INPUT);
  pinMode(IR2_IZQ_MED, INPUT);
  pinMode(IR3_IZQ_CEN, INPUT);
  pinMode(IR4_DER_CEN, INPUT);
  pinMode(IR5_DER_MED, INPUT);
  pinMode(IR6_DER_EXT, INPUT);

  // Configurar pines de los motores como SALIDA
  pinMode(MOTOR_IZQ_IN1, OUTPUT);
  pinMode(MOTOR_IZQ_IN2, OUTPUT);
  pinMode(MOTOR_IZQ_ENA, OUTPUT);
  pinMode(MOTOR_DER_IN3, OUTPUT);
  pinMode(MOTOR_DER_IN4, OUTPUT);
  pinMode(MOTOR_DER_ENB, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);

  Serial.println("SISTEMA CON 6 SENSORES INICIADO (V4.9)");
  Serial.println("Coloque el robot en la pista. Iniciando en 3 segundos...");
  startupLEDPattern();
  delay(3000);
  Serial.println("¡En marcha!");
}

void loop() {
  // Lectura de sensores (0 = línea negra, 1 = superficie blanca)
  int s1 = digitalRead(IR1_IZQ_EXT);
  int s2 = digitalRead(IR2_IZQ_MED);
  int s3 = digitalRead(IR3_IZQ_CEN);
  int s4 = digitalRead(IR4_DER_CEN);
  int s5 = digitalRead(IR5_DER_MED);
  int s6 = digitalRead(IR6_DER_EXT);

  Serial.print("Lectura [s1,s2,s3,s4,s5,s6]: ");
  Serial.print(s1); Serial.print(s2); Serial.print(s3);
  Serial.print(s4); Serial.print(s5); Serial.println(s6);

  // --- LÓGICA DE DECISIÓN PARA 6 SENSORES (ORDEN CORREGIDO) ---
  
  // Caso 7: Intersección o línea ancha (LA MÁS ESPECÍFICA PRIMERO)
  if (s1==0 && s2==0 && s3==0 && s4==0 && s5==0 && s6==0) {
    lineaPerdidaContador = 0;
    accion_Detener(); // Nos detenemos en una intersección
  }
  // Caso 0: Línea centrada (todo blanco). Asumimos que es la condición ideal.
  else if (s1==1 && s2==1 && s3==1 && s4==1 && s5==1 && s6==1) {
    lineaPerdidaContador = 0;
    accion_AvanzarRecto();
  }
  // Caso 1: Línea a la izquierda (bajo s3), corregir girando a la IZQUIERDA
  else if (s1==1 && s2==1 && s3==0 && s4==1 && s5==1 && s6==1) {
    lineaPerdidaContador = 0;
    accion_GiroSuaveIzquierda();
  }
  // Caso 2: Línea a la derecha (bajo s4), corregir girando a la DERECHA
  else if (s1==1 && s2==1 && s3==1 && s4==0 && s5==1 && s6==1) {
    lineaPerdidaContador = 0;
    accion_GiroSuaveDerecha();
  }
  // Caso 3: Línea muy a la izquierda (bajo s2), corregir girando FUERTE a la IZQUIERDA
  else if (s1==1 && s2==0 && s3==1 && s4==1 && s5==1 && s6==1) {
    lineaPerdidaContador = 0;
    accion_GiroFuerteIzquierda();
  }
  // Caso 4: Línea muy a la derecha (bajo s5), corregir girando FUERTE a la DERECHA
  else if (s1==1 && s2==1 && s3==1 && s4==1 && s5==0 && s6==1) {
    lineaPerdidaContador = 0;
    accion_GiroFuerteDerecha();
  }
  // Caso 5: Línea en extremo izquierdo (bajo s1), corregir girando FUERTE a la IZQUIERDA
  else if (s1==0 && s2==1 && s3==1 && s4==1 && s5==1 && s6==1) { 
    lineaPerdidaContador = 0;
    accion_GiroFuerteIzquierda();
  }
  // Caso 6: Línea en extremo derecho (bajo s6), corregir girando FUERTE a la DERECHA
  else if (s1==1 && s2==1 && s3==1 && s4==1 && s5==1 && s6==0) { 
    lineaPerdidaContador = 0;
    accion_GiroFuerteDerecha();
  }
  // --- LÓGICA DE LÍNEA PERDIDA ---
  else {
    lineaPerdidaContador++; // Incrementa el contador de intentos
    
    if (lineaPerdidaContador >= MAX_INTENTOS_BUSQUEDA) {
      // Demasiados intentos fallidos, parada total.
      Serial.println("!!! PARADA DE EMERGENCIA: Linea no encontrada. !!!");
      accion_Detener();
      while(true) {
        // Bucle infinito para detener el programa. Requiere reinicio manual.
      }
    } else {
      // Intenta buscar la línea avanzando un poco.
      accion_BuscarAdelante();
    }
  }

  // Detenemos los motores y hacemos una pausa para observar y depurar
  accion_Detener();
  delay(PAUSA_ENTRE_ACCIONES);
}
