#!/bin/bash

for f in *.png; do mv -- "$f" "${f//[!0-9a-zA-Z.-]/_}"; done

