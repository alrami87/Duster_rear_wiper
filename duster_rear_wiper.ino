#define PumpPin 2   // сигнал мотора заднего омывателя
#define SwitcherPin 3   // сигнал заднего дворника
#define WiperPin 4   // управление задним дворником

#define pump_interval 3500   // время непрерывной работы заднего дворника после омывания окна
#define min_switcher_interval 2000   // минимальный интервал работы заднего дворника
#define max_switcher_interval 20000   // максимальный интервал работы заднего дворника
#define switcher_duration 200   // интервал включения питания заднего дворника
long switcher_interval = 5000;   // дефолтный интервал работы заднего дворника

long on_time, off_time, pump_time = 0, switcher_off_time = 0, switch_time = 0, new_interval;

bool switcher_prev_state = 0, protect_state = 0, wiper_state = 0;
//  состояние переключателя / состояние защиты / состояние дворника

void setup() {
  Serial.begin(9600);
  Serial.println("Started!!!");
  pinMode(WiperPin, INPUT);  // по-умолчанию пин управлением дворника работает на вход, выключая дворник
  pinMode(PumpPin, INPUT);  // пин омывателя
  attachInterrupt(digitalPinToInterrupt(PumpPin), pump, CHANGE);   // отслеживание омывания
  pinMode(SwitcherPin, INPUT);  // пин переключателя дворников
  if (digitalRead(SwitcherPin)) {   // если дворник уже включен, то включаем защиту от забытого включения
    protect_state = 1;
    Serial.println("Protect ON");
  }
}

void loop() {
  if (digitalRead(SwitcherPin)) {   // если дворник включен

    if (!switcher_prev_state && long(millis() - switch_time) > switcher_duration) {   // включен только что, учитываем дребезг
      Serial.println("--ON");
      switcher_prev_state = 1;   // запомнили включенное состояние
      switch_time = millis();   // запомнили время переключения, чтобы не было дребезга

      on_time = millis() - switcher_interval;  // сразу включим дворник

      if (switcher_off_time != 0)  {   // не первое включение
        new_interval = millis() - switcher_off_time;   // вычисляем время с последнего выключения
        Serial.println("time = " + String(new_interval / 1000.0));

        if (new_interval > min_switcher_interval && new_interval < max_switcher_interval) {   // если интервал в диапазоне
          switcher_interval = new_interval;   // задаем новый интервал
          on_time = millis() - switcher_interval;   // сразу включим дворник
          Serial.println("set new interval = " + String(switcher_interval / 1000.0));
        }

        if (new_interval > switcher_duration && new_interval < min_switcher_interval) {   // если интервал меньше допустимого
          switcher_interval = 0;   // включаем непрерывный режим
          Serial.println("set new interval = " + String(switcher_interval / 1000.0));
        }
      }

    }

    if (long(millis() - on_time) > switcher_interval && !wiper_state && !protect_state) {   // пора включить задний зворник
      on_time = millis();   // запоминаем время текущего включения
      wiper_state = 1;
      power();   // включаем дворник
      Serial.println("             on");

    }

  } else {   // если дворник выключен

    if (switcher_prev_state  &&  long(millis() - switch_time) > switcher_duration) {   // выключен только что и это не дребезг
      Serial.println("   __OFF");
      switcher_prev_state = 0;   // запомнили отключенное состояние
      switch_time = millis();   // запомнили время переключения, чтобы не было дребезга

      if (switcher_interval == 0) {
        switcher_off_time = millis();   // запоминаем время отключения
        wiper_state = 0;
        power();   // отключаем дворник
      }

      if (protect_state) {
        protect_state = 0;   // отключаем защиту забытого включения
        Serial.println("Protect OFF");
      }

    }
  }

  if (long(millis() - on_time)  > switcher_duration && switcher_interval != 0 && wiper_state) {   // пора выключить дворник и не включен непрерывный режим
    wiper_state = 0;
    power();   // отключаем дворник
    switcher_off_time = millis();   // запоминаем время отключения
    protect_state = 0;   // отключаем защиту забытого включения
    Serial.println("                  off");
  }

}
////////// отслеживание омывателя
void pump() {
  if (millis() - pump_time > switcher_duration) {   // если включили не ранее последнего включения омывания (антидребезг)
    pump_time = millis();   // задаем время последнего омывания, чтоы не было дребезга

    wiper_state = 1;
    power();   // включаем на дворник
    on_time = millis() + pump_interval;   // задаем время следующего включения дворника после задержки непрерывной работы после омывания
    Serial.println("on -->");

    if (protect_state) {
      protect_state = 0;   // отключаем защиту забытого включения
      Serial.println("Protect OFF");
    }
  }
}
////////// управление питанием дворника
void power() {
  digitalWrite(LED_BUILTIN, wiper_state);   // светодиод показывает состояние питания дворника
  if (wiper_state) {   // если надо включить дворник
    pinMode(WiperPin, OUTPUT);
    digitalWrite(WiperPin, HIGH);   // подаем плюс, включаем дворник
  } else {   // если надо отключить дворник
    pinMode(WiperPin, INPUT); // включаем пин на вход, подтяжка на (+) отключает дворник
  }
}
