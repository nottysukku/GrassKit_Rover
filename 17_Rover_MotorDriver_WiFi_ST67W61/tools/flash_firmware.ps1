$ErrorActionPreference = "Stop"

$programmer = "C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
if (!(Test-Path $programmer)) {
    Write-Error "STM32CubeProgrammer CLI not found at $programmer"
    exit 1
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$binFile = Join-Path $scriptDir "..\firmware\build\rover_wifi.bin"

if (!(Test-Path $binFile)) {
    Write-Host "Binary not found, building firmware first..."
    & powershell -ExecutionPolicy Bypass -File (Join-Path $scriptDir "..\firmware\build_firmware.ps1")
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Build failed"
        exit 1
    }
}

Write-Host "Flashing GrassKit Rover firmware to NUCLEO-L073RG..."
Write-Host "Binary: $binFile"

& "$programmer" -c port=SWD mode=UR -w "$binFile" 0x08000000 -v -rst

if ($LASTEXITCODE -eq 0) {
    Write-Host "SUCCESS: GrassKit Rover Wi-Fi firmware flashed and running!"
} else {
    Write-Error "Flashing failed with exit code $LASTEXITCODE"
    exit 1
}
