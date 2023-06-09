#define FPSTR(pstr) (const __FlashStringHelper*)(pstr)

// Замок
#define LOCK_PIN 12

// Светодиод
#define RED_PIN 17
#define GREEN_PIN 16
#define BLUE_PIN 15  

// Динамик
#define PIEZO_PIN 14

// RX и TX датчика отпечатков пальцев.
#define RX_FINGERPRINT 11
#define TX_FINGERPRINT 10

// ==========================================================================================================================================================

#define ROWS 4
#define COLS 4

// Кнопки
#define OK_BUTTON 'A'
#define UP_BUTTON 'B'
#define DOWN_BUTTON 'C'
#define EXIT_BUTTON 'D'

// Статус коды
#define OK_CODE 0
#define EXIT_CODE -1
#define INCORRECT_PIN_CODE -2
#define FINGER_NOT_FOUND_CODE -3
#define FINGER_ERROR_CODE -4
#define CREATE_FINGER_MODEL_ERROR -5
#define STORE_FINGER_MODEL_ERROR -6
#define INCORRECT_TOKEN_CODE -7
#define FINGER_FOUND_CODE -8

// Режимы входа
#define PIN_MODE 0
#define FINGER_MODE 1
#define TOKEN_MODE 2
#define PIN_AND_FINGER_MODE 3
#define PIN_AND_TOKEN_MODE 4
#define FINGER_AND_TOKEN_MODE 5
#define PIN_FINGER_TOKEN_MODE 6

// Меню
#define ADD_USER_MENU 0
#define DELETE_USER_MENU 1
#define UPDATE_MODE_MENU 2
#define OPEN_MENU 3
#define EXIT_MENU 4

// Ограничения по размерам
#define MAX_SIZE_HOTP_SECRET 8
#define MAX_SIZE_HOTP_TOKEN 8
#define MAX_PIN_SIZE 4
#define MAX_USER_COUNT 5

// ==========================================================================================================================================================
// Размер клавиатуры
#define ROWS 4
#define COLS 4
const byte rowPins[ROWS] = {2, 3, 4, 5}; // Цифровые пины по ряды
const byte colPins[COLS] = {6, 7, 8, 9}; // Цифровые пины по столбцам

const char hexaKeys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'},
  };

const char chars[] PROGMEM = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";


const char MetuItem1[] PROGMEM = "Меню>Доб.Польз.";
const char MetuItem2[] PROGMEM = "Меню>Удал.Польз.";
const char MetuItem3[] PROGMEM = "Меню>Обн.Реж.";
const char MetuItem4[] PROGMEM = "Меню>Открыть";
const char MetuItem5[] PROGMEM = "Меню>Выйти";


const char* const MenuItems[] PROGMEM = {MetuItem1, MetuItem2, MetuItem3, MetuItem4, MetuItem5};
const int8_t MenuItemsSize = 5;

const char UpdateModeMenuItem1[] PROGMEM = "Код";
const char UpdateModeMenuItem2[] PROGMEM = "Палец";
const char UpdateModeMenuItem3[] PROGMEM = "Токен";
const char UpdateModeMenuItem4[] PROGMEM = "Код+Палец";
const char UpdateModeMenuItem5[] PROGMEM = "Код+Токен";
const char UpdateModeMenuItem6[] PROGMEM = "Палец+Токен";
const char UpdateModeMenuItem7[] PROGMEM = "Всё";


const char* const UpdateModeMenuItems[] PROGMEM = {UpdateModeMenuItem1, UpdateModeMenuItem2, UpdateModeMenuItem3, UpdateModeMenuItem4, UpdateModeMenuItem5, UpdateModeMenuItem6, UpdateModeMenuItem7};
const int8_t UpdateModeMenuItemsSize = 7;
