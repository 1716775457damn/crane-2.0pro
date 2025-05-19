# K210数字检测与串口发送
# 基于原有model-11975.nncase/main.py修改
import sensor, image, lcd, time
import KPU as kpu
import gc, sys
from fpioa_manager import fm
from machine import UART, Timer

# 映射串口引脚
fm.register(11, fm.fpioa.UART1_RX, force=True)
fm.register(10, fm.fpioa.UART1_TX, force=True)

# 初始化串口
uart = UART(UART.UART1, 115200, read_buf_len=4096)

# 基本设置与变量定义
input_size = (224, 224)
labels = ['5', '6', '7', '8', '1', '2', '3', '4']
anchors = [1.88, 2.38, 1.31, 2.11, 1.69, 2.19, 1.59, 1.94, 1.53, 1.66]

def lcd_show_except(e):
    import uio
    err_str = uio.StringIO()
    sys.print_exception(e, err_str)
    err_str = err_str.getvalue()
    img = image.Image(size=input_size)
    img.draw_string(0, 10, err_str, scale=1, color=(0xff,0x00,0x00))
    lcd.display(img)

def main(anchors, labels = None, model_addr="/sd/model-11975.kmodel", sensor_window=input_size, lcd_rotation=0, sensor_hmirror=False, sensor_vflip=False):
    # 初始化摄像头
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.set_windowing(sensor_window)
    sensor.set_hmirror(sensor_hmirror)
    sensor.set_vflip(sensor_vflip)
    sensor.run(1)
    
    # 初始化显示屏
    lcd.init(type=1)
    lcd.rotation(lcd_rotation)
    lcd.clear(lcd.WHITE)
    
    # 检查标签文件
    if not labels:
        try:
            with open('labels.txt','r') as f:
                exec(f.read())
        except Exception as e:
            print("读取labels.txt失败: {}".format(e))
            
    if not labels:
        print("错误: 找不到labels.txt文件")
        img = image.Image(size=(320, 240))
        img.draw_string(90, 110, "no labels.txt", color=(255, 0, 0), scale=2)
        lcd.display(img)
        return 1
    
    # 显示启动信息
    img = image.Image(size=(320, 240))
    img.draw_string(90, 110, "loading model...", color=(255, 255, 255), scale=2)
    lcd.display(img)

    # 加载AI模型
    try:
        task = None
        task = kpu.load(model_addr)
        kpu.init_yolo2(task, 0.5, 0.3, 5, anchors) # threshold:[0,1], nms_value: [0, 1]
        
        clock = time.clock()
        detection_count = 0
        
        # 主循环
        while(True):
            clock.tick()  # 更新时钟
            img = sensor.snapshot()
            t = time.ticks_ms()
            objects = kpu.run_yolo2(task, img)
            t = time.ticks_ms() - t
            
            # 发送检测结果
            if objects:
                detection_count += 1
                best_obj = objects[0]
                
                # 选择置信度最高的目标
                for obj in objects:
                    if obj.value() > best_obj.value():
                        best_obj = obj
                
                # 获取目标信息
                pos = best_obj.rect()
                classid = best_obj.classid()
                confidence = best_obj.value()
                label = labels[classid]
                
                # 发送数据格式: 数字,x,y,宽,高
                data = "{},{},{},{},{}\r\n".format(
                    label, pos[0], pos[1], pos[2], pos[3])
                uart.write(data)
                
                # 只打印识别内容和坐标数据
                print("数字: {}, 位置: ({}, {}, {}, {})".format(
                    label, pos[0], pos[1], pos[2], pos[3]))
                
                # 绘制检测框和标签
                for obj in objects:
                    pos = obj.rect()
                    img.draw_rectangle(pos)
                    img.draw_string(pos[0], pos[1], "%s : %.2f" %(labels[obj.classid()], obj.value()), scale=2, color=(255, 0, 0))
            
            # 显示FPS
            img.draw_string(0, 200, "t:%dms" %(t), scale=2, color=(255, 0, 0))
            lcd.display(img)
    except Exception as e:
        print("错误: {}".format(e))
        raise e
    finally:
        if task is not None:
            kpu.deinit(task)


if __name__ == "__main__":
    try:
        print("K210数字识别系统启动")
        
        # 尝试不同的模型路径
        try:
            main(anchors = anchors, labels=labels, model_addr="/sd/model-11975.kmodel")
        except Exception as e:
            print("SD卡模型加载失败，尝试从Flash加载")
            main(anchors = anchors, labels=labels, model_addr="/flash/model-11975.kmodel")
    except Exception as e:
        sys.print_exception(e)
        lcd_show_except(e)
    finally:
        gc.collect() 