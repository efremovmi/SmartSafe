#include <Arduino.h>
#include "lamps_action.h"
#include "variables.h"

// ==========================================================================================================================================================
// Работа с светодиодом
// Включить красный светодиод
void OnRed(){ digitalWrite(RED_PIN, LOW);}

// Выключить красный светодиод
void OffRed(){digitalWrite(RED_PIN, HIGH);}

// Включить зеленый светодиод
void OnGreen(){digitalWrite(GREEN_PIN, LOW);}

// Выключить зеленый светодиод
void OffGreen(){digitalWrite(GREEN_PIN, HIGH);}

// Включить синий светодиод
void OnBlue(){digitalWrite(BLUE_PIN, LOW);}

// Выключить синий светодиод
void OffBlue(){digitalWrite(BLUE_PIN, HIGH);}
// ==========================================================================================================================================================