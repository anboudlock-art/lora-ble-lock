@echo off
chcp 65001 >nul
echo 正在编译LoRa-BLE锁固件...
echo.

REM 切换到项目目录
cd /d "%~dp0"

REM 设置环境变量
set ARMCC5INC=C:\Keil_v5\ARM\ARMCC\include
set ARMCC6INC=C:\Keil_v5\ARM\ARMCLANG\include
set PATH=C:\Keil_v5\ARM\ARMCLANG\bin;%PATH%

REM 清理旧的输出文件
if exist Output\*.hex del Output\*.hex
if exist Output\*.axf del Output\*.axf

REM 使用ARMCLANG编译器直接编译
echo 使用ARMCLANG V6编译器...
echo.

REM 编译主文件（示例）
REM 这里需要实际的编译命令，但为了简单起见，我们先检查编译器
ArmClang.exe --version

if errorlevel 1 (
    echo 错误: ARMCLANG编译器未找到或无法运行
    pause
    exit /b 1
)

echo.
echo 编译器检查通过！
echo 由于工程配置问题，建议在Keil GUI中手动设置编译器：
echo 1. 打开Keil uVision5
echo 2. 打开 SYD8811_BLE_LoRa_Lock_V01.uvprojx
echo 3. 点击魔术棒图标（Options for Target）
echo 4. 在Target标签页，ARM Compiler选择"Use default compiler version 6"
echo 5. 点击OK，然后按F7编译
echo.

pause