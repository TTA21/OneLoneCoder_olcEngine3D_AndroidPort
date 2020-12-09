# OneLoneCoder_olcEngine3D_AndroidPort

This is a very lazy working port of OneLoneCoder_olcEngine3D to android using NDK to compile.
It does not use "olcConsoleGameEngine.h" , it uses SDL instea to draw to the screen.
It has a working file reader to import .obj files using SDL's file reader library as <fstream> is incopatible with android file system.
It also uses SDL's finger position reader functions to move the camera around, though it is not very good.
This code is updated to the "Part3" of the series. It can only draw vectors as there is no built in triangle rasterizer, so no textures.

All credits to javidx9. Thank you for teaching me 3D rendering!

Twitter: @javidx9
Blog: http://www.onelonecoder.com
Discord: https://discord.gg/WhwHUMV
