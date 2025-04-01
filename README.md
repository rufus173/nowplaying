# About

This is just a little application i made to try out connecting with dbus. The code absolutely sucks, but it at least works some of the time.
Don't count on it being maintained.

## How to use

Run it. If it doesn't work, then idk what to do. Feel free to report any bugs.
Unfortunately due to wayland semantics, always on top may not work for some desktop environments, so you may have to configure window manager rules to substitute for this.

## Configuration

You can configure the looks of the app using the style sheet in `~/.config/nowplaying/style.css`. Running `./configure` should move the provided example there if there isn't already one present.

# Install

## Requirements

qt6 (i gave up on the glib implementation)
qmake (technically optional, you can try your luck with the makefile my qmake produced)

## Installation

`./configure` (if you have qmake installed) then `./install`
