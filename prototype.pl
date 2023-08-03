#!/usr/bin/perl -wlnF/\s|,/
# sudo evtest /dev/input/event19 | ./prototype.pl
use Time::HiRes qw<time>;

BEGIN
{
    our $max_lag = 0.05; # increase for precision, decrease for speed
    our $target  = '/dev/input/event1'; # trackpad

    our %btns = (272 => 0, 273 => 0, 274 => 0);
    our %move;
    our $active_chord = 0;
}

# only handle events
next unless 0 == index $_, 'Event';
next unless -1 == index $_, 'SYN';

#print "$_ => $F[$_]" for keys @F;
#next;

my $time  = $F[2];
my $type  = $F[5];
my $code  = $F[9];
my $value = $F[13];

next if $code == 11; # HiRes scroll
if ($code == 8)      # LoRes scroll
{
    system( sprintf("xdotool click %d&", $value > 0 ? 4 : 5) );
    next;
}

if ($type == 2)
{
    # batch movements
    $move{$code}+= $value;

    # run batched movements when lagging less than 0.1
    if ($time > (time)-$max_lag)
    {
        system "sudo evemu-event $target --type 2 --code $_ --value $move{$_} --sync&" for keys %move;
        %move = ();
    }
    next;
}


# keep track of pressed buttons
$btns{$code} = $value;

# 272 = Left
# 273 = Right
# 274 = Middle

# emulate button presses
if ( ($code == 272 && !($btns{273} || $btns{274}))
  || ($code == 273 && !($btns{272} || $btns{274}))
  || ($code == 274 && !($btns{272} || $btns{273})) )
{
    if ($code == 272)
    {
        # left button can be held for selecting text
        system "sudo evemu-event $target --type 1 --code $code --value $value --sync&";
    }
    elsif($value == 0 && not $active_chord)
    {
        # middle and right will only trigger click - cannot be held
        system "sudo evemu-event $target --type 1 --code $code --value 1 --sync&";
        system "sudo evemu-event $target --type 1 --code $code --value 0 --sync&";
    }
    $active_chord = 0;
    next;
}

next unless $value; # all chords happen on press (not release)

# Chording syntax
#    click on Right      while Left is down
#   ( $code == 273   &&  $btns{272}        )

# Left + Middle = Snarf
if ($code == 274 && $btns{272})
{
    system "xdotool key ctrl+c key ctrl+x&";
    $active_chord = 1;
    next;
}

# Left + Right = Paste
if ($code == 273 && $btns{272})
{
    system "xdotool key ctrl+v&";
    $active_chord = 1;
    next;
}

# Right + Left = Undo
if ($code == 272 && $btns{273})
{
    system "xdotool key ctrl+z&";
    $active_chord = 1;
    next;
}

# Right + Middle = Redo
if ($code == 274 && $btns{273})
{
    system "xdotool key ctrl+shift+z&";
    $active_chord = 1;
    next;
}


END
{
    # release buttons to prevent getting stuck
    system "sudo evemu-event $target --type 1 --code 272 --value 0 --sync&";
    system "sudo evemu-event $target --type 1 --code 273 --value 0 --sync&";
    system "sudo evemu-event $target --type 1 --code 274 --value 0 --sync&";
}
