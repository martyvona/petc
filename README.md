<div style="width: 50em"> 

# Printer Enclosure Temperature Controller

![cutaway view](./petc-controller-cutaway-small.png)

[(more photos)](./photos) [(schematic)](./petc_schematic.jpg)

This is an enclosure temperature controller for 3D printers.  It's a work in progress.  I was inspired by [Lars' Automated Heating System for Original Enclosure](https://www.printables.com/model/561491-automated-heating-system-for-original-enclosure), but I wanted a different feature set.

There are pros and cons to experimenting with adding active heating or cooling to the enclosure of a 3D printer that was not specifically designed for it.  Of course, there can be dangers including fire hazards and the possibility of melting parts of your printer.  **If you use the information provided here, you do so at your own risk.**  I am only experimenting with this myself, and I make no claims as to the suitability or safety of any of this information.  Until fairly recently, Stratasys held key patents regarding 3D printers with heated build chambers.  It is my understanding that these patents have now effectively [expired](https://3dprintingindustry.com/news/stratasys-heated-build-chamber-for-3d-printer-patent-us6722872b1-set-to-expire-this-week-185012), but I am not a lawyer.

As this project is still a work in progress, I haven't even begun to test it in actual use.  The extent to which the kind of temperature control implemented here may or may not be helpful is still an open question, but I'm interested in experimenting with it.  When I do, I'll be using a Prusa MK4 with an [Original Prusa Enclosure](https://www.prusa3d.com/product/original-prusa-enclosure).  I've tried to make the design mostly agnostic to the specifics of the printer and enclosure.  There are some mounting holes and design choices that make it fit this Prusa setup nicely, but it should be adaptable to other configurations without too much difficulty.  The CAD is done in [OnShape](https://cad.onshape.com/documents/f44140cba6f8b67dad0ae1df/w/266792716668dc913b5493d9/e/d40b23f5562de2835c96fe1f), so it can be copied and modified.

The code, CAD, and documentation of this project are under the [MIT License](./LICENSE.txt).

Some materials, including PLA, prefer relatively cool air temperatures.  While PLA is generally considered easy to print on non-enclosed printers, if your printer is in an enclosure, you may experience issues with PLA due to ambient heating from the printer bed.  In my experience even leaving the enclosure doors open may not always be sufficient.  This controller allows you to add a fan to your enclosure to actively exhaust the hot air and draw in more cool air from the room.

Other materials, including ABS, ASA, Nylon (PA), and polycarbonate (PC), perfer relatively hot air temperatures to reduce warping and improve layer adhesion during printing.  And these materials can be desirable for some applications for their strength and temerature resistance.  This controller allows you to add an active heating element in the enclosure with closed loop temperature control to increase the enclosure temperature more quickly and to maintain hotter temperatures than would typically be reached by the bed heater alone.

How hot is hot?  Well, from what I've seen, many of these materials would be happy with air temperatures in the range of 90C.  Unfortunately, at that level, many of the plastic and electronic parts in most of the current low cost consumer 3D printers and enclosures are likely to become unstable.  For now this controller has a hardcoded limit of 45C, which is the same as [Lars'](https://www.printables.com/model/561491-automated-heating-system-for-original-enclosure).  That's a far cry from 90C, but it's a starting point, and by some accounts even 40-45C can help significantly reduce warping in ABS and ASA prints.  It may be possible to attempt up to around 60C by replacing key plastic parts on the printer with higher temperature materials, and some say that 60C is helpful for printing materials like ABS and PC.  Of course there are a wide variety of printers now available, and some may be more compatible with higher temperatures than others.  For example, apparently Prusa [recommends a maximum enclosure temperature of 40C](https://forum.prusa3d.com/forum/postid/630914) for MK3S+, which ships with PETG parts (some of which are now being switched to PC-CF on MK4S).  PETG has a significantly lower temperature resistance than ABS, ASA, PA, or PC.

You can also use this design as a starting point for various things including
* any project based on an Arduino Uno R3 compatible board with a 16x2 LCD, five pushbuttons, an optional high-current relay, and a clamp for strain relieving connecting wires
* just adding temperature and/or humidity sensing to a printer enclosure (or whatever), with a local display (though that can be easily bought for cheap) and optional USB connectivity for logging or serving remotely (which is probably less easy to buy off the shelf, especially if you don't love adding random cheap things with questionable firmware to your network)
* any project to add a control box to the front of a printer enclosure (or whatever), particularly the Original Prusa Enclosure
* any project to add a Mean Well power supply to the side of a printer enclosure (or whatever), particularly the Original Prusa Enclosure; you can also use the printed insulating cover for the Mean Well on its own to add a switched AC power entry port and strain relief clamp for output wires.

## Features
* printable parts designed in [OnShape](https://cad.onshape.com/documents/f44140cba6f8b67dad0ae1df/w/266792716668dc913b5493d9/e/d40b23f5562de2835c96fe1f)
* no custom PCBs, perfboards, or breadboards, only an off-the-shelf SparkFun [redboard plus](https://www.sparkfun.com/products/18158) (Arduino Uno R3 compatible), a [DFRobot I2C LCD Keypad Shield](https://www.dfrobot.com/product-1363.html), and a SparkFun [Beefcake Relay](https://www.sparkfun.com/products/13815)
* auto shutoff timer
* multiple material profiles (e.g. cooling mode for PLA, heating mode for ABS, etc)
* front panel user interface with LCD and buttons allows setting the profile, the shutoff timer, and also manual adjustments to the operating mode and set temperatures
* multiple hot pluggable daisy chained [DS18B20 1-Wire temperature sensors](https://www.sparkfun.com/products/18367) with 6ft leads
* optional hot pluggable [DHT20 I2C temperature and humidity sensor](https://www.sparkfun.com/products/18364)
* electronics box and power supply located outside the enclosure
* can be built only for sensing, sensing+heating, sensing+cooling, or sensing+heating+cooling
* optional exhaust fan for cooling mode
* optional 200W 24V automotive [PTC heater](https://www.amazon.com/gp/product/B081P7L32X) with fan for heating mode
* optional USB serial interface, can plug into e.g. a Raspberry PI to expose a remote web interface
* uses 24V PWM controlled 4 wire [PC fans](https://www.sameskydevices.com/product/thermal-management/dc-fans/axial-fans/cfm-6025bf-235-274-22) with tachometer feedback
* optional mounting to [Original Prusa Enclosure](https://www.prusa3d.com/product/original-prusa-enclosure)
* all connections pigtailed to keyed connectors of different types to facilitate servicing
* separate power supplies for controller/sensors/display and heater/fans, so if you only want sensing you don't need the 24V high current power supply.

# TODO
* heater mounting hardware
* fan mounting hardware
* document hardware
* test on printer
* add serial monitor
* read fan tachometer feedback
* web server that talks to serial monitor

## Building the Code

If on OS X:
```
brew install arduino-cli
```

One time config to install the Arduino AVR core:
```
arduino-cli config init
arduino-cli core update-index
arduino-cli core list --all
arduino-cli core install arduino:avr
```

Install libraries:
```
arduino-cli lib install DFRobot_RGBLCD1602 DallasTemperature DHT20
```

Attach USB, then run this to show available serial ports:
```
arduino-cli board list
```

Pick the one that corresponds to your hardware, in my case it is `/dev/cu.usbserial-110`.  Note that unfortunately the "Board Name" "FQBN" and "Core" columns might be empty or "Unknown".

To compile without uploading:
```
arduino-cli compile --fqbn arduino:avr:uno -e .
```

To upload without compiling:
```
arduino-cli upload --fqbn arduino:avr:uno -p /dev/cu.usbserial-110 .
```

To compile and then upload:
```
arduino-cli compile --fqbn arduino:avr:uno -e -u -p /dev/cu.usbserial-110 .
```

## Bill of Materials

Prices shown are approximate at time of writing not including tax or shipping.  Total cost is in the range of $250, but could be a lot less depending on which bells and whistles you want and what materials you already have.

I probably will not write up more detailed build instructions than this.  The assembly should be mostly self-evident from the CAD model and the [photos](./photos).  A fair level of experience with electronics is needed. This BOM is also annotated with some tips.

1. SparkFun [RedBoard Plus](https://www.sparkfun.com/products/18158), 22USD.  This is an Arduino Uno R3 compatible board with some extra features that are useful for this project:
    * relatively high current 5V power distribution
    * solderable plated through holes next to all the shield headers
    * optional solder pads for the USB connection.
1. [DFRobot DFR0374 I2C LCD Keypad Shield](https://www.dfrobot.com/product-1363.html) with RGB backlight and black text, 13USD ([mouser](https://www.mouser.com/ProductDetail/426-DFR0374)).  I believe [DFR0936](https://www.dfrobot.com/product-2612.html) (16USD [mouser](https://www.mouser.com/ProductDetail/426-DFR0936)) is also pin compatible but has text that changes with the RGB backlight instead of black.
1. [DS18B20 temperature sensors](https://www.sparkfun.com/products/18367) with included 6ft leads, 12USD.  These are rated for use up to 125C and use a daisy chainable 1-Wire interface.  The firmware auto detects between zero and four of these by default, but could probably be modified for more. (I actually mistakenly used a [slightly different version of these](https://www.sparkfun.com/products/11050) which is marginally cheaper but not specifically rated for high temperature use.)
1. [DHT20 I2C temperature and humidity sensor](https://www.sparkfun.com/products/18364), 7USD.  The firmware auto detects zero or one of these.  The relatively high speed I2C interface probably means that the usable cable length will be shorter for this than the DS18B20 sensors.  Useful if you want to monitor humidity or if you want to just use a single relatively cheap sensor.
1. 350W 24V Mean Well [LRS-350-24](https://www.meanwell.com/productPdf.aspx?i=459#1), 36USD ([mouser](https://mou.sr/3gO5GOw)), for powering the heater and fans.  Switchable 115V/230V AC input for international use.
1. TE Connectivity / Corocom [15CBS1](https://www.te.com/en/product-1-1609112-3.html) (aka 1-1609112-3) IEC 320-C14 AC power entry module with switch, 13USD ([mouser](https://mou.sr/3ChMV0O)).  Similar units might be available for lower cost from other sources, but it might be wise to at least try to get genuine parts from a reputable supplier.
1. Any reasonable C13 style power cord; I used one from my junk box, but this 7USD [Amazon basics 6ft](https://www.amazon.com/Amazon-Basics-Computer-Monitor-Replacement/dp/B072BYGKZZ) one is probably fine if you live in the US.
1. SparkFun [Beefcake Relay](https://www.sparkfun.com/products/13815), 10USD.  This is used to switch the high current 24V supply to the heater.  If you only want cooling, this is optional.
1. Bestol 200W 24V automotive [PTC heater](https://www.amazon.com/gp/product/B081P7L32X) for heating mode, 24USD.
1. Normally closed PTC thermostat, 95C opening temperature, 16A current rating, 8USD [mouser](https://www.mouser.com/ProductDetail/256-24T01B1P1953755).  Optional but recommended if you are using the heater.  This is intended to be a fallback safety measure to prevent the heater assembly from getting too hot.  Similar units might be available for lower cost from other sources, but it might be wise to at least try to get genuine parts from a reputable supplier.
1. Any standad 20mA 5mm LED or similar, plus a 1/2 watt 1k resistor to indicate when the heater is active (optional).
1. Same Sky (formerly CUI) [CFM-6025BF-235-274-22](https://www.sameskydevices.com/product/thermal-management/dc-fans/axial-fans/cfm-6025bf-235-274-22) 24V 4 wire fan 13USD/ea ([mouser](https://mou.sr/3YSNuHb)).  Up to two are supported, one for cooling and one for heating.  This particular fan
    * operates from the same 24V supply as the heater in this project
    * has the appropriate dimensions and bolt hole pattern to replace the two-wire fan that comes with the heater (though I did not verify the specified thermal compatibility of this fan in such a high heat application)
    * has a four wire interface with 5V compatible PWM signals to both control and measure fan speed
    * turns off completely when the control PWM is set to 0; apparently some other similar fans reduce to a slow speed but do not fully turn off
    * can accept the default 0.5kHz Arduino PWM frequency; apparently some other similar fans require much higher frequency PWM, which is possible but requires custom code.
1. 6-32x1-1/2 socket cap screws [10 pack](https://www.amazon.com/gp/product/B00HYM0J1W) for 8USD (M4x40mm might be made to work as well).  Four of these are used to mount the fan to the heater.  The four wire fan is much thicker than the two wire fan that the heater comes with, so its mounting screws cannot be reused.  
1. 1/2 inch split sleeve wire loom, 40USD [for 25ft](https://www.amazon.com/dp/B07HXSSBHB).  Optional but improves the appearance and durability of the the 24V cables.
1. Heat shrink tubing, 12USD for a [180 piece assortment](https://www.amazon.com/dp/B084GDLSCK).  For insulating and stress relieving connections.
1. 26AWG hookup wire for making a few connections inside the control box.
1. PC fan connector male/female extension cables, 9USD for a [four pack](https://www.amazon.com/dp/B0CNLDNZB2).  Cut two of these in half to make pigtails and connectors for the two fans.
1. 10ft good quality USB A cables, 26USD for a [four pack](https://www.amazon.com/dp/B07D6JQFLG).  Use one of these for the USB power/data pigtail from the control box.  Cut others up to make extension cables for the fans and the DHT20 sensor.
1. JST-SM 2.5mm latching connectors, 32USD for a [kit including the crimping tool](https://www.amazon.com/dp/B08GR76H3D).  Use these to make 3 pin connectors for the DS18B20 sensors and a 4 pin connector for the DHT20 sensor.
1. Tamiya connector pairs with 14AWG leads, 9USD for a [pack of three](https://www.amazon.com/FLY-RC-2Pairs-Battery-Connectors/dp/B07VL2B5C8).  Use two pairs to make a removable extension cable from the 24V power supply to the control box, and a third pair for the connection from the control box to the heater.  I recommend following the convention of using the ones with the male pins for power "sources", so the gendering would go `PSU---M | F---cable---M | F---control-box---M | F---cable-to-heater`.  Among other things that way the two connectors on the control box have different genders and so can't be confused.
1. 14AWG silicone wire, 43USD for a [25ft/ea six color assortment](https://www.amazon.com/dp/B089CVGL3H).  Use this to make the high current cables from the power supply to the control box and from the control box to the heater.
1. M3x12 or 4-40x1/2 flat head socket cap screws, 9 required for control box (*).
1. M3x6 or 4-40x1/4 socket cap screws, 8 required for control box.  There is a close clearance on one of the screw heads to a header component on the RedBoard and I found that the 4-40 screw head fit better there than M3.  However, it would also probably be fine to just leave that one screw out.
1. Two M3x8 or M3x10 socket head cap screws for attaching the control box to the front of the Original Prusa Enclosure.
1. Two M3x8 socket cap screws to mount the thermostat to the heater.
1. Two M3x10 and two M3x20 socket head cap screws, four M3 flat washers, and two M3 lockwashers, used on the standoffs below for mounting the heater.  This arrangement attemps to avoid putting 3D printed plastic in very close proximity to the heater, though if a high temperature filament like PC or PA is used (which is probably a very good idea anyway), a simpler mostly printed mount might be fine.  When ordering an assortment like the one above, which is not a bad idea for general use, you can get these all anyway.
1. Two 8-32x3/4 inch or two M4x20 socket head cap screws for the angle adjustments on the heater mount (*).
1. Three M3x8 or M3x10 socket head cap screws for attaching the heater mount to the Original Prusa Enclosure.
1. M3x25mm standoffs, 8USD for a [10 pack](https://www.amazon.com/dp/B0B97DX2RK).  Two of these are used to mount the heater.
1. TODO hardware for the exhaust fan.

(*) The tapped holes in the 3D printed parts are all sized for M3/4-40 or M4/8-32 tap diameter.  M3 tap diameter is about 0.010 inch (about 0.25mm) larger than the theoretical 4-40 tap diameter, however, the actual tolerance of the part will likely vary more than that, and perfect thread engagement is not required.  Similarly, 8-32 tap diameter about 0.007 inch (about 0.2mm) larger than M4.  You will likely want need a 4-40 or M3 and 8-32 or M4 tap lubricated with a little water to form the threads.  Consider printing these parts in PETG, ABS, or ASA instead of PLA because PLA tends to melt and snag when tapped.

