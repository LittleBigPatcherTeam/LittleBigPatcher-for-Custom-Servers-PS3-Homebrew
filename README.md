# LittleBigPatcher for Custom Servers (PS3 Homebrew)
![ICON0.PNG](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/ICON0.PNG?raw=true)<br/>
A simple way to patch your LittleBigPlanet games to connect to custom servers with only your PS3!

# Building
Youd want to use linux, or wsl on windows in order to build this, basically you need to be able to build the [Tiny3D samples](https://github.com/wargio/tiny3D/tree/master/samples/sprites2D) in order to build this.<br/>
also you'll need to install [dbglogger](https://github.com/bucanero/dbglogger)<br/>
then you can run the python script in python3 `python3 build.py` in order to build!

# Credits (If i missed anyone please let me know)
## [oscetool](https://github.com/spacemanspiff/oscetool)
### For decryption and rencryption of eboot.bin files, making this possible!
## [Apollo Save Tool PS3](https://github.com/bucanero/apollo-ps3)
### For helper functions such as the on screen keyboard input, getting idps, and for the idea of using analogue stick for controls as well as dpad
## [Tiny3d samples](https://github.com/wargio/tiny3D.git)
### the building blocks of the GUI, allowing to easily put text on the screen and navigate the menus
