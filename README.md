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
First of all, you want to get the title id of your game, and make sure that the game is updated, for discs, you can just look at the bottom of the spine of the case, or at the bottom of the disc (do not include the `-`)<br/>
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

# Editing stuff via ftp using a phone or pc
if youd prefer to copy text like title ids, or urls from your pc or phone then transfer them over, heres how you can do that (rerember to press TRIANGLE to refresh after making changes and to upload your changes)
## Editing the selected title id
go to `/dev_hdd0/game/LBPCSPPHB/USRDIR` and open up the `title_id_to_patch.txt` file in a text editor, here you can just paste in a title id then save

## Editing the urls
go to `/dev_hdd0/game/LBPCSPPHB/USRDIR` and open up the `saved_urls_x.txt` file in a text editor (replace x with the page number), then for each new line, you can paste in a url, or if there is a digest, put a space after the url and paste in the digest<br/><br/>
note, if the url or digest is too long, it will get truncated, and anything beyond 9 lines will not be shown, if you wish to add more urls then 9, edit a different page `saved_urls_x.txt`, the file would look something like
```
http://url_here_without_a_digest
http://url_here_with_a_digest CustomServerDigest
```

# Technical details (no programming or c knowledge needed to read this)
for those intrested how this patcher works, here it is

## Decryption of EBOOT.BIN files
The app will first try to look for .rap files, in this order<br/><br/>
`/dev_hdd0/exdata` (this is where PKGI stores .rap files)<br/>
`/dev_hdd0/home/xxxxxxxx/exdata/` replacing `xxxxxxxx` with all the folders in home, it will iterate over the dir `/dev_hdd0/home/`<br/>
checks every single usb port (for psnpatch)<br/>
`/dev_usb000/exdata`<br/>
`/dev_usb001/exdata`<br/>
`/dev_usb002/exdata`<br/>
`/dev_usb003/exdata`<br/>
`/dev_usb004/exdata`<br/>
`/dev_usb005/exdata`<br/><br/>
then, if it still is not able to find a .rap file, it will look for the first `.rif` file inside of `/dev_hdd0/home/xxxxxxxx/exdata/` (using same method as above) then it will grab the `act.dat` file from the exdata folder, and will finally grab the idps from your ps3 using syscalls<br/><br/>
if it cant find a .rif ethier, decryption will likely fail, although if youre able to boot the game, it ill most certainly be able to decrypt the eboot

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
