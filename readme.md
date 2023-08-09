# Mouse chording

The basic idea is that you hold down a mouse button and then trigger some
custom action by clicking another mouse button. Clicking the mouse buttons
individually still works as normally.

I first became aware of this when I tried Acme (the plan9 editor), with its
[mouse chords](http://acme.cat-v.org/mouse). Using Acme as my editor did not
work out for me, but I could not get the mouse chords out of my head, I wanted
to have such chords system wide.

## Chords

```
Left + Middle   = Cut
Left + Right    = Paste
Right + Left    = Undo
Right + Middle  = Redo
Middle + Left   = Return
Middle + Right  = Space
Middle + Scroll = Window switcher
```

## Getting started

### Windows

1. Install [AutoHotkey](https://www.autohotkey.com/).

2. Edit `windows/mouse-chording.ahk` to suit your needs

3. Run `windows/mouse-chording.ahk` as administrator.


### MacOS

1. Install [Hammerspoon](https://www.hammerspoon.org/)

2. Copy `macos/init.lua` to `~/.hammerspoon/init.lua`

3. Run Hammerspoon


### Linux (X11)

1. install `evtest`

2. run `evtest` and make note of your usb mouse, something like
   `/dev/input/event3`

3. make sure `mouse-chording.pl` is executable with `chmod +x mouse-choring.pl`

4. run `evtest --grav /dev/input/event3 | ./mouse-chording.pl`


If the mouse movements are noticibly slow, the C version might be a better
option: 

1. install the X11 development library

2. compile with `./compile.sh`

3. run with `run.sh`


## known issues


To preserve the normal function of the mouse buttons - when pressed
individually - the code has to issue click commands when a mouse button is
released (if no chord was executed). This causes things like the context menu
from appearing on release rather than on press.

Applications that allow you to select text using middle or right mouse buttons
will not work for the same reason. There is no "hold and drag" using any other
mouse button than the left button.

### MacOS issues

Mouse events are issues very differently for the touchpad and a usb mouse.
using the touchpad and usb mouse interchangably can therefore cause the
application to enter an undefiend state and become unresponsive. 

This can be fixed easily by disabling the mouse chords when not using a usb
mouse.

Since Mac has a different Window Switcher, which switches between applications
and windows separately (Alt+Tab = Switch app, Alt+NonUSBackslash = Switch
window for current app) - this version has a separate set up for window
switching on `Right + Scroll`

### Linux issues

This works by interrupting all events for the mouse, including movements. The
movements therefore must be reissued, which causes mouse accelleration and
other mouse settings in your desktop environment not to work.

This may also cause the mouse movements to feel slow, especially on computers
with limited resources. to overcome this, I've made a version in C, which runs
faster, but you'll have to compile it yourself.

### Windows issues

Chords involving modifier keys does not work in some windows applications,
typically older style applications. The cause is unknown, but it does not seem
to be a problem in more modern applications. One example of this is Putty.

Running without admin rights will cause it not to work when interacting with an
application that does run with admin rights.

## what computer mouse to use?

I've found that this works best with a three button mouse where the middle
button is a proper button and not hidden under the scroll wheel.

This is not easy to come by, but you can probably find one using the phrase
"CAD mouse".

This is the mouse I use: https://amzn.asia/d/gwQMNqZ

Here are some other similar options:
- https://a.co/d/eoXpdmS
- https://a.co/d/1f33M6c
- https://amzn.asia/d/0yckABy

## todo

- [ ] move linux verson to subdirectory too
- [ ] refactor mac version to follow similar logic as the others, and remve
  hyper-key setup
    - set up new binding to toggle chording
- [ ] figure out how to only grab mouse buttons (branch `grabdevice`) on linux
