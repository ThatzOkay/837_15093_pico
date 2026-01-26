# Duel 15093 LED Board emulator.

Credits to [SuperMonkeyLeds](https://github.com/akechi-haruka/SuperMonkeyLEDs) for the jvs parsing and led commands stuff.
And to [Whowechina](https://github.com/whowechina) for the base main.cpp inspiration, usb hid stuff and cli / save logic / base config
Tested to work in Chunithm, ONGEKI and APM3.
In config.h you can setup a per baord offset for example if you just want the air leds for Chunithm.
Default led pins is 2 for strip 1 and pin 3 for strip 2.
So for example left air pin 2 and right air pin 3.
It exposes itself as 2 serial interfaces. 

Demo: <br/>
<img src="https://github.com/ThatzOkay/837_15093_pico/raw/main/Demo.gif" height="100px">

Default LED pins are pico pin 15 / 16.

## CLI
Credits to [Whowechina](https://github.com/whowechina) for all the CLI code. <br>
I changed some stuff to make parsing arguments between quotes work. And better matching logic. <br>
You can change every important setting through the cli. Like per led strip pin, count, offset, brightness and format. <br>
Or to enable or disable UART and which pins to use <br>
Or change the board number, chip number and firmware sum.
If you connect to the CLI over serial using a serial monitor you get help with all the available commands. Or you can input "?" or "help"<br>
If you change something please wait a couple settings for it to save. Then you can use the reboot command to reboot the pico to take the new changes into effect.<br>
This is needed when you changed uart or led settings. This does not automatically change on the fly.
The CLI also contains some presets for common games. <br>
You can use the "preset" command to see all available presets. <br>

## For chuni airs:
Pre New:<br/>
Pin 16 Chuni Bpard 0 Left COM10 <br />
Pin 15 Chuni Board 1 right COM11 <br/>
SP MODE:<br/> 
Pin 16 Chuni Board 0 Left COM2 <br/>
Pin 15 Chuni Board 1 right COM3 <br/>
CVT MODE:<br/> 
Pin 16 Chuni Board 0 Left COM20 <br/>
Pin 15 Chuni Board 1 right COM21 <br/>
Please make sure LED board 0 is COM20 and LED board 1 is COM21. <br/>
This needs to be so the offset lines up corectly. <br/> The led count for chuni air is also set to 3 per side. <br/>
You can check which com port is which using any web serial console. Like [spacehuhn](https://terminal.spacehuhn.com/)
It should look like this <br/>
<img src="https://github.com/ThatzOkay/837_15093_pico/raw/main/Example.png" height="100px"> <br/>
Where LED board 0 is COM20 and LED board 1 is COM21.
<br>
This is the best option. But this can now be easily changes / fixed through the cli.

## Serial
Using the cli you can enable serial. This uses pico's uart 0 and 1. You can change to pins and baud rate through the cli.<br>
This is handy for when you want to directly hook up the pico to a serial port on your computer.<br>
While testing I have used a Keystudio RS232 TO  TTL adapter hooked up to an Ugreen USB to RS232 cable.<br>
This has been working without issues


