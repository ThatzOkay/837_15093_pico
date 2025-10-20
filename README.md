# Duel 15093 LED Board emulator.

Credits to [SuperMonkeyLeds](https://github.com/akechi-haruka/SuperMonkeyLEDs) for the jvs parsing and led commands stuff.
And to [Chu Pico](https://github.com/whowechina/chu_pico) for the base main.cpp inspiration, usb hid stuff and cli / save logic / base config
Tested to work in Chunithm, ONGEKI and APM3.
In config.h you can setup a per baord offset for example if you just want the air leds for Chunithm.
Default led pins is 2 for strip 1 and pin 3 for strip 2.
So for example left air pin 2 and right air pin 3.
It exposes itself as 2 serial interfaces. 

Demo: <br/>
<img src="https://github.com/ThatzOkay/837_15093_pico/raw/main/Demo.gif" height="100px">

Default LED pins are pico pin 15 / 16.

## For chuni airs:
Pin 16 Chuni Board 0 Left COM20 <br/>
Pin 15 Chuni Board 1 right COM21 <br/>
Please make sure LED board 0 is COM20 and LED board 1 is COM21. <br/>
This needs to be so the offset lines up corectly. <br/> The led count for chuni air is also set to 3 per side. <br/>
You can check which com port is which using any web serial console. Like [spacehuhn](https://terminal.spacehuhn.com/)
It should look like this <br/>
<img src="https://github.com/ThatzOkay/837_15093_pico/raw/main/Example.png" height="100px"> <br/>
Where LED board 0 is COM20 and LED board 1 is COM21.
