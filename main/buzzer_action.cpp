// ------------------------ УПРАВЛЕНИЕ ДИНАМИКОМ -----------------
/* 
  API для управления динамиком.
  Поддерживается две мелодии: 
  - мелодия разрешения
  - мелодия отказа
*/

#include <Arduino.h>
#include "buzzer_action.h"
#include "variables.h"

// AccessDeniedSound - проиграть мелодию "В доступе отказано"
void AccessDeniedSound(){
  tone(PIEZO_PIN, 349, 500);
  delay(100);
  tone(PIEZO_PIN, 220, 200);
}

// AccessAllowedSound - проиграть мелодию "доступ разрешен"
void AccessAllowedSound(){
  tone(PIEZO_PIN, 523, 100);
  delay(150);  
  tone(PIEZO_PIN, 1319, 100);
  delay(150);
  tone(PIEZO_PIN, 987, 500);
}
// ------------------------ УПРАВЛЕНИЕ ДИНАМИКОМ -----------------