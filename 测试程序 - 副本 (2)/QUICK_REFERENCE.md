# 简化绝对位置控制 - 快速参考

## 底盘控制 (仅Y轴前后移动)

### 基本命令
```bash
chassis_abs,Y,S         # 移动到绝对Y位置，速度S
chassis_pos             # 查看当前Y位置
chassis_reset           # 重置Y位置到0
chassis_reset,Y         # 重置Y位置到指定值
```

### 示例
```bash
chassis_abs,100000,50   # 移动到Y=100000，速度50
chassis_abs,0           # 返回原点
chassis_reset,50000     # 设置当前位置为50000
```

## 步进电机控制 (4个电机)

### 基本命令
```bash
stepper_abs,M,P,S,A     # 电机M移动到位置P，速度S，加速度A (自动使能)
stepper_pos,M           # 查看电机M位置
stepper_pos,all         # 查看所有电机位置
stepper_reset,M,P       # 重置电机M位置到P
stepper_reset,M         # 重置电机M位置到0
```

### 示例
```bash
# 无需手动使能，直接使用绝对位置命令
stepper_abs,2,-1600,75,25  # 电机2移动到位置-1600，速度75，加速度25
stepper_pos,2              # 查看电机2位置
stepper_reset,2,0          # 重置电机2到位置0
```

## 参数范围

### 底盘
- **Y位置**: -2,000,000 到 2,000,000
- **速度**: 1-255 (默认50)
- **大距离**: 自动分段移动，突破65535限制

### 步进电机
- **电机编号**: 1-4
- **位置**: -1,000,000 到 1,000,000
- **速度**: 1-200 (默认100)
- **加速度**: 1-255 (默认50)

## 兼容性

### 现有命令增强
```bash
# 这些命令现在会自动更新位置跟踪
chassis_adv,1,50000,50  # 前进50000，Y位置自动更新
stepper,1,200,100       # 电机1移动200步，位置自动更新
```

### 使能要求
- 步进电机自动使能: `stepper_abs`命令会自动使能电机
- 底盘无需使能，直接使用
- 手动使能仍可用: `stepper_enable,M` / `stepper_disable,M`

## 快速测试序列

```bash
# 1. 查看帮助和初始状态
help
chassis_pos
stepper_pos,all

# 2. 底盘测试
chassis_abs,10000,50
chassis_pos
chassis_abs,0

# 3. 大距离测试 (自动分段)
chassis_abs,100000,50    # 超过65535，自动分段
chassis_pos
chassis_abs,0

# 4. 步进电机测试 (自动使能)
stepper_abs,1,500,100    # 自动使能并移动
stepper_pos,1
stepper_abs,1,0

# 5. 混合使用测试
chassis_reset
stepper_reset,1
chassis_abs,50000,50
chassis_adv,1,20000,50
chassis_pos

# 6. 重置所有
chassis_reset
stepper_reset,1
```

## 故障排除

### 常见错误
- `Motor X is disabled` → 使用 `stepper_enable,X`
- `out of range` → 检查位置/速度参数范围
- `Invalid motor number` → 使用1-4的电机编号
- `Format error` → 检查命令格式和逗号分隔

### 位置不准确
- 使用 `chassis_reset` 或 `stepper_reset,M` 重新校准
- 检查硬件连接和现有命令是否正常工作
