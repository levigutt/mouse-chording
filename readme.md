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
