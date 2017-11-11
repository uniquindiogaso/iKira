#include <Wire.h>
#include "RTClib.h"

RTC_DS3231 reloj;

//Flag para determinar si en el momento actual se encuentra dispensado
bool dispensando = false;

void setup() {
  Serial.begin(9600);
  delay(1000);

  if (!reloj.begin()) {
    Serial.println(F("No se encontro el dispositivo de reloj"));
    while (1);
  }

  //Si dispositivo pierde energia se debe reconfigurar....
  if (reloj.lostPower()) {
    reloj.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

}

void loop() {
  DateTime now = reloj.now();

  if ( iniciarDispensacion(now)){
    // Aqui logica de inicio dispensacion 
  }
  
  pruebas(now);


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
  }
}

/**
 * Permite comprobar Instante a Instante si debe
 * iniciar con el mecanismo de dispensacion.
 */
bool iniciarDispensacion(DateTime momentoActual){
  float hora = momentoActual.hour() + momentoActual.minute() / 60.0;
  if ( hora == 7.00 || hora == 12.00 || hora == 17.00 ){
    dispensando = true;
    return dispensando;
  }
  return false;
}



