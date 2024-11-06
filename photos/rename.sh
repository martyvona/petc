#!/bin/bash

# get rid of spaces in filenames
# including the unicode "narrow no-break space"
# that OS X incredibly throws in there by default

for f in *.png; do mv -- "$f" "${f//[!0-9a-zA-Z.-]/_}"; done

