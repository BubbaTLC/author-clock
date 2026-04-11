# Specifications

## Description 7.5” E-PAPER DISPLAY

## Model Name 7.5inch e-Paper V

## Date 201 9/06/

## Revision 2.

```
10F, International Science & Technology Building, Fuhong Rd, Futian District,
Shenzhen, China
```
```
Email:
(order/shipment):sales@waveshare.com
(tech support) :service@waveshare.com
Website: http://www.waveshare.com
```

### Table of Contents

### 1. General Description....................................................

### 1.1 Overview............................................................

### 1.2 Feature ..............................................................

### 1.3 Mechanical Specification........................................

### 1.4 Mechanical Drawing of EPD module .......................

### 1.5 Input/Output Terminals........................................

### 1.6 Reference Circuit ................................................

### 2. Environmental............................................................

### 2.1 Handling, Safety and Environmental Requirements...

### 2.2 Reliability test.....................................................

### 3. Electrical Characteristics .............................................

### 3.1 Absolute maximum rating.....................................

### 3.2 Panel DC Characteristics.......................................

### 3.3 Panel AC Characteristics........................................

### 4. Typical Operating Sequence.........................................

### 4.1 Normal Operation Flow.........................................

### 4.2 Reference Program Code......................................

### 5. Command Table.........................................................

### 6. Optical characteristics.................................................

### 6.1 Specifications ....................................................

### 6.2 Definition of contrast ratio...................................

### 6.3 Reflection Ratio..................................................

### 6.4 Bi-stability ........................................................

### 7. Point and line standard...............................................

### 8. Packing.....................................................................

### 9 .Precautions ................................................................

### 4 4 4 4 5 6 8 9 9

### 10

### 12

### 12

### 12

### 13

### 18

### 18

### 20

### 22

### 48

### 48

### 48

### 49

### 49

### 50

### 51

### 52


#### Revision History

**Rev. Issued Date Revised Contents**
1.0 May.02.2018 1. Preliminary
2.0 Jun.28.2019 1. Updating


#### 1.General Description

1.1 Over View

```
The display is a TFT active matrix electrophoretic display, with interface and a reference
system design. The 7.5” active area contains 800 ×480 pixels, and has 1-bit white/black
full display capabilities. An integrated circuit contains gate buffer, source buffer, interface,
timing control logic, oscillator, DC-DC, SRAM, LUT, VCOM, and border are supplied with
each panel.
```
1.2 Features

- High contrast
- High reflectance
- Ultra wide viewing angle
- Ultra low power consumption
- Pure reflective mode
- Bi -stable
- Commercial temperature range
- Landscape, portrait mode
- Antiglare hard-co ated front-surface
- Low current deep sleep mode
- On chip display RAM
- Waveform stored in On-chip OTP
- Serial peripheral interface available
- On-chip oscillator
- On-chip booster and regulator control for generating VCOM, Gate and source driving
    voltage
- I^2 C Signal Master Interface to read external temperature sensor
- Available in COG package IC thickness 300um

#### 1.3 Mechanical Specifications

```
Parameter Specifications Unit Remark
Screen Size 7.5 In ch
Display Resolution 800(H)×480(V) Pixel Dpi: 125
Active Area 163. 2 (H)×97.92(V) mm
Pixel Pitch 0.205×0.204 mm
Pixel Configuration Square
Outline Dimension 170.2 (H)×111.2(V) ×1.18(D) mm
Weight 44±0.5 g
```

#### 1.4 Mechanical Drawing of EPD module

```
24
```
```
remove upper film
```
```
0,7±0,1 0,07±0,030,13±0,030,3±0,
```
```
1,18±0,
```
```
164,8±0,2 FP^1 63,2±0,1 AA
```
```
L
```
```
170,2±0,2 Ou167,8±0,2 PS
```
```
tline
```
```
6±0,
```
```
1
24
```
```
stifferener
```
```
90,3±0,
```
```
0.
0.
```
```
Note: 1. Unlabeled tolerances:±0.152. Resolution:800x4803. DPI: 125
```
```
1:
```
```
mm
```
```
SHEET 1 OF 1
```
```
Publish Date:
```
```
APPROVALSDRAWN:CHECKED:
```
```
SCALE:
```
```
SIZE:
```
```
DATE
```
```
File Serial Number:P/N:
```
```
Version Number:
```
```
Note:GD
```
```
1
```
```
97,92±0,1 AA104,09±0,2 FPL
107,09±0,2 PS
111,2±0,2 Outline
```
```
1,50,
1,
```
```
1,21,5 0 ,
```
```
3
6±0,29±0,
```
```
73,03±0,277,8±0,
```
```
24±0,
```
```
0,55±0,322,04±0,12±0,
```
```
4 *R^1
```

#### 1.5 Input/Output Terminals

#### 1.5-1 Pin out List

```
Pin # Type Single Description Remark
1 NC No connec tion and do not connec t with other NC pins Keep Open
2 O GDR This pin is N-MOS gate control
3 P RESE Current sense input fo r control loop
4 NC No connec tion and do not connec t with other NC pins Keep Open
5 P VSHR Positive source voltage for Red
```
6 O TSCL (^) I^2 C clock for external temperature sensor
7 I/O TSDA I^2 C data fo r external temperature sensor
8 I BS Input interface setting. Sel ec t 3 wire/ 4 wire SPI interface Note 1. 5-
9 O BUSY_N This pin indicates the driver status Note 1. 5-
10 I RST_N Global reset pin. Low reset Note 1. 5-
11 I DC Serial communication Command/Data input Note 1. 5-
12 I CSB Serial communication chip select Note 1 .5- 1
13 I SCL Serial communication clock input
14 I/O SDA Serial communication data input
15 P VDDIO IO voltage s up ply
16 P VDD Digital/Analog power
17 P VSS Digital gr ound
18 P VDD_
V
1.8V voltage input &output
19 P VOTP OTP program power ( 7.5V)
20 P VSH Positive s ource voltage
21 P VGH Positive gate voltage
22 P VSL Negative s ource voltage
23 P VGL Negative gate voltage
24 O VCOM VCOM output


Note 1.5- 1: This pin (CSB) is the chip select input connecting to the MCU. The chip is enabled for
MCU communication only when CSB is pulled Low.
Note 1. 5- 2: This pin (DC) is Data/Command control pin connecting to the MCU. When the pin is
pulled HIGH, the data will be interpreted as data. When the pin is pulled Low, the data
will be interpreted as command.
Note 1.5- 3: This pin (RST_N) is reset signal input. The Reset is active Low.
Note 1.5- 4: This pin (BUSY_N) is BUSY_N state output pin. When BUSY_N is low, the operation of
chip should not be interrupted and any commands should not be issued to the module.
The driver IC will put BUSY_N pin low when the driver IC is working such as:

- Outputting display waveform; or
- Programming with OTP
- Communicating with digital temperature sensor
Note 1.5- 5: This pin (BS) is for 3-line SPI or 4-line SPI selection. When it is “Low”, 4-line SPI is
selected. When it is “High”, 3-line SPI (9 bits SPI) is selected. Please refer to below
Table.

```
Table: Bus interface selection
BS MPU Interface
L 4- lines serial peripheral interface (SPI)
H 3- lines serial peripheral interface (SPI) – 9 bits SPI
```

#### 1. 6 Reference Circuit

1. Inductor L1 is wire-wound inductor. There are no special requirements
    for other parameters.
2. Suggests using Si1304BDL or Si1308EDL TUBE MOS (Q1) , otherwise it
    may affect the normal boost of the circuit.
3. The default circuit is 4-wire SPI. If the user wants to use 3-wire SPI.
4. Default voltage value of all capacitors is 50V.

**Note** ：


###### CAUTION

```
The display module should not be exposed to harmful gases, such as acid and
alkali gases, which corrode electronic components.
Disassembling the display module can cause permanent damage and invalidate
the warranty agreements.
```
Observe general precautions that are common to handling delicate electronic
components. The glass can break and front surfaces can easily be damaged. Moreover
the display is sensitive to static electricity and other rough environmental conditions.

```
Data sheet status
Product specification The data sheet contains final product specifications.
Limiting values
Limiting values given are in accordance with the Absolute Maximum Rating System
(IEC 134).
Stress above one or more of the limiting values may cause permanent damage to
the device.
These are stress ratings only and operation of the device at these or any other
conditions above those given in the Characteristics sections of the specification is
not implied. Exposure to limiting values for extended periods may affect device
reliability.
Application information
Where application information is given, it is advisory and dose not form part of
the specification.
```
```
Product Environmental certification
RoHS
```
###### WARNING

```
The display glass may break when it is dropped or bumped on a hard surface.
Handle with care.
Should the display break, do not touch the electrophoretic material. In case of
contact with electrophoretic material, wash with water and soap.
```
#### 2. Environmental

#### 2.1 Handling, Safety and Environmental Requirements


#### 2.2 Reliability test

###### TEST CONDITION METHOD REMARK

###### 1

```
High-
Temperature
Operation
```
###### T = 50 °C,

###### RH=35%

```
for 240 hrs
```
```
When the experimental cycle finished, the
EPD samples will be taken out from the
high temperature environmental chamber
and set aside for a few minutes. As EPDs
return to room temperature, testers will
observe the appearance, and test
electrical and optical performance based
on standard # IEC 60 068 -2-2Bp.
```
```
When
experiment
finished, the EPD
must meet
electrical and
optical
performance
standards.
```
###### 2

```
Low-
Temperature
Operation
```
```
T = 0°C for
240 hrs
```
```
When the experimental cycle finished, the
EPD samples will be taken out from the
low temperature environmental chamber
and set aside for a few minutes. As EPDs
return room temperature, testers will
observe the appearance, and test
electrical and optical performance based
on standard # IEC 60 068 -2-2Ab.
```
```
When
experiment
finished, the EPD
must meet
electrical and
optical
performance
standards.
```
###### 3

```
High-
Temperature
Storage
```
###### T = +70°C,

###### RH=35%

```
for 240 hrs
Test in white
pattern
```
```
When the experimental cycle finished, the
EPD samples will be taken out from the
high temperature environmental chamber
and set aside for a few minutes. As EPDs
return to room temperature, testers will
observe the appearance, and test
electrical and optical performance based
on standard # IEC 60 068 -2-2Bp.
```
```
When
experiment
finished, the EPD
must meet
electrical and
optical
performance
standards.
```
###### 4

```
Low-
Temperature
Storage
```
```
T = - 25 °C for
240 hrs
Test in white
pattern
```
```
When the experimental cycle finished, the
EPD samples will be taken out from the
low temperature environmental chamber
and set aside for a few minutes. As EPDs
return to room temperature, testers will
observe the appearance, and test
electrical and optical performance based
on standard # IEC 60 068 -2-2Ab
```
```
When
experiment
finished, the EPD
must meet
electrical and
optical
performance
standards.
```
###### 5

```
High
Temperature
, High-
Humidity
Operation
```
###### T=+40°C,

###### RH=80%

```
for240hrs
```
```
When the experimental cycle finished, the
EPD samples will be taken out from the
environmental chamber and set aside for
a few minutes. As EPDs return to room
temperature, testers will observe the
appearance, and test electrical and optical
performance based on standard # IEC
60 068 -2-3CA.
```
```
When
experiment
finished, the EPD
must meet
electrical and
optical
performance
standards.
```
###### 6

```
High
Temperature
, High-
Humidity
Storage
```
###### T=+60°C,

###### RH=80%

```
For 240hrs
Test in white
pattern
```
```
When the experimental cycle finished, the
EPD samples will be taken out from the
environmental chamber and set aside for
a f ew minutes. As EPDs return to room
temperature, testers will observe the
appearance, and test electrical and optical
performance based on standard # IEC
60 068 -2-3CA.
```
```
When
experiment
finished, the EPD
must meet
electrical
performance
standards.
```

```
7 TemperatureCycle^
```
```
[- 25°C 30mins]→
[+70°C, RH=35%
30mins],
70cycles,
Test in white
pattern
```
1. Samples are put in the Temp & Humid.
    Environmental Chamber. Temperature
    cycle starts with - 25 °C, storage period
    30 minutes. After 30 minutes, it needs
    30min to let temperature rise to 70°C.
    After 30min, temperature will be
    adjusted to 70°C, RH=35% and
    storage period is 30 minutes. After 30
    minutes, it needs 30min to let
    temperature rise to -25°C. One
    temperature cycle (2hrs) is complete.
2. Temperature cycle repeats 70 times.
3. When 70 cycles finished, the samples
    will be taken out from experiment
    chamber and set aside a few minutes.
    As EPDs return to room temperature,
    tests will observe the appearance, and
    test electrical and optical performance
    based on standard # IEC 60 068 -2-
    14NB.

```
When
experiment
finished, the
EPD must
meet
electrical and
optical
Performance
standards.
```
```
8 UV exposureResistance 765 W/m
```
(^2) for
168 hrs,40°C
Standard # IEC 60 068-2-5 Sa
9 Electrostadischargeti c^
Machine model:
+/-250V,
0 Ω,200pF
Standard # IEC61000-4-
(^10) VibrationPackage^
1.04G,Frequency
: 10~500Hz
Direction : X,Y,Z
Duration:1hours
in each direction
Full packed for shipment
(^11) DropPackage Impact^
Drop from height
of 122 cm on
Concrete surface
Drop sequence:
corner, 3edges,
6face One drop
for each.
Full packed for shipment
Actual EMC level to be measured on customer application.
Note:
(1) The protective film must be removed before temperature test.
(2) In order to make sure the display module can provide the best display quality, the update
should be made after putting the display module in stable temperature environment for 4
hours at 25 °C.


#### 3. Electrical Characteristics

#### 3.1 Absolute maximum rating

```
Parameter Symbol Rating Unit
Logic Supply Voltage VCI -0.3 to +6.0 V
```
Digital Input Voltage (^) VI -0.3 to TBD V
Operating Temp. range TOPR 0 to +50 (^) °C
Storage Temp. range TSTG -25 to +70 °C
Humidity range - 40 ~ (^70) %RH
*Note: Avoid direct sunlight.

#### 3.2 Panel DC Characteristics

```
The following specifications apply for: VSS = 0V, VCI = 3.3V, TA = 25 °C
Parameter Symbol Conditions Min Typ Max Unit
Single ground VSS - - 0 - V
IO supply Voltage VDDIO - 2.3 3.3 3.6 V
Digital/Analog supply voltage VDD - 2.3 3.3 3.6 V
High level input vo ltage VIH Digital input pins 0.7VIO - VIO V
Low level input voltage VIL Digital input pins GND - 0.3VDD V
High level output voltage VOH Digital input pins,IOH=400uA VIO-0.4 - - V
Low level output voltage VOL Digital input pins,IOL=-400uA GND - GND+0.4 V
Image update current IUPDATE - - 8 12 mA
Standby panel current Istand by - - 0.215 0.225 mA
Power panel (update) PUPDATE - - 26.4 45 mW
Standby power panel PSTBY - - 0.71 0.81 mW
```
Operating temperature - - 0 - (^50) °C
Storage temperature - - -25 - (^70) °C
Image update Time at 25°C - -^4 -^8 Sec^
Deep sleep mode cu rrent IVCI
DC/DC off
No clock
No input load
Ram data not retain

- 2 5 uA
- The Typical power consumption is measured with following pattern transition: from
horizontal 2 gray scale pattern to vertical 2 gray scale pattern.(Note 3-1)
- The standby power is the consumed power when the panel controller is in standby mode.
- The listed electrical/optical characteristics are only guaranteed under the controller &
waveform provided by Waveshare
- Vcom is recommended to be set in the range of assigned value ± 0.1V.

Note 3-1 The Typical power consumption


#### 3.3 Panel AC Characteristics

3.3- 1) Oscillator frequency

The following specifications apply for: VSS = 0V, VCI = 3.3V, TA = 25°C

```
Parameter Symbol Conditions Min Typ Max Unit
Internal Oscillator frequency Fosc VCI=2.3 to 3.6V - 1.625 - MHz
```
3.3- 2) MCU Interface

3.3-2- 1) MCU Interface Selection

In this module, there are 4-wire SPI and 3-wire SPI that can communicate with MCU. The
MCU interface mode can be set by hardware selection on BS pins. When it is “Low”, 4-wire
SPI is selected. When it is “High”, 3-wire SPI (9 bits SPI) is selected.
Pin Name Data/Command
Control Signal
Bus interfac e D1 D0 CSB DC RST_N
SPI4 SDA SCL CSB DC RST_N
SPI3 SDA SCL CSB L RST_N
Table 3- 1: MCU interface assignment under different bus interface mode

Note 3-2: L is connected to VSS
Note 3-3: H is connected to VCI


3.3-2-2) MCU Serial Interface (4-wire SPI)

The 4-wire SPI consists of serial clock SCL, serial data SDA, DC, CSB. In SPI mode, D0 acts as
SCL, D1 acts as SDA.

```
Function CSB DC SCL
Write Command L L ↑
Write data L H ↑^
Table 3- 2: Control pins of 4-wire Serial Peripheral interface
```
Note 3-4: ↑stands for rising edge of signal

SDA is shifted into an 8-bit shift register in the order of D7, D6, ... D0. The data byte in the
shift register is written to the Graphic Display Data RAM (RAM) or command register in the
same clock. Under serial mode, only write operations are allowed.

```
Figure 3-1: Write procedure in 4-wire Serial Peripheral Interface mode
```

3.3-2-3) MCU Serial Interface (3-wire SPI)

The 3-wire serial interface consists of serial clock SCL, serial data SDA and CSB.
In 3-wire SPI mode, D0 acts as SCL, D1 acts as SDA, The pin DC can be connected to an
external ground.

The operation is similar to 4-wire serial interface while DC pin is not used. There are
altogether 9-bits will be shifted into the shift register on every ninth clock in sequence:
DC bit, D7 to D0 bit. The DC bit (first bit of the sequential data) will determine the
following data byte in shift register is written to the Display Data RAM (DC bit = 1) or the
command register (DC bit = 0). Under serial mode, only write operations are allowed.

```
Function CSB DC SCL
Write Command L Tie LOW ↑
Write data L Tie LOW ↑
```
```
Table 7- 3: Control pins of 3-wire Serial Peripheral Interface
```
Note 3-5: ↑stands for rising edge of signal

```
Figure 7-2: Write procedure in 3-wire Serial Peripheral Interface mode
```

3.3- 3) Timing Characteristics of Series Interface

```
Symbol Signal Parameter Min Typ Max Unit
tcss
CSB
```
```
Chip Select Setup Time 100 - - ns
tcsh Chip Select Hold Time 100 - - ns
tscc Chip Select Setup Time 50 - - ns
tchw Chip Select Setup Time 500 - - ns
tscycw
SCL
```
```
Serial clock cycle (write) 100 - - ns
tshw SCL “H” pulse width (write) 35 - - ns
tslw SCL“L” pulse width (write) 35 - - ns
tscycr Serial clock cycle (Read) 200 - - ns
```

```
tshr SCL “H” pulse width (Read) 85 - - ns
tslr SCL “L” pulse width (Read) 85 - - ns
tsds
SDA
(DIN)
(DOUT)
```
Data setup time 30 - - ns
tsdh Data hold ti me 30 - - ns
tacc Access time 10 - - ns
toh Output disable time 15 - - ns
tcds
D/C
DC setup time 20 ns
tcdh DC hold time 20 ns


#### 4. Typical Operating Sequence

#### 4.1 Normal Operation Flow

**4.1-1)BWR mode & LUT from Register**

```
System Power
```
```
Reset the EPD driver IC
```
```
Power setting
```
```
Power saving
```
```
Display refresh
```
```
Border floating
```
```
Turn off Enter into deep
sleep mode
```
```
Set Vcom/read states
```
```
Load image data
```
```
Power on
```
```
Resolution setting
```
```
Booster soft start
```
```
Power optimization
```
```
Panel setting
```
```
PLL control
```

**4.1-2) BWR mode & LUT from OTP**

```
Reset the EPD driver IC
```
```
Power saving
```
```
Display refresh
```
```
Border floating
```
```
Turn off Enter into deep
sleep mode
```
```
Set Vcom/read states
```
```
Load image data
```
```
Power on
```
```
Resolution setting
```
```
Booster soft start
```
```
Power optimization
```
```
Panel setting
```
```
System Power
```

#### 4.2 Reference Program Code

**4.2-1) BWR mode & LUT from register**

Note1: Set border to floating.


**4.2-2) BWR mode & LUT from OTP**

Note1: Set border to floating.


#### 5.Command Table

W/R: 0: Write cycle 1: Read cycle C/D: 0: Command 1: Data
D7~D0: -: Don’t care #: Valid Data

```
# Command W/R^ C/D^ D7 D6 D5 D4 D3 D2 D1 D0 Registers Default
```
```
1 Panel Setting (PSR)
```
```
0 0 0 0 0 0 0 0 0 0 00H
0 1 -- -- # # # # # # REG, KW/R, UDSHD_N, RST_N, SHL, 0F H
```
```
2 Power Setting (PWR)
```
```
0 0 0 0 0 0 0 0 0 1 01H
0 1 -- -- -- # -- # # # BD_EN, VVG_ENSR_EN , VS_EN,^ 07H
```
```
0 1 # -- -- # -- 3 # # VPP_EN,VG_L VCOM_SLEVL[2:0] W,^ 17H
0 1 -- -- # # # # # # VDH_LVL[5:0] 3AH
0 1 -- -- # # # # # # VDL_LVL[5:0] 3AH
0 1 -- -- # # # # # # VDHR_LVL[5:0] 03H
3 Power OFF (POF) 0 0 0 0 0 0 0 0 1 0 02H
4 Power OFF SSetting (eqPFSuence )
0 0 0 0 0 0 0 0 1 1 03H
0 1 -- -- # # -- -- -- -- T_VDS_OFF[1:0] 00H
5 Power ON (PON) 0 0 0 0 0 0 0 1 0 0 04H
6 Power ON Measure (PMES) 0 0 0 0 0 0 0 1 0 1 05H
```
```
7 Booster Soft Start (BTST)
```
```
0 0 0 0 0 0 0 1 1 0 06H
0 1 # # # # # # # # BT_PHA[7:0] 17H
0 1 # # # # # # # # BT_PHB[7:0] 17H
0 1 -- -- # # # # # # BT_PHC1[5:0] 17H
0 1 # -- # # # # # # PHC2_EN, BT_PHC2[5:0] 17H
8 Deep sleep (DSLP) 0 0 0 0 0 0 0 1 1 1 07H^
0 1 1 0 1 0 0 1 0 1 Check code A5H
```
```
9
```
```
Display Start Transmission
1 (DTM1, White/Black Data)
(x-byte command)
```
```
0 0 0 0 0 1 0 0 0 0 K/W or (800x600):OLD Pixel Data^ 10H
0 1 # # # # # # # # KPXL[1:8] -
0 1 : : : : : : : : : :
0 1 # # # # # # # # KPXL[n-7:n] -
```
```
10 Data Stop (DSP)
```
```
0 0 0 0 0 1 0 0 0 1 11H
1 1 # -- -- -- -- -- -- -- 00H
11 Display Refresh (DRF) 0 0 0 0 0 1 0 0 1 0 12H
```
```
12
```
```
Display Start transmission 2
(DTM2, Red Data) (x-byte
command)
```
```
0 0 0 0 0 1 0 0 1 1 Red or (800x600):NEW Pixel Data^ 13H
0 1 # # # # # # # # RPXL[1:8] -
0 1 : : : : : : : : : :
0 1 # # # # # # # # RPXL[n-7:n] -
```
```
13 Dual SPI
```
```
0 0 0 0 0 1 0 1 0 1 15H
1 1 -- -- # # -- -- -- -- MM_EN, DUSPI_EN 00H
14 Auto Sequence (AUTO) 0 0 0 0 0 1 0 1 1 1 17H
```

```
# Command W/R^ C/D^ D7 D6 D5 D4 D3 D2 D1 D0 Registers Default
0 1 1 0 1 0 0 1 0 1 Check code A5H
```
15 KW LUT option (KWOPT)

```
0 0 0 0 1 0 1 0 1 1 2BH
0 1 -- -- -- -- -- -- # # ATRED, NORED 00H
0 1 # # -- -- -- -- -- -- KWE[9:8] 00H
0 1 # # # # # # # # KWE[7:0] 00H
```
16 PLL control (PLL)

```
0 0 0 0 1 1 0 0 0 0 30H
0 1 -- -- -- -- # # # # FRS[3:0] 06H
```
17 Temperature SCalibration (TSC)ensor

```
0 0 0 1 0 0 0 0 0 0 40H
1 1 # # # # # # # # D[10:3] / TS[7:0] 00H
1 1 # # # -- -- -- -- -- D[2:0] / - 00H
```
18 Temperature SSelection (TSenE) sor

```
0 0 0 1 0 0 0 0 0 1 41H
0 1 # -- -- -- # # # # TSE,TO[3:0] 00H
```
19 Temperature(TSW) Sen sor Write^

```
0 0 0 1 0 0 0 0 1 0 42H
0 1 # # # # # # # # WATTR[7:0] 00H
0 1 # # # # # # # # WMSB[7:0] 00H
0 1 # # # # # # # # WLSB[7:0] 00H
```
20 Temperature Sen(TSR) sor Read^

```
0 0 0 1 0 0 0 0 1 1 43H
1 1 # # # # # # # # RMSB[7:0] 00H
1 1 # # # # # # # # RLSB[7:0] 00H
```
21 Panel Break Check (PBC)

```
0 0 0 1 0 0 0 1 0 0 44H
1 1 -- -- -- -- -- -- -- # PSTA 00H
```
22 VCOM and dasetting (CDI)ta interval

```
0 0 0 1 0 1 0 0 0 0 50H
0 1 # -- # # -- -- # # BDZ, BDV[1:0], DDX[1:0] 31H
0 1 -- -- -- -- # # # # CDI[3:0] 07H
```
23 Lower Power (LPD)D etection^

```
0 0 0 1 0 1 0 0 0 1 51H
1 1 -- -- -- -- -- -- -- # LPD 01H
```
24 End Voltage Se tting (EVS)

```
0 0 0 1 0 1 0 0 1 0 52H
0 1 -- -- -- -- # -- # # VCEND, BDEND[1:0] 02H
```
25 TCON setting (TCON)

```
0 0 0 1 1 0 0 0 0 0 60H
0 1 # # # # # # # # S2G[3:0], G2S[3:0] 22H
```
26 Resolution setting (TRES)

```
0 0 0 1 1 0 0 0 0 1 61H
0 1 -- -- -- -- -- -- # # HRES[9:8] 03H
0 1 # # # # # 0 0 0 HRES[7:3] 20H
0 1 -- -- -- -- -- -- # #
VRES[9:0]
```
```
02H
0 1 # # # # # # # # 58H
```
27 Gate/Source(GSST)^ Start setting 0 0 0 1 1 0 0 1 0 1 65H


# Command W/R^ C/D^ D7 D6 D5 D4 D3 D2 D1 D0 Registers Default
0 1 -- -- -- -- -- -- # # HST[9:8] 00H
0 1 # # # # # 0 0 0 HST[7:3] 00H
0 1 -- -- -- -- -- -- # #
VST[9:0]

```
00H
0 1 # # # # # # # # 00H
```
28 Revision (REV) 0 0 0 1 1 1 0 0 0 0 70H

```
1 1 # # # # # # # # PROD_REV[23:16] FFH
1 1 # # # # # # # # PROD_REV[15:8] FFH
1 1 # # # # # # # # PROD_REV[7:0] FFH
1 1 # # # # # # # # LUT_REV[23:16] FFH
1 1 # # # # # # # # LUT_REV[15:8] FFH
1 1 # # # # # # # # LUT_REV[7:0] FFH
1 1 # # # # # # # # CHIP_REV[7:0] 0CH
```
29 Get Status (FLG) 0 0 0 1 1 1 0 0 0 1 71H

```
1 1 -- # # # # # # #
```
```
PTL_FLAG ,I^2 C_ERR,
I^2 C_BUSYN, DATA_FLAG,
PON, POF, BUSY_N
```
```
13H
```
30 Auto Measurement VCOM
(AMV)

```
0 0 1 0 0 0 0 0 0 0 80H
0 1 -- -- # # # # # # AMVT[1:0], XON,AMVS,
AMV, AMVE
```
```
10H
```
31 Read VCOM Value (VV) 0 0 1 0 0 0 0 0 0 1 81H

```
1 1 -- # # # # # # # VV[6:0] 00H
```
32 VCOM_DC Setting (VDCS) 0 0 1 0 0 0 0 0 1 0 82H

```
0 1 -- # # # # # # # VDCS[6:0] 00H
```

```
# Command W/R^ C/D^ D7 D6 D5 D4 D3 D2 D1 D0 Registers Default
```
33 Partial Window (PTL)

```
0 0 1 0 0 1 0 0 0 0 90H
0 1 -- -- -- -- -- -- # # HRST[9:8] 00H
0 1 # # # # # 0 0 0 HRST[7:3] 00H
0 1 -- -- -- -- -- -- # # HRED[9:8] 03H
0 1 # # # # # 1 1 1 HRED[7:3] 1FH
0 1 -- -- -- -- -- -- # #
VRST[9:0]
```
```
00H
0 1 # # # # # # # # 00H
0 1 -- -- -- -- -- -- # #
VRED[8:0]
```
```
02H
0 1 # # # # # # # # 57H
0 1 -- -- -- -- -- -- -- # PT_SCAN 01H
```
34 Partial In (PTIN) 0 0 1 0 0 1 0 0 0 1 91H

35 Partial Out (PTOUT) 0 0 1 0 0 1 0 0 1 0 92H

36 Program Mode (PGM) 0 0 1 0 1 0 0 0 0 0 A0H

37 Active Programming (APG) 0 0 1 0 1 0 0 0 0 1 A1H

38 Read OTP (ROTP)

```
0 0 1 0 1 0 0 0 1 0 A2H
1 1 # # # # # # # # Data of Address = 000h N/A
1 1 : : : : : : : : : N/A
1 1 # # # # # # # # Data of Address = n N/A
```
39 Cascade Setting (CCSET)

```
0 0 1 1 1 0 0 0 0 0 E0H
0 1 -- -- -- -- -- -- # # TSFIX, CCEN 00H
```
40 Power Saving (PWS)

```
0 0 1 1 1 0 0 0 1 1 E3H
0 1 # # # # # # # # VCOM_W[3:0], SD_W[3:0] 00H
```
41 LVD Voltage Select (LVSEL)

```
0 0 1 1 1 0 0 1 0 0 E4H
0 1 -- -- -- -- -- -- # # LVD_SEL[1:0] 03H
```
42 Force Temperature (TSSET)

```
0 0 1 1 1 0 0 1 0 1 E5H
0 1 # # # # # # # # TS_SET[7:0] 00H
```
43 TemperaturePhase-C2 (TSBDRY)^ Boundary

```
0 0 1 1 1 0 0 1 1 1 E7H
0 1 # # # # # # # # TSBDRY_PHC2[7:0] 00H
```

(1) Panel Setting (PSR) (Register: R00h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Setting the panel
```
###### 0 0 0 0 0 0 0 0 0 0

###### 0 1 - - REG KW/R UD SHL SHD_N RST_N

REG: LUT selection
0: LUT from OTP. (Default)
1: LUT from register.
KW/R: Black / White / Red
0: Pixel with Black/White/Red, KWR mode. (Default)
1: Pixel with Black/White, KW mode.
UD: Gate Scan Direction

```
0: Scan down. First line to Last line: Gn-1→Gn-2→Gn-3→...→G0
1: Scan up. (Default) First line to Last line: G0→G1→G2 →... .... →Gn-1
```
SHL: Source Shift Direction

```
0: Shift left. First data to Last data: Sn-1→Sn-2→Sn-3→...→S0
1: Shift right. (Default) First data to Last data: S0→S1→S2→... .... →Sn-1
```
SHD_N: Booster Switch 0: Booster OFF
1: Booster ON (Default)
When SHD_N becomes LOW, charge pump will be turned OFF, register and
SRAM data will keep until VDD OFF. And Source/Gate/Border/VCOM will be
released to floating.
RST_N: Soft Reset
0: Reset. Booster OFF, Register data are set to their default values, all drivers
will be reset, and all functions will be disabled. Source/Gate/Border/VCOM will
be released to floating.
: No effect (Default).

(2) Power Setting (PWR) (R01h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Selecting
Internal/External
Power
```
###### 0 0 0 0 0 0 0 0 0 1

###### 0 1 - - - BD_EN - VSR_EN VS_EN VG_EN

###### 0 1 VPP_EN - -

###### VCOM

###### _SLEW -^ VG_LVL[2:0]^

###### 0 1 - - VDH_LVL[5:0]

###### 0 1 - - VDL_LVL[ 5 :0]

###### 0 1 - - VDHR_LVL[5:0]


BD_EN: Border LDO enable
0 : Border LDO disable (Default)
Border level selection: 00b: VCOM 01b: VDH 10b: VDL 11b: VDHR
1 : Border LDO enable
Border level selection: 00b: VCOM 01b: VBH(VCOM-VDL)
10b:VBL(VCOM-VDH) 11b: VDHR
VSR_EN: Source LV power selection
0 : External source power from VDHR pins
1 : Internal DC/DC function for generating VDHR. (Default)
VS_EN: Source power selection
0 : External source power from VDH/VDL pins
1 : Internal DC/DC function for generating VDH/VDL. (Default)
VG_EN: Gate power selection
0 : External gate power from VGH/VGL pins
1 : Internal DC/DC function for generating VGH/VGL. (Default)
VPP_EN: OTP program power selection
0 : External OTP program power from VPP pin
1 : OTP program power from internal power circuit.
Internal OTP program power voltage is selected by VDHR_LVL[5:0].
VCOM_SLEW: VCOM slew rate selection for voltage transition

0 : Slow slew rate
1 : Fast slew rate
VG_LVL[2:0]:VGH / VGL Voltage Level selection.
VG_LVL[2:0] VGH/VGL Voltage Level
000 VGH=9V, VGL= -9V
001 VGH=1 0 V, VGL= - 10V
010 VGH=11V, VGL= -11V
011 VGH=1 2 V, VGL= - 12V
100 VGH=17V, VGL= -17V
101 VGH=18V, VGL= - 18V
110 VGH=19V, VGL= - 19V
111 (Default) VGH=2 0 V, VGL= - 20V


VDH_LVL[5:0]: Internal VDH power selection for K/W pixel.(Default value: 111010b)
VDH_LVL Voltage VDH_LVL Voltage VDH_LVL Voltage VDH_LVL Voltage
000000 2. 4 V 010001 5.8 V 100010 9.2 V 110011 12.6 V
000001 2.6 V 010010 6.0 V 100011 9.4 V 110100 12.8 V
000010 2.8 V 010011 6.2 V 100100 9.6 V 110101 13.0 V
000011 3.0 V 010100 6.4 V 100101 9.8 V 110110 13.2 V
000100 3.2 V 010101 6.6 V 100110 10.0 V 110111 13.4 V
000101 3.4 V 010110 6.8 V 100111 10.2 V 111000 13.6 V
000110 3.6 V 010111 7.0 V 101000 10.4 V 111001 13.8 V
000111 3.8 V 011000 7.2 V 101001 10.6 V 111010 14.0 V
001000 4.0 V 011001 7.4 V 101010 10.8 V 111011 14.2 V
001001 4.2 V 011010 7.6 V 101011 11.0 V 111100 14.4 V
001010 4.4 V 011011 7.8 V 101100 11.2 V 111101 14.6 V
001011 4. 6 V 011100 8.0 V 101101 11.4 V 111110 14.8 V
001100 4.8 V 011101 8.2 V 101110 11.6 V 111111 15.0 V
001101 5.0 V 011110 8 .4 V 101111 11.8 V
001110 5.2 V 011111 8. 6 V 110000 12.0 V
001111 5.4 V 100000 8 .8 V 110001 12.2 V
010000 5.6 V 100001 9.0 V 110010 12.4 V

VDL_LVL[5: 0 ]: Internal VDL power selection for K/W pixel. (Default value: 111010b)
VDL_LVL Voltage VDL_LVL Voltage VDL_LVL Voltage VDL_LVL Voltage
000000 -2.4 V 01 0 001 -5.8 V 100010 -9.2 V 110011 -12.6 V
000001 -2.6 V 01 0 010 -6.0 V 100011 -9.4 V 110100 -12.8 V
000010 -2.8 V 01 0 011 -6.2 V 100100 -9.6 V 110101 -13.0 V
000011 -3.0 V 01 0 100 -6.4 V 100101 -9.8 V 110110 -13.2 V
000100 -3.2 V 01 0 101 -6.6 V 100110 -10.0 V 110111 -13.4 V
000101 -3.4 V 01 0 110 -6.8 V 100111 -10.2 V 111000 -13.6 V
000110 -3.6 V 01 0 111 -7.0 V 101000 -10.4 V 111001 -13.8 V
000111 -3.8 V 01 1 000 -7.2 V 101001 -10.6 V 111010 -14.0 V
001000 -4.0 V 01 1 001 -7.4 V 101010 -10.8 V 111011 -14.2 V
001001 -4.2 V 011010 -7.6 V 101011 -11.0 V 111100 -14.4 V
001010 -4. 4 V 011011 -7.8 V 101100 -11.2 V 111101 -14.6 V
001011 -4.6 V 01 1 100 -8.0 V 101101 -11.4 V 111110 -14.8 V
001100 -4.8 V 01 1 101 -8.2 V 101110 -11.6 V 111111 -15.0 V
001101 -5.0 V 011110 -8. 4 V 101111 -11.8 V
001110 -5.2 V 011111 -8.6 V 110000 -12.0 V
001111 -5.4 V 10 0 000 -8.8 V 110001 -12.2 V
010000 -5.6 V 100001 -9.0 V 110010 -12.4 V


VDHR_LVL[5:0]: Internal VDHR power selection for Red pixel. (Default value: 000011b)
VDHR_LVL Voltage VDHR_LVL Voltage VDHR_LVL Voltage VDHR_LVL Voltage
000000 2.4 V 010001 5.8 V 100010 9.2 V 110011 12.6 V
000001 2.6 V 010010 6.0 V 100011 9.4 V 110100 12.8 V
000010 2. 8 V 010011 6.2 V 100100 9.6 V 110101 13.0 V
000011 3.0 V 010100 6.4 V 100101 9.8 V 110110 13.2 V
000100 3.2 V 010101 6.6 V 100110 10.0 V 110111 13.4 V
000101 3.4 V 010110 6.8 V 100111 10.2 V 111000 13.6 V
000110 3.6 V 010111 7.0 V 101000 10.4 V 111001 13.8 V
000111 3.8 V 011000 7.2 V 101001 10.6 V 111010 14.0 V
001000 4.0 V 011001 7.4 V 101010 10.8 V 111011 14.2 V
001001 4. 2 V 011010 7.6 V 101011 11.0 V 111100 14.4 V
001010 4.4 V 011011 7.8 V 101100 11.2 V 111101 14.6 V
001011 4.6 V 011100 8.0 V 101101 11.4 V 111110 14.8 V
001100 4.8 V 011101 8.2 V 101110 11.6 V 111111 15.0 V
001101 5. 0 V 011110 8 .4 V 101111 11.8 V
001110 5.2 V 011111 8 .6 V 110000 12.0 V
001111 5.4 V 100000 8.8 V 110001 12.2 V
010000 5.6 V 100001 9.0 V 110010 12.4 V

(3) Power OFF (POF) (R02h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Turning OFF the power 0 0 0 0 0 0 0 0 1 0
```
After the Power OFF command, the driver will be powered OFF. Refer to the POWER
MANAGEMENT section for the sequence.
This command will turn off booster, controller, source driver, gate driver, VCOM, and
temperature sensor, but register data will be kept until VDD turned OFF or Deep Sl eep
Mode. Source/Gate/Border/VCOM will be released to floating.

(4) Power OFF Sequence Setting (PFS) (R03h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Setting Power OFF
sequence
```
###### 0 0 0 0 0 0 0 0 1 1

###### 0 1 - - T_VDS_OFF[1:0] - - - -

T_VDS_OFF[1:0]: Source to gate power off interval time.
00b: 1 frame (Default) 01b: 2 frames 10b: 3 frames 11b: 4 frame

(5) Power ON (PON) (Register: R04h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Turning ON the power 0 0 0 0 0 0 0 1 0 0
```

After the Power ON command, the driver will be powered ON. Refer to the POWER
MANAGEMENT section for the sequence.

This command will turn on booster, controller, regulators, and temperature sensor will
be activated for one-time sensing before enabling booster. When all voltages are ready,
the BUSY_N signal will return to high.

(6) Power ON Measure (PMES) (R05h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Internal Bandgap Set 0 0 0 0 0 0 0 1 0 1
```
This command enables the internal bandgap, which will be cleared by the next POF.

(7) Booster Soft Start (BTST) (R06h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Booster
Software Start
Set
```
###### 0 0 0 0 0 0 0 1 1 0

###### 0 1 BT_PHA[7:6] BT_PHA[5:3] BT_PHA[2:0]

###### 0 1 BT_PHB[7:6] BT_PHB[5:3] BT_PHB[2:0]

###### 0 1 - - BT_PHC1[5:3] BT_PHC1[2:0]

###### 0 1 PHC2E

###### N

###### - BT_PHC2[5:3] BT _PHC2[2:0]

BT_PHA[7:6]: Soft start period of phase A.
00b: 10mS 01b: 20mS 10b: 30mS 11b: 40mS
BT_PHA[5:3]: Driving strength of phase A
000b: strength 1 001b: strength 2 010b: strength 3 011b: strength 4
100b: strength 5 101b: strength 6 110b: strength 7 111b: strength 8 (strongest)

BT_PHA[2:0]: Minimum OFF time setting of GDR in phase A
000b: 0.27uS 00 1b: 0.34uS 010b: 0.40uS 011b: 0.54uS
100b: 0.80uS 101b: 1.54uS 110b: 3.34uS 111b: 6.58uS

BT_PHB[7:6]: Soft start period of phase B.
00b: 10mS 01b: 20mS 10b: 30mS 11b: 40mS
BT_PHB[5:3]: Driving strength of phase B
000b: strength 1 001b: strength 2 010b: strength 3 011b: strength 4
100b: strength 5 101b: strength 6 110b: strength 7 111b: strength 8 (strongest)

BT_PHB[2:0]: Minimum OFF time setting of GDR in phase B
000b: 0.27uS 00 1b: 0.34uS 010b: 0.40uS 011b: 0.54uS
100b: 0.80uS 101b: 1.54uS 110b: 3.34uS 111b: 6.58uS

BT_PHC1[5:3]: Driving strength of phase C1
000b: strength 1 001b: strength 2 010b: strength 3 011b: strength 4
100b: strength 5 101b: strength 6 110b: strength 7 111b: strength 8 (strongest)


BT_PHC 1 [ 2 : 0 ]: Minimum OFF time setting of GDR in phase C1
000b: 0.27uS 00 1b: 0.34uS 010b: 0.40uS 011b: 0.54uS
100b: 0.80uS 101b: 1.54uS 110b: 3.34uS 111b: 6.58uS

PHC2EN: Booster phase-C2 enable
0: Booster phase-C2 disable
Phase-C1 setting always is applied for booster phase-C.
1: Booster phase-C2 enable
If temperature > temperature boundary phase-C2(RE7h[7:0]), phase-C1
setting is applied for booster phase-C.
If temperature <= temperature boundary phase-C2(RE7h[7:0]), phase-C2
setting is applied for booster phase-C.
BT_PHC2[5:3]: Driving strength of phase C2
000b: strength 1 001b: strength 2 010b: strength 3 01 1b: strength 4
100b: strength 5 101b: strength 6 110b: strength 7 11 1b: strength 8 (stro ngest)

BT_PHC2[2: 0 ]: Minimum OFF time setting of GDR in phase C2
000b: 0.27uS 00 1b: 0.34uS 010b: 0.40uS 011b: 0.54uS
100b: 0.80uS 101b: 1.54uS 110b: 3.34uS 111b: 6.58uS

(8) Deep Sleep (DSLP) (R07h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0

```
Deep Sleep
```
###### 0 0 0 0 0 0 0 1 1 1

###### 0 1 1 0 1 0 0 1 0 1

After this command is transmitted, the chip will enter Deep Sleep Mode to save power.
Deep Sleep Mode will return to Standby Mode by hardware reset. The only one parameter
is a check code, the command will be executed if check code = 0xA5.

(9) Data Start Transmission 1 (DTM1) (R10h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0

```
Starting data
transmission
```
```
0 0 0 0 0 1 0 0 0 0
0 1 Pixel1 Pixel2 Pixel3 Pixel4 Pixel5 Pixel6 Pixel7 Pixel8
0 1 : : : : : : : :
```
(^0 1) (n-Pixel7)^ (n-Pix6) el (n-Pixel5)^ (n-Pixel4)^ (n-Pixel3)^ (n-Pixel2)^ (n-Pixel1)^ Pixel(n)
This command starts transmitting data and write them into SRAM.
In KW mode, this command writes “OLD” data to SRAM.
In KWR mode, this command writes “K/W” data to SRAM.
In Program mode, this command writes “OTP” data to SRAM for programming.


(10) Data Stop (DSP) (R11h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Stopping data
tr ansmission
```
###### 0 0 0 0 0 1 0 0 0 1

```
1 1 data_flag - - - - - - -
```
Check the completeness of data. If data is complete, start to refresh display.
Data_flag: Data flag of receiving user data.
0: Driver didn’t receive all the data.
1: Driver has already received all the one-frame data (DTM1 and DTM2).
After “Data Start” (R10h) or “Data Stop” (R11h) commands and when data_flag=1, the
refreshing of panel starts and BUSY_N signal will become “0”.

(11) Display Refresh (DRF) (R12h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Refreshing the display 0 0 0 0 0 1 0 0 1 0
```
While user sent this command, driver will refresh display (data/VCOM) according to SRAM
data and LUT.
After Display Refresh command, BUSY_N signal will become “0” and the refreshing of
panel starts.

(12) Da ta Start Transmission 2 (DTM2) (R13h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Starting data
transmission
```
###### 0 0 0 0 0 1 0 0 1 1

```
0 1 Pixel1 Pixel2 Pixel3 Pixel4 Pixel5 Pixel6 Pixel7 Pixel8
0 1 : : : : : : : :
```
(^0 1) (n-7Pixel ) (n-6Pixel ) (n-5) Pixel^ (n-4) Pixel (n-3) Pixel^ (n-2) Pixel^ (n-1) Pixel^ Pixel(n)
This command starts transmitting data and write them into SRAM. In KW mode, this
command writes “NEW” data to SRAM.
In KWR mode, this command writes “RED” data to SRAM.
(13) Dual SPI Mode (DUSPI) (R15h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Stopping data
transmission

###### 0 0 0 0 0 1 0 1 0 1

###### 0 1 - - MM_EN DUSPI_EN - - - -


This command sets dual SPI mode.
MM_EN: MM input pin definition enable.
0: MM input pin definition disable
1: MM input pin definition enable.
DUSPI_EN: Dual SPI mode enable.
0: Dual SPI mode disable (single SPI mode)
1: Dual SPI mode enable

(14) Auto Sequence (AUTO) (R17h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Auto Sequence
```
###### 0 0 0 0 0 1 0 1 1 1

###### 0 1 1 0 1 0 0 1 0 1

The command can enable the internal sequence to execute several commands
continuously. The successive execution can minimize idle time to avoid unnecessary
power consumption and reduce the complexity of host’s control procedure. The
sequence contains several operations, including PON, DRF, POF, DSLP.
AUTO (0x17) + Code(0xA5) = (PON  DRF  POF)
AUTO (0x17) + Code(0xA7) = (PON  DRF  POF  DSLP)

(15) KW LUT Option (KWOPT) (R2Bh)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
KW LUT Option
```
###### 0 0 0 0 1 0 1 0 1 1

###### 0 1 - - - - - - ATRED NORED

###### 0 1 KWE[9:8] - - - - - -

###### 0 1 KWE[7:0]

This command sets KW LUT mechanism option in KWR mode’s LUT and only valid in
K/W/R mode.
{ATRED, NORED}: KW LUT or KWR LUT selection control

```
ATRED NORED Description
0 0 KWR LUT always
0 1 KW LUT only
1 0 Auto detect by red data
1 1 KW LUT only
```
KWE[9:0]:
KW LUT enable control bits. Each bit controls one state, KWE[0] for state-1, KWE[1]
for state-2, ....
At least 1 Enable Control bit should be set when KW LUT only is selected in KWR mode.
00 0000 0001b: KW LUT enable in State-1
00 0000 0011b: KW LUT enable in State-1 and State2
00 0000 1011b: KW LUT enable in State-1, State2 and State-4


(16) PLL Control (PLL) (R30h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Controlling PLL
```
###### 0 0 0 0 1 1 0 0 0 0

###### 0 1 - - - - FRS[3:0]

The command controls the PLL clock frequency. The PLL structure must support the
following frame rates: FMR[3:0]: Frame rate setting

```
Horizontal
```
```
Vertical
```
(17) Temperature Sensor Calibration (TSC) (R40h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0

```
Sensing
Temperature
```
###### 0 0 0 1 0 0 0 0 0 0

###### 1 1 D10/TS7 D9 /TS6 D8/TS5 D7/TS4 D6/TS3 D5/TS2 D4/TS1 D3/TS0

###### 1 1 D2 D1 D0 - - - - -

This command enables internal or external temperature sensor, and reads the result.
TS[7:0]: When TSE (R41h) is set to 0, this command reads internal temperature sensor
value.
D[10:0]: When TSE (R41h) is set to 1, this command reads external LM75 temperature
sensor value.

```
FRS Frame rate
0000 5Hz
0001 10Hz
0010 15Hz
0011 20 Hz
0100 30Hz
0101 40Hz
0110 50Hz
0111 60Hz
```
```
FRS Frame rate
1000 70Hz
1001 80Hz
1010 90Hz
1011 100Hz
1100 110 Hz
1101 130Hz
1110 150Hz
1111 200Hz
```
```
Hsync H Active
```
###### DE

```
82 0 clocks
```
```
Vsyn V_Active
```
###### DE

```
620 lines
```

(18) Temperature Sensor Enable (TSE) (R41h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Enable Temperature
Sensor/Offset
```
###### 0 0 0 1 0 0 0 0 0 1

###### 0 1 TSE - - - TO[3:0]

```
This command selects Internal or External temperature sensor.
TSE: Internal temperature sensor switch
0: Enable (default) 1: Disable; using external sensor.
TO[3:0]: Temperature offset.
```
###### TS[7:0]/

###### D[10:3]

```
Temp.
(oC)
11 10_0111 -2 5
11 10_1000 -2 4
11 10_1001 -2 3
11 10_1010 -2 2
11 10_1011 -2 1
11 10_1 100 -20
11 10_1 101 -19
11 10_1110 -1 8
11 10_1 111 -17
1111_0000 -16
1111_0001 -15
1111_0010 -14
1111_0011 -13
1111_0100 -12
1111_0101 -11
1111 _ 0110 - 10
1111 _ 0111 - 9
1111_1000 -8
1111_1001 -7
1111_10 10 -6
1111_1011 -5
1111_1 100 -4
1111_1 101 -3
1111 _ 1110 - 2
1111 _ 1111 - 1
```
###### TS[7:0]/

###### D[10:3]

```
Temp.
(oC)
0000 _ 0000 0
0000 _ 0001 1
0000_0010 2
0000_0011 3
0000_0100 4
0000_0101 5
0000_0110 6
0000_0111 7
0000_1000 8
0000_1001 9
0000_1010 10
0000_1011 11
0000_1 100 12
0000_1 101 13
0000_1110 14
0000_ 1111 15
0001_0000 16
0001_0001 17
0001_0010 18
0001_0011 19
0001_0100 20
0001_0101 21
0001_0110 22
0001_0111 23
0001_1000 24
```
###### TS[7:0]/

###### D[10:3]

```
Temp.
(oC)
0001_1001 25
0001_1010 26
0001_1011 27
0001_1 100 28
0001_1 101 29
0001_1110 30
0001_1111 31
0010_0000 32
0010 _ 0001 33
0010 _ 0010 34
0010_0011 35
0010_0100 36
0010_0101 37
0010_0110 38
0010_0111 39
0010 _ 1000 40
0010_1001 41
0010_1010 42
0010_1011 43
0010_1 100 44
0010_1 101 45
0010_1110 46
0010 _ 1111 47
0011 _ 0000 48
0011 _ 0001 49
```

(19) Temperature Sensor Write (TSW) (R42h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Write External
Temperature Sensor
```
###### 0 0 0 1 0 0 0 0 1 0

###### 0 1 WATTR[7:0]

###### 0 1 WMSB[7:0]

###### 0 1 WLSB[7:0]

This command writes the temperature sensed by the temperature sensor.
WATTR[7:6]: I^2 C Write Byte Nu mber
00b : 1 byte (head byte only)
01b : 2 bytes (head byte + pointer)
10b : 3 bytes (head byte + pointer + 1st parameter)
11b : 4 bytes (head byte + pointer + 1st parameter + 2nd parameter)
WATTR[5:3]: User-defined address bits (A2, A1, A0)
WATTR[2:0]: Pointer setting
WMSB[7:0]: MSByte of write-data to external temperature sensor
WLSB[7:0]: LSByte of write-data to external temperature sensor

(20) Temperature Sensor Read (TSR) (R43h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Read External
Temperature Sensor
```
###### 0 0 0 1 0 0 0 0 1 1

###### 1 1 RMSB[7:0]

###### 1 1 RLSB[7:0]

This command reads the temperature sensed by the temperature sensor.
RMSB[7:0]: MSByte read data from external temperature sensor
RLSB[7:0]: LSByte read data from external temperature sensor

```
TO[ 3 : 0 ] Calibration
1000 -8
1001 -7
1010 -6
1011 -5
1100 -4
1101 -3
1110 -2
1111 -1
```
```
TO[ 3 : 0 ] Calibration
0000 b +0 (Default)
0001 + 1
0010 + 2
0011 + 3
0100 + 4
0101 + 5
0110 + 6
0111 + 7
```

(21) Panel Glass Check (PBC)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Check Panel Glass
```
###### 0 0 0 1 0 0 0 1 0 0

###### 1 1 - - - - - - - PSTA

This command is used to enable panel check, and to disable after reading result.
PSTA: 0: Panel check fail (panel broken) 1: Panel check pass

(22) VCOM and Data interval Setting (CDI) (R50h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Set Interval between
VCOM and Data
```
###### 0 0 0 1 0 1 0 0 0 0

###### 0 1 BDZ - BDV[1:0] N2OCP - DDX[1:0]

###### 0 1 - - - - CDI[3:0]

This command indicates the int erval of VCOM and data output. When setting the
vertical back porch, the total blanking will be kept (20 Hsync).
BDZ: Border Hi-Z control
0: Border output Hi-Z disabled (default) 1: Border output Hi-Z enabled
BDV[1:0]: Border LUT selection
KWR mode (KW/R=0)
DDX[0] BDV[1:0] LUT

###### 0

###### 00 LUTBD

###### 01 LUTR

###### 10 LUTW

###### 11 LUTK

###### 1

```
(Default)
```
###### 00 LUTK

###### 01 LUTW

###### 10 LUTR

###### 11 LUTBD

KW mode (KW/R= 1 )

```
DDX[0] BDV[1:0] LUT
```
###### 0

###### 00 LUTBD

(^01) LUTKW (1→0)
(^10) LUTWK (0→1)
(^11) LUTKK ( 0 →0)

###### 1

```
(Default)
```
(^00) LUTKK (0→0)
(^01) LUTWK (1→0)
(^10) LUTKW (0→1)
11 LUTBD


N2OCP: Copy frame data from NEW data to OLD data enable control after display refresh
with NEW/OLD in KW mode.
0: Copy NEW data to OLD data disabled (default)
1: Copy NEW data to OLD data enabled
DDX[1:0]: Data polality.
Under KWR mode (KW/R=0):
DDX[1] is for RED data.
DDX[0] is for K/W data,

```
Under KW mode (KW/R=1):
DDX[1]=0 is for KW mode with NEW/OLD,
DDX[1]=1 is for KW mode without NEW/OLD.
```
```
CDI[3:0]: VCOM and data interval
```
```
CDI[3:0] VCOM Inteand Drval ata^
1000 9
1001 8
1010 7
1011 6
1100 5
1101 4
1110 3
1111 2
```
```
CDI[3:0] VCOM Inteand Drval ata^
0000 b 17 hsync
0001 16
0010 15
0011 14
0100 13
0101 12
0110 11
0111 10 (Default)
```
```
DDX[1:0] Data {NEW} LUT
```
###### 10

(^0) LUTKW (1  0)

###### 1 LUTWK ( 0  1 )

###### 11

###### 0 LUTWK (1  0)

###### 1 LUTKW(0  1)

###### DDX

###### [1:0]

```
Data
{NEW, OLD} LUT^
```
###### 00

###### 00 LUTWW (0  0)

(^01) LUTKW(1  0)
(^10) LUTWK(0  1)
11 LUTKK (1  1)

###### 01

```
(Default)
```
###### 00 LUTKK (0  0)

(^01) LUTWK(1  0)
10 LUTKW(0  1)
11 LUTWW (1  1)
DDX[1:0] Data {Red, K/W} LUT

###### 10

###### 00 LUTR

###### 01 LUTR

###### 10 LUTW

###### 11 LUTK

###### 11

###### 00 LUTR

###### 01 LUTR

###### 10 LUTK

###### 11 LUTW

```
DDX[1:0] Data {Red, K/W} LUT
```
###### 00

###### 00 LUTW

###### 01 LUTK

###### 10 LUTR

###### 11 LUTR

###### 01

```
(Default)
```
###### 00 LUTK

###### 01 LUTW

###### 10 LUTR

###### 11 LUTR


(23) Low Power Detection (LPD) (R51h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Detect Low Power
```
###### 0 0 0 1 0 1 0 0 0 1

###### 1 1 - - - - - - - LPD

This command indicates the input power condition. Host can read this flag to learn the
battery condition.
LPD: Internal Low Power Detection Flag
0: Low power input (VDD < 2.5V, 2.4V, 2.3V, or 2.2V, selected by LVD_SEL[1:0] in
command LVSEL)
1: Normal status (default)

(24) End Voltage Setting (EVS) (R52h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
End Voltage Setting
```
###### 0 0 0 1 0 1 0 0 1 0

###### 0 1 - - - - VCEND - BDEND[1:0]

This command selects source end voltage and border end voltage after LUTs are finished.
VCEND: VCOM end voltage selection
0b: VCOM_DC 1b: floating
BDEND[1:0]: Border end voltage selection
00b: 0V 01b: 0V 10b: VCOM_DC 11b: floating

```
Internal
Vsync VCOMbefo re sou^ needrces datato^ be out^ readputy
```
```
Internal
Hsync
```
```
Internal
```
DE (^) VCOM
output
VCOM Frame (N) VCOM FraVmeCOM^ (N+1 )
Source data
output Frame^ (N)^ data
Frame (N+1)
data
CDI setting


(25) TCON Setting (TCON) (R60h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Set Gate/Source Non-
overlap Period
```
###### 0 0 0 1 1 0 0 0 0 0

###### 0 1 S2G[3:0] G2S[3:0]

This command defines non-overlap period of Gate and Source.
S2G[3:0] or G2S[3:0]: Source to Gate / Gate to Source Non-overlap period

```
Period Unit = 667 nS.
```
(26) Resolution Setting (TRES) (R61h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Set Display
Resolution
```
###### 0 0 0 1 1 0 0 0 0 1

###### 0 1 - - - - - - HRES[9:8]

###### 0 1 HRES[7:3] 0 0 0

###### 0 1 - - - - - - VRES[9:8]

###### 0 1 VRES[7:0]

This command defines resolution setting.
HRES[9:3]: Horizontal Display Resolution (Value range: 01h ~ 64h)
VRES[9:0]: Vertical Display Resolution (Value range: 001h ~ 258h)
Active channel calculation, assuming HST[9:0]=0, VST[9:0]=0:
Gate: First active gate = G0;
Last active gate = VRES[9:0] – 1
Source: First active source = S0;
Last active source = HRES[9:3]*8 – 1

```
S2G[3:0] or G2S[3:0] Period
1000 b 36
1001 40
1010 44
1011 48
1100 52
1101 56
1110 60
1111 64
```
```
S2G[3:0] or G2S[3:0] Period
0000 b 4
0001 8
0010 12 (Default)
0011 16
0100 20
0101 24
0110 28
0111 32
```

```
Example: 128 (source) x 272 (gate), assuming HST[9:0]=0, VST[9:0]=0
Gate: First active gate = G0,
Last active gate = G271; (VRES[9:0] = 272, 272 – 1= 271)
Source: First active source = S0,
Last active source = S127;^ (HRES[9:3]=16, 16*8 – 1 = 127)
```
```
(27) Gate/Source Start Setting (GSST) (R65h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Set
Gate/Source
Start
```
###### 0 0 0 1 1 0 0 1 0 1

###### 0 1 - - - - - - HST[9:8]

###### 0 1 HST[7:3] 0 0 0

###### 0 1 - - - - - - VST[9:8]

###### 0 1 VST[7:0]

```
This command defines resolution start gate/source position.
HST[9:3]: Horizontal Display Start Position (Source). (Value range: 00h ~ 63h)
VST[9:0]: Vertical Display Start Position (Gate). (Value range: 000h ~ 257h)
Example : For 128(Source) x 240(Gate)
HST[9:3] = 4 (HST[9:0] = 4*8 = 32),
VST[9:0] = 32
Gate: First active gate = G32 (VST[9:0] = 32),
Last active gate = G271 (VRES[9:0] = 240, VST[9:0] = 32, 240 -1+32=271)
Source: First active source = S32 (HST[9:0]= 32),
Last active source = S239 (HRES[9:0] = 128, HST[9:0] = 32, 128 -1+32=239)
```
```
(28) Revision (REV) (R70h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
LUT/Chip
Revision
```
###### 0 0 0 1 1 1 0 0 0 0

###### 1 1 PROD_REV[23:16]

###### 1 1 PROD_REV[15:8]

###### 1 1 PROD_REV[7:0]

###### 1 1 LUT_REV[ 2 3:16]

###### 1 1 LUT_REV[ 1 5:8]

###### 1 1 LUT_REV[ 7 :0]

###### 1 1 CHIP_REV[7:0]

The command reads the product revision, LUT revision and chip revision.
PROD_REV[23:0]: Product Revision. PROD_REV[23:0] is read from OTP address 0x0BDD
~ 0X0BDF or 0x17DD ~ 0x17DF.
LUT_REV[23:0]: LUT Revision. LUT_REV[23:0] is read from OTP address 0x0BE0 ~
0X0BE2 or 0x17E0.~ 0x17E2.
CHIP_REV[7:0]: Chip Revision, fixed at 00001100b.


```
(29) Get Status (FLG) (R 71h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Read Flags
```
###### 0 0 0 1 1 1 0 0 0 1

###### 1 1

```
PTL_Flag I^2 C_ERR I^2 C_BUSYN Data_Flag PON POF BUSY_N
This command reads the IC status.
PTL_Flag: Partial display status (high: partial mode)
I^2 C_ERR: I^2 C master error status
I^2 C_BUSYN: I^2 C master busy status (lo w active)
Data_Flag: Driver has already received all the one frame data
PON: Power ON status
POF: Power OFF status
BUSY_N: Driver busy status (low active)
```
```
(30) Auto Measure VCOM (AMV) (R80h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Automatically
measure VCOM
```
###### 0 0 1 0 0 0 0 0 0 0

###### 0 1 - - AMVT[1:0] XON AMVS AMV AMVE

This command triggers auto VCOM sensing mechanism.
AMVT[1:0]: Auto Measure VCOM Time
00b: 3s 01b: 5s (default)
10b: 8s 11b: 10s

XON: All Gate ON of AMV

```
0: Gate normally scan during Auto Measure VCOM period. (default)
1: All Gate ON during Auto Measure VCOM period.
AMVS: Source output of AMV
0: Source output 0V during Auto Measure VCOM period. (default)
1: Source output VDHR during Auto Measure VCOM period.
AMV: Analog signal
0: Get VCOM value with the VV command (R81h) (default)
1: Get VCOM value in analog signal. (External analog to digital converter)
AMVE: Auto Measure VCOM Enable (/Disable)
0: No effect (default)
1: Trigger auto VCOM sensing.
```
```
(31) VCOM Value (VV) (R81h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Automatically
measure VCOM
```
###### 0 0 1 0 0 0 0 0 0 1

###### 1 1 - VV[6:0]


VV[6:0]: VCOM Value Output

```
VV [6:0] VCOM Vo(V) ltage^ VV [6:0] VCOM Vo(V) ltage^ VV [6:0] VCOM Vo(V) ltage^
000 0000b -0.10 001 1011b -1.45 011 0110b -2.80
000 0001b -0.15 001 1100b -1.50 011 0 111b -2.85
000 0010b -0.20 00 1 1101b -1.55 011 1000b -2.90
000 0011b -0.25 00 1 1110b -1.60 011 1001b -2.95
000 0100b -0.30 00 1 1111b -1.65 011 1010b -3.00
000 0101b -0.35 01 0 0000b -1.70 011 1011b -3.05
000 0110b -0.40 01 0 0001b -1.7 5 011 1100b -3.10
000 0111b -0.45 010 00 10b -1.80 011 1101b -3.15
000 1000b -0.50 01 0 0011b -1.85 011 1110b -3.20
000 1001b -0.55 01 0 0100b -1.90 011 1111b -3.25
000 1010b -0.60 01 0 0101b -1.95 100 0000b -3.30
000 1011b -0.65 01 0 0110b -2.00 100 0001b -3.35
000 1100 b -0. 70 01 0 0111b -2.05 100 0010b -3.40
000 1101b -0.75 01 0 1000b -2.10 100 0011b -3.45
000 1110b -0.80 010 1 001b -2.15 100 0100b -3.50
000 1111b -0. 85 010 101 0b -2.20 100 0101b -3.55
001 0000b -0.90 01 0 1011b -2.25 100 0110b -3.60
001 0001b -0.95 01 0 1100b -2.30 100 0111b -3.65
001 0010b -1.00 01 0 1101b -2.35 100 1000b -3.70
001 0011b -1.05 01 0 1110b -2.40 100 1001b -3.75
001 0100b -1.10 01 0 1111b -2.45 100 1010b -3.80
001 0101b -1.15 01 1 0000b -2.50 100 1011b -3.85
001 0110b -1.2 0 01 1 0001b -2.55 100 1100b -3.90
001 0111b -1.25 011 00 10b -2.60 100 1101b -3.95
001 1000b -1.30 01 1 0011b -2.65 100 1110b -4.00
001 1001b -1.35 01 1 0100b -2.70 100 1111b -4.05
001 1010b -1.40 01 1 0101b -2.75
```

(32) VCOM_DC Setting (VDCS) (R82h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Set VCOM_DC
```
###### 0 0 1 0 0 0 0 0 1 0

###### 0 1 - VDCS[6:0]

This command sets VCOM_DC value VDCS[6:0]: VCOM_DC Setting

```
VDCS [6:0] VCOM Vo(V) ltage^ VDCS [6:0] VCOM Vo(V) ltage^ VDCS [6:0] VCOM Vo(V) ltage^
000 0000b -0.10 001 1011b -1.45 011 0110b -2.80
000 0001b -0.15 001 1100b -1.50 011 0 111b -2.85
000 0010b -0.20 001 1101b -1.55 011 1 000b -2.90
000 0011b -0.25 001 1110b -1.60 011 1 0 01b -2.95
000 0100b -0.30 001 1111b -1.65 011 1010b -3.00
000 0101b -0.35 010 0000b -1.70 011 1011b -3.05
000 0110 b -0.40 010 0 0 01b -1.75 011 1100b -3.10
000 0111b -0.45 010 0010b -1.80 011 1101b -3.15
000 1000b -0.50 010 0011b -1.85 011 1110 b -3.20
000 1001b -0.55 010 0100b -1.90 011 1111b -3.25
000 1010b -0.60 010 0101b -1.95 100 0000b -3.30
000 1011b -0.65 010 0110 b -2.00 100 0 0 01b -3.35
000 1100b -0.70 010 0 111b -2.05 100 0010b -3.40
000 1101b -0.7 5 0 10 1000b -2.10 100 0011b -3.45
000 1110 b -0.80 010 1 0 01b -2.15 100 0100b -3.50
000 1111b -0.85 010 1010b -2.20 100 0101b -3.55
001 0000b -0.90 010 1011b -2.25 100 0110b -3.60
001 0001b -0.95 010 1100b -2.30 100 0111 b -3.65
001 0010b -1.00 010 1101b -2.35 100 1 000b -3.70
001 0011b -1.05 010 1110b -2.40 100 1 0 01b -3.75
001 0100b -1.10 010 1111b -2.45 100 1010b -3.80
001 0101b -1.15 011 0000b -2.50 100 1011b -3.85
001 0110 b -1.20 011 0001b -2.55 100 1100b -3.90
001 0111b -1.25 011 0010b -2.60 100 110 1b -3.95
001 1000b -1.30 011 0011b -2.65 100 1110b -4.00
001 1001b -1.35 011 0100b -2.70 100 1111b -4.05
001 1010b -1.40 011 0101b -2.75
```

(33) Partial Window (PTL) (R90h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Set Partial
Window
```
###### 0 0 1 0 0 1 0 0 0 0

###### 0 1 - - - - - - HRST[9:8]

###### 0 1 HRST[7:3] 0 0 0

###### 0 1 - - - - - - HRED[9:8]

###### 0 1 HRED[7:3] 1 1 1

###### 0 1 - - - - - - VRST[9:8]

###### 0 1 VRST[7:0]

###### 0 1 - - - - - - VRED[9:8]

###### 0 1 VRED[7:0]

###### 0 1 - - - - - - - PT_SCAN

This command sets partial window.
HRST[9:3]: Horizontal start channel bank. (Value range: 00h~63h)
HRED[9:3]: Horizontal end channel bank. (Value range: 00h~63h). HRED must be greater
than HRST.
VRST[9:0]: Vertical start line. (Value range: 000h~257h)
VRED[9:0]:Vertical end line. (Value range: 000h~257h). VRED must be greater than VRST.
PT_SCAN: 0: Gates scan only inside of the partial window.
1: Gates scan both inside and outside of the partial window. (default)

(34) Partial In (PTIN) (R91h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Partial In 0 0 1 0 0 1 0 0 0 1

This command makes the display enter partial mode.

(35) Partial Out (PTOUT) (R92h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Partial Out 0 0 1 0 0 1 0 0 1 0

This command makes the display exit partial mode and enter normal mode.

(36) Program Mode (PGM) (RA0h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Enter Program Mode 0 0 1 0 1 0 0 0 0 0

After this command is issued, the chip would enter the program mode.
After the programming procedure completed, a hardware reset is necessary for leaving
program mode.


(37) Active Program (APG) (RA1h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Active Program OTP 0 0 1 0 1 0 0 0 0 1

After this command is transmitted, the programming state machine would be activated.
The BUSY_N flag would fall to 0 until the programming is completed.

(38) Read OTP Data (ROTP) (RA2h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0

```
Read OTP data
for check
```
###### 0 0 1 0 1 0 0 0 1 0

```
1 1 The data of address 0x000 in the OTP
1 1 The data of address 0x001 in the OTP
1 1 :
1 1 The data of address (n-1) in the OTP
1 1 The data of address (n) in the OTP
```
The command is used for reading the content of OTP for checking the data of
programming.
The value of (n) is depending on the amount of programmed data, the max address =
0x17FF.

(39) Cascade Setting (CCSET) (RE0h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Set Cascade Option
```
###### 0 0 1 1 1 0 0 0 0 0

###### 0 1 - - - - - - TSFIX CCEN

This command is used for cascade.
TSFIX: Let the value of slave’s temperature is same as the master’s.
0: Temperature value is defined by internal temperature sensor/external LM75.(default)
1: Temperature value is defined by TS_SET[7:0] registers.
CCEN: Output clock enable/disable.
0: Output 0V at CL pin. (default)
1: Output clock at CL pin to slave chip.

(40) Power Saving ( PWS) (RE3h)
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Power Saving for VCOM
& Source

###### 0 0 1 1 1 0 0 0 1 1

###### 0 1 VCOM_W[3:0] SD_W[ 3 :0]

This command is set for saving power during refreshing period. If the output voltage of
VCOM / Source is from negative to p ositive or from positive to negative, the power saving
mechanism will be activated. The active period width is defined by the following two
parameters.


VCOM_W[3:0]: VCOM power saving width (Unit: line period)

SD_W[3:0]: Source power saving width (Unit: 660nS)

(41) LVD Voltage Select (LVSEL) (RE4h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
```
```
Select LVD Voltage
```
###### 0 0 1 1 1 0 0 1 0 0

###### 0 1 - - - - - - LVD_SEL[1:0]

LVD_SEL[1:0]: Low Power Voltage selection

```
LVD_SEL[1:0] LVD value
00 < 2. 2 V
01 < 2. 3 V
10 < 2.4 V
11 < 2.5 V (default)
```
(42) Force Temperature (TSSET) (RE5h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Force Temperature
Value for Cascade
```
###### 0 0 1 1 1 0 0 1 0 1

###### 0 1 TS_SET[7:0]

This command is used for cascade to fix the temperature value of master and slave chip.

(43) Temperature Boundary Phase-C2 (TSBDRY) (RE7h)

```
Action W/R C/D D7 D6 D5 D4 D3 D2 D1 D0
Temperature Boundary
Phase-C2
```
###### 0 0 1 1 1 0 0 1 1 1

###### 0 1 TSBDRY_PHC2[7:0]

This command is used to set the temperature boundary to judge whether booster phase-
C2 is applied or not.


#### 6. Optical characteristics

#### 6.1 Specifications

Measurements are made with that the illumination is under an angle of 45 degrees, the
detection is perpendicular unless otherwise specified.
T=25°C
SYMBOL PARAMETER CONDITIONS MIN TYPE MAX UNIT Note
R Reflectance White 30 35 - % Note6-1^
Gn 2Grey Level - - DS+(WS-DS )xn(m-1) - L* -
CR Contrast Ratio indoor 8 - - -

Panel’s life (^0) °C~40°C 100 0000 times or 5 years Note6-2^
Panel
Image Update (^) transStorageportation and^ Update the white screen
Update Time Operation
Suggest update once
every 24 hours or at least
10 days to update again.
WS : White state, DS : Dark state
Gray state from Dark to White : DS、WS
m : 2
Note 6- 1 : Luminance meter : Eye – One Pro Spectrophotometer
Note 6-2 : Panel life will not guaranteed when work in temperature below 0 degree or
above 40 degree. Each update interval time should be minimum at 180 seconds.

#### 6.2 Definition of contrast ratio

The contrast ratio (CR) is the ratio between the reflectance in a full white area (R1) and
the reflectance in a dark area (Rd)() :
R1: white reflectance Rd: dark reflectance
CR = R1/Rd

##### Ring light

#### Detector

##### θ

#### Display


#### 6.3 Reflection Ratio

The reflection ratio is expressed as :

R = Reflectance Factor white board x (L center / L white board )
L center is the luminance measured at center in a white area (R=G =B=1). L white board is the
luminance of a standard white board. Both are measured with equivalent illumination
source. The viewing angle shall be no more than 2 degrees.

```
9 o'clock
direction 180°
```
```
3 o'clock
direction 0°
```
#### 6.4 Bi-stability

The Bi-stability standard as follows:
Bi-stability Result

```
24 hours Luminance drift
```
###### AVG MAX

White state △L* (^) - 3
Black state △L* - 3
Viewing
direction α direction 90° 12 o'clock
θ
6 o'clock
direction 270°


#### 7. Point and line standard

Shipment Inseption Standard
Part-A：Active area Part-B：Border area

Equipment：Electrical test fixture, Point gauge

Outline dimension：
1 70.2(H)×111.2(V)×1.18(D) Unit：mm

```
Environment
```
```
Temperature Humidity Illuminance Distance Time Angle
```
```
23 ± 2 °C
```
###### 55 ±

###### 5%RH

###### 1200 ～

```
1500Lux 300 mm^35 Sec^
Name Causes Spot size Part -A Part-B
```
```
Spot
```
```
B/W spot in
glass or
protection sheet,
foreign mat. Pin
hole
```
D ≤ 0.25mm (^) Ignore
Ignore
0.25mm ＜ D ≤ 0.4mm (^4)
0.4mm ＜ D ≤ 0.5mm 1
0.5mm ＜ D 0
Scratch or line
defect
Scratch on glass
or Scratch on
FPL or Particle is
Protection sheet.
Length Width Part-A
Ignore
L ≤ 2 .0mm W≤0.2 mm Ignore
2.0mm<L≤8.0mm 0.2mm<W≤0.5mm 2
8.0mm<L 0.5mm < W 0
Air bubble Air bubble
D 1 , D 2 ≤ 0.25 mm Ignore
0.25 mm < D 1 ,D2 ≤ 0.4mm 4 Ignore
0.4mm < D1, D2 0
Side Fragment
X≤6mm，Y≤1mm & display is ok, Ignore
Remarks: Spot define: That only can be seen under WS or DS defects.
Any defect which is visible under gray pattern or transition process but invisible
under black and white is disregarded.
Here is definition of the “Spot” and “Scratch or line defect”.
Spot: W > 1/4L Scratch or line defect: W ≤1/4L
Definition for L/W and D (major axis)
FPC bonding area pad doesn’t allowed visual inspection.
Note: AQL = 0.4


#### 8. Packing

```
2
```
```
empty tray
```
```
2
```
```
4
```
```
3
```
```
total 12 layer
```
```
2
```
```
4
```
```
3 Foam
Box
```
```
Label
```
```
Tape
```
```
45mm 38mm
```
```
16.5mm
```
```
2nd layer
```
```
1
```
```
vacuum
bag
```
```
6
```
```
4(PCS)×12(Layer)=48PCS
```
```
1 2
```
(^43)
Pallet
Protector
48(PCS)×16(BOX)=768PCS
9000mm
7800mm
7550mm
1150mm
PP belt


### 9. Precautions

```
(1) Do not apply pressure to the EPD panel in order to prevent damaging it.
(2) Do not connect or disconnect the interface connector while the EPD panel is in operation.
(3) Do not touch IC bonding area. It may scratch TFT lead or damage IC function.
(4) Please be mindful of moisture to avoid its penetration into the EPD panel, which may
cause damage during operation.
(5) If the EPD Panel / Module is not refreshed every 24 hours, a phenomena known as
“Ghosting” or “Image Sticking” may occur. It is recommended to refreshed the ESL /
EPD Tag every 24 hours in use case. It is recommended that customer ships or stores the
ESL / EPD Tag with a completely white image to avoid this issue
(6) High temperature, high humidity, sunlight or fluorescent light may degrade the EPD
panel’s performance. Please do not expose the unprotected EPD panel to high
temperature, high humidity, sunlight, or fluorescent for long periods of time.
```

