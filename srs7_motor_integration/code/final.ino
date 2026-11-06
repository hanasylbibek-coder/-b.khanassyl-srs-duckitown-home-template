// ============================================================
// СРС №7: HC-SR04 + L298N (логика) + LED индикация
// Wokwi-версия: мотор заменен на LED, ENA выведен на Logic Analyzer
// ============================================================

// --- Пины HC-SR04 ---
const int TRIG_PIN = 9;
const int ECHO_PIN = 10;

// --- Пины кнопки ---
const int BUTTON_PIN = 2;

// --- Пины светодиодов расстояния ---
const int LED_RED = 13;    // < 20 см
const int LED_YELLOW = 12; // 20-50 см
const int LED_GREEN = 11;  // > 50 см

// --- Пины управления "мотором" (L298N) ---
const int IN1 = 5;   // Направление 1 (вперед)
const int IN2 = 6;   // Направление 2 (назад)
const int ENA = 3;   // ШИМ (скорость)

// --- Переменные ---
unsigned long duration;
int distance;
bool motorActive = false;

void setup() {
  Serial.begin(9600);

  // Настройка пинов
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Кнопка с подтяжкой к питанию
  
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);

  // Изначально мотор выключен
  stopMotor();
}

void loop() {
  // 1. Проверка кнопки
  // Если кнопка отпущена (HIGH из-за INPUT_PULLUP) - всё выключаем
  if (digitalRead(BUTTON_PIN) == HIGH) {
    stopMotor();
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    
    // Чтобы не спамить в Serial, выводим сообщение только при смене состояния
    if (motorActive) {
      Serial.println("Кнопка отпущена: мотор СТОП, все LED выключены.");
      motorActive = false;
    }
    return; // Пропускаем остальной цикл
  }

  // Если кнопка нажата - начинаем измерение
  motorActive = true;
  measureDistance();

  // 2. Проверка на ошибку датчика (тайм-аут)
  if (duration == 0) {
    stopMotor();
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    Serial.println("ОШИБКА: Тайм-аут датчика (эхо не получено). Мотор СТОП.");
    return;
  }

  // 3. Логика управления на основе расстояния
  // distance уже рассчитано в measureDistance()

  if (distance < 20) {
    // Меньше 20 см: СТОП, красный LED
    stopMotor();
    digitalWrite(LED_RED, HIGH);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, LOW);
    Serial.print("Расстояние: "); Serial.print(distance); Serial.println(" см. СТОП. Красный LED.");
  } 
  else if (distance >= 20 && distance <= 50) {
    // 20-50 см: ВПЕРЕД, PWM 120, желтый LED
    moveForward(120);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, HIGH);
    digitalWrite(LED_GREEN, LOW);
    Serial.print("Расстояние: "); Serial.print(distance); Serial.println(" см. ВПЕРЕД (PWM 120). Желтый LED.");
  } 
  else {
    // Больше 50 см: ВПЕРЕД, PWM 255, зеленый LED
    moveForward(255);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_YELLOW, LOW);
    digitalWrite(LED_GREEN, HIGH);
    Serial.print("Расстояние: "); Serial.print(distance); Serial.println(" см. ВПЕРЕД (PWM 255). Зеленый LED.");
  }

  delay(100); // Небольшая пауза для стабильности
}

// --- Вспомогательные функции ---

// Измерение расстояния
void measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Ждем ответа, тайм-аут 30 мс (около 5 метров)
  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  if (duration > 0) {
    distance = duration / 58; // Перевод в сантиметры
  } else {
    distance = 0; // Ошибка
  }
}

// Движение вперед
void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, speed);
}

// Движение назад (для теста реверса)
void moveBackward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  analogWrite(ENA, speed);
}

// Остановка мотора
void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);
}
