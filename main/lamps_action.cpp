// ------------------------ УПРАВЛЕНИЕ ЛАМПАМИ -------------------
/* 
  API для включеня и выключения светодиодов.
  Всего три светодиода: 
  - красный
  - зеленый
  - синий
*/

#include <Arduino.h>
#include "lamps_action.h"
#include "variables.h"

// OnRed - включить красный светодиод
void OnRed(){ digitalWrite(RED_PIN, LOW);}

// OffRed - выключить красный светодиод
void OffRed(){digitalWrite(RED_PIN, HIGH);}

// OnGreen - включить зеленый светодиод
void OnGreen(){digitalWrite(GREEN_PIN, LOW);}

// OffGreen - выключить зеленый светодиод
void OffGreen(){digitalWrite(GREEN_PIN, HIGH);}

// OnBlue - включить синий светодиод
void OnBlue(){digitalWrite(BLUE_PIN, LOW);}

// OffBlue - выключить синий светодиод
void OffBlue(){digitalWrite(BLUE_PIN, HIGH);}
// ------------------------ УПРАВЛЕНИЕ ЛАМПАМИ -------------------