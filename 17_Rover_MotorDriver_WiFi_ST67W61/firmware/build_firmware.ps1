$ErrorActionPreference = "Stop"

$toolsDir = "C:\ST\STM32CubeIDE_2.2.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740\tools\bin"
$gcc = "$toolsDir\arm-none-eabi-gcc.exe"
$objcopy = "$toolsDir\arm-none-eabi-objcopy.exe"
$size = "$toolsDir\arm-none-eabi-size.exe"

Write-Host "Using GCC: $gcc"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$fw = $scriptDir
$buildDir = "$fw\build"

if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

$cFiles = @(
    "$fw\Core\Src\main.c",
    "$fw\Core\Src\app.c",
    "$fw\Core\Src\motor_control.c",
    "$fw\Core\Src\wifi_st67w61.c",
    "$fw\Core\Src\web_page.c",
    "$fw\Core\Src\command_parser.c",
    "$fw\Core\Src\safety.c",
    "$fw\Core\Src\debug.c",
    "$fw\Core\Src\stm32l0xx_it.c",
    "$fw\Core\Src\syscalls.c",
    "$fw\Core\Src\sysmem.c",
    "$fw\Core\Src\system_stm32l0xx.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_cortex.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_rcc.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_rcc_ex.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_pwr.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_pwr_ex.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_gpio.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_dma.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_tim.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_tim_ex.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_uart.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_uart_ex.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_spi.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_flash.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_flash_ex.c",
    "$fw\Drivers\STM32L0xx_HAL_Driver\Src\stm32l0xx_hal_exti.c"
)

$sFile = "$fw\Core\Startup\startup_stm32l073rztx.s"
$ldScript = "$fw\STM32L073RZTx_FLASH.ld"
$elfFile = "$buildDir\rover_wifi.elf"
$binFile = "$buildDir\rover_wifi.bin"

$compileArgs = @(
    "-mcpu=cortex-m0plus",
    "-mthumb",
    "-O2",
    "-DSTM32L073xx",
    "-DUSE_HAL_DRIVER",
    "-I$fw\Core\Inc",
    "-I$fw\Drivers\STM32L0xx_HAL_Driver\Inc",
    "-I$fw\Drivers\STM32L0xx_HAL_Driver\Inc\Legacy",
    "-I$fw\Drivers\CMSIS\Device\ST\STM32L0xx\Include",
    "-I$fw\Drivers\CMSIS\Include",
    "-T$ldScript",
    "-Wl,--gc-sections",
    "-Wl,-Map=$buildDir\rover_wifi.map",
    "--specs=nano.specs",
    "-u", "_printf_float",
    "-lc",
    "-lm",
    "-o", $elfFile,
    $sFile
) + $cFiles

Write-Host "Compiling GrassKit Rover Wi-Fi firmware..."
& "$gcc" $compileArgs

if ($LASTEXITCODE -ne 0) {
    Write-Error "GCC compilation failed with exit code $LASTEXITCODE"
    exit 1
}

Write-Host "Memory layout:"
& "$size" $elfFile

Write-Host "Generating binary $binFile..."
& "$objcopy" -O binary $elfFile $binFile

if (Test-Path $binFile) {
    Write-Host "SUCCESS: Firmware binary generated: $(Get-Item $binFile | Select-Object -ExpandProperty Length) bytes"
} else {
    Write-Error "Binary generation failed."
    exit 1
}
