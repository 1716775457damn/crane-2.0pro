
#define SLEEP1  4
#define DIR1    5
#define STEP1   6

#define SLEEP2  7
#define DIR2    15
#define STEP2   16

#define ADC     17

long int ADC_Val;
long int Voltage_Val;

int Get_ADC_Average(char times)
{
  long int temp = 0;
  char i;
  for(i=0; i<times; i++)
  {
    temp += analogRead(ADC);
  }
  return (int)temp/times;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(SLEEP1, OUTPUT);          //控制步进电机1的SLEEP引脚，输出0时进入休眠
  pinMode(DIR1, OUTPUT);            //控制步进电机1的DIR引脚，控制步进电机转动方向

  pinMode(SLEEP2, OUTPUT);          //控制步进电机2的SLEEP引脚，输出0时进入休眠
  pinMode(DIR2, OUTPUT);            //控制步进电机2的DIR引脚，控制步进电机转动方向

  pinMode(ADC, INPUT);              //初始化A0为ADC引脚

  digitalWrite(SLEEP1, 1);          //不休眠
  digitalWrite(DIR1, 1);            //顺时针转动
  digitalWrite(SLEEP2, 1);          
  digitalWrite(DIR2, 1);  

  analogWrite(STEP1, 50);
  analogWrite(STEP2, 50);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  ADC_Val = Get_ADC_Average(5);                   //每五次取一次ADC的平均值
  Voltage_Val = (long int)(ADC_Val*5*11*100/1023);     //转换为电源电压值
  Serial.print("当前电池电压为：");
  Serial.print(Voltage_Val/100);
  Serial.print(".");
  Serial.print(Voltage_Val%100);
  Serial.println("V");
  delay(500);
}
