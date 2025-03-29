---
title: LittleBigPatcher for Custom Servers (PS3 Homebrew)
---
<div id='section-id-1'/>

# LittleBigPatcher for Custom Servers (PS3 Homebrew)
<div id='section-id-2'/>

## Icon by royalpaks
![ICON0.PNG](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/ICON0.PNG?raw=true)<br/>
A simple way to patch your LittleBigPlanet games to connect to custom servers with only your PS3!

# source code
[https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew)
# Latest release (.pkg download)
[http://bit.ly/patchps3](http://bit.ly/patchps3)

# Video guide
[https://www.youtube.com/watch?v=CtDRYwz7iUw](https://www.youtube.com/watch?v=CtDRYwz7iUw)

# Table of contents
- [Keyboard mapping](#keybaord-mapping-id)
- [Features and FAQ](#section-id-8)
  - [works on both PS3 and RPCS3](#section-id-9)
  - [Not a pretty GUI, but functional!](#section-id-10)
  - [Saving and selecting urls](#section-id-13)
  - [Why are there 2 parts to a url?](#section-id-17)
  - [What is a Title id?](#section-id-21)
    - [RPCS3](#section-id-22)
    - [Browse games for Title id](#section-id-24)
    - [PS3](#section-id-30)
  - [Patching!](#section-id-34)
  - [Normalise digest](#section-id-37)
  - [Revert patches](#section-id-40)
  - [Actually Patching!](#section-id-46)
  - [Patching process](#section-id-53)
- [Editing stuff via ftp using a phone or pc](#section-id-59)
  - [Editing the selected title id](#section-id-61)
  - [Editing the urls](#section-id-64)
- [Technical details (no programming or c knowledge needed to read this)](#section-id-72)
  - [What font is used?](#section-id-75)
  - [Decryption of EBOOT.BIN files](#section-id-78)
  - [oscetool vs scetool](#section-id-92)
  - [Decryption oscetool command](#section-id-95)
  - [Encryption oscetool command (disc)](#section-id-101)
  - [Encryption oscetool command (digital)](#section-id-107)
  - [Patching method for main series](#section-id-113)
  - [Patching method for LittleBigPlanet Karting](#section-id-131)
- [Adding your own patch method, or editing the exsiting ones (only using lua, no c or compiler needed)](#using-the-lua-api)
- [Building](#section-id-134)
- [Credits (If i missed anyone please let me know)](#section-id-139)

![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/main_menu.png?raw=true)<br/>

<div id='keybaord-mapping-id'/>

# Keyboard mapping
The keys mapped are:<br><br>
WASD and ARROW keys to dpad<br>
ENTER and SPACE key to CROSS<br>
ESC and BACKSPACE key to CIRCLE<br>
R and F5 key to TRIANGLE<br>

<div id='section-id-8'/>

# Features and FAQ
<div id='section-id-9'/>

## works on both PS3 and RPCS3
<div id='section-id-10'/>

## Not a pretty GUI, but functional!
The gui will probably not take up your entire screen, this is to ensure it will fit on all screens

<div id='section-id-13'/>

## Saving and selecting urls
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/menu_select_urls.png?raw=true)<br/>
You can save up to 9 urls for easy switching between servers, and with 98 possible differnt pages of urls, that adds up to 882 differnt urls to be saved! (use D-Pad left and right to change pages)

<div id='section-id-17'/>

## Why are there 2 parts to a url?
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/menu_edit_urls_on_custom_digest.png?raw=true)<br/>
You'll notice when going to the Edit urls menu, when you go down, it selects a space after the actual url, or text after the space. This is the digest, now you do not need to know what a digest is, all you need to know is that if you are adding a Refresh based custom server, for the digest enter in `CustomServerDigest` otherwise leave it empty. Press CROSS to edit a url or digest

<div id='section-id-21'/>

## What is a Title id?
<div id='section-id-22'/>

### RPCS3
If you are on RPCS3, it will be the Serial next to your game<br/>![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/rpcs3_title_id_where.png?raw=true)
<div id='section-id-24'/>

### Browse games for Title id
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/about2clickbrowsegames.png?raw=true)
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/browsing_my_games.png?raw=true)<br/>
If you'd prefer, select the Browse games for Title id option on the Patch a game menu, there you can browse your games that the app can detect, then select a title id by pressing CROSS<br/><br/>
Yes, all the games will show, just keeping going down and the rest will load
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/browsing_my_games_bottom_of_list.png?raw=true)<br/>
<div id='section-id-30'/>

### PS3
First of all, you want to get the title id of your game, and make sure that the game is updated, for discs, you can just look at the bottom of the spine of the case, or at the bottom of the disc (do not include the `-`)<br/>
or, if you have webMAN MOD installed, you can boot the game, then press PS button to be on xmb, but do not quit the game, then press and hold R2 + CIRCLE then the `ID: NPUA81116` will show up in top right corner

<div id='section-id-34'/>

## Patching!
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/patch_a_game.png?raw=true)<br/>
Once you found your title id, and put it in Edit Title id, you can then go onto to patching<br/>
<div id='section-id-37'/>

## Normalise digest
Normalise digest means it will replace the digest in this game with the main servers digest. I reccomend leaving this checked (yellow) unless you spefically do not want to edit the digest in the eboot.bin

<div id='section-id-40'/>

## Revert patches
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/revert_patches_1.png?raw=true)<br/>
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/revert_patches_2.png?raw=true)<br/>
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/revert_patches_3.png?raw=true)<br/>
If, for whatever reason, you no longer want your game to be patched, and wish to go back to the orignal, select this option (you do not need to do this if you want to patch again after already patching once)

<div id='section-id-46'/>

## Actually Patching!
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/about2patch.png?raw=true)<br/>
Now, we are ready to actually patch, go to Patch! (LittleBigPlanet Main Series) (or whatever game it is), and if you entered your title id correctly a menu like this will show
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/press_yes_to_patch.png?raw=true)<br/>
Otherwise, double check you entered in the title id correctly and make sure its updated<br/>
You can now press yes to patch

<div id='section-id-53'/>

## Patching process
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/patching_in_proc.png?raw=true)<br/>
You will be waiting on this screen for a while, just pay attention to the solid 1 or 0 thing. This usually takes around a minute to 2 minutes, but if it takes more then 5 minutes, likley something went wrong, and youll have to quit the app, feel free to report any issues or bugs you have on the issues tab
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/patching_done.png?raw=true)<br/>
When you see this message, your game is now patched to the custom server! if you see an error, make sure you selected the correct game, otherwise, feel free to report an issue

<div id='section-id-59'/>

# Editing stuff via ftp using a phone or pc
if youd prefer to copy text like title ids, or urls from your pc or phone then transfer them over, heres how you can do that (rerember to press TRIANGLE to refresh after making changes and to upload your changes)
<div id='section-id-61'/>

## Editing the selected title id
go to `/dev_hdd0/game/LBPCSPPHB/USRDIR` and open up the `title_id_to_patch.txt` file in a text editor, here you can just paste in a title id then save

<div id='section-id-64'/>

## Editing the urls
go to `/dev_hdd0/game/LBPCSPPHB/USRDIR` and open up the `saved_urls_x.txt` file in a text editor (replace x with the page number), then for each new line, you can paste in a url, or if there is a digest, put a space after the url and paste in the digest<br/><br/>
note, if the url or digest is too long, it will get truncated, and anything beyond 9 lines will not be shown, if you wish to add more urls then 9, edit a different page `saved_urls_x.txt`, the file would look something like
```
http://url_here_without_a_digest
http://url_here_with_a_digest CustomServerDigest
```

<div id='section-id-72'/>

# Technical details (no programming or c knowledge needed to read this)
for those intrested how this patcher works, here it is

<div id='section-id-75'/>

## What font is used?
<div id='section-id-76'/>

### NotoSans-Regular.ttf is used, it is a free open source font, and i also edited it to replace empty characters with the action buttons as show in the gui (they are crappy cause i made them)

<div id='section-id-78'/>

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
if it cant find a .rif ethier, decryption will likely fail, although if youre able to boot the game, it ill most certainly be able to decrypt the eboot.<br/><br/>
if it fails to decrypt now, it will try to use unself to decrypt instead
<div id='section-id-92'/>

## oscetool vs scetool
[oscetool](https://github.com/spacemanspiff/oscetool) is a clone of scetool, it works almost the exact same way as scetool but the long options in the commands are broken, so although ill refer to long options here, in the app it uses the short options

<div id='section-id-95'/>

## Decryption oscetool command
command was taken from [UnionPatcher](https://github.com/LBPUnion/UnionPatcher/blob/c45b9ec37eedade40490a1c000311b099ed71f31/UnionPatcher/RemotePatch.cs#L248)
```
./oscetool -v -d path/to/EBOOT.BIN /dev_hdd0/game/LBPCSPPHB/USRDIR/temp_files/EBOOT.ELF
```

<div id='section-id-101'/>

## Encryption oscetool command (disc)
command was taken from [UnionPatcher](https://github.com/LBPUnion/UnionPatcher/blob/c45b9ec37eedade40490a1c000311b099ed71f31/UnionPatcher/RemotePatch.cs#L255)
```
./oscetool -v --sce-type=SELF --skip-sections=FALSE --key-revision=0A --self-app-version=0001000000000000 --self-auth-id=1010000001000003 --self-vendor-id=01000002 --self-ctrl-flags=0000000000000000000000000000000000000000000000000000000000000000 --self-cap-flags=00000000000000000000000000000000000000000000003B0000000100040000 --self-type=APP --self-fw-version=0003005500000000 --compress-data=true --encrypt /dev_hdd0/game/LBPCSPPHB/USRDIR/temp_files/EBOOT.ELF path/to/EBOOT.BIN
```

<div id='section-id-107'/>

## Encryption oscetool command (digital)
command was taken from [UnionPatcher](https://github.com/LBPUnion/UnionPatcher/blob/c45b9ec37eedade40490a1c000311b099ed71f31/UnionPatcher/RemotePatch.cs#L191)
```
./oscetool --verbose --sce-type=SELF --skip-sections=FALSE --self-add-shdrs=TRUE --compress-data=TRUE --key-revision=0A --self-app-version=0001000000000000 --self-auth-id=1010000001000003 --self-vendor-id=01000002 --self-ctrl-flags=0000000000000000000000000000000000000000000000000000000000000000 --self-cap-flags=00000000000000000000000000000000000000000000003B0000000100040000 --self-type=NPDRM --self-fw-version=0003005500000000 --np-license-type=FREE --np-app-type=SPRX --np-content-id=UP0001-LBPCSPPHB_00-0000000000000000 --np-real-fname=EBOOT.BIN --encrypt /dev_hdd0/game/LBPCSPPHB/USRDIR/temp_files/EBOOT.ELF path/to/EBOOT.BIN
```

<div id='section-id-113'/>

## Patching method for main series
patch.lua is the code used for patching the eboot.elf, and also works outside of this project, if you'll like to know more about how the lua code works, refer to [Adding your own patch method](#using-the-lua-api). Although i did say you dont need any programming knowledge so ill explain it here<br/><br/>
The code is based of [this regex](https://github.com/LBPUnion/UnionPatcher/blob/c45b9ec37eedade40490a1c000311b099ed71f31/UnionPatcher/Patcher.cs#L53) `"http?[^\x00]*?LITTLEBIGPLANETPS(3|P)_XML\x00*"` although i did not use regex and just took the idea from it.<br/>
it first looks through the eboot.elf to find any `http` strings, if it finds any, it makes sure that the string starting with `http` ends with `LITTLEBIGPLANETPS3_xml` or `LITTLEBIGPLANETPSP_xml`. if it indeed does, then it calculates how much null bytes it will need to write, and makes sure the user url fits in that size, otherwise it will fail<br/><br/>
<div id='section-id-117'/>

### step 1, Find the http string
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/elf_patch_step_1.png?raw=true)<br/>
<div id='section-id-119'/>

### step 2, Check if it ends with /LITTLEBIGPLANETPS3_XML\0 or /LITTLEBIGPLANETPSP_XML\0
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/elf_patch_step_2_alt.png?raw=true)<br/>
<div id='section-id-121'/>

### step 3, Count the amount of extra null bytes (\0) at the end, in this case theres one more extra null byte
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/elf_patch_step_3.png?raw=true)<br/>
<div id='section-id-123'/>

### step 4, Check if the url is less then or equal to the full selection (including 1 null byte at the end)
<div id='section-id-124'/>

### step 5, Fill the entire thing with null bytes
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/elf_patch_step_4.png?raw=true)<br/>
<div id='section-id-126'/>

### step 6, Paste in the new url
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/elf_patch_step_5.png?raw=true)<br/>

<div id='section-id-129'/>

### for digest, it looks through a list of known digests, and does the above, just find the digest string instead of http, and skip step 2

<div id='section-id-131'/>

## Patching method for LittleBigPlanet Karting
it will look for the string `lbpk.ps3.online.scea.com\0`, basically the same as above just find the lbp k url string instead of http, and skip step 2

<div id='using-the-lua-api'/>

# Adding your own patch method, or editing the exsiting ones (only using lua, no c or compiler needed)
`patch.lua` is licenced with the MIT Licence, so you can edit and change it all you like, refer to [https://github.com/LittleBigPatcherTeam/LittleBigElfPatcherLua](https://github.com/LittleBigPatcherTeam/LittleBigElfPatcherLua) for Licence file<br><br>
If you will like to edit the patching code, or add your own patching method then you can edit the `patch.lua` file!<br/>
You will find this file in
`ux0:/app/LBPC59548/` on vita, `/dev_hdd0/game/LBPCSPPHB/USRDIR/` on ps3. You can edit this file freely, although i highly reccomend to do most of your testing on your dev machine using the game's EBOOT.ELF (or `eboot.bin.elf` on vita), and to do final testing, upload it to the folder whichever device youre using and reboot the app.

## How the app finds methods?
the app will run the lua file on boot, then it looks for functions starting with `patch_`, this is  the function that will get called during the pach process. It will also look for global strings starting with `patch_method_`, this is to show in the `Patch games` menu. please note that that each `patch_` function needs a global `patch_method_` string, otherwise the app will quit upon boot.<br/><br/>
Here's an example patch method entry in `patch.lua` that just does nothing (the type comments are not required)
```lua
patch_method_vita_tearaway = "Tearaway vita"
---@param eboot_elf_path string The path to the eboot.elf file which is ready to be patched
---@param url string The user entered url, it wont have a trailing null character, you should patch to this. if you need you can do stuff like just get the domain and port from a url
---@param digest string Same as url but for digest, most games you can ignore this, or maybe use it as something else, eg perhaps custom warning text? will be empty string if not provided
---@param normalise_digest boolean This you can most certainly ingnore, but you can always utlise it for something else, as an optinal boolean, false if uchecked, true if checked
---@param working_dir string Place to put files if you need to, but you likely wont
---@return boolean Does not matter, if something goes wrong during patching, you should `error("something went wrong")`, try to make sure you close the eboot file.
function patch_vita_tearaway(eboot_elf_path, url, digest, normalise_digest, working_dir)

end
```
(if the function starts with `patch_method_vita` it won't show on ps3, and `patch_method_ps3` wont show on vita, otherwise will show on both systems)<br/>
If you know what you're doing now, feel free to add in a new patch method now. You can use the `patch_ps3_lbpk` method as a base, but if you would like some more help...

## Making a basic patch
Lets make a patch method for Tearaway on the vita, we would like to patch in a url to replace `tearaway.me:10090`, first we need to find the in eboot.bin.elf url size, so if you're using HxD, open up the `eboot.bin.elf` (or `EBOOT.ELF` on ps3) on HxD
### Finding the url size
If we search for `tearaway.me:10090` in eboot.bin.elf, we can find this
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/tearaway_basic_search_hxd.PNG?raw=true)<br/>
If you pay attention on the hex editor side, you should notice some `00` bytes after the url (theese are called null bytes or null characters), what you want to do is select that url, plus all of the null bytes to the right of it, stopping when you reach the first non `00` byte, like so
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/tearaway_select_null_bytes.png?raw=true)<br/>
If you're using HxD, look near the bottom of the HxD window, for `Length(h): ` and you want to take note of the number
![idk](https://github.com/LittleBigPatcherTeam/LittleBigPatcher-for-Custom-Servers-PS3-Homebrew/blob/main/screenshots_for_readme/tearaway_highligh_url_len_hex.PNG?raw=true)<br/>
here, we can see its `Length(h): 14`, so our number will be `0x14` (if it says `Length(d): 20` then the number will be `20` instead)

## Adding in the patch
Open up patch.lua in an editor and go to the bottom to add in your number, like so (give a descriptive name) (there is no harm in having a bigger number, but do not make it smaller than any of the urls with the null bytes in the eboot, make it bigger to be safe)
```lua
local BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL <const> = 0x14
```
now here is the boiler plate code
```lua
local BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL <const> = 0x14

---@param offset integer
---@param file file
---@param url string
---@param respect_https boolean
---@return boolean
local function tearaway_patch(offset, file, url, respect_https)
	return basic_replace(offset, file, url, BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL, "url")
end

patch_method_vita_tearaway = "Tearaway vita"
---@param eboot_elf_path string The path to the eboot.elf file which is ready to be patched
---@param url string The user entered url, it wont have a trailing null character, you should patch to this. if you need you can do stuff like just get the domain and port from a url
---@param digest string Same as url but for digest, most games you can ignore this, or maybe use it as something else, eg perhaps custom warning text? will be empty string if not provided
---@param normalise_digest boolean This you can most certainly ingnore, but you can always utlise it for something else, as an optinal boolean, false if uchecked, true if checked
---@param working_dir string Place to put files if you need to, but you likely wont
---@return boolean Does not matter, if something goes wrong during patching, you should `error("something went wrong")`, try to make sure you close the eboot file.
function patch_vita_tearaway(eboot_elf_path, url, digest, normalise_digest, working_dir)
	---@type (BasePatchInstruction)[]
	local patches_list = { BasePatchInstruction:new(nil, url .. "\x00", tearaway_patch,
		BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL, "tearaway.me:10090\x00", false, 1) }
	base_patch(eboot_elf_path, working_dir, patches_list)
	if not patches_list[1].found_a_match then
		error("Could not find any urls to patch")
	end
	return true
end
```
heres what you want to edit
```lua
local function tearaway_patch(offset, file, url, respect_https)
	return basic_replace(offset, file, url, BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL, "url")
end
```
you can change this function name to anything you want, just dont make it start with `patch_` and you want to replace the `BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL` with your own number constant you made
```lua
patch_method_vita_tearaway = "Tearaway vita"
function patch_vita_tearaway(eboot_elf_path, url, digest, normalise_digest, working_dir)

```
Now you need to come up with an internal patch name, something short like the games name. `tearaway` will do it cannot be over 256 characters long. If youre targeting for vita only, make the internal patch name `vita_myname`, `ps3_myname` for ps3<br/>
now, change the function name to `patch_internalname`, and the patch_method_ to `patch_method_internalname`, and of course change the string, this will be shown in the patcher<br/>
```lua
	local patches_list = { BasePatchInstruction:new(nil, url .. "\x00", tearaway_patch,
		BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL, "tearaway.me:10090\x00", false, 1) }
```
now, you can change the `tearaway.me:10090\x00` url to the url you found, (make sure it ends with `\x00`), and replace `BIGGEST_POSSIBLE_URL_IN_TEARAWAY_VITA_EBOOT_INCL_NULL` with your number constant, and finally replace `tearaway_patch` with the basic replace function you made earlier
<div id='section-id-134'/>

# Building
Youd want to use linux, or wsl on windows in order to build this, basically you need to be able to build the [Tiny3D samples](https://github.com/wargio/tiny3D/tree/master/samples/sprites2D) in order to build this.<br/>
also you'll need to install [dbglogger](https://github.com/bucanero/dbglogger)<br/>
then you can run the python script in python3 `python3 build.py` in order to build!

<div id='section-id-139'/>

# Credits (If i missed anyone please let me know)
<div id='section-id-140'/>

## [oscetool](https://github.com/spacemanspiff/oscetool)
<div id='section-id-141'/>

### For decryption and rencryption of eboot.bin files, making this possible!
<div id='section-id-142'/>

## [Apollo Save Tool PS3](https://github.com/bucanero/apollo-ps3)
<div id='section-id-143'/>

### For helper functions such as the on screen keyboard input, getting idps, and for the idea of using analogue stick for controls as well as dpad
<div id='section-id-144'/>

## [Tiny3d samples](https://github.com/wargio/tiny3D.git)
<div id='section-id-145'/>

### the building blocks of the GUI, allowing to easily put text on the screen and navigate the menus
<div id='section-id-146'/>

## [UnionPatcher](https://github.com/LBPUnion/UnionPatcher)
<div id='section-id-147'/>

### For the idea of a patcher, and the oscetool commands for encryption and decryption
<div id='section-id-148'/>

## [unself fail0verflow-PS3-tools](https://github.com/daryl317/fail0verflow-PS3-tools)
<div id='section-id-149'/>

### for backup decrypt if the oscetool decrypt fails

<div id='lua-credit'/>

## [Lua](https://www.lua.org/ftp/lua-5.4.7.tar.gz)

In no way am i saying i made lua lol, please refer to [lua](https://www.lua.org/license.html) but i did delete some unused files from the tar.gz to save space, and had to rename 2 functions named `main` to `luac_main` and `lua_main`
