#!/usr/bin/perl -wlnF/\s|,/
# xinput set-button-map {id} 0 0 0 0 0 0 0 0 0 0
# sudo evtest /dev/input/event19 | ./mouse-chording.pl
use strict;

our (%btns, $active_chord, $alt_down,
$left_code, $right_code, $middle_code, $scroll_code);
BEGIN
{
    %btns         = (272 => 0, 273 => 0, 274 => 0);
    $active_chord = 0;
    $alt_down     = 0;

    # mouse button codes
    $left_code   = 272;
    $right_code  = 273;
    $middle_code = 274;
    $scroll_code = 8;
}

# only handle events
next unless  0 == index $_, 'Event';
next unless -1 == index $_, 'SYN';

# extract event values
my ($time, $type, $code, $value) = @F[2, 5, 9, 13];

# skip mouse movements and hi-res scroll
next if $code == 11;
next if $type == 2 && $code != 8;

# keep track of pressed buttons
$btns{$code} = $value;

# emulate button presses
if ( ($code == $left_code   && !($btns{$right_code} || $btns{$middle_code}))
  || ($code == $right_code  && !($btns{$left_code}  || $btns{$middle_code}))
  || ($code == $middle_code && !($btns{$left_code}  || $btns{$right_code})) )
{
    if ($code == $left_code)
    {
        # left button can be held for selecting text
        system $value ? "xdotool mousedown 1&" : "xdotool mouseup 1&";
    }
    elsif($value == 0 && not $active_chord)
    {
        # middle and right will only trigger click - cannot be held
        system "xdotool click " . ($code == $middle_code ? 2 : 3) . "&";
    }

    # release alt to select from window-switcher
    system "xdotool keyup alt&";
    $alt_down = 0;
    $active_chord = 0;
    next;
}
# emulate scroll wheel
elsif ($code == 8 && !$btns{$middle_code})
{
    system( sprintf("xdotool click %d&", $value > 0 ? 4 : 5) );
}

# only do chords when button is pressed (not released)
next unless $value;

# Left + Middle = Cut
if ($btns{$left_code} && $code == $middle_code)
{
    system "xdotool mouseup 1&"; # prevent accidental selection
    system "xdotool key ctrl+c key ctrl+x&";
    $active_chord = 1;
    next;
}

# Left + Right = Paste
if ($btns{$left_code} && $code == $right_code)
{
    system "xdotool mouseup 1&"; # prevent accidental selection
    system "xdotool key ctrl+v&";
    $active_chord = 1;
    next;
}

# Middle + Left = Return
if ($btns{$middle_code} && $code == $left_code)
{
    system "xdotool key Return&";
    $active_chord = 1;
    next;
}

# Middle + Right = Space
if ($btns{$middle_code} && $code == $right_code)
{
    system "xdotool key space&";
    $active_chord = 1;
    next;
}

# Right + Left = Undo
if ($btns{$right_code} && $code == $left_code)
{
    system "xdotool key ctrl+z&";
    $active_chord = 1;
    next;
}

# Right + Middle = Redo
if ($btns{$right_code} && $code == $middle_code)
{
    system "xdotool key ctrl+shift+z&";
    $active_chord = 1;
    next;
}

# Middle + Scroll
if ($btns{$middle_code} && $code == 8)
{
    system "xdotool keydown alt&" unless $alt_down;
    $alt_down = 1;
    system sprintf("xdotool key %s&", $value > 0 ? 'shift+Tab' : 'Tab');
    $active_chord = 1;
    next;
}


END
{
    # release buttons and modifiers to prevent getting stuck
    system "xdotool mouseup $_&" for (1..3);
    system "xdotool keyup alt&";
}
