M190 S120    ; set heatbed to 120C and wait for it
M73 P0 R5400 ; set print job timer to 0% complete, 90 min remaining
G4 S5400     ; dwell for 90 min
M140 S0      ; turn heatbed off
M73 P100 R0  ; set print job timer to 100% complete, 0 min remaining
