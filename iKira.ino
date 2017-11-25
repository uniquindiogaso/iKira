//Importacion de Librerias
#include <Wire.h>
#include "RTClib.h" // Manejo Reloj
#include "HX711.h" //Controlador Censores Peso
#include <CayenneEthernet.h> //Internet de las Cosas
#include <Servo.h> //Manejo Motor


// Definicion Objecto Tipo Motor
Servo s;
//Definicion Objeto Reloj
RTC_DS3231 reloj;


//Constantes Manejo Pines Sensores de Peso
const int sp5KDt = 8;
const int sp5KSck = 9;
const int sp1kDt = 5;
const int sp1KSck = 6;

//iot pin virtual
#define VIRTUAL_PIN V0 //Sensor de Dispensador
#define VIRTUAL_PIN1 V1 //Sensor de Plato

//pin del servo
int motor = 3;

//seteo de pines
HX711 sensor1K(sp1kDt, sp1KSck);
HX711 sensor5K(sp5KDt, sp5KSck);

//Factores de Calibracion Censores Peso
float factorCalibracion1k = 1670;
float factorCalibracion5k = 400;

//Flag para determinar si en el momento actual se encuentra dispensado
bool dispensando = false;

//Token Acceso Cayennes
char token[] = "lp4aye9edg";

float tRef = 600;
float pRef = 190;

void setup() {
  //Indicar Velocidad de Refrezco Consola.
  Serial.begin(9600);
  //Establecer Conexion a iot
  Cayenne.begin(token);
  delay(1000);

  //Identificar si Reloj esta conectado
  if (!reloj.begin()) {
    Serial.println(F("No se encontro el dispositivo de reloj"));
    while (1);
  }

  //Si dispositivo pierde energia se debe reconfigurar....
  if (reloj.lostPower()) {
    //Configurar con fecha actual
    reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  configSensoresPeso();
  configMotor();

}

void loop() {
  //Ejecutar iot
  Cayenne.run();
  //Obtener fecha actual
  DateTime now = reloj.now();

  //if ( iniciarDispensacion(now)) {
  // prepararDispensacion();
  //}

  pruebas(now);
  consola();

}

/**
   Metodo empleado para pruebas de ejecucion
*/
void pruebas(DateTime now) {
  /*
     Uso del reloj :
     Para pruebas el mecanismo se activara
     cada 30 segundos
  */
  //Para que se active un vez por segundo ( espere 1 segundo para la sig ejecucion)
  delay(1000);
  //Se dispara
  if ( now.second() == 0 || now.second() == 30) {
      Serial.println("Dispensar por tiempo de espera ....");
      prepararDispensacion();
  }
}

/**
 * Verificador si puede dispensar o no  
 */
void prepararDispensacion() {
  Serial.println("Verificar si Debe Dispensar ....");
  //Valor de Referencia
  tRef = 600;
  //Si hay Alimento en Dispensador Principal
  if ( pesoActualSensor5K() >= pRef ) {
    // Obtener la porcion faltante
    long faltante = pRef - pesoActualSensor1K();
    // Calcula en base al faltante un estimado de tiempo de apertura
    tRef = (tRef * faltante) / pRef;

    // Para evitar la apertura del motor cuando la porcion es vacia
    if (tRef > 0 ) {
      Serial.println("Abrir el Dispensador por");
      Serial.println(tRef);
      encenderMotor();
      apagarMotor();
    }

  } else {
    Serial.println(" ¡No hay alimento!, debe cargar dispensador principal ....");
  }
}

/**
   Permite comprobar Instante a Instante si debe
   iniciar con el mecanismo de dispensacion.
*/
bool iniciarDispensacion(DateTime momentoActual) {
  float hora = momentoActual.hour() + momentoActual.minute() / 60.0;
  if ( hora == 9.00 || hora == 16.00) {
    dispensando = true;
    return dispensando;
  }
  return false;
}


/**
 * Configuracion de los sensores
 * de peso con el factorCalibracion de cada uno
 */
void configSensoresPeso() {
  Serial.println("Configurando Sensor de Peso 1 Kg ...");
  sensor1K.set_scale(factorCalibracion1k);
  sensor1K.tare();
  Serial.println("Configurando Sensor de Peso 5 Kg ...");
  sensor5K.set_scale(factorCalibracion5k);
  sensor5K.tare();


}

/**
 * Obtener el Peso Actual del
 * Sensor de 1 Kilo
 */
float pesoActualSensor1K() {
  float unidades1K;
  //Factor decimal
  unidades1K = sensor1K.get_units(), 10;
  if (unidades1K < 0)
  {
    unidades1K = 0.00;
  }
  return unidades1K;
}


/**
 * Obtener el Peso Actual del
 * Sensor de 5 Kilos
 */
float pesoActualSensor5K() {
  float unidades5K;
  //Factor decimal
  unidades5K = sensor5K.get_units(), 10;
  if (unidades5K < 0)
  {
    unidades5K = 0.00;
  }
  //Retorna Valor en Gramos
  //float gramos = unidades1K * 0.035274;
  return unidades5K;
}

/**
 * Metodo de configuracion de Consola
 */
void consola() {
  if (Serial.available()) {
    char accion =  Serial.read();
    if (accion == '1') {
      Serial.print("Valor Actual Sensor 1 KILO ");
      Serial.print(pesoActualSensor1K());
      Serial.println();
    } else if (accion == '5') {
      Serial.print("Valor Actual Sensor 5 KILO ");
      Serial.print(pesoActualSensor5K());
      Serial.println();
    }
  }
}

/**
   Comunicacion Salida
*/
CAYENNE_OUT(VIRTUAL_PIN)
{
  //Escribir en cayenne el peso del Dispensador
  Cayenne.virtualWrite(VIRTUAL_PIN, (pesoActualSensor5K() / 500) );
  //Escribir en cayenne el peso del plato
  Cayenne.virtualWrite(VIRTUAL_PIN1, pesoActualSensor1K() );
}

/**
 * Configuracion del Servo en el PIN Correspondiente
 */
void configMotor() {
  s.attach(motor);
}


/**
 * Ejecucion del Motor
 */
void encenderMotor() {
  s.write(0);
  // Esperamos 0.6 segundo
  delay(tRef);

  // Desplazamos a la posición 100º
  s.write(100);
  // Esperamos 5 segundo
  delay(5000);
}

/**
 * Apagar Motor
 */
void apagarMotor() {
  // Desplazamos a la posición 180º
  s.write(110);
  // Esperamos 5 segundo
  delay(5000);
}

/**
 * Recibir del Cayenne el valor del boton
 */
CAYENNE_IN(V4)
{
  if ( getValue.asInt() == 0 ){
    Serial.println("Dispensar desde App");
    prepararDispensacion();
    Cayenne.virtualWrite(V4, 1 );
  }
}



