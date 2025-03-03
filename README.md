# LittleBigPatcher for Custom Servers (PS3 Homebrew)
![ICON0.PNG](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/ICON0.PNG?raw=true)<br/>
A simple way to patch your LittleBigPlanet games to connect to custom servers with only your PS3!

![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/main_menu.png?raw=true)<br/>

# Features and FAQ
## Not a pretty GUI, but functional!
The gui will probably not take up your entire screen, this is to ensure it will fit on all screens

## Saving and selecting urls
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/menu_select_urls.png?raw=true)<br/>
You can save up to 9 urls for easy switching between servers, and with 98 possible differnt pages of urls, that adds up to 882 differnt urls to be saved! (use D-Pad left and right to change pages)

## Why are there 2 parts to a url?
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/menu_edit_urls_on_custom_digest.png?raw=true)<br/>
You'll notice when going to the Edit urls menu, when you go down, it selects a space after the actual url, or text after the space. This is the digest, now you do not need to know what a digest is, all you need to know is that if you are adding a Refresh based custom server, for the digest enter in `CustomServerDigest` otherwise leave it empty. Press CROSS to edit a url or digest

## What is a Title id?
First of all, you want to get the title id of your game, and make sure that the game is updated, for discs, you can just look at the bottom spin of the case, or at the bottom of the disc (do not include the `-`)<br/>
or, if you have webMAN MOD installed, you can boot the game, then press PS button to be on xmb, but do not quit the game, then press and hold R2 + CIRCLE then the `ID: NPUA81116` will show up in top right corner

## Patching!
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/patch_a_game.png?raw=true)<br/>
Once you found your title id, and put it in Edit Title id, you can then go onto to patching<br/>
## Normalise digest
Normalise digest means it will replace the digest in this game with the main servers digest. I reccomend leaving this checked (yellow) unless you spefically do not want to edit the digest in the eboot.bin

## Revert patches
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/revert_patches_1.png?raw=true)<br/>
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/revert_patches_2.png?raw=true)<br/>
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/revert_patches_3.png?raw=true)<br/>
If, for whatever reason, you no longer want your game to be patched, and wish to go back to the orignal, select this option (you do not need to do this if you want to patch again after already patching once)

## Actually Patching!
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/about2patch.png?raw=true)<br/>
Now, we are ready to actually patch, go to Patch! (LittleBigPlanet Main Series) (or whatever game it is), and if you entered your title id correctly a menu like this will show
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/press_yes_to_patch.png?raw=true)<br/>
Otherwise, double check you entered in the title id correctly and make sure its updated<br/>
You can now press yes to patch

## Patching process
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/patching_in_proc.png?raw=true)<br/>
You will be waiting on this screen for a while, just pay attention to the solid 1 or 0 thing. This usually takes around a minute to 2 minutes, but if it takes more then 5 minutes, likley something went wrong, and youll have to quit the app, feel free to report any issues or bugs you have on the issues tab
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/patching_done.png?raw=true)<br/>
When you see this message, your game is now patched to the custom server! if you see an error, make sure you selected the correct game, otherwise, feel free to report an issue

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
