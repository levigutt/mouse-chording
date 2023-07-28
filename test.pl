#!/usr/bin/perl -wln
# sudo evtest /dev/input/event19 | ./mouse-chording.pl

our $buttons;
/type 1/ or next;
/type 1 \(EV_KEY\), code \d+ \(BTN_(LEFT|MIDDLE|RIGHT)\), value (\d)/
    and $buttons{$1} = $2;

$buttons{LEFT} && $buttons{MIDDLE} 
    and system "xdotool key ctrl+x";
$buttons{LEFT} && $buttons{RIGHT}
    and system "xdotool key ctrl+v";


#print "$_ => $buttons{$_}" for sort keys %buttons;
