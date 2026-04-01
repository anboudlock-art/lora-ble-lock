@echo off
chcp 65001 >nul
echo ========================================
echo 修复并编译 LoRa-BLE 项目
echo ========================================
echo.

REM 设置路径
set PROJECT_DIR=%~dp0
set KEIL_PATH=C:\keil-MDK-V5.43
set UV4_PATH=%KEIL_PATH%\UV4\UV4.exe

echo [步骤1] 转换文件编码为UTF-8...
cd /d "%PROJECT_DIR%\.."
for %%f in (*.c *.h) do (
    if exist %%f (
        echo 转换: %%f
        powershell -Command "Get-Content '%%f' -Encoding Default | Set-Content '%%f' -Encoding UTF8"
    )
)

echo.
echo [步骤2] 清理旧的编译产物...
cd /d "%PROJECT_DIR%\Keil\output"
del /Q *.o *.d *.axf *.hex *.bin 2>nul
echo 清理完成

echo.
echo [步骤3] 编译项目...
cd /d "%PROJECT_DIR%\Keil"
"%UV4_PATH%" -b SYD8811_BLE_LoRa_Lock_V01.uvprojx -j0 -o build_log.txt

echo.
echo [步骤4] 检查编译结果...
if exist build_log.txt (
    type build_log.txt
) else (
    echo 警告: 未生成编译日志
)

echo.
echo ========================================
echo 完成！请检查编译结果
echo ========================================
pause
