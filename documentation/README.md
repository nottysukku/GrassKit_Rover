# Documentation

Vendor PDFs for the GrassKit rover / `16_RaceCar_MotorDriver_Bluetooth_Audio` project, downloaded on 2026-10-12.
Folders are numbered in reading order: start with the board being driven, work back to the MCU that drives it, then the radio, the tools, and finally the material on using the RBT01 as a Linux robotics board and on SLAM. Files inside each folder are numbered the same way (overview first, reference material last).

Every PDF was opened after download to confirm it is the document its filename says it is; the revision shown below is the one printed in the file.

## 01 — Motor driver board: X-STM32MP-RBT01

| File | Document | Use it for |
|:---|:---|:---|
| `01_DataBrief_X-STM32MP-RBT01.pdf` | DB5412 Rev 1 | Two-page overview, features, block diagram |
| `02_UM3482_Getting_Started_X-STM32MP-RBT01.pdf` | UM3482 Rev 1 | **The authority for wiring.** Table 2 (p. 6) motor terminals, Table 11 (p. 14) 40-pin CN2 pinout, p. 16 STSPIN948 mode jumpers |
| `03_Schematic_X-STM32MP-RBT01.pdf` | Schematic pack v1.0 | Full-size schematic sheets (the same ones UM3482 reprints small) |
| `04_BOM_X-STM32MP-RBT01.pdf` | BOM v1.0 | Part values: shunts, trimmers R51/R55/R71/R76, fuse, bulk capacitor |

## 02 — Motor driver IC: STSPIN948

| File | Document | Use it for |
|:---|:---|:---|
| `01_DS14271_STSPIN948_Datasheet.pdf` | DS14271 Rev 4 | Table 8 (p. 13) driving-mode selection, Table 9 (p. 13) truth table for dual independent full-bridge mode, current limiter, EN/nFAULT behaviour, UVLO |
| `02_UM3206_EVSPIN948_Nucleo_Expansion_Board_Getting_Started.pdf` | UM3206 Rev 2 | ST's own Nucleo-form-factor STSPIN948 board — a reference for driving this IC from a Nucleo, which is what this project does with the RBT01 |

## 03 — MCU board: NUCLEO-L073RZ

| File | Document | Use it for |
|:---|:---|:---|
| `01_DataBrief_NUCLEO-L073RZ.pdf` | DB2196 Rev 22 | Nucleo-64 family overview |
| `02_UM1724_STM32_Nucleo-64_Boards_MB1136.pdf` | UM1724 Rev 17 | Table 18 (p. 47) Arduino connector pins and Table 30 (p. 60) ST morpho pins for the NUCLEO-L073RZ; solder bridges, ST-LINK, power |
| `03_Schematic_MB1136-C03.pdf` | MB1136 rev C-03 | Board schematic — pick the one matching the revision on your board's sticker |
| `04_Schematic_MB1136-C04.pdf` | MB1136 rev C-04 | 〃 |
| `05_Schematic_MB1136-C05.pdf` | MB1136 rev C-05 | 〃 |

## 04 — MCU: STM32L073RZ

| File | Document | Use it for |
|:---|:---|:---|
| `01_DS10685_STM32L073x8_xB_xZ_Datasheet.pdf` | DS10685 Rev 7 | Pinout, alternate-function tables (TIM3 on PB4/PB5/PB0/PB1), electrical limits |
| `02_RM0367_STM32L0x3_Reference_Manual.pdf` | RM0367 Rev 8 | Register-level reference: TIM3, GPIO, USART1/2, SPI1, RCC |
| `03_PM0223_Cortex-M0plus_Programming_Manual.pdf` | PM0223 Rev 10 | Core, NVIC, SysTick |
| `04_ES0292_STM32L07x_L08x_Errata.pdf` | ES0292 Rev 8 | Known silicon limitations |
| `05_AN4467_STM32L0_Hardware_Development.pdf` | AN4467 Rev 2 | Power, reset, clock and debug hardware guidelines |
| `06_AN4013_STM32_Timers_Introduction.pdf` | AN4013 Rev 14 | Timer features across STM32 families |
| `07_AN4776_General_Purpose_Timer_Cookbook.pdf` | AN4776 Rev 4 | Worked timer examples, including PWM generation |

## 05 — Bluetooth shield: X-NUCLEO-IDB05A2

| File | Document | Use it for |
|:---|:---|:---|
| `01_DataBrief_X-NUCLEO-IDB05A2.pdf` | DB4170 Rev 1 | Overview |
| `02_UM2700_Getting_Started_X-NUCLEO-IDB05A2.pdf` | UM2700 Rev 1 | Which Nucleo pins the shield uses (SPI, CS, IRQ, reset), jumpers |
| `03_Schematic_X-NUCLEO-IDB05A2.pdf` | Schematic v1 | Schematic |
| `04_BOM_X-NUCLEO-IDB05A2.pdf` | BOM v1 | Bill of materials |
| `05_BlueNRG-M0_Module_Datasheet.pdf` | DS13023 Rev 8 | The module on the shield: SPI timing, electrical limits |
| `06_UM1873_X-CUBE-BLE1_Getting_Started.pdf` | UM1873 Rev 6 | ST's BLE software expansion for STM32Cube |
| `07_UM1865_BlueNRG-MS_Stack_ACI.pdf` | UM1865 Rev 8 | Every ACI command and event the stack accepts over SPI |
| `08_PM0237_BlueNRG-MS_Stack_Programming_Guidelines.pdf` | PM0237 Rev 7 | How to build GATT services, advertising and connections on that stack |

## 06 — Sensors fitted on the RBT01

Not used by the current Nucleo firmware, but they are on the board and are what any odometry or SLAM work would read.

| File | Document | Sensor |
|:---|:---|:---|
| `01_ISM330DHCX_IMU_Datasheet.pdf` | DS13012 Rev 7 | 6-axis IMU (accelerometer + gyroscope) |
| `02_IIS2MDC_Magnetometer_Datasheet.pdf` | DocID030986 Rev 1 | 3-axis magnetometer |
| `03_LPS22HH_Pressure_Sensor_Datasheet.pdf` | DocID030890 Rev 2 | Barometric pressure sensor |
| `04_VL53L5CX_ToF_Sensor_Datasheet.pdf` | DS13754 Rev 13 | 8x8 multizone Time-of-Flight ranging sensor |
| `05_UM2884_VL53L5CX_ULD_Driver_Guide.pdf` | UM2884 Rev 7 | How to drive the VL53L5CX with ST's Ultra Lite Driver |

## 07 — Firmware tools and HAL

| File | Document | Use it for |
|:---|:---|:---|
| `01_UM1754_Getting_Started_STM32CubeL0.pdf` | UM1754 Rev 6 | Layout of the STM32CubeL0 package |
| `02_UM1749_STM32L0_HAL_and_LL_Drivers.pdf` | UM1749 Rev 7 | HAL / LL API reference (1694 pages) |
| `03_UM1718_STM32CubeMX.pdf` | UM1718 Rev 51 | CubeMX, for the `.ioc` in `firmware/STM32CubeMX/` (77 MB) |
| `04_UM2609_STM32CubeIDE_User_Guide.pdf` | UM2609 Rev 18 | Building and debugging in STM32CubeIDE |
| `05_UM2237_STM32CubeProgrammer.pdf` | UM2237 Rev 31 | Flashing over ST-LINK, including `STM32_Programmer_CLI` |

## 08 — Using the RBT01 with an STM32MP Linux host

The RBT01 was designed as a 40-pin expansion board for ST's STM32MP discovery kits (and Raspberry Pi), not for a Nucleo. This is ST's own material on using it that way.

| File | Document | Use it for |
|:---|:---|:---|
| `01_DataBrief_X-LINUX-RBT1.pdf` | DB5457 Rev 1 | Overview of ST's Linux software package for the board |
| `02_UM3481_Getting_Started_X-LINUX-RBT1.pdf` | UM3481 Rev 1 | Setting up the package: motor control, sensor fusion, ToF obstacle detection, web remote control |
| `03_DataBrief_STM32MP157F-DK2.pdf` | DB4022 Rev 2 | The discovery kit UM3482 is written for |
| `04_UM2637_Discovery_Kits_STM32MP157F-DK2.pdf` | UM2637 Rev 4 | That kit's user manual, including its 40-pin GPIO header |
| `05_DataBrief_STM32MP257F-DK.pdf` | DB5299 Rev 2 | The newer 64-bit kit the board also fits |
| `06_UM3385_Discovery_Kit_STM32MP257F.pdf` | UM3385 Rev 2 | That kit's user manual |

## 09 — SLAM and navigation

| File | Document | Use it for |
|:---|:---|:---|
| `01_SLAM_Toolbox_JOSS_2021_Macenski_Jambrecic.pdf` | Journal of Open Source Software 6(61), 2783 — CC BY 4.0 | The paper behind `slam_toolbox`, the 2D lidar SLAM package ROS 2's Nav2 uses by default |

### Not found: the Shivam Munjal RBT01 / SLAM document

No public document, paper, repository or ST page tying that name to SLAM or to the X-STM32MP-RBT01 turned up in web, ST, ST Community or GitHub searches. If it is an internal ST document, drop it in this folder as `00_...pdf` so it sorts first.

Also note that none of ST's published RBT01 material (UM3481, UM3482, both data briefs) mentions SLAM or ROS; the X-LINUX-RBT1 package stops at sensor fusion and ToF obstacle detection.

### Further reading (linked, not copied)

These are free to read but their licences do not allow redistribution, so they are not stored here.

- Cadena et al., *Past, Present, and Future of Simultaneous Localization and Mapping* — the standard survey: <https://arxiv.org/abs/1606.05830>
- Macenski et al., *The Marathon 2: A Navigation System* — the Nav2 paper: <https://arxiv.org/abs/2003.00368>
- Niculescu et al., *NanoSLAM: Enabling Fully Onboard SLAM for Tiny Robots* — SLAM from VL53L5CX ToF sensors, the same sensor the RBT01 carries: <https://arxiv.org/abs/2309.12008>

## Not downloaded

- **UM1727** (*Getting started with STM32 Nucleo board software development tools*): st.com no longer serves the PDF. It dates from 2016 and covers IAR / Keil / TrueSTUDIO rather than STM32CubeIDE, so UM2609 above replaces it.
- **MB1136 BOM and X-STM32MP-RBT01 Gerbers**: ST publishes these as ZIP archives, not PDFs.

## Sources

ST documents come from `https://www.st.com/resource/en/<type>/<name>.pdf` and are copyright STMicroelectronics. Check st.com for newer revisions before relying on a table for a hardware decision.
