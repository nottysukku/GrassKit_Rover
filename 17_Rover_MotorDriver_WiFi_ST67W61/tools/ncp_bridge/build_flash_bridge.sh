#!/usr/bin/env bash
# Build ncp_bridge.c and flash it to the NUCLEO-L073RZ.
# usage: build_flash_bridge.sh [extra gcc flags, e.g. -DTX_PIN=11]
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
FW="$HERE/../../firmware"
T="/c/ST/STM32CubeIDE_2.2.0/STM32CubeIDE/plugins/com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3.rel1.win32_1.0.100.202602081740/tools/bin"
PROG="/c/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe"
H="$FW/Drivers/STM32L0xx_HAL_Driver/Src"
mkdir -p "$HERE/build"
"$T/arm-none-eabi-gcc.exe" -mcpu=cortex-m0plus -mthumb -O2 -DSTM32L073xx -DUSE_HAL_DRIVER "$@" \
  -I"$FW/Core/Inc" -I"$FW/Drivers/STM32L0xx_HAL_Driver/Inc" -I"$FW/Drivers/CMSIS/Device/ST/STM32L0xx/Include" \
  -I"$FW/Drivers/CMSIS/Include" -Wall -T"$FW/STM32L073RZTx_FLASH.ld" -Wl,--gc-sections --specs=nano.specs -lc -lm \
  -o "$HERE/build/ncp_bridge.elf" "$HERE/ncp_bridge.c" "$FW/Core/Startup/startup_stm32l073rztx.s" \
  "$FW/Core/Src/system_stm32l0xx.c" "$FW/Core/Src/sysmem.c" "$FW/Core/Src/syscalls.c" \
  "$H/stm32l0xx_hal.c" "$H/stm32l0xx_hal_cortex.c" "$H/stm32l0xx_hal_rcc.c" "$H/stm32l0xx_hal_rcc_ex.c" \
  "$H/stm32l0xx_hal_gpio.c" "$H/stm32l0xx_hal_uart.c" "$H/stm32l0xx_hal_uart_ex.c" "$H/stm32l0xx_hal_dma.c" \
  "$H/stm32l0xx_hal_pwr.c" "$H/stm32l0xx_hal_pwr_ex.c"
"$T/arm-none-eabi-objcopy.exe" -O binary "$HERE/build/ncp_bridge.elf" "$HERE/build/ncp_bridge.bin"
"$PROG" -c port=SWD mode=UR -w "$HERE/build/ncp_bridge.bin" 0x08000000 -v -rst 2>&1 | grep -E "verified|Error|error"
