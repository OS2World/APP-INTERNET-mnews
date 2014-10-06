#!/bin/sh
cat $* | sed '/^END/,/^BEGIN/d' | uudecode
