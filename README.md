# LoRa-BLE智能锁固件

## 项目简介

基于SYD8811芯片的蓝牙锁项目，集成E220-900T22S LoRa模块，实现远程控制功能。

## 功能特性

- ✅ LoRa远程通信（E220-900T22S模块）
- ✅ 蓝牙BLE配对和控制
- ✅ 开锁/上锁/剪断报警
- ✅ 状态上报
- ✅ 低功耗设计

## 硬件要求

- 主控芯片：SYD8811 (ARM Cortex-M0)
- LoRa模块：E220-900T22S
- 电源：3.3V

## 编译环境

### 必需软件：
- **IDE**：Keil MDK v5.43 或更高版本
- **编译器**：ARM Compiler v6.19 (ARMCLANG) - **必须使用V6版本**
- **CMSIS**：5.9.0 或更高版本
- **芯片支持包**：SYD8811 Device Family Pack

### 编译器安装说明：
**重要**：本工程已从ARM-ADS V5.06升级到ARMCLANG V6.19。如果编译失败，请按以下步骤配置：

1. **安装ARMCLANG编译器**（如果未安装）：
   - 打开Keil MDK
   - 点击菜单：File → Device Database
   - 检查ARMCLANG编译器是否已安装

2. **配置工程使用ARMCLANG**：
   - 打开工程：`firmware/keil/SYD8811_BLE_LoRa_Lock_V01.uvprojx`
   - 点击魔术棒图标（Options for Target）
   - 在Target标签页，找到"ARM Compiler"选项
   - 选择 **"Use default compiler version 6"** 或 **"ARMCLANG"**
   - 点击OK保存设置

3. **验证编译器版本**：
   - 编译后查看输出窗口，应显示：`Toolchain: ARMCLANG Version: 6.19`

## 快速开始

### 1. 克隆仓库

```bash
git clone https://github.com/anboudlock-art/lora-ble-lock.git
cd lora-ble-lock
```

### 2. 环境准备

#### 安装Keil MDK：
1. 下载并安装Keil MDK v5.43
2. 安装ARMCLANG编译器（MDK安装包中通常包含）
3. 安装SYD8811设备支持包

### 3. 编译固件

#### 方法一：使用Keil GUI（推荐）
1. 打开Keil uVision5
2. 打开工程：`firmware/keil/SYD8811_BLE_LoRa_Lock_V01.uvprojx`
3. **重要**：检查编译器设置（见上方"编译器安装说明"）
4. 按 **F7** 编译
5. 查看输出：`firmware/keil/output/Ble_Vendor_Service.hex`

#### 方法二：命令行编译
```bash
cd firmware/keil
# 确保Keil路径在系统PATH中
UV4.exe -b SYD8811_BLE_LoRa_Lock_V01.uvprojx
```

#### 编译输出文件：
- `Ble_Vendor_Service.hex` - HEX烧录文件
- `Ble_Vendor_Service.axf` - 调试文件
- `Ble_Vendor_Service_assembly.txt` - 反汇编文件

## 目录结构

```
lora-ble-lock/
├── firmware/          # 固件源代码
│   ├── src/          # C源文件
│   ├── include/      # 头文件
│   └── keil/         # Keil工程
├── tools/            # 工具脚本
├── docs/             # 文档
└── examples/         # 示例代码
```

## 版本历史

### v1.0.0 (2024-04-01)
- 初始版本
- 支持LoRa通信
- 支持蓝牙BLE

## 许可证

MIT License

## 联系方式

- GitHub: [@anboudlock-art](https://github.com/anboudlock-art)
