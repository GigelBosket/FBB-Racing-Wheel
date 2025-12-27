# Total Time Spent so far: 79 Hours

## Date: 12/7/2025
## Time Spent: 25 Hours
(note: at the moment I had very little experience with the cad and everything took a lot as I encountered problems, and the documentation was very little as I only later read the requiremets to document around every 5 hours)
### Designed motor attachments 
**I designed the hoverboard motor which I will use in my wheelbase as well as some attachments for it.**

**The attachments consists of:** 
	**1**. a part which will be screwed in the motor and secures a good connection between the motor and the slip ring support.
	**2**. a slip ring support which also has the mounting holes for the wheel it self. This slip ring support will fit a slip ring which will prevent the wires from twisting and also a magnetic pogo pin female for a quick release.
	After watching others DIY wheels I saw that every one was simple with no wheel buttons or a usb on the wheel, which made experience worse in my opinion. So I tried to resolve those problems on my own. So I did some research, I watched at some great wheels from moza and trustmaster and I saw they use some pins and pads to make the connections but when I thought I got it than I realised that the wires will twist if I don't do anything but just connect them straight, and so I had an idea to make a circle from a conductive material and than have a still wire which will make the connections but that of course will fail cuz of the friction and sometimes bad contact but then I found out there is a thing named slip ring which does exactly what I wanted and it's reliable for long term use.
	Right now I m still thinking if I should also send the signal from the wheel to wheel base throw the wires or send it wireless and use the wires just for power alimentation. But I added a 6 wires slip ring with max 2Amp just in case if I will want to send the signal throw wires.
	<img width="644" height="886" alt="image" src="https://github.com/user-attachments/assets/026cdb2a-4cc9-4555-8cbf-6c5871118f82" />

## Date: 12/18/2025
## Time Spent: 18 Hours
### Designed the motor cover case + most of the parts
I designed the motor base case with hexagonal front-back plates and some panels for each margin of the hexagon. I got the idea from the Thrustmaster t818.<br/><br/>



So I have some sk16 metal supports which will hold the metal axis of the hoverboard motor, so the motor can spin freely with the front support of the wheel.

A rotary encoder will be used to get the exact position of the motor using 2 gears, one glued to the motor cover and one on the shaft of the encoder.

Everything will be attached to some aluminium profile of 20mm and probably a few parts of 15mm profile

Also there is a vertical support made of plastic for the pcb.<br/><br/>


Previously I wanted to make a custom pcb but that will take a lot to design as my experience making pcb is very low. So I decided to use a bldc motor driver(probably a ODESC V4.2 but I m still researching) but I don't know how I m gonna support pedals with that and also connecting to pc might be a small problem. This took less time than the previous post as I learned to use a cad while working on this project.
<img width="1229" height="695" alt="image" src="https://github.com/user-attachments/assets/8297263a-111b-4dc3-962e-01d71afd749f" />

## Date:12/21/2025
## Time Spend: 8 Hours
### Chose which boards to use + added them in the 3d model
So after some researched I found a great board for controlling the motor which has an stm32 and seems pretty powerful. The board is ODESC V4.2 24v which is based on an open source project and can be found for a bit under $30.

The power supply will be connected to the ODESC which will run on 24v. The board will output power to the brushless motor throw 3 wires. Also the board will receive input throw 2 wires as A and B from the rotary encoder to read the motor movement so it can transfer the movement in game.

And because I want to have buttons and maybe in the future even a screen on the wheel, I have to send that to the computer somehow. The transfer of information from the wheel to pc is handled by a Pico 2 which receive the data from the wheel throw SPI protocol and the data from the pedals directly throw analog. After the Pico 2 get's the data it will send it to the pc using TinyUSB library. So the Pico will get detected by windows as and HID(Human Interface Devices) which will act as a controller.

I made supports for the ODESC V4.2 and Pico 2 which will be screwed into the vertical support. Because the usb ports are not where they should be, in the back, I will probably use some usb extensions to keep everything clean.
<img width="1307" height="733" alt="image" src="https://github.com/user-attachments/assets/19e09c37-7a9a-4ad0-b3a9-545a2ecfe062" />
<img width="382" height="713" alt="image" src="https://github.com/user-attachments/assets/fd7c85ed-aa2f-4c3f-a08b-276011e464c9" />
<img width="767" height="521" alt="image" src="https://github.com/user-attachments/assets/9cc4aeda-6de6-48fc-9166-4e1d5a4e035c" />
<img width="965" height="538" alt="image" src="https://github.com/user-attachments/assets/7b605948-4d9d-4650-8228-295e5a404d63" />

## Date: 12/22/2025
## Time Spent: 6 Hours
### Making the steering wheel
Before starting to work on the steering wheel I thought it would be pretty easy but now that I m working on it I changed my opinion. I had so many problems with the shape as a well isn't completely round and smooth, it also has some shapes that help you get more grip, and those shapes are harder to make for somebody will limited experience like me.

Firstly I tried to make a 2d sketch with everything the wheel has, extrude and then just fillet but that didn't work.
<img width="740" height="571" alt="image" src="https://github.com/user-attachments/assets/c78e4f2c-37e7-4d2b-879b-1e8bfb7edc52" />

So then I tried with loft extrude which was also bugging because of a shape.
<img width="800" height="600" alt="image" src="https://github.com/user-attachments/assets/532431fa-6c20-477d-8489-3d02e2662169" />

That's the current wheel->
<img width="857" height="543" alt="image" src="https://github.com/user-attachments/assets/5171c617-d90e-4cb5-a793-87d5d5997c08" />
I will stop modeling useless things that doesn't help me or anyone. The wheel will be bought from aliexpress but I wanted to model the whole wheel as I need to make the box with the buttons and the raspberry/arduino which willbe atached with screws into the wheel.


## Date:12/23/2025
## Time Spent: 4.5 Hours
### Making the button layout
So far I made the shape of the button box in sketch and I almost finished the button layout.

I will mainly use 10mm buttons ,a few 15mm encoders, 2 5mm buttons for menu and back and in the final an encoder which also can tilt in 4 directions, rotate(left/right) and also push(I saw them on some more expensive wheels and I liked them so I added one to my wheel)
<img width="837" height="665" alt="image" src="https://github.com/user-attachments/assets/6e4f443c-b55f-4683-9058-81a23bc7c45d" />

## Date: 12/24/2025
## Time Spent: 3.5 Hours
### Added the buttons to the wheel
I added a few buttons of 10mm and 5mm, rotary encoders with knobs and a encoder that moves in 4 directions as well of rotating (left right) which can be pressed like a button.(The buttons rotary encoders, and the 4d encoder 3d models I found online, the only thing I had to model was the buttons caps)

I added some textures so I could see better how the wheel will look in the reality with this shape and buttons.
<img width="1350" height="1080" alt="image" src="https://github.com/user-attachments/assets/b2dda368-a440-4b8e-b378-7c7e6976e345" />
<img width="1087" height="355" alt="image" src="https://github.com/user-attachments/assets/f64dee44-b3cf-43ef-86c6-6b0096d778ef" />

## Date: 12/25/2025
## Time Spent: 7 Hours
### Made the wheel attachment and the the side part
So I made the part that will cover the sides with all the screw holders it needs in order to connect with the front plate which has the buttons.

Also made the wheel attachment and back plate. The wheel attachment will slide and lock with the part from the wheel base where the quick release is. I still have to model the quick release for the wheel attachment.

To hold the big strain I will use m6 hexagonal head screw of 60mm length in order to hold the wheel attachment and the wheel together. And m3 screws of 12mm to hold the plates together.
<img width="976" height="716" alt="image" src="https://github.com/user-attachments/assets/e3dde789-0c76-46e1-bf11-779102dfbc5b" />
<img width="755" height="486" alt="image" src="https://github.com/user-attachments/assets/b4065fc7-5986-44df-af59-8f5f38a26ac5" />
<img width="1019" height="692" alt="image" src="https://github.com/user-attachments/assets/aa03c2da-8330-467e-93da-caab00758c3d" />
<img width="769" height="580" alt="image" src="https://github.com/user-attachments/assets/eb62dc34-3637-43c7-9bfe-7ba2c7894a9b" />
<img width="566" height="416" alt="image" src="https://github.com/user-attachments/assets/b8a8bf69-4e09-4d47-a165-1061e68ce9a4" />

## Date: 12/25/2025
## Time Spent: 0.5 Hours
###Nuts and screws overlap with buttons
Just a quick update: 3 nuts and 1 screw overlaps with 3 buttons. It's not hard no repair but I just wanted to let you know about that, that's why I modeled more than I should and it took way longer than it should.
<img width="1247" height="707" alt="image" src="https://github.com/user-attachments/assets/d7039ef9-3d6d-4067-95a8-233e114e076b" />

UPDATE 2: I fixed them by moving each by a few milimeters.
<img width="1119" height="645" alt="image" src="https://github.com/user-attachments/assets/641adf90-1b66-40a7-8ce8-c520339d0db4" />

## Date: 12/26/2025
## Time Spent: 2.5 Hours
###Made Bill Of Materials
The Bill Of Materials took longer than expected but it's finally done. It took longer because some parts were unavailable/out of stock or their price has increased on the sites I looked initially.

Also the overall price is higher than expected because the currency exchange rate has changed quite a bit.

So I have to change the tier of the project to tier 1 from tier 2
<img width="1797" height="577" alt="image" src="https://github.com/user-attachments/assets/f1b5f822-c8c0-4c72-b5b5-283c2237d8ca" />

## Date: 12/27/2025
## Time Spent: 4 Hours
###Added cad files and BOM to github
I exported the assembly file with the whole project together with step file(which took a long time to export) and the STL file for each part that has to be 3d printed.

Updated the BOM as I forgot a few things including power supply and some wires.
<img width="524" height="787" alt="image" src="https://github.com/user-attachments/assets/b1593c2b-c2da-4859-b465-f3bda911841c" />
<img width="1157" height="990" alt="image" src="https://github.com/user-attachments/assets/cfa84ddf-a0ed-403f-b30d-2bdda7971081" />

### Note: I saw I have to write how much I spend each day but I can't remember. Until 20 december I had school and each day from monday to friday I worked around 2 hours(to this project) per day and in the weekends some times even 10-12 hours but I didn't include that because I was working pretty slow and inefficient.

