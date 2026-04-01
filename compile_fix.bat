@echo off
echo 修复Keil编译器配置并编译...
echo.

REM 设置Keil路径
set KEIL_PATH=C:\Keil_v5
set UV4=%KEIL_PATH%\UV4\UV4.exe
set PROJECT=SYD8811_BLE_LoRa_Lock_V01.uvprojx

echo 正在编译项目: %PROJECT%
echo 使用编译器: ARMCLANG V6
echo.

REM 使用Keil命令行编译，指定编译器版本
"%UV4%" -b "%PROJECT%" -o "Output\build_result.txt"

if %ERRORLEVEL% EQU 0 (
    echo 编译成功！
    echo.
    dir Output\*.hex /b
    echo.
    echo HEX文件已生成在Output目录
) else (
    echo 编译失败，错误代码: %ERRORLEVEL%
    echo 请查看 Output\build_result.txt 获取详细信息
)

pause