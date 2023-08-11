# Mouse chording

The basic idea is that while holding down a mouse button, the other mouse
buttons become shortcuts for custom actions. Like cut, paste, or anything you'd
like. It's a powerful feature that can increase productivty and reduce fatique.

I first became aware of this when I tried Acme (the plan9 editor), with its
[mouse chords](http://acme.cat-v.org/mouse). Using Acme as my editor did not
work out for me, but I could not get the mouse chords out of my head, I wanted
to have such chords system wide.

## Chords

These are the default mouse chords, but they can be customized to anything.

```
Left + Middle   = Cut
Left + Right    = Paste
Right + Left    = Undo
Right + Middle  = Redo
Middle + Left   = Return
Middle + Right  = Space
Middle + Scroll = Window switcher
```

Why cut instead of copy? Because Cut + Paste = Copy, that way it is more
useful.

## Getting started

0. read up on the [known issues](#known-issues) for your platform.

### Windows

1. Install [AutoHotkey](https://www.autohotkey.com/).
2. Edit `windows/mouse-chording.ahk` to suit your needs
3. Run `windows/mouse-chording.ahk` as administrator.


### MacOS

1. Install [Hammerspoon](https://www.hammerspoon.org/)
2. Copy `macos/init.lua` to `~/.hammerspoon/init.lua`
3. Run Hammerspoon


### Linux (X11)

1. install `xinput` and `evtest`
2. run `evtest` and make note of your usb mouse, something like `/dev/input/event3`
3. run `xinput -list` and make note of the device id
4. run `xinput set-button-map {id} 0 0 0 0 0 0 0 0 0 0` (10 zeroes)
5. run `evtest /dev/input/event3 | ./mouse-chording.pl`

## What computer mouse to use?

I've found that this works best with a three button mouse where the middle
button is a proper button and not hidden under the scroll wheel.

This is kind of rare, but you can probably find one using the phrase
"CAD mouse".

This is the mouse I use: https://amzn.asia/d/gwQMNqZ

Here are some other good options:
- https://a.co/d/eoXpdmS
- https://a.co/d/1f33M6c
- https://amzn.asia/d/0yckABy

## Known issues

To get a reliable Cut, `Left+Middle` is implemented as `Ctrl+C Ctrl+X`. This is
because when doing `Ctrl+X` on text that cannot be cut, like the text on a
website or in a PDF, the text will not be copied. So we copy first to make sure
we get something.

To preserve the normal function of the mouse buttons - when pressed
individually - the code has to issue click events when a mouse button is
released (if no chord was executed). This causes things like the context menu
to appear on release rather than on press. Hold and drag with Middle or Right
button will not work for the same reason.

### Windows issues

Some windows applications will not allow shortcuts using modifier keys to work
while a mouse button is pressed. In these applications, mouse chording simply
won't work. It seems to only be a problem in older applications (like Putty).

Running AutoHotkey without admin rights will cause it not to work when
interacting with application that do run with admin rights. 

### MacOS issues

The touchpad issues mouse events very differently from a USB mouse. Using the
touchpad can therefore cause the app to enter an undefined state and fail to
run chords or run them incorrectly.

Avoid this problem by disabling mouse chording when swithing to the touchpad.

MacOS has a different window switcher than Windows and Linux, `Alt+Tab` will
switch between applications instead of windows. And `Alt+NonUSBackslash` (the
key above Tab) will switch between windows for the current application. To
account for this, the Mac version will switch between apps with `Middle+Scroll` and
between windows with `Right+Scroll`.

### Linux issues

The `xinput` remapping will cause the buttons for the usb mouse to not work
unless the script is running. re-enable the mouse buttons with this command:
`xinput set-button-map {id} 1 2 3 4 5 6 7 8 9 10`

Since the solution depends on a specific device, it will crash and require a
restart if the device is unplugged.

## todo

- [x] move each platform version to separate directory
- [x] figure out how to retain normal mouse movements on linux
- [ ] refactor mac version to follow similar logic as the others
- [ ] add option for defining unique chords by application


