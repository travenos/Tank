#define led1 13//Лампочка 1
#define dir1 6
#define val1 5//10
#define dir2 3
#define val2 11//5

#define mI1 1
#define mI2 7

short int u2 = 0; //Значение ШИМ-модуляции для левого двигателя
short int u1 = 0; //Значение ШИМ-модуляции для правого двигателя
bool MRg = false; //Едит ли вперёд правый мотор
bool MRb = false; //Едит ли назад правый мотор
bool MLg = false; //Едит ли вперёд левый мотор
bool MLb = false; //Едит ли назад левый мотор

bool SAUon = false; //Запущено ли автоматическое регулирование

const double Kpv = 700; //Коэффициенты регулятора
const double Kpw = 90;
const double Kiv = 5000; //Коэффициенты регулятора
const double Kiw = 800;

const double d = 0.0875; //Половина ширины МРП
const double R2 = 3.3; //Сопротивление якорной обмотки левого мотора
const double R1 = 3.1; //Сопротивление якорной обмотки правого мотора
const double Keds1 = 0.1165; //Связь ЭДС якоря со скоростью
const double Keds2 = 0.11; //Связь ЭДС якоря со скоростью

double v0 = 0; //Скорость уставки
double w0 = 0; //Угловая скорость уставки
long double integv = 0; //Интеграл ошибки линейной скорости
long double integw = 0; //Интеграл ошибки угловой скорости

long t = 0; //Текущее время в микросекундах
long tn;
double h; //шаг

double v1, v2, v, w;

double v1last[] = {0, 0, 0, 0, 0};
double v2last[] = {0, 0, 0, 0, 0};
const short int Nmed = 5; //Длина массива

long umean = 0; //Средннее значение воздействий
long udif = 0; //Разность воздействий
//
void setup()
{
  pinMode(led1, OUTPUT); //Устанавливаем пин на выход
  pinMode(dir2, OUTPUT); //
  pinMode(dir1, OUTPUT); //
  pinMode(val2, OUTPUT); //
  pinMode(val1, OUTPUT); //
  Serial.begin(115200);//Запуск аппаратного сериал порта на скорости 9600 бод

  long t = 0; //Время, в микросекундах
}

char inData[1000];//Массив элементов char - буфер для хранения принятых данных от вайфай модуля по последовательному порту
short int n = 0;
//

void loop()
{
  tn = micros();
  h = (tn - t) / 1000000.0;
  t = tn;

/*
  delay(500);
  double v1 = getV(mI1, u1, R1, true, TCNT0); //Скорость правой гусеницы
  double v2 = getV(mI2, u2, R2, true, TCNT2); //Скорость левой гусеницы
      v1 = medFilter(v1last, v1);
    v2 = medFilter(v2last, v2);
  
  Serial.print("v1=");
  Serial.print(v1);
  Serial.print("; v2=");
  Serial.print(v2);
  Serial.print('\n');
  */
  
  if (SAUon) //Регулятор
  {

    v1 = getV(mI1, u1, R1, Keds1, MRg, TCNT0); //Скорость правой гусеницы
    v2 = getV(mI2, u2, R2, Keds2, MLg, TCNT2); //Скорость левой гусеницы

    //Фильтрация
    v1 = medFilter(v1last, v1);
    v2 = medFilter(v2last, v2);

    v = (v1 + v2) / 2; //Определение линейной скорости
    w = (v2 - v1) / (2 * d); //Определение угловой скорости

    //Интегрирование методом Эйлера
    integv += (v0 - v) * h;
    integw += (w0 - w) * h;

    //Выработка управляющих значений
    umean = Kpv * (v0 - v) + Kiv * integv;
    udif = Kpw * (w0 - w) + Kiw * integw;
    if (udif > 255)
      udif = 255;
    else if (udif < -255)
      udif = -255;
    if ((umean + abs(udif / 2)) > 255)
      umean = 255 - abs(udif / 2);

    //Применение воздействий
    drive1(umean - udif / 2);
    drive2(umean + udif / 2);
  }
  readCOM(); //Получение команды по COM-порту
}

//Считывание скорости
double getV(short int pin, short int u, double R, double Keds, bool dir, volatile uint8_t &counter)
{
  int level;
  double U = (counter < u); //Определение U
  level = analogRead(pin);
  level -= 512;
  double I = level * 0.024414063;
  U *= 8.1;
  if (!dir)
    U*=-1;
  v = (U + I * R) * Keds;
  return v;
}

double medFilter(double vlast[], double vi) //Медианный фильтр
{
  double *vsort = new double[Nmed];
  for (int i = Nmed - 1; i >= 1; i--) //Обновление массивов последних значений
  {
    vlast[i] = vlast[i - 1];
    vsort[i] = vlast[i];
  }
  vlast[0] = vi;
  vsort[0] = vi;
  //Сортировка (выборочная сортировка)
  for (int i = 0; i <= Nmed - 2; i++)
  {
    int min_i = i;
    for (int j = i + 1; j <= Nmed - 1; j++)
    {
      if (vsort[j] < vsort[min_i])
        min_i = j;
    }
    double temp = vsort[i];
    vsort[i] = vsort[min_i];
    vsort[min_i] = temp;
  }
  double mediana = vsort[Nmed / 2];
  delete [] vsort;
  return mediana;
}

void readCOM() //Чтение COM-порта
{
  while (Serial.available() > 0) // Пока доступна инфа с серийного порта
  {
    inData[n] = Serial.read();
    n++;
    delay(1);//задержка 1мс
  }
  if (n > 0)
  {
    char *p = NULL;
    p = strtok(inData, ":");
    while (p != NULL)
    {
      findCommand(p);
      p = strtok (NULL, ":");
    }
    for (short unsigned int i = 0; i <= n - 1; i++) // "Очищаем массив"
      inData[i] = 0;
    n = 0;
  }
}

void SAUoff()
{
  SAUon = false;
  drive1(0);
  drive2(0);
  
  w0 = 0;
  v0 = 0;
  for (int i = 0; i <= Nmed - 1; i++)
  {
    v1last[i] = 0;
    v2last[i] = 0;
  }
  integv = 0;
  integw = 0;
}
//
void findCommand(char *p) //Дешифратор
{
  if (p[0] == 'v')
  {
    p = &p[1];
    v0 = atof(p);
    if (v0 > 0.7)
      v0 = 0.7; //Максимальная линейная скорость
    if (!MRg && !MRb)
      u1 = 0;
    if (!MLg && !MLb)
      u2 = 0;
    if (v0 == 0)
      SAUoff();
    else
      SAUon = true;
    return;
  }
  if (p[0] == 'w')
  {
    p = &p[1];
    double wn = atof(p);
    //if (sign(wn)!=sign(w0))
    integw = 0;
    w0 = wn;
    if (w0 > 3)
      w0 = 3; //Максимальная угловая скорость
    else if (w0 < -3)
      w0 = -3; //Максимальная угловая скорость
    SAUon = true;
    return;
  }
  if (!strcmp(p, "SAUoff"))
  {
    SAUoff();
    return;
  }
  if (!SAUon)
  {
    if (p[0] == 'R') //Запуск правого двигателя с заданной скоростью
    {
      p = &p[1];
      int temp = atoi(p); //Проверка на подходящее значение
      drive1(temp);
      return;
    }
    if (p[0] == 'L') //Запуск левого двигателя с заданной скоростью
    {
      p = &p[1];
      int temp = atoi(p); //Проверка на подходящее значение
      drive2(temp);
      return;
    }
    if (p[0] == 'B' && isdigit(p[1]) && isdigit(p[2]) && isdigit(p[3])) //Общая скорость
    {
      p = &p[1];
      short int temp = atoi(p); //Проверка на подходящее значение
      if (temp > 255)
        temp = 255;
      else if (temp <= 0)
      {
        temp = 0;
        MLg = false;
        MRg = false;
        MLb = false;
        MRb = false;
      }
      u2 = temp;
      u1 = temp;
      if (MRg)
        analogWrite(dir1, u1);
      else if (MRb)
        analogWrite(val1, u1);
      if (MLg)
        analogWrite(dir2, u2);
      else if (MLb)
        analogWrite(val2, u2);
      return;
    }
    if (strstr(p, "MR")) //Операции с правым двигателем
    {
      if (!strcmp(p, "MRgo"))
      {
        drive1(u1);
        return;
      }
      if (!strcmp(p, "MRback"))
      {
        drive1(-u1);
        return;
      }
      if (!strcmp(p, "MRstop"))
      {
        int temp=u1;
        drive1(0);
        u1=temp;
        return;
      }
      return;
    }
    if (strstr(p, "ML")) //Операции с левым двигателем
    {
      if (!strcmp(p, "MLgo"))
      {
        drive2(u2);
        return;
      }
      if (!strcmp(p, "MLback"))
      {
        drive2(-u2);
        return;
      }
      if (!strcmp(p, "MLstop"))
      {
        int temp=u2;
        drive2(0);
        u2=temp;
        return;
      }
      return;
    }
  }
  if (!strcmp(p, "led1on")) //Тестовое включение лампочки
  {
    digitalWrite(led1, HIGH);
    return;
  }
  if (!strcmp(p, "led1off"))
  {
    digitalWrite(led1, LOW);
    return;
  }
}

void drive1(int u) //Управление правым двигателем
{
  bool dir=true; //Направление движения
  if (u<0)
    dir=false; 
  u1=abs(u);
  if (u1>255)
    u1=255;
  if (u==0)
  {    
    MRg=false;
    MRb=false;
  }
  else
  {    
    MRg=dir;
    MRb=!dir;
  }
  if (dir)
  {
    analogWrite(dir1, u1);
    digitalWrite(val1, 0);
  }
  else
    {
    analogWrite(val1, u1);
    digitalWrite(dir1, 0);
  }
}

void drive2(int u) //Управление левым двигателем
{
  bool dir=true; //Направление движения
  if (u<0)
    dir=false; 
  u2=abs(u);
  if (u2>255)
    u2=255;
  if (u==0)
  {    
    MLg=false;
    MLb=false;
  }
  else
  {    
    MLg=dir;
    MLb=!dir;
  }
  if (dir)
  {
    analogWrite(dir2, u2);
    digitalWrite(val2, 0);
  }
  else
    {
    analogWrite(val2, u2);
    digitalWrite(dir2, 0);
  }
}

