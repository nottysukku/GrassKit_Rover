# Updating the X-NUCLEO-67W61M1 Wi-Fi module firmware from a NUCLEO-L073RZ

The ST67W611M1 module on the X-NUCLEO-67W61M1 must run ST's **mission profile T01** firmware (AT commands over SPI) at the version that matches X-CUBE-ST67W61. A module still on its earliest factory binary answers SPI with header magic **0x5501** instead of 0x55AA, and never answers `AT` ([ST community thread 168072](https://community.st.com/interface-and-connectivity-ics-52/st67w611m1-x-nucleo-67w61m1-on-nucleo-h753zi-spi-magic-mismatch-0x5501-unclear-if-firmware-flashed-qconn-flash-lock-question-168072)). The rover firmware logs this as `hdr=[01 55 00 00 00 00 01 00]`.

ST's update scripts only support four big Nucleo boards. `ncp_bridge.c` does the same job on the NUCLEO-L073RZ: it holds the module in reset with BOOT high, releases it into its boot ROM when the PC starts talking, and copies UART levels between the module and the ST-LINK virtual COM port. The motor driver is held disabled throughout.

## Wiring

On NUCLEO-64 boards the Arduino D0/D1 pins are not connected to the MCU (SB62/SB63 off), so add two jumpers:

| Shield pin | Module signal | Nucleo pin |
|---|---|---|
| D0 | UART TX (module output) | CN7 pin 2 (PC11) |
| D1 | UART RX (module input) | CN7 pin 1 (PC10) |

Built without `-DTX_PIN=...`, the bridge detects which jumper is the module TX and reports it at 115200 baud on the ST-LINK COM port.

## Steps

1. Get the tools and binaries from [STMicroelectronics/x-cube-st67w61](https://github.com/STMicroelectronics/x-cube-st67w61), folder `Projects/ST67W6X_Scripts`: `Binaries/QConn_Flash/`, `Binaries/NCP_info/`, `Binaries/NCP_Binaries/` (boot2, partition, efusedata, `st67w611m_mission_t01_v2.0.106.bin`, `mission_t01_flash_prog_cfg.ini`) and `LittleFS/littlefs/littlefs.bin`, keeping the folder layout.
2. The bridge copies bits in software, so lower the UART speed: in both `chips/qcc743/eflash_loader/eflash_loader_cfg.conf` files set `speed_uart_boot`, `speed_uart_load` and `isp_mode_speed` to `115200`.
3. Copy `mission_t01_flash_prog_cfg.ini` to `mission_t01_flash_prog_cfg_auto.ini` and set the `[FW]` line to `filedir = ./st67w611m_mission_t01_v2.0.106.bin`.
4. `bash build_flash_bridge.sh -DTX_PIN=11` (builds and flashes the bridge; the Nucleo resets).
5. Without opening the COM port first, run a read-only check from `Binaries/NCP_info`:
   `QConn_Eflash.exe -r --efuse --chipname=qcc743 -p COM7 --start=0x0 --end=0x1ff --file=flash_read.bin`
   Expect `shake hand success`, a chip ID and `[All Success]`.
6. Reset the Nucleo (`STM32_Programmer_CLI -c port=SWD mode=UR -rst`), then from `Binaries`:
   `QConn_Flash\QConn_Flash_Cmd.exe --port COM7 --config <full path>\NCP_Binaries\mission_t01_flash_prog_cfg_auto.ini --efuse=<full path>\NCP_Binaries\efusedata.bin`
   This is ST's own command from `NCP_update_mission_profile_t01.bat`. It takes several minutes at 115200 baud.
7. Flash the rover firmware back (`firmware/build/rover_wifi.bin`). The jumpers can stay.
