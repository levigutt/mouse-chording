#!/usr/bin/perl -wln
# sudo evtest /dev/input/event19 | ./mouse-chording.pl

BEGIN {
    our %btns;
    our $target = '/dev/input/event1'; #trackpad
    #our $listen = '/dev/input/event19'; #usb-mouse
    our $re_move = qr/type 2 \(EV_REL\), code \d \((REL_(?:X|Y))\), value (-?\d+)/;
}


#/type (?:1|2)/ or next;;
$_ =~ $re_move
    and system "sudo evemu-event $target --type EV_REL --code $1 --value $2 --sync";
#next;

#
#/type 1 \(EV_KEY\), code \d+ \(BTN_(LEFT|MIDDLE|RIGHT)\), value (\d)/
#    and $btns{$1} = $2;
#
#click('BTN_LEFT')   if $btns{LEFT}   && !($btns{MIDDLE} || $btns{RIGHT});
#click('BTN_MIDDLE') if $btns{MIDDLE} && !($btns{LEFT}   || $btns{RIGHT});
#click('BTN_RIGHT')  if $btns{RIGHT}  && !($btns{LEFT}   || $btns{MIDDLE});
#
#$btns{LEFT} && $btns{MIDDLE} 
#    and system "xdotool key ctrl+x";
#$btns{LEFT} && $btns{RIGHT}
#    and system "xdotool key ctrl+v";
#
#
##print "$_ => $btns{$_}" for sort keys %btns;
#
#sub click {
#    my $button = shift;
#    system "sudo evemu-event /dev/input/event1",
#        "--type EV_KEY",
#        "--code $button",
#        "--value 1",
#        "--sync";
#    system "sudo evemu-event /dev/input/event1",
#        "--type EV_KEY",
#        "--code $button",
#        "--value 0",
#        "--sync";
#}
