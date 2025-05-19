/**
 * 通过超声波传感器测量距离，并根据距离设置chaosheng状态。
 */
const int TRIGPIN = 25; // 超声波发射端
const int ECHOPIN = 33;
int chaosheng = 0;       // chaosheng状态，0为停止，1为行驶
int averageDistance = 0; // 平均距离
int measureDistanceAndSetState()
{
    float totalDistance = 0;         // 用于存储多次读取的总和
    const int numReadings = 3;       // 定义读取次数，取平均值以提高准确性
    const float distanceFactor = 58; // 距离转换系数，基于声速和脉冲时间

    // 清零TRIGPIN，准备触发超声波传感器
    digitalWrite(TRIGPIN, LOW);
    delayMicroseconds(2);

    // 循环进行多次测量，计算平均距离
    for (int i = 0; i < numReadings; i++)
    {
        digitalWrite(TRIGPIN, HIGH); // 触发超声波传感器发送信号
        delayMicroseconds(10);
        digitalWrite(TRIGPIN, LOW); // 信号发送完毕

        // 使用pulseIn函数读取脉冲宽度，计算距离
        float distance = pulseIn(ECHOPIN, HIGH) / distanceFactor;
        totalDistance += distance; // 累加每次的距离读数

        // 等待一段时间，确保传感器稳定
        delay(10);
    }

    averageDistance = totalDistance / numReadings; // 计算平均距离

    // 根据平均距离设置chaosheng状态
    if (averageDistance <= 4)
    {
        chaosheng = 0; // 距离小于等于4时，设置为0
    }
    else
    {
        chaosheng = 1; // 距离大于4时，设置为1
    }
    return chaosheng;
    // 打印平均距离值，帮助调试（根据需要取消注释）
    // Serial.print("Average Distance: ");
    // Serial.println(averageDistance);
}