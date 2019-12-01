
#include <Wire.h>     // библиотека для управления устройствами по I2C
#include <OneWire.h>  // библиотека для управления устройствами по 1-Wire, датчик температуры DS18B20
OneWire ds(8);        // Создаем объект OneWire для шины 1-Wire, подключен к 8 пину и через резистор 4,7 кОм к VCC
#include <DS3231.h>                 // подключаем библиотеку для RTC DS3231
DS3231  rtc(SDA, SCL);              // инициализация DS3231, ,RTC(0x57)
#include <LiquidCrystal_I2C.h>      // подключаем библиотеку для LCD 1602
LiquidCrystal_I2C lcd(0x3F, 16, 2); // присваиваем имя lcd для дисплея 16х2, ,LCD(0x3F)

// создаем символы и буквы на кириллице
byte GR[8] = { 0b00110, 0b01001, 0b01001, 0b00110, 0b00000, 0b00000, 0b00000, 0b00000 }; //знак грудус
byte L[8] = { 0b11111, 0b10001, 0b01110, 0b00000, 0b01010, 0b10101, 0b00100, 0b00000 };  //свет
byte J[8] = { 0b00100, 0b10101, 0b01010, 0b00000, 0b01010, 0b10101, 0b00100, 0b00000 };  //солнышко (холод)
byte S[8] = { 0b00101, 0b01010, 0b10100, 0b01010, 0b00101, 0b01010, 0b10100, 0b00000 };  //волны (тепло)
byte II[8] = { 0b10001, 0b10001, 0b10001, 0b11101, 0b10101, 0b10101, 0b11101, 0b00000 }; // Ы
byte PP[8] = { 0b11111, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b00000 }; // П
byte IA[8] = { 0b01111, 0b10001, 0b10001, 0b01111, 0b00101, 0b01001, 0b10001, 0b00000 }; // Я
#define buttMenu 2        // кнопка меню, INT
#define buttOk 3          // кнопка ОК, INT, PWM
#define buttPlus 4        // кнопка +
#define buttMinus 5       // кнопка -, PWM
boolean press_Menu = false;
boolean press_Ok = false;
boolean press_Plus = false;
boolean press_Minus = false;
unsigned long butt_timer; // таймер нажатия кнопки
boolean butt_lcd = false;
int ReleTeplo = 11;       // пин включения обогрева
int ReleSvet = 10;        // пин включения освещение
int ReleHolod = 9;        // пин включения вентелятор
int photocellPin = 0;     // сенсор и резистор 10 кОм подключены к a0
int photocellReading;     // считываем аналоговые значения с делителя сенсора
int LEDbrightness;        // переменная для хранения значения освещености
int LEDreleTeplo = 22;    // параметр температуры для вкл обогреваертикальные
boolean LED_Teplo = false;
int LEDreleHolod = 28;    // параметр температуры для вкл вентелятора
boolean LED_Holod = false;
int LEDreleSvet = 20;     // параметр освещенности для вкл освещения
boolean LED_Svet = false;
int TimeSvetOn = 16;      // параметр для вкл освещения по времени
int TimeSvetOff = 23;     // параметр для выкл освещения по времени
boolean Time_Svet = false;
int VKS;                  // переменная для плавного вкл(выкл) освещения
Time t;                   // переменная для хранения текущего времени
int hh, mm, ss;           // переменные для настройки времени
void setup()
{
  Serial.begin(9600);
  rtc.begin();
  pinMode(ReleTeplo, OUTPUT);
  pinMode(ReleSvet, OUTPUT);
  pinMode(ReleHolod, OUTPUT);
  pinMode(buttMenu, INPUT_PULLUP); // через внутренний подтягивающий резистор к питанию (к VCC), кнопка к GND
  pinMode(buttOk, INPUT_PULLUP);
  pinMode(buttPlus, INPUT_PULLUP);
  pinMode(buttMinus, INPUT_PULLUP);
  // rtc.setDOW(WEDNESDAY);        // Установить день недели
  /*
    Monday    понедельник 1
    Tuesday   вторник     2
    Wednesday среда       3
    Thursday  четверг     4
    Friday    пятница     5
    Saturday  суббота     6
    Sunday    воскресенье 7
  */
  // rtc.setTime(15, 37, 0);       // Установить время 15:55:00 (формат 24 часа)
  // rtc.setDate(13, 2, 2019);     // Установить дату 5 atdhfkz 2019 года

  lcd.begin(16, 2);
  lcd.init();                   // инициализация LCD дисплея
  lcd.backlight();              // включение подсветки дисплея
  lcd.createChar(1, GR);        // присваиваем символам порядковый номер, от 0 до 7
  lcd.createChar(2, L);         // свет
  lcd.createChar(3, J);         // солнце
  lcd.createChar(4, II);        // Ы
  lcd.createChar(5, S);         // тепло (волны)
  lcd.createChar(6, PP);        // П
  lcd.createChar(7, IA);        // Я
}
void loop()
{
  /////// Определяем время, дату .... ///////////////////////////////// Вычесляем параметр для вкл /////////////
  t = rtc.getTime();
  Serial.print(rtc.getDOWStr());     // Отправляем день-неделя
  Serial.print(" ");
  Serial.print(rtc.getDateStr());    // Отправляем дату
  Serial.print(" -- ");
  Serial.println(rtc.getTimeStr());  // Отправляем время
  Serial.print("Температура RTC: ");
  Serial.println(rtc.getTemp());     // Отправляем температуру RTC
  delay (100);
  Serial.println("  -  -  -  -  -  -  -  -  -  -  -");
  hh = t.hour;                       // фиксируем час
  Serial.println(hh);                // отправляем час
  //Serial.println(t.hour);          // проверяем час
  if (TimeSvetOn <= hh && hh <= TimeSvetOff ) {
    Time_Svet = true;
  } else {
    Time_Svet = false;
  }
  if (hh == 12) lcd.backlight();     // вкл подсветки дисплея
  
  /////// Определяем освещенность ////////////////////////////////// Вкючение света ////////////////////////////////
  photocellReading = analogRead(photocellPin);             // Считываем свет
  LEDbrightness = map(photocellReading, 0, 1023, 0, 100);  // Переводим в %
  if (LEDbrightness < LEDreleSvet && Time_Svet && VKS != 255) {
    VKS = VKS + 1;
    if (VKS > 254) {
      VKS = 255;
      lcd.noBacklight();                                   // выкл подсветку дисплея для экономии Ам
    }
    Serial.print("Вкл света: ");
    Serial.println(VKS);
    analogWrite(ReleSvet, VKS);                            // плавное вкл освещения, экран выкл
    LED_Svet = true;
  } else if (!Time_Svet && VKS != 0) {
    VKS = VKS - 1;
    if (VKS < 0) {
      VKS = 0;
    }
    Serial.print("Выкл света = ");
    Serial.println(VKS);
    analogWrite(ReleSvet, VKS);                            // плавное выкл освещения
    LED_Svet = false;
  }
  /////// Определяем температуру от датчика DS18b20 //////////////// Включение обогрева ////////////////////////
  byte data[2];   // Место для значения температуры

  ds.reset();     // Начинаем взаимодействие со сброса всех предыдущих команд и параметров
  ds.write(0xCC); // Даем датчику DS18b20 команду пропустить поиск по адресу. В нашем случае только одно устрйоство.
  ds.write(0x44); // Даем датчику DS18b20 команду измерить температуру. Само значение температуры мы еще не получаем - датчик его положит во внутреннюю память
  delay(900);     // Микросхема измеряет температуру, а мы ждем.(миним 750)
  ds.reset();     // Теперь готовимся получить значение измеренной температуры
  ds.write(0xCC);
  ds.write(0xBE); // Просим передать нам значение регистров со значением температуры
  // Получаем и считываем ответ
  data[0] = ds.read(); // Читаем младший байт значения температуры
  data[1] = ds.read(); // А теперь старший
  // Формируем итоговое значение:
  //    - сперва "склеиваем" значение,
  //    - затем умножаем его на коэффициент, соответсвующий разрешающей способности (для 12 бит по умолчанию - это 0,0625)
  int temperature =  ((data[1] << 8) | data[0]) * 0.0625;
  Serial.print("Температура воды: ");
  Serial.println(temperature);                             // Выводим полученное значение температуры в монитор порта

  if (temperature < LEDreleTeplo) {
    digitalWrite(ReleTeplo, LOW);                          // Вкл питание для обогрева
    LED_Teplo = true;
  } else {
    digitalWrite(ReleTeplo, HIGH);                         // Выкл питание для обогрева
    LED_Teplo = false;
  }

  if (temperature > LEDreleHolod) {
    digitalWrite(ReleHolod, HIGH);                         // Вкл питание для вентелятора
    LED_Holod = true;
  } else {
    digitalWrite(ReleHolod, LOW);                          // Выкл питание для вентелятора
    LED_Holod = false;
  }

  /////// Выводим дынные на экран LCD 1602 //////////////////////////////////////////////////////////////////////
  lcd.setCursor(0, 0);         // первая строка
  lcd.print(rtc.getTimeStr()); // 8 символ
  lcd.setCursor(8, 0);
  lcd.print(" T:");            // 3 символ
  if (LED_Teplo) {
    lcd.print("\5");           // 1 символ
  } else if (LED_Holod) {
    lcd.print("\3");           // 1 символ
  } else {
    lcd.print("-");            // 1 символ
  }
  lcd.print(" C:");            // 3 символ
  if (LED_Svet) {
    lcd.print(char(2));        // 1 символ
  } else {
    lcd.print("-");            // 1 символ
  }
  lcd.setCursor(0, 1);                    // вторая строка
  lcd.print("TEM"); lcd.print(":");       // 3 символ
  lcd.print(temperature);                 // 2 символ
  lcd.print(char(1));  lcd.print("C");    // 2 символ
  lcd.setCursor(8, 1);
  lcd.print(" OCB"); lcd.print(":");      // 4 символ
  lcd.print(LEDbrightness);               // 2 символ
  lcd.print("%");                         // 1 символ
  /////// Проверка нажатия кнопок ///////////////////////////////////////////////////////////////////////////////
  if (!digitalRead(buttMenu) && press_Menu && millis() - butt_timer > 80) {   // при нажатие кнопки меню
    press_Menu = !press_Menu;
    butt_timer = millis();
    Serial.println("_- Нажата Меню -_");
    lcd.clear();
    Click_MenuTime();                                                         // запустить функцию
  }
  if (digitalRead(buttMenu) && !press_Menu && millis() - butt_timer > 80) {   // при отпускание кнопки меню, можно без millis
    press_Menu = !press_Menu;
    butt_timer = millis();
  }
  if (!digitalRead(buttOk) && press_Ok && millis() - butt_timer > 80) {       // при нажатие кнопки Ok
    press_Ok = !press_Ok;
    butt_timer = millis();
    Serial.println("_- Нажата ОК -_");
    Click_LCD();                                                               // запустить функцию
  }
  if (digitalRead(buttOk) && !press_Ok && millis() - butt_timer > 80) {       // при отпускание кнопки Ok, можно без millis
    press_Ok = !press_Ok;
    butt_timer = millis();
    lcd.clear();                                                              // очищаем экран
  }
}
/////// Функции /////////////// Функции ////////////////////////////////////////////////////////////////////////
void Click_MenuTime() {
  delay(333);
  int var = 1;
  hh = t.hour;                                                        // фиксируем час
  mm = t.min;                                                         // фиксируем минуты
  ss = t.sec;                                                         // фиксируем секунды
  for ( int i = 0; i <= 200; i++ ) {                                  // по истичению 10 сек, если ни чего не нажал - выход из меню
    if (!digitalRead(buttPlus) && press_Plus && millis() - butt_timer > 80) {      // при нажатие кнопки Plus
      press_Plus = !press_Plus;
      butt_timer = millis();
      i = 0;
      switch (var) {
        case 1:
          hh = hh + 1;
          if (hh == 24) hh = 0;
          break;
        case 2:
          mm = mm + 1;
          if (mm == 60) mm = 0;
          break;
        case 3:
          ss = ss + 1;
          if (ss == 60) ss = 0;
          break;
        case 4:
          TimeSvetOn = TimeSvetOn + 1;
          if (TimeSvetOn >= 24) TimeSvetOn = 0;
          break;
        case 5:
          TimeSvetOff = TimeSvetOff + 1;
          if (TimeSvetOff >= 24) TimeSvetOff = 0;
          break;
        default:  // default необязателен
          break;  // выполняется, если не выбрана ни одна альтернатива
      }
    }
    if (digitalRead(buttPlus) && !press_Plus && millis() - butt_timer > 80) {      // при отпускании кнопки Plus
      press_Plus = !press_Plus;
      butt_timer = millis();
    }
    if (!digitalRead(buttMinus) && press_Minus && millis() - butt_timer > 80) {    // при нажатие кнопки Minus
      press_Minus = !press_Minus;
      butt_timer = millis();
      i = 0;
      switch (var) {
        case 1:
          hh = hh - 1;
          if (hh <= 0) hh = 24;
          break;
        case 2:
          mm = mm - 1;
          if (mm <= 0) mm = 60;
          break;
        case 3:
          ss = ss - 1;
          if (ss <= 0) ss = 60;
          break;
        case 4:
          TimeSvetOn = TimeSvetOn - 1;
          if (TimeSvetOn < 0) TimeSvetOn = 23;
          break;
        case 5:
          TimeSvetOff = TimeSvetOff - 1;
          if (TimeSvetOff < 0) TimeSvetOff = 23;
          break;
        default:  // default необязателен
          break;  // выполняется, если не выбрана ни одна альтернатива
      }
    }
    if (digitalRead(buttMinus) && !press_Minus && millis() - butt_timer > 80) {    // при отпускании кнопки Minus
      press_Minus = !press_Minus;
      butt_timer = millis();
    }
    if (!digitalRead(buttOk) && press_Ok && millis() - butt_timer > 80) {          // при нажатие кнопки Ok
      press_Ok = !press_Ok;
      butt_timer = millis();
      i = 0;
      var = var + 1;
      if (var == 6) var = 1;
    }
    if (digitalRead(buttOk) && !press_Ok && millis() - butt_timer > 80) {          // при отпускание кнопки Ok
      press_Ok = !press_Ok;
      butt_timer = millis();
    }
    if (!digitalRead(buttMenu) && millis() - butt_timer > 80) {                    // при нажатие кнопки Menu
      butt_timer = millis();
      lcd.clear();
      rtc.setTime(hh, mm, ss);
      Click_MenuParam();   // потом возвращается сюда ??? а нужен выход
      break;
    }
    Serial.print("Выход через: ");
    Serial.println(i);
    lcd.setCursor(1, 0);                    // первая строка, 1 символ
    lcd.print("BPEM"); lcd.print("\7");     // 5 символ
    lcd.setCursor(9, 0);
    lcd.print("BK");                        // 2 символ
    lcd.setCursor(12, 0);
    lcd.print("B"); lcd.print("\4"); lcd.print("K");

    lcd.setCursor(0, 1);                    // вторая строка
    lcd.print(hh); lcd.print(":");          // 3 символ
    lcd.print(mm); lcd.print(":");          // 3 символ
    lcd.print(ss); lcd.print(" ");          // 3 символ
    lcd.setCursor(9, 1);
    lcd.print(TimeSvetOn);                  // 2 символ
    lcd.print(" ");
    lcd.setCursor(12, 1);
    lcd.print(TimeSvetOff);                 // 2 символ
    lcd.print(" ");
  }
  rtc.setTime(hh, mm, ss);
  lcd.clear();
}

void Click_MenuParam() {
  delay(333);
  int var = 1;
  for ( int i = 0; i <= 200; i++ ) {                                               // по истичению 10 сек - выход из меню
    if (!digitalRead(buttPlus) && press_Plus && millis() - butt_timer > 80) {      // при нажатие кнопки Plus
      press_Plus = !press_Plus;
      butt_timer = millis();
      i = 0;
      switch (var) {
        case 1:
          LEDreleTeplo = LEDreleTeplo + 1;
          break;
        case 2:
          LEDreleHolod = LEDreleHolod + 1;
          break;
        case 3:
          LEDreleSvet = LEDreleSvet + 1;
          break;
        default:  // default необязателен
          break;  // выполняется, если не выбрана ни одна альтернатива
      }
    }
    if (digitalRead(buttPlus) && !press_Plus && millis() - butt_timer > 80) {      // при отпускании кнопки Plus
      press_Plus = !press_Plus;
      butt_timer = millis();
    }
    if (!digitalRead(buttMinus) && press_Minus && millis() - butt_timer > 80) {    // при нажатие кнопки Minus
      press_Minus = !press_Minus;
      butt_timer = millis();
      i = 0;
      switch (var) {
        case 1:
          LEDreleTeplo = LEDreleTeplo - 1;
          break;
        case 2:
          LEDreleHolod = LEDreleHolod - 1;
          break;
        case 3:
          LEDreleSvet = LEDreleSvet - 1;
          break;
        default:  // default необязателен
          break;  // выполняется, если не выбрана ни одна альтернатива
      }
    }
    if (digitalRead(buttMinus) && !press_Minus && millis() - butt_timer > 80) {    // при отпускании кнопки Minus
      press_Minus = !press_Minus;
      butt_timer = millis();
    }
    if (!digitalRead(buttOk) && press_Ok && millis() - butt_timer > 80) {          // при нажатие кнопки Ok
      press_Ok = !press_Ok;
      butt_timer = millis();
      i = 0;
      var = var + 1;
      if (var == 4) var = 1;
    }
    if (digitalRead(buttOk) && !press_Ok && millis() - butt_timer > 80) {          // при отпускание кнопки Ok
      press_Ok = !press_Ok;
      butt_timer = millis();
    }
    if (!digitalRead(buttMenu) && millis() - butt_timer > 80) {                    // при нажатие кнопки Menu
      butt_timer = millis();
      i = 190;
      break;  // как выйти из for ?? while(выражение){ делается пока выражение истина}
    }
    Serial.print("Выход через: ");
    Serial.println(i);
    lcd.setCursor(1, 0);                      // первая строка, 1 символ
    lcd.print("TE"); lcd.print("\6");         // 4 символ
    lcd.print("\5");
    lcd.setCursor(6, 0);
    lcd.print("BEH"); lcd.print("\3");        // 4 символ
    lcd.setCursor(11, 0);
    lcd.print("CBE"); lcd.print("\2");        // 4 символ

    lcd.setCursor(1, 1);                      // вторая строка, 1 символ
    lcd.print(LEDreleTeplo); lcd.print("\1"); // 3 символ
    lcd.setCursor(6, 1);
    lcd.print(LEDreleHolod); lcd.print("\1");  // 3 символ
    lcd.setCursor(11, 1);
    lcd.print(LEDreleSvet); lcd.print("%");   // 3 символ
  }
  lcd.clear();
}

void Click_LCD() {
  delay(10);
  if (butt_lcd) {
    lcd.backlight();                          // вкл подсветку lcd
  } else {
    lcd.noBacklight();                        // выкл подсветку lcd
  }
  butt_lcd = !butt_lcd;
}
