# FBB-Racing-Wheel
Custom Racing Wheel with Force Feedback containing the wheelbase and steering wheel with the attachments to connect the 2 together. A ODESC V4.2 will be used to control the hoverboard motor and read it's position throw a rotary encoder connected to the motor with 2 gears. A Pico 2 is used in the wheelbase to receive data from the wheel(which also has a pico 2) throw SPI protocol. The pico 2 isn't used only for receiving data from the steering wheel, it could also get data from other devices like pedals, shifter, handrbrake and many more. The pico 2 supports up to 6 SPI/I2C connections and also sensors can directly be connected to it's GPIO pins.

The big difference between my racing wheel and others is that mine has direct data transfer throw the magnetic pins from steering wheel to wheelbase which goes in the pc. That means very small latency and no cable twisting all thanks to the magnetic pins (pogo pins) and the slipring. The downside that comes with this is the plastic parts for the high strain parts like the motor extension and the quick release all from plastic(3d printed) but it still should work fine.

I'm making the project so I could inspire others with my design as it's different than most I've seen. And also it's way cheaper than buying a new one with similar specs.

**Specs:**
 - up to 15nm of torque
 - fast response time of 1ms(1000hz)
 - wheel base is  L=20cm, W=20cm(at the corner, max dist) H=17cm.
 - steering wheel is 33cm
 - the 3d printed case for the steering wheel L=16.2cm, W=3.2cm, H=7.5.
 - 12 buttons
 - 5 encoders
 - 1 funcky switch(for easier customization of the car in games/other actions like camera fix position)
 - high precision encoder(for motor)

Wheel final version below
<img width="2700" height="2160" alt="Rendered_model" src="https://github.com/user-attachments/assets/4ad0c0ba-7ba9-4ac2-b20a-4ed86052c3d6" />

Wheel base
<img width="1037" height="703" alt="Screenshot_970" src="https://github.com/user-attachments/assets/251fb067-d079-46ef-ac8b-58bd3bfb4026" />

Close image from the steering wheel face
<img width="1119" height="759" alt="Screenshot_2" src="https://github.com/user-attachments/assets/293f904f-e37f-4b4d-a492-fae1b916f17b" />

The back of the steering wheel 
<img width="1143" height="755" alt="Screenshot_3" src="https://github.com/user-attachments/assets/cf56ca17-b959-4753-bbb2-f91b9f69da4d" />

Simple wiring scheme for the Pico 2 of the wheel base to the wires of the slip ring. The image also includes a simple connections from the pico 2 to some sensors if you want to directly connect any sensor to it.
<img width="882" height="882" alt="image" src="https://github.com/user-attachments/assets/f8afc24e-18cd-4b4b-a6d5-c3ef8f1d3372" />

Steering wheel buttons/encoders will be connected to the pico 2 throw a matrix system in which each button is connected to column and row and the pico 2 will send a signal to each column and then wait for output from any of the rows and that's how it will find which buttons got pressed. This matrix system allows the usage of a high number of buttons without haveing a pin for each button. Also a diode will be used to prevent the current from flowing the opposite direction and causing ghosting. The order in which the buttons are connected doesn't matter so I won't include a photo of that. Also I won't add the file for schematic as it's a very simple schematic which isn't linked to other files and it's used to have and idea how the connections work for who doesn't already know(the wires can be connected to others pins and still work fine).
