#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

char *read_line(char *buffer);

Display *dpy;
Window root_win;
Screen *screen;

int buttons_state     = 0;
bool active_chord     = 0;
bool alt_down         = 0;
long mouse_x, mouse_y = 0;

int left_button     = 272;
int right_button    = 273;
int middle_button   = 274;
int scroll_wheel    = 8;
int button_mask[4]  = {0, Button1Mask, Button2Mask, Button3Mask};


int main(int argc, char * argv[])
{
    // prepare x11 api
    dpy = XOpenDisplay(0);
    if(dpy == NULL)
    {
        fprintf(stderr, "Err: Could not open display.\n");
        exit(1);
    }
    root_win = XRootWindow(dpy, 0);
    screen = DefaultScreenOfDisplay(dpy);

    // read lines from STDIN
    char line[1024];
    char cmd[32];
    while (read_line(line))
    {
        long type;
        long code;
        long value;

        // split line by spaces to pull out values
        int i = 0;
        char *ptr;
        char *token;
        char s[2] = " ";
        token = strtok(line, s);
        while (token != NULL)
        {
            switch (i++)
            {
                case 4  : type  = strtol(token, &ptr, 10); break;
                case 7  : code  = strtol(token, &ptr, 10); break;
                case 10 : value = strtol(token, &ptr, 10); break;
            }
            token = strtok(NULL, s);
        }

        // ignore unused events
        if (!type || code == 11) continue;

        // emulate mouse movement
        if (type == 2 && code <= 1)
        {
            mouse_x = 0;
            mouse_y = 0;
            if (code == 1)
                mouse_y = value;
            else
                mouse_x = value;

            XWarpPointer(dpy, None, None, 0, 0, 0, 0, mouse_x, mouse_y);
            XFlush(dpy);
            continue;
        }

        // detect mouse clicks
        int current_btn = code == left_button   ? 1
                        : code == middle_button ? 2
                        : code == right_button  ? 3 : 0;

        // keep track of button presses
        if (value)
            buttons_state |= button_mask[current_btn];
        else
            buttons_state &= ~button_mask[current_btn];

        // simulate mouse clicks that happened in isolation
        if (!(buttons_state & ~button_mask[current_btn]))
        {
            cmd[0] = '\0';
            if (code == left_button)
                sprintf(cmd, "xdotool %s %d&",
                             value ? "mousedown" : "mouseup",
                             current_btn);
            else if (code == scroll_wheel)
                sprintf(cmd, "xdotool click %d&", value > 0 ? 4 : 5);
            else if (!value && active_chord == 0)
                sprintf(cmd, "xdotool click %d&", current_btn);

            if (cmd[0]) system(cmd);

            // reset chording
            system("xdotool keyup alt&");
            alt_down     = 0;
            active_chord = 0;
            continue;
        }

        // only run chords on button presses (not releases)
        if( !value ) continue;

        // chording syntax (example Left + Right)
        //          when Left is down       and   we click Right
        // if ((buttons_state & Button1Mask) && code == right_button)

        //Left + Middle = Snarf
        if ((buttons_state & Button1Mask) && code == middle_button)
        {
            system("xdotool key ctrl+c key ctrl+x&\n");
            active_chord = 1;
        }

        //Left + Right = Paste
        if ((buttons_state & Button1Mask) && code == right_button)
        {
            system("xdotool key ctrl+v&\n");
            active_chord = 1;
        }

        //Middle + Left = Return
        if ((buttons_state & Button2Mask) && code == left_button)
        {
            system("xdotool key Return&\n");
            active_chord = 1;
        }

        //Middle + Right = Space
        if ((buttons_state & Button2Mask) && code == right_button)
        {
            system("xdotool key space&\n");
            active_chord = 1;
        }

        //Right + Left = Undo
        if ((buttons_state & Button3Mask) && code == left_button)
        {
            system("xdotool key ctrl+z&\n");
            active_chord = 1;
        }

        //Right + Middle = Redo
        if ((buttons_state & Button3Mask) && code == middle_button)
        {
            system("xdotool key ctrl+shift+z&\n");
            active_chord = 1;
        }

        //Middle + Scroll = window switcher
        if ((buttons_state & Button2Mask) && code == scroll_wheel)
        {
            if (!alt_down)
                system("xdotool keydown alt&\n");
            alt_down = 1;
            if (value > 0)
                system("xdotool key shift+Tab&\n");
            else
                system("xdotool key Tab&\n");
            active_chord = 1;
        }
    }

    XCloseDisplay(dpy);
    return 0;
}


char *read_line(char *buffer)
{
    char *result = fgets(buffer, 1024, stdin);
    int len;

    // fgets returns NULL on error of end of input,
    // in which case buffer contents will be undefined
    if (result == NULL)
    {
        fprintf(stderr, "Err: Could not read event stream.\n");
        return NULL;
    }

    len = strlen (buffer);
    if (len == 0)
        return NULL;

    if (buffer[len - 1] == '\n')
        buffer[len - 1] = 0;

    return buffer;
}
