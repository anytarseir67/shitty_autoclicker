# shitty_autoclicker
### shitty_autoclicker is, thats right, a shitty autoclicker with some cool features:
* #### the ability to search for a given template and click on it (-t)
* #### the ability to lock to a given position (-p)
* #### the ability to press keys instead of mouse buttons (-k)
* #### variable click speed (-i)
## Usage:
> ### toggle on and off with F6
> ### kill with F7
## CMD args:
> ### -t [template image filepath]
> ### -p [x cord] [y cord]
#### when both a position and a template is passed, the autoclicker will prioritize the template and only use the position when the template can not be found on screen.
> ### -k [win32 virtual-key code]
> ### -i [interval between clicks in milliseconds] default is 15
## Setup:
#### (this assumes you are using visual studio)
> ### [add opencv to your projects config](https://towardsdatascience.com/install-and-configure-opencv-4-2-0-in-windows-10-vc-d132c52063a1)
> ### add opencv_world453d.dll to the build directory (eg: add to {user}/source/repos/{project name}/x64/Debug)
#### the dll is included in the repo, but i felt it should be mentioned anyway.
> ### try to compile and remember why you main langs with standardized package managers