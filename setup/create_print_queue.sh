#!/bin/sh
# Copyright (c) 2026 Haiku. 
# 
# Author: 
# 	Kevin Adams, kevinadams05@gmail.com
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is furnished to do
# so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


# This replaces the queue that Haiku used to have by default. 
# See Haiku Trac #20110.

set -e

PRINTERS_DIR="$HOME/config/settings/printers"
QUEUE_DIR="$PRINTERS_DIR/Save as PDF"

mkdir -p "$PRINTERS_DIR"

if [ -d "$QUEUE_DIR" ]; then
	exit 0
fi

mkdir "$QUEUE_DIR"

addattr -t mime_string  "BEOS:TYPE"          "application/x-vnd.Be.printer" "$QUEUE_DIR"
addattr                 "Printer Name"       "Save as PDF"                  "$QUEUE_DIR"
addattr                 "Driver Name"        "PDF Writer"                   "$QUEUE_DIR"
addattr                 "transport"          "Print To File"                "$QUEUE_DIR"
addattr                 "transport_address"  ""                             "$QUEUE_DIR"
addattr                 "connection"         "Local"                        "$QUEUE_DIR"
addattr                 "state"              "free"                         "$QUEUE_DIR"
