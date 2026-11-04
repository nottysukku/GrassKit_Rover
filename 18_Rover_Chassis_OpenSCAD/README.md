# GrassKit Rover chassis (OpenSCAD)

`grasskit_rover.scad` is one parametric model of the rover: deck, cutter tower, guard skirt, motor clamps, board trays, ToF brackets, camera mast, canopy, and stand-ins for every bought part. It also prints a weight, centre-of-gravity, wheel-load, torque and battery report in the OpenSCAD console.

Open it in OpenSCAD 2021.01 or newer.

| `MODE` | What you get |
|:---|:---|
| `assembly` | The whole rover with all parts, plus the console report |
| `deck_2d`, `canopy_2d` | Flat outlines: File > Export > DXF/SVG for laser or CNC cutting |
| `deck_front_half`, `deck_rear_half`, `deck_splice` | The deck split for a desktop 3D printer |
| `motor_bracket`, `cutter_tower`, `guard_skirt_half`, `tof_bracket`, `camera_mast`, `nucleo_tray`, `rbt01_tray` | One printable part, laid flat: F6, then File > Export > STL |

Values marked `[ST]` come from the ST documents in `documentation/`; values marked `[EST]` are estimates to replace with measured ones.

To save a picture of the assembly from the command line:

```
openscad -o rover.png --imgsize=2400,1600 --camera=620,520,430,0,0,95 --colorscheme=Tomorrow grasskit_rover.scad
```

## Model vs the final BOM

The model still shows the first build: 75 mm mecanum wheels, 25 mm (JGA25-370) gearmotors, an RS-775 brushed cutter motor and a 4 x 18650 pack. The final parts list (`documentation/00_Project_Status/GrassKit_Rover_Final_BOM_2026-11-04.xlsx`) changes these, so update the parameters before cutting metal:

| Parameter | Now | Final BOM |
|:---|:---|:---|
| `wheel_d`, `wheel_w` | 75, 35 | 130, 60 (treaded off-road wheel) |
| `dm_d`, `dm_len` | 25, 55 | 36, ~95 (36 mm planetary gearmotor, check the datasheet) |
| `cm_d`, `cm_len` (cutter motor) | RS-775: 42, 67 | DYS D3536 outrunner: ~35, ~36 |
| `DECK_MATERIAL` | `PETG_PRINT` | `ALU_3` (3 mm aluminium, laser cut from `deck_2d`) |
| `bat_lx`, `bat_ly`, `bat_h` | 4 x 18650 holder | 4S4P 10 Ah pack: measure it |
| `INCLUDE_AA_PACK` | `true` | `false` |
