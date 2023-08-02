## why mouse chording?

Like most _superusers_ I prefer using the keyboard for everything, and for a
long time I thought I hated the mouse. 

But after trying Acme (the plan9 editor) and its 
[mouse chords](http://acme.cat-v.org/mouse) I realized that what I actually
hated was switching back and forth between the mouse and the keyboard.

Using mouse chording I can greatly reduce the number of times I have to switch
back and forth, and simple tasks like copying and pasting text from one
application into another can become a simple one-handed operation.

## why this project?

I was able to set up mouse chording very easily on windows using AutoHotkey
(see `mouse-chording.ahk`). I got hooked, and was able to bring the setup with
me to MacOS using Hammerspoon. 

Then I've moved to Linux, and was not able to find a way to replicate my setup. 
Some of my attempts are documented here: 
[unix.stackexchange.com/q/751786/360766](https://unix.stackexchange.com/q/751786/360766)

it turns out that while `xbindkeys` and `xdotool` are quite capable, they
simply lack the exact features I need.

Specifically, I want mouse chording using a three button mouse (not a special
mouse with extra buttons), and I want to have the buttons keep their original
function when used "normally" (individually). This final point is where 
`xbindkeys` fails.

## how this works

It uses `evtest` to grab (interrupt) mouse events from a specified input device,
and passes the event stream to the program which runs commands when chord 
combos are detected, or else recreates the mouse events.

Since it has to recreate mouse movement events, it will slow down the mouse. 
Like using low DPI. It also recreates X-axis and Y-axis seperately, which 
makes it feel weird.

## how to use

**1\.**
compile with `./compile.sh` 

_this requires some x11 development libraries_

**2\.**
run `sudo evtest` to get list of input devices

my mouse is named `USB OPTICAL MOUSE` and is listed as `/dev/input/event19`

replace with appropriate input device for your setup in the `run.sh` script

**3\.**

run `run.sh`

## chording shortcuts

```
Left+Middle   = Cut   (Ctrl+X)
Left+Right    = Paste (Ctrl+V)
Middle+Left   = Return
Middle+Right  = Space
Right+Left    = Undo  (Ctrl+Z)
Right+Middle  = Redo  (Ctrl+Shift+Z / Ctrl+Y)
```

## chord ideas

```
Middle + Scroll = Switch app (Alt+Tab)
Right  + Scroll = Clipboard menu (using dmenu, paste selection on release)??
Middle + Left   = Back
Middle + Right  = Forward
```

these are not implemented

## development

The immediate problem is that it slows down the mouse.

`mouse-chording2.c` is attempting to solve this by grabbing the mouse events 
through the X11 API, instead of relying on `evtest` - the idea is to only grab
button events, and therefore not need to recreate mouse movement.

It does this by using `XGrabDeviceButton()` to only grab button events from a
specified device. However, it does not stop the normal function of the mouse
buttons, for some reason.

Using `XGrabButton()` does stop the normal function, but will intercept all mouse
events - not just from the external mouse. This means that any mouse event we
recreate will also be grabbed - causing infinite loops.

The documentation for `XGrabDeviceButton()` is not very clear and I think I might 
be using it wrong. Which is probably why it doesn't do what I want. It also 
issues `XDeviceButtonEvent` instead of `XEvent` which makes it a poor fit for 
`XNextEvent()`. 

### other solutions

**use ioctl** 

use `ioctl(fd,EVIOCGRAB,1)` to capture mouse events from device instead of
`XGrabDeviceButton`

downside:

- more complex, and we must filter out the mouse movement manually.

upside: 
- could be made as a patch to `evtest` giving it an option to only grab
  button-events. which would allow for this:
  `evtest /dev/input/event3 --grab --buttons-only | ./mouse-chording.exe`
    - `mouse-chording.exe` could then be done in any language, since we don't
      need the speed of C for replaying mouse movements or the xlib library.

## todo 

- [x] fix bug where left mouse button is not released after chord
- [x] pass through mouse scrolling event
- [x] fix repeating middle and right clicks
- [ ] use x11 api to grap events, instead of relying on `evtest`
- [ ] add config file for specifying chords?
- [ ] add more chording combos
- [ ] clean up code


## research

### recreating mouse events

current implementation use system calls to `xdotool`

tried using the mouseclick routine from here:
[snipplr.com/view/1599/xlib--mouseclick](https://snipplr.com/view/1599/xlib--mouseclick)
but it does not work right, I can't change active window and it wont let me
resize windows

`XTestFakeButtonEvent` from `X11/extensions/Xtest.h` seems to have the same
problems

### preventing infinite loops

recreating mouse events and then capturing them will cause an infinite loop. 

#### solutions

**ungrab before recreating events, regrab after**

- this requires a delay to let the event go through before regrabbing.
    - regrabbing proved to cause some problems, perhaps XDisplay needs to be closed and reopened? 
- my attempts at this approach was agonizing, will explore other options first. 

**only capture from specific device**

- essentially what `evtest | ./mouse-chording.exe` is doing, so we know it works
- events that are replayed will not be captured.. problem solved.
- `XGrabDeviceButton` seems to not want to _grab_, it only listens for events, but does not prevent them from going through.
