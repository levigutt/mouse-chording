#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <regex.h>
#include <unistd.h>
#include <X11/Xutil.h>
#include <X11/Xlib.h>

char *read_line(char *buffer);
//void mouseClick(int button, int type);

bool debug = false;
bool verbose = true;

Display *dpy;
Window root_win;
Screen *screen;

int mouse_button_state = 0;
bool mouse_chord_active = 0;


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

    // prepare regex matcher
    char *mouse_move_restr  = "type 2 [(]EV_REL[)], code [0-9] [(](REL_X|REL_Y)[)], value (-?[0-9]+)";
    char *mouse_click_restr = "type 1 [(]EV_KEY[)], code [0-9]+ [(](BTN_[A-Z]+)[)], value ([0-9])";
    size_t group_limit = 3;
    regmatch_t group_arr[group_limit];
    regex_t mouse_move_regex;
    regex_t mouse_click_regex;
    if (regcomp(&mouse_move_regex, mouse_move_restr, REG_EXTENDED)
     || regcomp(&mouse_click_regex, mouse_click_restr, REG_EXTENDED))
    {
        fprintf(stderr, "Err: Failed to compile regex.\n");
        exit(1);
    }

    // read lines from STDIN
    char *line[1024];
    while (read_line(line))
    {
        char *code[6];
        char *value[6];

        if (regexec(&mouse_move_regex, line, group_limit, group_arr, 0) == 0)
        {
            unsigned int g = 0;
            char line_copy[strlen(line) + 1];
            strcpy(line_copy, line);

            for (g = 0; g < group_limit; g++)
            {
                if (group_arr[g].rm_so == (size_t)-1)
                    break;

                line_copy[group_arr[g].rm_eo] = 0;
                if (debug)
                    printf("Group %u: [%2u-%2u]: %s\n",
                        g, group_arr[g].rm_so, group_arr[g].rm_eo,
                        line_copy + group_arr[g].rm_so);

                if (g == 1)
                    strcpy(code, line_copy + group_arr[g].rm_so);
                if (g == 2)
                    strcpy(value, line_copy + group_arr[g].rm_so);
            }

            char *long_ptr;
            long num_value = strtol(value, &long_ptr, 10);
            long mouse_x = 0;
            long mouse_y = 0;

            if (strcmp("REL_Y", code) == 0)
                mouse_y = num_value;
            else
                mouse_x = num_value;

            if (debug)
                printf("%d  x  %d\n", mouse_x, mouse_y);

            // move cursor
            XWarpPointer(dpy, None, None, 0, 0, 0, 0, mouse_x, mouse_y);
            XFlush(dpy);
        }

        // detect mouse clicks
        if (regexec(&mouse_click_regex, line, group_limit, group_arr, 0) == 0)
        {
            unsigned int g = 0;
            char line_copy[strlen(line) + 1];
            strcpy(line_copy, line);

            char *button_name[6];
            int button_code = 0;
            int button_action = 0;

            for (g = 0; g < group_limit; g++)
            {
                if (group_arr[g].rm_so == (size_t)-1)
                    break;

                line_copy[group_arr[g].rm_eo] = 0;
                if (debug)
                    printf("Group %u: [%2u-%2u]: %s\n",
                        g, group_arr[g].rm_so, group_arr[g].rm_eo,
                        line_copy + group_arr[g].rm_so);

                if (g == 1)
                    strcpy(button_name, line_copy + group_arr[g].rm_so);
                if (g == 2)
                    button_action = (strcmp("1", line_copy + group_arr[g].rm_so) == 0)
                                    ? ButtonPress
                                    : ButtonRelease;
            }


            if (strcmp("BTN_LEFT", button_name) == 0)
            {
                button_code = Button1;
                if (button_action == ButtonPress)
                    mouse_button_state |= Button1Mask;
                else
                    mouse_button_state &= ~Button1Mask;
            }
            if (strcmp("BTN_MIDDLE", button_name) == 0)
            {
                button_code = Button2;
                if (button_action == ButtonPress)
                    mouse_button_state |= Button2Mask;
                else
                    mouse_button_state &= ~Button2Mask;
            }
            if (strcmp("BTN_RIGHT", button_name) == 0)
            {
                button_code = Button3;
                if (button_action == ButtonPress)
                    mouse_button_state |= Button3Mask;
                else
                    mouse_button_state &= ~Button3Mask;
            }

            if (debug || verbose)
                printf("mouse_button_state: %011b\n", mouse_button_state);

            if (button_action == ButtonPress 
             && mouse_button_state == (Button1Mask | Button2Mask))
            {
                if (debug)
                    printf("matched state: %011b\n", Button1Mask | Button2Mask );
                if (button_code == 1)
                {
                    if (debug || verbose)
                        printf("Middle + Left = Return\n");
                    system("echo xdotool key Return");
                }
                if (button_code == 2)
                {
                    if (debug || verbose)
                        printf("Left + Middle = Snarf\n");
                    system("echo xdotool key ctrl+c key ctrl+x");
                }
                mouse_chord_active = 1;
            }

            if (button_action == ButtonPress 
             && mouse_button_state == (Button1Mask | Button3Mask))
            {
                //if (debug)
                    printf("matched state: %011b\n", Button1Mask | Button3Mask );
                if (button_code == 1)
                {
                    if (debug || verbose)
                        printf("Right + Left = Undo\n");
                    system("echo xdotool key ctrl+z");
                }
                if (button_code == 3)
                {
                    if (debug || verbose)
                        printf("Left + Right = Snarf\n");
                    system("echo xdotool key ctrl+v");
                }
                mouse_chord_active = 1;
            }

            if (button_action == ButtonPress 
             && mouse_button_state == (Button2Mask | Button3Mask))
            {
                if (debug)
                    printf("matched state: %011b\n", Button2Mask | Button3Mask );
                if (button_code == 3)
                {
                    if (debug || verbose)
                        printf("Middle + Right = Space\n");
                    system("echo xdotool key Space");
                }
                if (button_code == 2)
                {
                    if (debug || verbose)
                        printf("Right + Middle = Redo\n");
                    system("echo xdotool key ctrl+shift+z");
                }
                mouse_chord_active = 1;
            }


            if (mouse_chord_active == 0 && button_code > 0)
            {
                if (debug)
                    printf("mouse click\n button: %d\nstate: %d\n", button_code, button_action);

                char *cmd[32];
                if (button_code == 1 && button_action == ButtonPress)
                    sprintf(cmd, "xdotool mousedown %d\n", button_code);
                else if (button_code == 1)
                    sprintf(cmd, "xdotool mouseup %d\n", button_code);
                else if (button_action == ButtonRelease)
                    sprintf(cmd, "xdotool click %d\n", button_code);
                system(cmd);

                // buggy: cannot resize or change active window
                //mouseClick(button_code, button_action);
            }

            if (mouse_chord_active && mouse_button_state == 0)
            {
                mouse_chord_active = 0;
            }
        }

    }

    regfree(&mouse_move_regex);
    regfree(&mouse_click_regex);

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






//void mouseClick(int button, int type)
//{
//
//    XEvent event;
//    memset(&event, 0x00, sizeof(event));
//
//    event.type = type;
//    event.xbutton.button = button;
//    event.xbutton.same_screen = True;
//
//    XSetInputFocus(dpy, PointerRoot, RevertToNone, CurrentTime);
//
//    XQueryPointer(dpy, RootWindow(dpy, DefaultScreen(dpy)), &event.xbutton.root, &event.xbutton.window, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
//
//    event.xbutton.subwindow = event.xbutton.window;
//
//    while(event.xbutton.subwindow)
//    {
//        event.xbutton.window = event.xbutton.subwindow;
//
//        XQueryPointer(dpy, event.xbutton.window, &event.xbutton.root, &event.xbutton.subwindow, &event.xbutton.x_root, &event.xbutton.y_root, &event.xbutton.x, &event.xbutton.y, &event.xbutton.state);
//    }
//
//    if (type == ButtonRelease)
//        event.xbutton.state = 0x100;
//
//    if (XSendEvent(dpy, PointerWindow, True, 0xfff, &event) == 0)
//        fprintf(stderr, "Err: Could not send mouse click.\n");
//
//    XFlush(dpy);
//}



