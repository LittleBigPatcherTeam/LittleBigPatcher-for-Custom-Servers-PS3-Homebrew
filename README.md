# LittleBigPatcher for Custom Servers (PS3 Homebrew)
![ICON0.PNG](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/ICON0.PNG?raw=true)<br/>
A simple way to patch your LittleBigPlanet games to connect to custom servers with only your PS3!

![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/main_menu.png?raw=true)<br/>

# Features
## Not a pretty GUI, but functional!
The gui will probably not take up your entire screen, this is to ensure it will fit on all screens

## Saving and selecting urls
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/menu_select_urls.png?raw=true)<br/>
You can save up to 9 urls for easy switching between servers, and with 98 possible differnt pages of urls, that adds up to 882 differnt urls to be saved! (use D-Pad left and right to change pages)

## Why are there 2 parts to a url?
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/menu_edit_urls_on_custom_digest.png?raw=true)<br/>
You'll notice when going to the Edit urls menu, when you go down, it selects a space after the actual url, or text after the space. This is the digest, now you do not need to know what a digest is, all you need to know is that if you are adding a Refresh based custom server, for the digest enter in `CustomServerDigest` otherwise leave it empty

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
