#define _LCD_TYPE 1
#include <LCD_1602_RUS_ALL.h>

#include <Keypad.h>
#include <Wire.h> 
#include <Adafruit_Fingerprint.h> 
#include <EEPROM.h>
#include <Arduino.h>
#include <math.h>

#include "lock_action.h"
#include "variables.h"
#include "buzzer_action.h"
#include "lamps_action.h"
#include "sha1.h"


// ==========================================================================================================================================================
// Экзепляры классов

// Создание экземляра экземляра класса клавиатуры
Keypad mainKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Создание экземляра класса дисплея
LCD_1602_RUS lcd(0x27, 16, 2);

// Создание экземляра класса FINGERPRINT для общения с датчиком отпечатка пальца
// объявляем объект finger   для работы с библиотекой Adafruit_Fingerprint ИМЯ_ОБЪЕКТА = Adafruit_Fingerprint(ПАРАМЕТР); // ПАРАМЕТР - rx_pin и tx_pin для работы с UART к которому подключен модуль
Adafruit_Fingerprint finger = Adafruit_Fingerprint(RX_FINGERPRINT, TX_FINGERPRINT);   

// ==========================================================================================================================================================

// Cтруктура данных пользователя
struct User {
  char pin[MAX_PIN_SIZE];
  char secret[MAX_SIZE_HOTP_SECRET+1];
  bool isExists;
};

#define MODE_POS sizeof(User)*MAX_USER_COUNT+2

// ==========================================================================================================================================================
// Флаг для проверка того, зашел ли кто-то в систему
bool isAccessAllowed = false;

// ==========================================================================================================================================================
// Первоначальная настройка
void setup() {
  randomSeed(analogRead(A1));

  // Инициализация пинов
  pinMode(LOCK_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(PIEZO_PIN, OUTPUT);

  // Выключение светодиодов 
  OffRed();
  OffGreen();
  OffBlue();

  // Работа с экраном
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Датчик отпечатка пальцев 
  finger.begin(57600);
  finger.LEDcontrol(false);
  // Для отладки
  Serial.begin(9600);

 
  // Проверка, что датчик работает
  if(finger.verifyPassword()){
    Serial.println(F("Found sensor"));
  }               // если модуль отпечатков    обнаружен, выводим сообщение "сенсор обнаружен"
  else {
    Serial.println(F("Sensor not found")); while(1); // если модуль отпечатков не обнаружен, выводим сообщение "сенсор не обнаружен" и входим в бесконечный цикл: while(1);
  }
 

  // SaveUser(0, User{"1111", "bbbbbbbbbb", true});
  // SaveUser(1, User{"1234", "aaaaaaaaaa", true});
  // SaveUser(2, User{"", "", false});
  // SaveUser(3, User{"", "", false});
  // SaveUser(4, User{"", "", false});
  // SaveCurrentMode(PIN_MODE);
  // finger.emptyDatabase();


  // digitalWrite(LOCK_PIN, HIGH);
  // delay(100);
  
  // Вывести форму входа
  DrawMainForm();

  // Конфигурация замка
  set_lock();
}


void loop() {
  char inputChar = mainKeypad.getKey();

  // Если клавиша нажата
  if (inputChar){
    // Serial.println(String(inputChar));
    switch (inputChar){
      case OK_BUTTON:
        // Если вход был выполнен
        if (!isAccessAllowed){
          uint8_t mode = GetCurrentMode();
          int8_t id, code;
          bool ids[MAX_USER_COUNT];
          for(int8_t i=0; i<MAX_USER_COUNT; i++) ids[i] = false;
          switch(mode){
            case PIN_MODE:
              if (PinCheck(ids)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
            case FINGER_MODE:
              if (FingerCheck(ids, false)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
            case TOKEN_MODE:
              if (TokenCheck(ids, false)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
            case PIN_AND_FINGER_MODE:
              if (PinAndFingerCheck(ids)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
            case PIN_AND_TOKEN_MODE:
              if (PinAndTokenCheck(ids)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
            case FINGER_AND_TOKEN_MODE:
              if (FingerAndTokenCheck(ids)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
            case PIN_FINGER_TOKEN_MODE:
              if (PinFingerAndTokenCheck(ids)){
                lock();
                isAccessAllowed = true;
              }
              DrawMainForm();
              break;
          }
        // Если вход не был выполнен
        }else{
          DrawMenu();
        }
        break;
      // case UP_BUTTON:
      //   SaveFingerprint(1);
      //   break;
      case DOWN_BUTTON:
        lock();
        isAccessAllowed = false;
        break;
      case EXIT_BUTTON:
        isAccessAllowed = false;
        DrawMainForm();
        return;
    }
  }
}


// Форма входа
void DrawMainForm(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Доступ: "));
  if (isAccessAllowed){
    lcd.print(F("разрешен"));
    lcd.setCursor(0, 1);
    lcd.print(F("Меню: А"));
  }
  else{
    lcd.print(F("запрещен"));
    lcd.setCursor(0, 1);
    lcd.print(F("Войти: А"));
  }
  return;
}

// Меню>Доб.Польз.
//    Ввод пина: 
//    A-да, D-нет
//    Ввод Отпеч.: 
//    A-да, D-нет
//    Созд. токен: 
//    A-да, D-нет
// 
// Меню>Ред.Польз.
//    Меню>Ред.Польз.
//    Пользователь: N
//        Меню>Ред.Польз.>
//        N > Код
//        Меню>Ред.Польз.>
//        N > Палец
//        Меню>Ред.Польз.>
//        N > Токен
// 
// Меню>Удал.Польз.
//    Меню>Удал.Польз.
//    Пользователь: N
// 
// Главное меню настроек
void DrawMenu(){
  int8_t ind = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  
  lcd.print(FPSTR(pgm_read_word(MenuItems + 0)));

  while (true){
    char inputChar = mainKeypad.getKey();
    // Если клавиша нажата
    switch (inputChar){
      case OK_BUTTON:
        switch (ind){
          case ADD_USER_MENU:
            DrawAddUserMenu();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(FPSTR(pgm_read_word(MenuItems + ind)));
          break;
          case DELETE_USER_MENU: 
            DrawDeleteUserMenu();
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(FPSTR(pgm_read_word(MenuItems + ind)));
          break;
          case UPDATE_MODE_MENU:
            DrawUpdateModeMenu(FPSTR(pgm_read_word(MenuItems + ind)));
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(FPSTR(pgm_read_word(MenuItems + ind)));
          break;
          case OPEN_MENU:
            lock();
          break;
          case EXIT_MENU: 
            isAccessAllowed = false;
            DrawMainForm();
            return;
          break;
        }
        break;
      case UP_BUTTON:
        ind = (ind+1) % MenuItemsSize;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(FPSTR(pgm_read_word(MenuItems + ind)));
        break;
      case DOWN_BUTTON:
        ind -=1;
        if (ind == -1) ind = MenuItemsSize-1;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(FPSTR(pgm_read_word(MenuItems + ind)));
        break;
      case EXIT_BUTTON:
        isAccessAllowed = false;
        DrawMainForm();
        return;
    }
  }
}

// Подменю удаления пользователя
void DrawDeleteUserMenu(){
  lcd.clear();
  lcd.setCursor(0, 0);

  char buffer[15];
  

  User users[MAX_USER_COUNT];
  GetAllUsers(users);
  int position = GetNextExistsUserPosition(users, -1);
  if (position == -1){
    lcd.setCursor(0, 0);
    lcd.print(F("Пользов. нет   "));
    OnRed();
    delay(3000);
    OffRed();
    return;
  }

  lcd.print(F("Выберите позицию"));
  lcd.setCursor(0, 1);
  lcd.print(String(position+1));
  
  bool isEndLoop = false;
  while (!isEndLoop){
    char inputChar = mainKeypad.getKey();
    switch (inputChar){
      case OK_BUTTON:
        isEndLoop = true;
        break;
      case UP_BUTTON:
        position = GetNextExistsUserPosition(users, position);
        lcd.setCursor(0, 1);
        lcd.print(String(position+1));
        break;
      case DOWN_BUTTON:
        position = GetPrevExistsUserPosition(users, position);
        lcd.setCursor(0, 1);
        lcd.print(String(position+1));
        break;
      case EXIT_BUTTON:
        return;
        break;
    }
  }

  DeleteUser(position);
  DeleteFingerprintByID(position);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Пользователь: ");
  lcd.print(String(position+1));
  lcd.setCursor(0, 1);
  lcd.print("удален!");
  OnBlue();
  delay(3000);
  OffBlue();
}


// Добавление нового пользователя в систему
void DrawAddUserMenu(){
  lcd.clear();
  lcd.setCursor(0, 0);

  User users[MAX_USER_COUNT];
  User user;
  GetAllUsers(users);
  int position = GetNextEmptyUserPosition(users, -1);
  user.isExists = false;
  if (position == -1){
    lcd.print(F("Нет мест."));
    lcd.setCursor(0, 1);
    lcd.print(F("Уд./Ред. польз."));
    OnRed();
    delay(3000);
    OffRed();
    return;
  }

  lcd.print(F("Выберите позицию"));
  lcd.setCursor(0, 1);
  lcd.print(String(position+1));
  
  bool isEndLoop = false;
  char inputChar;
  while (!isEndLoop){
    inputChar = mainKeypad.getKey();
    switch (inputChar){
      case OK_BUTTON:
        isEndLoop = true;
        break;
      case UP_BUTTON:
        position = GetNextEmptyUserPosition(users, position);
        lcd.setCursor(0, 1);
        lcd.print(String(position+1));
        break;
      case DOWN_BUTTON:
        position = GetPrevEmptyUserPosition(users, position);
        lcd.setCursor(0, 1);
        lcd.print(String(position+1));
        break;
      case EXIT_BUTTON:
        return;
        break;
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Ввод кода"));
  delay(2000);

  int code;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Код: "));
  int i = 0;
  code = GetPin(5, 0, user.pin);
  if (code != OK_CODE){
    return;
  }

  delay(300);
     
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Скан. отпечатка"));
  delay(2000);

  isEndLoop = false;
  while (!isEndLoop){
    code = SaveFingerprint(position);
    if (code != OK_CODE){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Повтор попытки."));
      delay(1500);
      continue;
    };
    break;
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("HOTP"));
  delay(2000);

  generateRandomSecret().toCharArray(user.secret, MAX_SIZE_HOTP_SECRET+1);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Секрет:"));
  lcd.setCursor(8, 0);
  lcd.print(user.secret);
  lcd.setCursor(0, 1);
  user.isExists = true;

  lcd.print(F("Нажм. люб. кл."));
  while (true){
    inputChar = mainKeypad.getKey();
    if (inputChar) break;
  }

  user.isExists = true;

  if (user.isExists){
    SaveUser(position, user);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Пользователь: "));
    lcd.print(String(position+1));
    lcd.setCursor(0, 1);
    lcd.print(F("Сохранен!"));
    OnGreen();
    delay(3000);
    OffGreen();
  }else{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Пользователь: "));
    lcd.print(String(position+1));
    lcd.setCursor(0, 1);
    lcd.print(F("Не сохранен!"));
    OnRed();
    delay(3000);
    OffRed();
  }
 
  return;
}

// Подменю обновления режима входа
void DrawUpdateModeMenu(const __FlashStringHelper* firstString){
  int8_t ind = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(firstString);
  lcd.setCursor(0, 1);
  lcd.print(FPSTR(pgm_read_word(UpdateModeMenuItems + ind)));
  while (true){
    char inputChar = mainKeypad.getKey();
    switch (inputChar){
      case OK_BUTTON:
        switch (ind){
          case PIN_MODE:
            SaveCurrentMode(PIN_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
          case FINGER_MODE:
            SaveCurrentMode(FINGER_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
          case TOKEN_MODE:
            SaveCurrentMode(TOKEN_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
          case PIN_AND_FINGER_MODE:
            SaveCurrentMode(PIN_AND_FINGER_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
          case PIN_AND_TOKEN_MODE:
            SaveCurrentMode(PIN_AND_TOKEN_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
          case FINGER_AND_TOKEN_MODE:
            SaveCurrentMode(FINGER_AND_TOKEN_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
          case PIN_FINGER_TOKEN_MODE:
            SaveCurrentMode(PIN_FINGER_TOKEN_MODE);
            OnGreen();
            delay(300);
            OffGreen();
            return;
        }
        return;
      case UP_BUTTON:
        ind = (ind+1) % UpdateModeMenuItemsSize;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(firstString);
        lcd.setCursor(0, 1);
        lcd.print(FPSTR(pgm_read_word(UpdateModeMenuItems + ind)));
        break;
      case DOWN_BUTTON:
        ind -=1;
        if (ind == -1) {ind = UpdateModeMenuItemsSize-1;}
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(firstString);
        lcd.setCursor(0, 1);
        lcd.print(FPSTR(pgm_read_word(UpdateModeMenuItems + ind)));
        break;
      case EXIT_BUTTON: 
        return;
    }
  } 
}
// ==========================================================================================================================================================

// Проверка пин-кода
bool PinCheck(bool ids[]){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Код: "));

  int8_t code = EnterPinWithCheckUsers(ids);
  switch (code){
    case EXIT_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Отмена входа!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      break;
    case INCORRECT_PIN_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Неверный пин!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      break;
    default:
      lcd.clear();
      OnGreen();
      lcd.setCursor(0, 0);
      lcd.print(F("Верный пин!"));
      AccessAllowedSound();
      delay(2000);
      OffGreen();
      lcd.clear();
      return true;
  }

  return false;
}

// Проверка отпечатока
bool FingerCheck(bool* ids, bool needCheckId){
  finger.LEDcontrol(true);
  int8_t code = getFingerprintID();
  finger.LEDcontrol(false);
  switch (code){
    case EXIT_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Отмена ввода!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      return false;
    case FINGER_ERROR_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Ошибка ввода!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      return false;
    case FINGER_NOT_FOUND_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Неверный"));
      lcd.setCursor(0, 1);
      lcd.print(F("отпечаток!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      return false;
    default:
      if (!needCheckId){
        lcd.clear();
        OnGreen();
        lcd.setCursor(0, 0);
        lcd.print(F("Верный"));
        lcd.setCursor(0, 1);
        lcd.print(F("отпечаток!"));
        AccessAllowedSound();
        delay(2000);
        OffGreen();
        lcd.clear();
        ids[code] = true;
        return true;
      }else if (needCheckId && ids[code]){
          lcd.clear();
          OnGreen();
          lcd.setCursor(0, 0);
          lcd.print(F("Верный"));
          lcd.setCursor(0, 1);
          lcd.print(F("отпечаток!"));
          AccessAllowedSound();
          delay(2000);
          OffGreen();
          lcd.clear();
          ids[code] = true;
          return true;
      }
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Неверный"));
      lcd.setCursor(0, 1);
      lcd.print(F("отпечаток!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      return false;
  }
}

// Проверка токена
bool TokenCheck(bool* ids, bool needCheckId){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Зерно:"));
  int seed = random(32768);
  lcd.setCursor(7, 0);
  lcd.print(String(seed));

  lcd.setCursor(0, 1);
  lcd.print(F("Токен:"));

  int8_t code = EnterTokenWithCheckUsers(ids, seed, needCheckId);
  switch (code){
    case EXIT_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Отмена входа!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      break;
    case INCORRECT_TOKEN_CODE:
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Неверный токен!"));
      AccessDeniedSound();
      delay(2000);
      OffRed();
      lcd.clear();
      break;
    default:
      lcd.clear();
      OnGreen();
      lcd.setCursor(0, 0);
      lcd.print(F("Верный токен!"));
      AccessAllowedSound();
      delay(2000);
      OffGreen();
      lcd.clear();
      return true;
  }

  return false;
}

// Проверка пин-кода и отпечатока
bool PinAndFingerCheck(bool* ids){
  if (PinCheck(ids)){
    if (FingerCheck(ids, true)){
      return true;
    }
  }

  return false;
}

// Проверка пин-кода и токена
bool PinAndTokenCheck(bool* ids){
  if (PinCheck(ids)){
    if (TokenCheck(ids, true)){
      return true;
    }
  }

  return false;
}

// Проверка отпечатка и токена
bool FingerAndTokenCheck(bool* ids){
  if (FingerCheck(ids, false)){
    if (TokenCheck(ids, true)){
      return true;
    }
  }

  return false;
}

// Проверка пин-кода, отпечатока и токена
bool PinFingerAndTokenCheck(bool* ids){
  if (PinCheck(ids)){
    if (FingerCheck(ids, true)){
      if (TokenCheck(ids, true)){
        return true;
      }
    }
  }

  return false;
}


// ==========================================================================================================================================================
// Сохранить отпечаток
int8_t SaveFingerprint(uint8_t id){
  finger.LEDcontrol(true);
  int8_t code = getFingerprintEnroll(id);
  finger.LEDcontrol(false);
  if (code == OK_CODE){
    lcd.clear();
    OnGreen();
    lcd.setCursor(0, 0);
    lcd.print(F("Отпечаток"));
    lcd.setCursor(0, 1);
    lcd.print(F("сохранен!"));
    delay(1500);
    OffGreen();
    lcd.clear();
  }else if (code == FINGER_FOUND_CODE){
    lcd.clear();
    OnRed();
    lcd.setCursor(0, 0);
    lcd.print(F("Отпечаток уже"));
    lcd.setCursor(0, 1);
    lcd.print(F("есть"));
    delay(1500);
    OffRed();
    lcd.clear();
  }else{
    lcd.clear();
    OnRed();
    lcd.setCursor(0, 0);
    lcd.print(F("Отпечаток не"));
    lcd.setCursor(0, 1);
    lcd.print(F("сохранен!"));
    delay(1500);
    OffRed();
    lcd.clear();
  }

  return code;
}

// Удалить отпечаток из датчика
int8_t DeleteFingerprintByID(int8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
      lcd.clear();
      OnBlue();
      lcd.setCursor(0, 0);
      lcd.print(F("Отпечаток"));
      lcd.setCursor(0, 1);
      lcd.print(F("удален!"));
      AccessAllowedSound();
      delay(3000);
      OffBlue();
      lcd.clear();
      return OK_CODE;
  }
  else{
      lcd.clear();
      OnRed();
      lcd.setCursor(0, 0);
      lcd.print(F("Ошибка ввода"));
      AccessDeniedSound();
      delay(3000);
      OffRed();
      lcd.clear();
      return FINGER_NOT_FOUND_CODE;
  }
}

// Получить ID по отпечатку пальца
// returns -1 if failed, otherwise returns ID #
int8_t getFingerprintID() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Приложите палец"));
  lcd.setCursor(0, 1);
  int8_t p = -1;
  uint8_t position = 0;
  char inputChar;
  while (p != FINGERPRINT_OK) {
    lcd.setCursor(position, 1);
    lcd.print(F("."));
    position += 1;
    delay(50);
    if (position == 16){
      position = 0;
      lcd.setCursor(position, 1);
      lcd.print(F("                "));
      lcd.setCursor(position, 1);
    }

    inputChar = mainKeypad.getKey();
    if(inputChar && inputChar == EXIT_BUTTON){
      return EXIT_CODE;
    }

    p = finger.getImage();
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return FINGER_ERROR_CODE;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return FINGER_NOT_FOUND_CODE;

  // found a match!
  // Serial.print("Found ID #"); Serial.print(finger.fingerID);
  // Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

// Запомнить отпечаток пальца
int8_t getFingerprintEnroll(uint8_t id) {
  int p = -1;
  uint8_t position = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Приложите палец"));
  lcd.setCursor(0, 1);
  char inputChar;

  while (p != FINGERPRINT_OK) {
    char inputChar = mainKeypad.getKey();
    if(inputChar && inputChar == EXIT_BUTTON){
      return EXIT_CODE;
    }
    
    lcd.print(F("."));
    position += 1;
    delay(50);
    if (position == 16){
      position = 0;
      lcd.setCursor(position, 1);
      lcd.print(F("                "));
      lcd.setCursor(position, 1);
    }

    p = finger.getImage();
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    default:
      return FINGER_ERROR_CODE;
  }
  delay(100);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Повторите"));
  lcd.setCursor(0, 1);
  lcd.print(F("действие!"));
  
  delay(1500);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Приложите палец"));
  lcd.setCursor(0, 1);

  position = 0;
  p = -1;
  while (p != FINGERPRINT_OK) {
    inputChar = mainKeypad.getKey();
    if(inputChar && inputChar == EXIT_BUTTON){
      return EXIT_CODE;
    }
    
    lcd.print(F("."));
    position += 1;
    delay(50);
    if (position == 16){
      position = 0;
      lcd.setCursor(position, 1);
      lcd.print(F("                "));
      lcd.setCursor(position, 1);
    }

    p = finger.getImage();
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      break;
    default:
      // Serial.println(F("Unknown error"));
      return FINGER_ERROR_CODE;
  }

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    // Serial.println("Found a print match!");
    return FINGER_FOUND_CODE;
  } else if (p == FINGERPRINT_NOTFOUND) {
    // Serial.println("Did not find a match");
  } else {
    // Serial.println("Unknown error");
    return FINGER_ERROR_CODE;
  }

  // OK converted!
  // Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p != FINGERPRINT_OK) {
    return CREATE_FINGER_MODEL_ERROR;
  }

  // Serial.print(F("ID ")); Serial.println(id);
  p = finger.storeModel(id);
  if (p != FINGERPRINT_OK) {
    return STORE_FINGER_MODEL_ERROR;
  }

  return OK_CODE;
}
// ==========================================================================================================================================================



// ==========================================================================================================================================================
// Проверка пин-кода и вывод всей информации на дисплей.
// Получить пин-код
int8_t GetPin(int offsetX, int offsetY, char* pin){
  char inputChar;
  for (uint8_t i=0; i<MAX_PIN_SIZE; i++){
    while(true){
      inputChar = mainKeypad.getKey();
      if(inputChar){
        switch (inputChar) {
        case EXIT_BUTTON:
          return EXIT_CODE;
        default:
          pin[i] = inputChar;
          lcd.print(F("*"));
          lcd.setCursor(offsetX+i+1, offsetY);
        }
        break;
      }
    }
  }

  return OK_CODE;
}

// Ввод пин-кода и сравнение его с пин-кодом в памяти
int8_t EnterPinWithCheckUsers(bool* ids){
  char pin[MAX_PIN_SIZE];
  int code = GetPin(5, 0, pin);
  if (code != OK_CODE){
    return code;
  }

  delay(200);

  User user;
  int8_t result = INCORRECT_PIN_CODE;

  bool isTruePin;
  for (uint8_t i=0; i<MAX_USER_COUNT; i++){
    user = GetUserByID(i);
    isTruePin = true;
    for (uint8_t j=0; j<MAX_PIN_SIZE; j++){
      if(pin[j] != user.pin[j]){
        isTruePin = false;
        break;
      }
    }
    if (isTruePin == true){
      ids[i] = true; 
      result = OK_CODE;
    }else{
      ids[i] = false; 
    }
  }

  return result;
}
// ==========================================================================================================================================================



// ==========================================================================================================================================================
// Проверка токена и вывод всей информации на дисплей.
int8_t GetSecret(int offsetX, int offsetY, char* token){
  char inputChar;
  for (uint8_t i=0; i<MAX_SIZE_HOTP_TOKEN; i++){
    while(true){
      inputChar = mainKeypad.getKey();
      if(inputChar){
        switch (inputChar) {
        case EXIT_BUTTON:
          return EXIT_CODE;
        default:
          lcd.setCursor(offsetX+i, offsetY);
          token[i] = inputChar;
          lcd.print(String(inputChar));
        }
        break;
      }
    }
  }

  return OK_CODE;
}

// Ввод токена и сравнение его с паролем в памяти
int8_t EnterTokenWithCheckUsers(bool* ids, int seed, bool needCheckId){
  char token[MAX_SIZE_HOTP_TOKEN];
  int code = GetSecret(7, 1, token);
  if (code != OK_CODE){
    return code;
  }

  delay(200);

  User user;

  bool isTrueToken;
  int8_t result = INCORRECT_TOKEN_CODE;
  for (uint8_t i=0; i<MAX_USER_COUNT; i++){
    user = GetUserByID(i);

    if (!user.isExists){
      continue;
    }
    isTrueToken = true;
    String userToken = String(generateHOTP(String(user.secret), seed, MAX_SIZE_HOTP_TOKEN));
    for (uint8_t j=0; j<MAX_SIZE_HOTP_TOKEN; j++){
      if(token[j] != userToken[j]){
        isTrueToken = false;
        break;
      }
    }
    if (isTrueToken == true){
        if (!needCheckId || ids[i] == true){
        ids[i] = true; 
        result = OK_CODE;
      }
    }
  }

  return result;
}

// ==========================================================================================================================================================



// ==========================================================================================================================================================
// Получить пользователя из памяти по ID
User GetUserByID(int8_t id){
  if (id >= MAX_USER_COUNT){
    return User{"-", "-", false};
  }

  User user;
  EEPROM.get(id*sizeof(User), user);
  if (!user.isExists){
    return User{"-", "-", false};
  }

  return user;
}

// Получить все доступные позиции для добавления пользователя в список пользователей.
void GetAllUsers(User* users){
  for (int id=0; id<MAX_USER_COUNT; id++){
    users[id] = GetUserByID(id);
  }
  return users;
}

// Получить следующего свободного пользователя
int GetNextEmptyUserPosition(User* users, int8_t index){
  index++;
  if (!users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (!users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (!users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (!users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (!users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;

  return -1;
}

// Получить следующего свободного пользователя
int GetPrevEmptyUserPosition(User* users, int8_t index){
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (!users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (!users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (!users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (!users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (!users[index].isExists) return index;

  return -1;
}


// Получить следующего существующего пользователя
int GetNextExistsUserPosition(User* users, int8_t index){
  index++;
  if (users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;
  index++;
  if (users[index % MAX_USER_COUNT].isExists) return index % MAX_USER_COUNT;

  return -1;
}

// Получить следующего существующего пользователя
int GetPrevExistsUserPosition(User* users, int8_t index){
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (users[index].isExists) return index;
  if (--index < 0) index = MAX_USER_COUNT-1;
  if (users[index].isExists) return index;

  return -1;
}

// Записать пользователя в память
void SaveUser(int8_t id, User user){
  EEPROM.put(id*sizeof(User), user);
  return;
}

// Стереть пользователя из памяти
void DeleteUser(int8_t id){
  EEPROM.put(id*sizeof(User), User{"-", "-", false});
  return;
}
// ==========================================================================================================================================================



// ==========================================================================================================================================================
// Получить текущий режим входа
uint8_t GetCurrentMode(){
  uint8_t mode;
  EEPROM.get(MODE_POS, mode);
  return mode;
}

// Сохранить текущий режим входа
void SaveCurrentMode(uint8_t mode){
  EEPROM.put(MODE_POS, mode);
}
// ==========================================================================================================================================================



// ==========================================================================================================================================================
// Функция для генерации HOTP
long generateHOTP(const String key, unsigned long counter, int digits) {
  byte hmac_key[key.length()+1];
  key.getBytes(hmac_key, sizeof(hmac_key));

  // преобразуем счетчик в байты
  byte counter_bytes[8];
  for (int i = 0; i < 8; i++) {
    counter_bytes[i] = (counter >> (56 - i * 8)) & 0xff;
  }
  
  Sha1.initHmac(hmac_key, sizeof(hmac_key));
  Sha1.write(counter_bytes, sizeof(counter_bytes));
  byte* hmac_result = Sha1.resultHmac();
  int8_t hmac_res_signed[sizeof(hmac_result)];
  for (int i = 0; i < 20; i++) {
    if (hmac_result[i] > 127){
      hmac_res_signed[i] = (int8_t)hmac_result[i] - 256;
    }else{
       hmac_res_signed[i] = hmac_result[i];
    }
  }

  // вычисляем код
  int offset = hmac_res_signed[19] & 0x0f;

  long binary = (((long)hmac_res_signed[offset] & 0x7f) << 24);
  long b = (((long)hmac_res_signed[offset + 1] & 0xff) << 16);


  binary = binary | b;

  b = (((long)hmac_res_signed[offset + 2] & 0xff) << 8);

  binary = binary | b;

  b = ((long)hmac_res_signed[offset + 3] & 0xff);

  binary = binary | b;
  
  return binary % (long) pow(10, digits);
}

String generateRandomSecret() {
  // Символы, из которых будет состоять токен
  
  const int numChars = sizeof(chars) - 1; // Количество символов
  
  // Генерация токена
  String token = "";
  for (int i = 0; i < MAX_SIZE_HOTP_SECRET; i++) {
    int index = random(0, numChars - 1);
    char c = pgm_read_byte(&chars[index]);
    token += c;
  }
  
  return token;
}
