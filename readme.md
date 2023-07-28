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

---

borrowed mouseclick routine from here:
https://snipplr.com/view/1599/xlib--mouseclick

but it does not work right, I can't change active window and it wont let me resize windows

switched to using xdotool as a system call, for now - atleast it works :p 

using X11/extensions/Xtest.h should give us a FakeMouseClick function that is supposed to work better

current versions slows down the mouse, like using low dpi


## todo 

- get Xtest and replace xdotool with XTestFakeButtonEvent()
- use x11 api to grap events, instead of relying on `evtest` and a pipe
- clean up code
