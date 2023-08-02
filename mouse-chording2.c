#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "X11/extensions/XI.h"
#include "X11/extensions/XInput.h"
#include "X11/Xlib.h"

Display *dpy;
Window win;

// mask for event state
int ScrollDnState = 0b1000000000000;
int ScrollUpState = 0b0100000000000;
int RightState    = 0b0010000000000;
int MiddleState   = 0b0001000000000;
int LeftState     = 0b0000100000000;
//                = 0b0000010000000; // unknown
int MetaState     = 0b0000001000000;
//                = 0b0000000100000; // unknown
//                = 0b0000000010000; // unknown
int AltState      = 0b0000000001000;
int CtrlState     = 0b0000000000100;
int CapsState     = 0b0000000000010;
int ShiftState    = 0b0000000000001;

long evt_mask = ButtonPressMask | ButtonReleaseMask;
int active_chord = 0;

int main(int argc, char *argv[])
{
    dpy = XOpenDisplay(NULL);
    if (NULL == dpy) 
    {
        fprintf(stderr, "Err: Could not open display\n");
        exit(1);
    }
    win = XDefaultRootWindow(dpy);
    // todo: error check win

    int device_count;
    XDeviceInfo *devices = XListInputDevices(dpy, &device_count);
    if (!devices)
    {
        fprintf(stderr, "Err: Could not read devices\n");
        exit(1);
    }

    int mouse_id;
    for (int i = 0; i < device_count; i++)
    {
        printf("id: %d, name: %s\n", devices[i].id, devices[i].name);
        if (0 == strcmp(devices[i].name, "SIGMACHIP SG 2.4G Wireless Mouse")
         || 0 == strcmp(devices[i].name, " USB OPTICAL MOUSE"))
        {
            mouse_id = devices[i].id;
        }
    }
    //XFreeDeviceList(devices);
    if (!mouse_id)
    {
        fprintf(stderr, "Err: Could not find device\n");
        exit(1);
    }

    XDevice *mouse = XOpenDevice(dpy, mouse_id);
    if (!mouse)
    {
        fprintf(stderr, "Err: Could not open device\n");
        exit(1);
    }
    printf("mouse id: %d, num_classes: %d\n", mouse->device_id, mouse->num_classes);

    if (NULL == mouse->classes )
    {
        printf("mouse->classes is an array\n");
    }
    XInputClassInfo *classes = mouse->classes;
    for(int j = 0; j < mouse->num_classes; j++)
    {
        char input_class = classes[j].input_class;
        char event_type_base = classes[j].event_type_base;
        printf("input_class: %#x, event_type_base: %#x\n", 
                input_class, event_type_base);
    }

    int event_list_count = 0;
    XEventClass event_list[32];
    XEventClass cls;

    int dev_btn_press;
    int dev_btn_release;

    DeviceButtonPress(mouse, dev_btn_press, cls);
    if (cls) event_list[event_list_count++] = cls;

    DeviceButtonRelease(mouse, dev_btn_release, cls);
    if (cls) event_list[event_list_count++] = cls;

#if 0
    // normal, grabs all mouse button events - interrupts normal function
    XSelectInput(dpy, win, NoEventMask);
    XGrabButton(dpy, AnyButton, AnyModifier, win, False,
                ButtonPressMask | ButtonReleaseMask,
                GrabModeAsync, GrabModeSync, None, None);
#else
    // grabs mouse button events for device - DOES NOT interrupt normal function.... :( 
    XSelectExtensionEvent(dpy, win, event_list, event_list_count);
    //XGrabDevice(dpy, mouse, win, False,
    //            event_list_count, event_list,
    //            GrabModeAsync, GrabModeSync, CurrentTime);
    XGrabDeviceButton(dpy, mouse, AnyButton, AnyModifier, NULL, win, False, 
                      event_list_count, event_list,
                      GrabModeAsync, GrabModeSync);
#endif

    int prevBtn = 0;
    int k = 0;
    while(1) {
        // safety net
        if (k > 100) break;

#if 0
        XEvent evt;
        XNextEvent(dpy, &evt);
        int state  = evt.xbutton.state;
        int button = evt.xbutton.button;
        int press  = evt.type == 4 ? 1 : 0;
#else
        // GrabDevice issues different events.. :/
        XDeviceButtonEvent evt;
        XNextEvent(dpy, &evt);
        int state  = evt.state;
        int button = evt.button;
        int press  = evt.type == 69 ? 1 : 0;
#endif

        printf("#%04d: %013b, %d, %d\n", k++, state, button, press);

        if (!press)
        {
            if (button == prevBtn)
            {
                if (button == 1)
                    printf("xdotool mouseup 1\n");
                else
                    printf("xdotool click %d\n", prevBtn);
                prevBtn = 0;
            }
            continue;
        }

        if (button == 2 && state & LeftState)
        {
            // Left + Middle = Snarf
            printf("xdotool key ctrl+x\n");
            if (prevBtn != 1) prevBtn = 0;
            continue;
        }
        else if (button == 3 && state & LeftState)
        {
            // Left + Right = Paste
            printf("xdotool key ctrl+v\n");
            if (prevBtn != 1) prevBtn = 0;
            continue;
        }
        else if (button == 1 && state & RightState)
        {
            // Right + Left = Undo
            printf("xdotool key ctrl+y\n");
            if (prevBtn != 1) prevBtn = 0;
            continue;
        }
        else if (button == 2 && state & RightState)
        {
            // Right + Middle = Redo
            printf("xdotool key ctrl+shift+y\n");
            if (prevBtn != 1) prevBtn = 0;
            continue;
        }
        else
        {
            prevBtn = button;
            if (button == 1)
                printf("xdotool mousedown 1\n");
        }
    }

    //todo: XUngrabDeviceButton
    XFreeDeviceList(devices);
    XCloseDevice(dpy, mouse);
    XCloseDisplay(dpy);
    return 0;
}

