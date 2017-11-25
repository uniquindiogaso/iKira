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
#define VIRTUAL_PIN V0

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

void setup() {
  //Indicar Velocidad de Refrezco Consola. 
  Serial.begin(9600);
  //Establecer Conexion a iot
  Cayenne.begin(token);
  delay(1000);

  if (!reloj.begin()) {
    Serial.println(F("No se encontro el dispositivo de reloj"));
    while (1);
  }

  //Si dispositivo pierde energia se debe reconfigurar....
  if (reloj.lostPower()) {
    reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }


  configSensoresPeso();

}

void loop() {
  //Ejecutar iot
  Cayenne.run();
    
  DateTime now = reloj.now();
  

  //if ( iniciarDispensacion(now)) {
    // Aqui logica de inicio dispensacion
  //}
 
  pruebas(now);
  debug();
  configMotor();
}


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
    Serial.println("Empezar a Dispensar");
    encenderMotor();
    delay(3000);
    apagarMotor();
    
  }
}

/**
   Permite comprobar Instante a Instante si debe
   iniciar con el mecanismo de dispensacion.
*/
bool iniciarDispensacion(DateTime momentoActual) {
  float hora = momentoActual.hour() + momentoActual.minute() / 60.0;
  if ( hora == 7.00 || hora == 12.00 || hora == 17.00 ) {
    dispensando = true;
    return dispensando;
  }
  return false;
}



void configSensoresPeso() {
  Serial.println("Configurando Sensor de Peso 1 Kg ...");
  sensor1K.set_scale(factorCalibracion1k);
  sensor1K.tare();
  Serial.println("Configurando Sensor de Peso 5 Kg ...");
  sensor5K.set_scale(factorCalibracion5k);
  sensor5K.tare();
  
  
}


float pesoActualSensor1K() { 
   float unidades1K;
   unidades1K = sensor1K.get_units(), 10;
  if (unidades1K < 0)
  {
    unidades1K = 0.00;
  }
  //Retorna Valor en Gramos
  //float gramos = unidades1K * 0.035274;  
  return unidades1K;
}

float pesoActualSensor5K() { 
   float unidades5K;
   unidades5K = sensor5K.get_units(), 10;
  if (unidades5K < 0)
  {
    unidades5K = 0.00;
  }
  //Retorna Valor en Gramos
  //float gramos = unidades1K * 0.035274;  
  return unidades5K;
}


void debug(){
    if(Serial.available()){
      char accion =  Serial.read();
      if (accion == '1'){
           Serial.print("Valor Actual Sensor 1 KILO ");
           Serial.print(pesoActualSensor1K());
           Serial.println();
      }else if (accion == '5'){
           Serial.print("Valor Actual Sensor 5 KILO ");
           Serial.print(pesoActualSensor5K());
           Serial.println();        
      }
    }
}

CAYENNE_OUT(VIRTUAL_PIN)
{
  Cayenne.virtualWrite(VIRTUAL_PIN, (pesoActualSensor5K() / 500) );
}


void configMotor(){
  s.attach(motor);
}

void encenderMotor(){
  s.write(0);
  // Esperamos 1 segundo
  delay(300);
  
  // Desplazamos a la posición 90º
  s.write(100);
  // Esperamos 1 segundo
  delay(300);
}


void apagarMotor(){  
  // Desplazamos a la posición 180º
  s.write(110);
  // Esperamos 1 segundo
  delay(300);
}




