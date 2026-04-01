Write-Host "修复Keil编译器配置并编译..." -ForegroundColor Cyan
Write-Host ""

# 设置Keil路径
$KEIL_PATH = "C:\Keil_v5"
$UV4 = "$KEIL_PATH\UV4\UV4.exe"
$PROJECT = "SYD8811_BLE_LoRa_Lock_V01.uvprojx"

Write-Host "正在编译项目: $PROJECT" -ForegroundColor Yellow
Write-Host "使用编译器: ARMCLANG V6" -ForegroundColor Yellow
Write-Host ""

# 使用Keil命令行编译
& "$UV4" -b "$PROJECT" -o "Output\build_result.txt"

if ($LASTEXITCODE -eq 0) {
    Write-Host "编译成功！" -ForegroundColor Green
    Write-Host ""
    Get-ChildItem -Path "Output" -Filter "*.hex" | Select-Object -ExpandProperty Name
    Write-Host ""
    Write-Host "HEX文件已生成在Output目录" -ForegroundColor Green
} else {
    Write-Host "编译失败，错误代码: $LASTEXITCODE" -ForegroundColor Red
    Write-Host "请查看 Output\build_result.txt 获取详细信息" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "按任意键继续..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")