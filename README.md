# Introduction
I found it cumbersome when working with the ESP32 and PxMatrix to constantly have to build and flash the embedded system in order to test the pattern I was currently working on.

With this project you are able to emulate your pattern on a windows machine.

Installation should be straight forward for those who have prior knowledge with C/C++ within VSCODE. But I have included a small guide below for those who are new to programming and compiling.

# BACKGROUND
This uses SDL2 Platform, likely overkill. I wanted to improve my ability with using C++ and didnt want to uses Visual Studio as this would of been too easy.

Surprisingly the FPS of this is terrible! I will be working out why. The ESP32 has a better framerate which is rather concerning...

I've posted this online as some of you might find this interesting.

# Setup
This code is compiled with the GCC compiler which can be done with the command promt. As Vscode is essentially a 'suave' text editor a handy vscode extension (https://code.visualstudio.com/docs/languages/cpp) allows for an easy way for vscode to execute the 'build string from within'. The extension also allows for debugging which uses GDB.

The .vscode folder contains four .json files which contain the extension properties (c_cpp_properties) and general settings(settings.json). We will get on to(tasks.json) and (launch.json) shortly. 

I haven't put anything in the settings json and this is automatically generated. The CPP properties contains information for the intellisense as well as the location of the compiler.

I have used MSYS to download the compiler and debugger. Once you have installed the MSYS. The SDL2 Libraries work only with 32bit. So run the folloing commands in MSYS terminal to install these.

 pacman -S mingw-w64-i686-gcc
 pacman -S mingw-w64-i686-gdb

These will install GCC and GDB within the C:\msys64\mingw32\bin folder.

Pressing CTRL+SHIFT+B whilst looking at main.cpp within VSCODE will carry out the Build task which can be seen in the tasks.json file. This just pushes the build string to the console.

Pressing F5 (Debug) will first carry out the build task above and then connect VSCODE to the Debugger where you can step through the program.

The project will probably work with other compilers but I found it had to be 32bit (Probably the SDL2 library I have used for this project).

I hope you found this useful.


#Demo
![Alt text](Demo.png?raw=true "Title")