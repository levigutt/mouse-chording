#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

bool debug = false;
bool verbose = false;

char *read_line(char *buffer);
int run_mouse_chord(int button);
long long current_timestamp();

Display *dpy;
Window root_win;
Screen *screen;

int mouse_button_state = 0;
bool mouse_chord_active = 0;

int LMB = 272;
int RMB = 273;
int MMB = 274;

long mouse_x, mouse_y = 0;

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
    char *line = malloc(sizeof(char)*1024);
    while (read_line(line))
    {
        long long now = current_timestamp();
        double etime;
        long type;
        long code;
        long value;
        char *ptr;

        // split line by spaces to pull out values
        int i = 0;
        char s[2] = " ";
        char *token;
        token = strtok(line, s);
        while (token != NULL)
        {
            switch (i++)
            {
                case 2: etime  = strtod(token, &ptr); break;
                case 4:  type  = strtol(token, &ptr, 10); break;
                case 7:  code  = strtol(token, &ptr, 10); break;
                case 10: value = strtol(token, &ptr, 10); break;
            }
            token = strtok(NULL, s);
        }

        if (!type)
            continue;

        if (debug)
            printf("time: %.6f, type: %d, code: %d, value: %d\n",
                    etime, type, code, value);

        // mouse movement and scrolling
        if (type == 2)
        {
            if (code == 8)
            {
                if (value > 0)
                    system("xdotool click 4&");
                else
                    system("xdotool click 5&");
            }
            else if (code <= 1)
            {
                if (code == 1)
                    mouse_y+= value;
                else
                    mouse_x+= value;

                if (debug)
                    printf("%d  x  %d\n", mouse_x, mouse_y);

                // calculate mouse lag
                long long diff = (now*10) - (long  long)(etime * 10000);

                // move cursor unless lagging
                if (diff <= 5)
                {
                    XWarpPointer(dpy, None, None, 0, 0, 0, 0, mouse_x, mouse_y);
                    mouse_x = 0; mouse_y = 0;
                    XFlush(dpy);
                }
                else if (debug)
                    printf("lag: %d\n", diff);
            }

            continue;
        }

        // detect mouse clicks
        if (type == 1)
        {
            int button_code = 0;

            if (code == LMB)
            {
                button_code = Button1;
                if (value)
                    mouse_button_state |= Button1Mask;
                else
                    mouse_button_state &= ~Button1Mask;
            }
            if (code == RMB)
            {
                button_code = Button3;
                if (value)
                    mouse_button_state |= Button3Mask;
                else
                    mouse_button_state &= ~Button3Mask;
            }
            if (code == MMB)
            {
                button_code = Button2;
                if (value)
                    mouse_button_state |= Button2Mask;
                else
                    mouse_button_state &= ~Button2Mask;
            }

            if (debug || verbose)
                printf("mouse_button_state: %011b\n", mouse_button_state);

            if (debug)
                printf("mouse_chord_active: %d\n", mouse_chord_active);

            // check for chording combos and run commands
            if (value)
                run_mouse_chord(button_code);

            // trigger default mouse events
            if (button_code > 0)
            {
                if (debug)
                    printf("mouse click\n button: %d\nstate: %d\n", button_code, value);

                char *cmd = malloc(sizeof(char)*32);
                sprintf(cmd, "");
                if (mouse_chord_active == 0 
                 && button_code == 1 
                 && value)
                {
                    sprintf(cmd, "xdotool mousedown %d\n", button_code);
                }
                else if (button_code == 1)
                {
                    sprintf(cmd, "xdotool mouseup %d\n", button_code);
                }
                else if (mouse_chord_active == 0
                 && mouse_button_state == 0
                 && !value)
                {
                    sprintf(cmd, "xdotool click %d\n", button_code);
                }
                if (strlen(cmd) > 0)
                    system(cmd);
            }

            // reset when all buttons are released
            if (mouse_chord_active && mouse_button_state == 0)
                mouse_chord_active = 0;

        }
    }

    XCloseDisplay(dpy);

    return 0;
}


// where the magic happens
int run_mouse_chord(int button)
{
    if (mouse_button_state == (Button1Mask | Button2Mask))
    {
        if (debug)
            printf("matched state: %011b\n", Button1Mask | Button2Mask );
        if (button == 1)
        {
            if (debug || verbose)
                printf("Middle + Left = Return\n");
            system("xdotool key Return");
        }
        else if (button == 2)
        {
            if (debug || verbose)
                printf("Left + Middle = Snarf\n");
            system("xdotool key ctrl+c key ctrl+x");
        }
        mouse_chord_active = 1;
        return 1;
    }

    if (mouse_button_state == (Button1Mask | Button3Mask))
    {
        if (debug)
            printf("matched state: %011b\n", Button1Mask | Button3Mask );
        if (button == 1)
        {
            if (debug || verbose)
                printf("Right + Left = Undo\n");
            system("xdotool key ctrl+z");
        }
        else if (button == 3)
        {
            if (debug || verbose)
                printf("Left + Right = Paste\n");
            system("xdotool key ctrl+v");
        }
        mouse_chord_active = 1;
        return 1;
    }

    if (mouse_button_state == (Button2Mask | Button3Mask))
    {
        if (debug)
            printf("matched state: %011b\n", Button2Mask | Button3Mask );
        if (button == 3)
        {
            if (debug || verbose)
                printf("Middle + Right = Space\n");
            system("xdotool key space");
        }
        else if (button == 2)
        {
            if (debug || verbose)
                printf("Right + Middle = Redo\n");
            system("xdotool key ctrl+shift+z");
        }
        mouse_chord_active = 1;
        return 1;
    }
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


long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
