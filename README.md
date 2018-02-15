# Remote Control Wireless Energizer Puck Lights

There are a new breed of low power LED puck lights available which make use of 433 Mhz radios 
for remote control. However, they currently do not integrate with any sort of Micro allowing 
for more accesible remote control.

This project connects to the provided remote and adds the ability to put a motion detector 
into the system which will turn the lights on or off based on movement and available ambient 
light. The idea is to use the puck lights remotely without having to run power for stair lights 
while achieving a viable benefit.

## What these are
https://www.lowes.com/pd/Energizer-Remote-Controlled-2-Pack-3-in-Battery-Puck-Light/1000268307

## The Plan
Basically, these are battery operated with a 433Mhz receiver, and a 12V remote that has two 
buttons, on and off. By powering the remote with a 12V source, and then regulating the power 
down to 5V for the micro, you can simply run two leads from NPN transistors to the two buttons 
to act as remote triggers. Add in a IR motion detector and a photocell, and you can remotely
trigger the lights on activity. Add in a micro that can easily receive events from the internet
and you can even turn the lights on or off from anywhere.
