
int chaosheng ; // 初始化全局变量，默认值为-1
#define ECHOPIN 33
#define TRIGPIN 25
float distance;
int b = 0;
int c = 0;
int measureDistanceAndSetState()
{
    pinMode(ECHOPIN, INPUT);
    pinMode(TRIGPIN, OUTPUT);
    float totalDistance = 0; // 用于存储三次读取的总和
    int numReadings = 5;     // 定义读取次数

    for (int i = 0; i < numReadings; i++)
    {
        // 触发超声波传感器发送信号
        digitalWrite(TRIGPIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIGPIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIGPIN, LOW);

        // 使用pulseIn函数读取脉冲宽度
        float distance = pulseIn(ECHOPIN, HIGH);

        // 计算距离，58是根据声速和脉冲时间计算出的距离转换系数
        distance = distance / 58;
        totalDistance += distance; // 累加每次的距离读数

        // 等待一段时间，确保传感器稳定
        delay(50);
    }
    float averageDistance = totalDistance / numReadings;

    // 打印平均距离值
    // Serial.print("Average Distance: ");
    // Serial.println(averageDistance);


    // 根据平均距离设置chaosheng状态
    if (averageDistance <= 9)
    {
        chaosheng = 0; // 距离小于等于4时，设置为0
    }
    else
    {
        chaosheng = 1; // 距离大于4时，设置为1
    }
    return chaosheng;
}

// 在setup或loop中调用measureDistanceAndSetState函数
// void setup()
// {
//     // 初始化代码，如设置引脚模式、启动串口通信等
// }

// void loop()
// {
//     // 根据需要周期性地调用测量和设置状态函数
//     measureDistanceAndSetState();
//     // 根据chaosheng的值执行相应动作...
// }

/**
 * 读取传感器值，并根据设定的阈值转换为二进制状态，然后根据状态决定循迹控制行为。
 */
int tracingcontrol;
int combinedSensorControl()
{
    // 定义各个传感器的阈值
    uint16_t thresholdR1 = 800;
    uint16_t thresholdR2 = 800;
    uint16_t thresholdM = 800;
    uint16_t thresholdL1 = 800;
    uint16_t thresholdL2 = 800;
    int a;
    int b;
    int c;
    int d;
    int e;

    pinMode(13, INPUT);
    pinMode(12, INPUT);
    pinMode(14, INPUT);
    pinMode(27, INPUT);
    pinMode(26, INPUT);

    a = analogRead(13);
    b = analogRead(12);
    c = analogRead(14);
    d = analogRead(27);
    e = analogRead(26);
    // Serial.print("a: ");
    // Serial.print(a);
    // Serial.print("b: ");
    // Serial.print(b);
    // Serial.print("c: ");
    // Serial.print(c);
    // Serial.print("d: ");
    // Serial.print(d);
    // Serial.print("e: ");
    // Serial.print(e);

    // 根据各个传感器的阈值转换为二进制状态
    a = a > thresholdR2 ? 1 : 0;
    b = b > thresholdR1 ? 1 : 0;
    c = c > thresholdM ? 1 : 0;
    d = d > thresholdL1 ? 1 : 0;
    e = e > thresholdL2 ? 1 : 0;


    // 根据传感器状态设置循迹控制变量
    // 此处简化了条件判断，合并了相同的控制状态设置
    if ((d == 0 || e == 0) && c == 0 && b == 1 && a == 1)
    {
        tracingcontrol = 0;
    }
    else if ((d == 0 || e == 0) && c == 1 && b == 1 && a == 1)
    {
        tracingcontrol = 0;
    }
    else if (d == 1 && e == 1 && c == 0 && b == 1 && a == 1)
    {
        tracingcontrol = 1;
    }
    else if (d == 1 && e == 1 && c == 1 && b == 1 && a == 1)
    {
        tracingcontrol = 1;
    }
    else if ((a == 0 || b == 0) && c == 0 && d == 1 && e == 1)
    {
        tracingcontrol = 2;
    }
    else if ((a == 0 || b == 0) && c == 1 && d == 1 && e == 1)
    {
        tracingcontrol = 2;
    }
    else if (a == 0 && b == 0 && c == 0 && d == 0 && e == 0)
    {
        tracingcontrol = 3;
    }
    else
    {
        tracingcontrol = 1; // 当以上条件都不满足时执行
    }
    // tracingcontrol =
    //     ((d == 0 || e == 0) && (b == 1 && a == 1)) ? 0 : (d == 1 && e == 1 && (c == 0 || c == 1)) ? 1
    //                                                  : ((a == 0 || b == 0) && d == 1 && e == 1)   ? 2
    //                                                                                               : 0; // 如果以上条件都不满足，默认设置为0

    // // 特殊情况，所有传感器都未检测到信号
    // if (a == 0 && b == 0 && c == 0 && d == 0 && e == 0)
    // {
    //     tracingcontrol = 3;
    // }
    return tracingcontrol;
    // 执行循迹控制动作，这里用打印代替实际动作
    // Serial.print("Tracing control set to: ");
    // Serial.println(tracingcontrol);
}

int tracing()
{
    // 工业流程控制框架，可根据自身需求增加case 数量
    tracingcontrol = combinedSensorControl();
    // Serial.println(tracingcontrol);
    switch (tracingcontrol)
    {

    case 0:                      // 按位置向前进，运动距离7200个脉冲，运动速度5（速度参数数值越小，速度越快）
        Serial.print("2,12,18"); //right
        delay(30);
        b = 0;
        break;

    case 1:                      // 按位置向后运动，运动距离7200个脉冲，运动速度5（速度参数数值越小，速度越快）
        Serial.print("2,20,20"); //forward
        delay(30);
        b = 1;
        break;

    case 2:                      // 按位置向左平移（全向轮可左右平移），运动距离7200个脉冲，运动速度5（速度参数数值越小，速度越快）
        Serial.print("2,18,12"); //left
        delay(30);
        b = 2;
        break;

    case 3:                      // 按位置向右运动（全向轮可左右平移），运动距离7200个脉冲，运动速度5（速度参数数值越小，速度越快）
        Serial.print("0,10,10"); // stop
        delay(30);
        b = 3;
        break;
    }
    return b;
}

int stop(){
    b = tracing();
    chaosheng = measureDistanceAndSetState();
    if(chaosheng == 0 or b == 3){       
        c = 0;
    }
    else {
        c = 1;
    }
    return c;
}

