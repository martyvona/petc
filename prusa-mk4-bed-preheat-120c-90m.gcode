; based on https://www.reddit.com/r/prusa3d/comments/16clrhl/comment/jzk8pgm
M84        ; steppers off
M104 S0    ; extruder off
M190 R120  ; wait for heatbed to reach 120C
M85  S5460 ; set Prusa inactivity timer to 91 min
M0   S5400 ; delay 90 min
M140 S0    ; turn heatbed off
M85  S1800 ; set Prusa inactivity timer to 30 min (default)
