# shitty_autoclicker
### shitty_autoclicker is, thats right, a shitty autoclicker with some cool features:
* #### the ability to search for a given template and click on it (-t)
* #### the ability to lock to a given position (-p)
* #### the ability to press keys instead of mouse buttons (-k)
* #### clicking the mouse and a key at the same time
* #### variable click speed (-i)
## Usage:
> ### toggle on and off with F6
> ### kill with F7
## CMD args:
> ### -t [template image filepath] (mouse automatically enabled)
> ### -p [x cord] [y cord] (mouse automatically enabled)
#### when both a position and a template is passed, the autoclicker will prioritize the template and only use the position when the template can not be found on screen.
> ### -k [win32 virtual-key code]
> ### -i [interval between clicks in milliseconds] default is 4
> ### -km [win32 virtual-key code] same as -k, but with the mouse enabled
> ### -l enables lower level mouse clicks, these are slower but can interface with more programs
## Setup:
> ### open AutoClicker.sln in visual stuido
> ### build 
### use the "Release" build config for better template performance 