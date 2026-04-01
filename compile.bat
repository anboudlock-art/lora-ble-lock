@echo off
chcp 65001 >nul
echo ========================================
echo    LoRa-BLE智能锁固件编译工具
echo ========================================
echo.

REM 检查当前目录
set PROJECT_DIR=%~dp0..\firmware\keil
set PROJECT_FILE=SYD8811_BLE_LoRa_Lock_V01.uvprojx
set OUTPUT_DIR=Output

echo 项目目录: %PROJECT_DIR%
echo 工程文件: %PROJECT_FILE%
echo.

REM 检查Keil安装
if not exist "C:\Keil_v5\UV4\UV4.exe" (
    echo 错误: 未找到Keil MDK安装
    echo 请安装Keil MDK v5.43或更高版本
    echo 默认安装路径: C:\Keil_v5
    pause
    exit /b 1
)

REM 检查工程文件
if not exist "%PROJECT_DIR%\%PROJECT_FILE%" (
    echo 错误: 未找到工程文件
    echo 预期路径: %PROJECT_DIR%\%PROJECT_FILE%
    pause
    exit /b 1
)

echo 正在检查编译器配置...
echo.
echo 重要提示：本工程使用ARMCLANG V6.19编译器
echo 如果之前使用过ARM-ADS V5.06，可能需要重新配置
echo.
echo 请确保在Keil中已正确设置编译器：
echo 1. 打开Keil uVision5
echo 2. 打开 %PROJECT_FILE%
echo 3. 点击魔术棒图标（Options for Target）
echo 4. 在Target标签页，选择"Use default compiler version 6"
echo 5. 点击OK
echo.

set /p choice=是否已配置好编译器？(y/n): 
if /i "%choice%" neq "y" (
    echo 请先配置编译器，然后重新运行此脚本
    pause
    exit /b 1
)

echo.
echo 开始编译...
echo ========================================

REM 切换到项目目录
pushd "%PROJECT_DIR%"

REM 清理旧的输出文件（可选）
echo 清理旧输出文件...
if exist "%OUTPUT_DIR%\Ble_Vendor_Service.hex" del "%OUTPUT_DIR%\Ble_Vendor_Service.hex"
if exist "%OUTPUT_DIR%\Ble_Vendor_Service.axf" del "%OUTPUT_DIR%\Ble_Vendor_Service.axf"

REM 使用Keil命令行编译
echo 正在编译，请稍候...
"C:\Keil_v5\UV4\UV4.exe" -b "%PROJECT_FILE%" -o "%OUTPUT_DIR%\build_log.txt"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo           编译成功！
    echo ========================================
    echo.
    
    REM 检查输出文件
    if exist "%OUTPUT_DIR%\Ble_Vendor_Service.hex" (
        echo 生成的HEX文件: %OUTPUT_DIR%\Ble_Vendor_Service.hex
        dir "%OUTPUT_DIR%\Ble_Vendor_Service.hex"
    ) else (
        echo 警告: 未找到HEX文件，但编译过程成功
    )
    
    if exist "%OUTPUT_DIR%\Ble_Vendor_Service.axf" (
        echo 生成的AXF文件: %OUTPUT_DIR%\Ble_Vendor_Service.axf
    )
    
    echo.
    echo 编译日志: %OUTPUT_DIR%\build_log.txt
) else (
    echo.
    echo ========================================
    echo           编译失败！
    echo ========================================
    echo.
    echo 错误代码: %ERRORLEVEL%
    echo 请查看日志文件: %OUTPUT_DIR%\build_log.txt
    echo.
    echo 常见问题：
    echo 1. 编译器未正确配置 - 请参考 docs\COMPILER_SETUP.md
    echo 2. Keil许可证问题
    echo 3. 缺少必要的头文件或库
)

popd

echo.
echo ========================================
echo 按任意键退出...
pause >nul