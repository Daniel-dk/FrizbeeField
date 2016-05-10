# FrizbeeField
LED controller for LED frizbee field at Afrikaburn.
The field will automatically cycle through animations every 25 minutes, the animation can also be set by pressing the mode button.
pressing "Left goal" or "Right goal" plays an animation on that side of the field

should be run on an Arduino Mega or similar with enough RAM to adress all the LEDs
# Install these additional Libraries:
* Fastled : https://github.com/FastLED/FastLED 
* OneButton : https://github.com/mathertel/OneButton

* SHORTLED should contain the amount of LEDs in the "goal lines" / 20m strips
* LONGLED should contain half the amount of LEDs that cover the length of the field

#Pinout
this can be changed at the top of the .ino file
* Left border strip : A0
* Left goal line : A1
* Right border strip : A2
* Right Goal line : A3
* Left Goal button : 3
* Right Goal button : 4
* field mode button : 5
