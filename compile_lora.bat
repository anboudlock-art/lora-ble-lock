@echo off
chcp 65001 >nul
setlocal enabledelayedexpansion

echo ========================================
echo 编译纯净的 LoRa 版本
echo ========================================
echo.

REM 设置Keil路径
set KEIL_PATH=C:\keil-MDK-V5.43
set UV4_PATH=%KEIL_PATH%\UV4\UV4.exe
set PROJECT_DIR=%~dp0

echo [步骤1] 检查文件编码...
cd /d "%PROJECT_DIR%"
python fix_encoding.py

echo.
echo [步骤2] 清理旧的编译产物...
cd /d "%PROJECT_DIR%\Keil\output"
if exist *.o del /Q *.o
if exist *.d del /Q *.d  
if exist *.axf del /Q *.axf
if exist *.hex del /Q *.hex
if exist *.bin del /Q *.bin
echo 清理完成

echo.
echo [步骤3] 编译项目...
cd /d "%PROJECT_DIR%\Keil"
if exist "%UV4_PATH%" (
    echo 使用Keil编译器: %UV4_PATH%
    "%UV4_PATH%" -b SYD8811_BLE_LoRa_Lock_V01.uvprojx -j0 -o build_log.txt
) else (
    echo 错误: 未找到Keil编译器
    echo 请检查路径: %UV4_PATH%
    pause
    exit /b 1
)

echo.
echo [步骤4] 检查编译结果...
if exist build_log.txt (
    echo ===== 编译日志 =====
    type build_log.txt
    echo.
    
    findstr /C:"0 Error(s)" build_log.txt >nul
    if !errorlevel! equ 0 (
        echo ✓ 编译成功！无错误
    ) else (
        echo ✗ 编译失败，请检查错误
    )
) else (
    echo 警告: 未生成编译日志
)

echo.
echo [步骤5] 检查生成的文件...
if exist "%PROJECT_DIR%\Keil\output\Ble_Vendor_Service.hex" (
    echo ✓ 已生成 HEX 文件
    dir "%PROJECT_DIR%\Keil\output\Ble_Vendor_Service.hex"
) else (
    echo ✗ 未生成 HEX 文件
)

if exist "%PROJECT_DIR%\Keil\output\Ble_Vendor_Service.bin" (
    echo ✓ 已生成 BIN 文件
    dir "%PROJECT_DIR%\Keil\output\Ble_Vendor_Service.bin"
) else (
    echo ! 未生成 BIN 文件
)

echo.
echo ========================================
echo 编译完成
echo ========================================
pause
