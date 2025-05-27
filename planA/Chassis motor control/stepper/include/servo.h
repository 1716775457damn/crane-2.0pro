const int servoPin1 = 15; // 舵机接口引脚，接橙色信号线。

void servo1(int angle)
{ // 定义一个脉冲函数
    // 发送50个脉冲
    pinMode(servoPin1, OUTPUT);
    for (int i = 0; i < 50; i++)
    {
        int pulsewidth = (angle * 11) + 500; // 将角度转化为500-2480的脉宽值
        digitalWrite(servoPin1, HIGH);       // 将舵机接口电平至高
        delayMicroseconds(pulsewidth);       // 延时脉宽值的微秒数
        digitalWrite(servoPin1, LOW);        // 将舵机接口电平至低
        delayMicroseconds(20000 - pulsewidth);
    }
    delay(100);
}
