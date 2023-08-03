;app infrastructure
#SingleInstance, Force
SendMode, Input

;copy and paste with mouse
~LButton & MButton::
mutemiddle = 1
Send, ^x^c
return
~LButton & RButton::^v

;enter / newline
MButton & LButton::Enter

;space
MButton & RButton::Space

;window switching
GroupAdd AltTabWindow, ahk_class MultitaskingViewFrame  ;win 10
GroupAdd AltTabWindow, ahk_class TaskSwitcherWnd        ;win vista, 7, 8
GroupAdd AltTabWindow, ahk_class #32771                 ;classic

#IfWinNotExist ahk_group AltTabWindow
MButton & ~WheelDown::
Send, {Alt down}{Tab}{Tab up}
Keywait, MButton
Send, {Alt up}
#If

#IfWinExist ahk_group AltTabWindow
MButton & ~WheelDown::AltTab
MButton & ~WheelUp::ShiftAltTab
#If

;undo & redo
RButton & LButton::^z
RButton & MButton::^y

;prevent loss of simple button clicks
MButton up::
if mutemiddle = 0
{
    Click, Middle
}
mutemiddle = 0
return
RButton up::
Click, Right
return

