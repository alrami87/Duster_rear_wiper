#define PumpPin 2   // сигнал мотора заднего омывателя
#define SwitcherPin 3   // сигнал заднего дворника
#define WiperPin 13   // управление задним дворником

#define pump_interval 2000   // время непрерывной работы заднего дворника после омывания окна
#define min_switcher_interval 2000   // минимальный интервал работы заднего дворника
#define max_switcher_interval 30000   // максимальный интервал работы заднего дворника
#define switcher_duration 100   // интервал включения питания заднего дворника
long switcher_interval = 5000;   // дефолтный интервал работы заднего дворника

long on_time, off_time, pump_time, switcher_time = 0, switcher_on_time = 0, new_interval, wiper_state = 0;

bool switcher_prev_state = 0, protect_state = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Started!!!");
  pinMode(WiperPin, OUTPUT);
  digitalWrite(WiperPin, LOW);   // подаем минус на дворник
  pinMode(PumpPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PumpPin), pump, LOW);   // отслеживание омывания
  if (digitalRead(SwitcherPin))  protect_state = 1;   // если уже включен, то включаем защиту от забытого включения
}

void loop() {
  if (digitalRead(SwitcherPin)) {   // если дворник включен

    if (!switcher_prev_state && long(millis() - switcher_on_time) > switcher_duration) {   // включен только что, учитываем дребезг
      Serial.println("--ON");
      switcher_prev_state = 1;   // запомнили включенное состояние
      switcher_on_time = millis();   // запомнили время включения, чтобы не было дребезга

      if (switcher_time != 0)  {   // не первое включение
        new_interval = millis() - switcher_time;
        Serial.println("interval = " + String(new_interval / 1000.0));

        if (new_interval > min_switcher_interval && new_interval < max_switcher_interval) {   // если интервал в диапазоне
          switcher_time = millis();
          switcher_interval = new_interval;   // задаем новый интервал
          on_time = millis() - switcher_interval;   // сразу включим дворник
          Serial.println("new interval = " + String(switcher_interval / 1000.0));
        }

        if (new_interval > switcher_duration && new_interval < min_switcher_interval) {   // если интервал меньше допустимого
          switcher_time = millis();
          switcher_interval = 0;   // включаем непрерывный режим
          Serial.println("new interval = " + String(switcher_interval / 1000.0));
        }
      }

    }

    if (long(millis() - on_time) > switcher_interval && !wiper_state && !protect_state) {   // пора включить задний зворник
      on_time = millis();   // время текущего включения
      off_time = millis();   // время текущего отключения
      wiper_state = 1;
      digitalWrite(WiperPin, wiper_state);   // подаем плюс на дворник
      Serial.println("             on");

    }

  } else {   // если дворник выключен

    if (switcher_prev_state  && long(millis() - switcher_on_time) > switcher_duration) {   // выключен только что и работал в непрерывном режиме
      Serial.println("   __OFF");
      switcher_prev_state = 0;   // запомнили отключенное состояние
      if (switcher_interval == 0)  switcher_time = millis();   // запоминаем время отключения
      wiper_state = 0;
      digitalWrite(WiperPin, wiper_state);   // подаем минус на дворник
      switcher_on_time = millis();   // запомнили время выключения, чтобы не было дребезга
      on_time = millis() + switcher_duration;   // защита от дребезга выключения в непрерывном режиме
      protect_state = 0;   // отключаем защиту забытого включения
    }
  }

  if (long(millis() - off_time)  > switcher_duration && switcher_interval != 0 && wiper_state) {   // пора выключить дворник
    off_time = millis() + switcher_interval;   // // время следующего отключения
    wiper_state = 0;
    digitalWrite(WiperPin, wiper_state);   // подаем минус на дворник
    switcher_time = millis();   // запоминаем время отключения
    Serial.println("                  off");
  }

}

void pump() {
  if (millis() - pump_time > switcher_duration) {   // если включили не ранее последнего включения омывания
    pump_time = millis();   // задаем время омывания

    wiper_state = 1;
    digitalWrite(WiperPin, wiper_state);   // подаем плюс на дворник
    //on_time = millis() - switcher_interval + pump_interval;   // задаем время следующего включения дворника после задержки непрерывной работы после омывания
    off_time = millis() + pump_interval;   // задаем задержку непрерывной работы дворника после омывания
    Serial.println("on -->");
  }
}
