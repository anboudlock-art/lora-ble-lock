# 编译器配置指南

## 问题描述

本工程已从ARM-ADS V5.06编译器升级到ARMCLANG V6.19。如果编译时出现以下错误：

```
*** Target 'SYD8811' uses ARM-Compiler 'Default Compiler Version 5' which is not available.
```

请按照本指南配置编译器。

## 解决方案

### 步骤1：检查编译器安装

1. 打开Keil MDK
2. 点击菜单：Project → Manage → Project Items
3. 选择"Folders/Extensions"标签
4. 查看"ARM Compiler"列表，确保有V6版本

### 步骤2：配置工程使用ARMCLANG

1. 打开工程：`firmware/keil/SYD8811_BLE_LoRa_Lock_V01.uvprojx`
2. 点击魔术棒图标（Options for Target）
3. 在Target标签页，找到"ARM Compiler"选项
4. 选择以下之一：
   - **"Use default compiler version 6"**（推荐）
   - **"ARMCLANG"**
5. 点击OK保存

### 步骤3：验证配置

重新编译工程，查看Build Output窗口，应显示：
```
Toolchain:       MDK-Lite  Version: 5.43.0.0
Toolchain Path:  C:\Keil_v5\ARM\ARMCLANG\bin\
C Compiler:      ArmClang.exe V6.19
```

### 步骤4：如果仍然失败

如果上述步骤无效，尝试：

1. **完全重新配置**：
   - 关闭Keil
   - 删除`firmware/keil/Output`文件夹中的所有文件（除了HEX2BIN.EXE）
   - 重新打开工程并配置

2. **手动修改工程文件**：
   - 用文本编辑器打开`SYD8811_BLE_LoRa_Lock_V01.uvprojx`
   - 确保以下设置：
     ```xml
     <ToolsetNumber>0x6</ToolsetNumber>
     <ToolsetName>ARMCLANG</ToolsetName>
     <pCCUsed>6190010::V6.19::ARMCLANG</pCCUsed>
     ```

## 常见问题

### Q1: 如何安装ARMCLANG编译器？
A: ARMCLANG通常包含在Keil MDK安装包中。如果未安装，可以从ARM官网下载或通过Keil的Pack Installer安装。

### Q2: 可以回退到ARM-ADS V5.06吗？
A: 可以，但不推荐。工程已针对ARMCLANG V6.19优化。如需回退，需要修改工程设置和可能的部分代码。

### Q3: 编译成功但无HEX文件？
A: 检查Output目录设置。工程配置为生成HEX文件，路径为：`.\Output\Ble_Vendor_Service.hex`

### Q4: 出现"Invalid Flash"警告？
A: 这是正常警告，不影响编译。与SYD8811的Flash配置相关。

## 技术支持

如果问题仍未解决：
1. 查看`Output/Ble_Vendor_Service.build_log.htm`获取详细错误信息
2. 在GitHub Issues提交问题
3. 联系项目维护者

## 版本历史

- **2024-04-01**: 初始版本，升级到ARMCLANG V6.19
- **2024-03-31**: 原始版本，使用ARM-ADS V5.06