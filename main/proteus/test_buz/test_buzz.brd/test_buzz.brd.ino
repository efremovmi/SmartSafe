// Динамик
#define PIEZO_PIN A0


void setup() {
  pinMode(PIEZO_PIN, OUTPUT);
  // put your setup code here, to run once:
}

void loop() {
  tone(PIEZO_PIN, 523, 100);
  delay(150);  
  tone(PIEZO_PIN, 1319, 100);
  delay(150);
  tone(PIEZO_PIN, 987, 500);
  
  delay(4000);
  // put your main code here, to run repeatedly:
}
