# Device Case Assembly Instructions

Follow these steps to assemble the case for your device, which includes a top lid, bottom lid, and an OLED screen.

## Required Components
- Assembed & programmed ephys test board
- [Milled & andoized aluminum base](./case/stl/base.STL)
- [Milled, yapped, andoized, and silkscreened aluminum top](./case/stl/top.STL)
- OLED screen: [Adafruit 5228](https://www.adafruit.com/product/5228)
- 3x 5mm M3 x 0.5 pan head screws  (e.g [McMaster-Carr 92000A114](https://www.mcmaster.com/92000A114/))
- 4x 15mm M3 x 0.5 flat head screws (e.g. [McMaster-Carr 92125A103](https://www.mcmaster.com/92125A103/))
- Battery: [Adafruit 258](https://www.adafruit.com/product/258)
- Rotary encoder knob: [Vishay ACCKMR1316NLT14](https://www.digikey.com/short/5rpqt2rb)
- Light pipe: [Bivar PLPC3-125](https://www.digikey.com/en/products/detail/bivar-inc/PLPC3-125/5721873)
- 2mm Hex driver
- Light duty foam mounting tape (e.g.  [McMaster-Carr 75785T13](https://www.mcmaster.com/75785T13/))
- Heavy Duty Tape: e.g. [McMaster-Carr 1640N11](https://www.mcmaster.com/1640N11/)

## Assembly Steps

1. **Prepare the components**
    - Ensure you have all the required components listed above.
    - Place them on a clean, flat surface.

2. **Attach the OLED screen**
    - Plot the top face down.
    - Align the OLED screen with the designated slot or mounting area on the top.
    - Secure the screen in place using heavy duty tape.

![OLED installation](./resources/oled-install.jpg)

3. **Insert the PCB**
    - Insert the PCB in the top.
    - Insert OLED screen's flex cable into the ZIF receptacle and close the latch.
    - Plug in the battery.

![PCB installation](./resources/pcb-install.jpg)

4. **Fasten the PCB**
    - Fasten the PCB to the top using 3x 5mm M3 screws through
      the inner plated holes on the PCB.

5. **Mount the battery**
    - Place double sided foam tape within the silkscreen box on the back of the PCB
    - Plug in the battery.
    - Press the battery into position on the double sided tape

6. **Fasten the base**
    - Align the base over the top of the battery so that the battery is contained within its compartment.
    - Fasten the  base to to the top using 4x 15mm M3 screws.

![Base installation](./resources/base-install.jpg)

7. **Install the knob and light pipe**
    - Flip the device so its face up.
    - Install the rotary encoder knob using the its included set screw.
    - Press-fit the light pipe into position.

![Complete device](./resources/complete.jpg)

8. **Ensure functionality**
    - Plug in the USB-C connection.
    - Green charging LED should illuminate
    - Flip the power switch to ensure the OLED works.
