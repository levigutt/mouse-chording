## why?

my attempts at setting up mouse button chording on linux: 
https://unix.stackexchange.com/q/751786/360766

it turns out that while `xbindkeys` and `xdotool` are quite capable, they simply lack the featurset needed to replicate the exact setup that I'd gotten used to on Windows and MacOS.

specifically, I want mouse chording using a three button mouse (not a special mouse with extra buttons), and I want to have the buttons keep their original function when used individually.

`xbindkeys` will grab the mouse events, preventing the default actions. 

`xdotool` cannot send a mouse event while the same event is in effect.

neither tools can limit their function to a specific input device.

## how it works

this uses `evtest` to interrupts mouse events from a specified input device, and passes the event stream to the program which runs commands when chord combos are detected, or else recreates the mouse events.

I haven't yet figured out all I need to fully utilize the X11 API - so many of the commands are done through xdotool. This makes it noticeably slow and unsuitable for gaming.

## how to use

**1\.**
compile with `./compile.sh` 

_this requires some x11 development libraries_

**2\.**
run `sudo evtest` to get list of input devices

in my case `/dev/input/event19` is the USB OPTICAL MOUSE, which I want to use for chording. 

replace with appropriate input device for your setup in the `run.sh` script

**\3.**

run `run.sh` - this will _grab_ (interrupt) input events from the selected device, padding all events to the executable - which in turn will either do the mouse chording actions, if the events correspond to any of the chording shortcuts, or pass on the events through the X11 API if not.

## chording shortcuts

```
Left+Middle   = Cut   (Ctrl+x)
Left+Right    = Paste (Ctrl+v)
Middle+Left   = Return
Middle+Right  = Space
Right+Left    = Undo  (Ctrl+z)
Right+Middle  = Redo  (Ctrl+Y / Ctrl+Shift+z)
```

## chord ideas

these are not yet implemented

```
Middle + Scroll = Switch app (Alt+Tab)
Right  + Scroll = Clipboard menu (using dmenu, paste selection on release)??
```

decided against these, since they'd mainly be useful in the browser, which has prominent back and forward buttons in the gui (and one can use Ctrl+Z in the adress bar too)
``` 
Middle + Left   = Back
Middle + Right  = Forward
```

---

borrowed mouseclick routine from here:
https://snipplr.com/view/1599/xlib--mouseclick

but it does not work right, I can't change active window and it wont let me resize windows

switched to using xdotool as a system call, for now - atleast it works :p 

using X11/extensions/Xtest.h provides a function `XTestFakeButtonEvent` which is supposed to work better
- https://bharathisubramanian.wordpress.com/2010/04/01/x11-fake-mouse-events-generation-using-xtest/

current versions slows down the mouse, like using low dpi, but is otherwise quite usable. 

## todo 

- [x] fix bug where left mouse button is not released after chord
- [x] pass through mouse scrolling event
- [x] fix repeating middle and right clicks
- [ ] add scroll-chording (middle + scroll)
- [ ] get Xtest extension and replace xdotool with XTestFakeButtonEvent() (should be faster)
- [ ] use x11 api to grap events, instead of relying on `evtest` (should be faster)
- [ ] add config file for specifying chords and commands (more customizable)
- [ ] clean up code
