Vedev
=====

This is a work in progress.  It's using evdev/uinput to do arbitrary keyboard remapping, with [Karabiner](https://pqrs.org/osx/karabiner/) as inspiration.  However, it's using Lua for configuration because that seems like the Right Thing.

Vedve (evdev backwards, but hard to pronounce) is the original prototype with the minimum implemented in C, and then everything else in the Lua configuration; I'd like to rewrite the core in Rust, and probably add a few more features (so common cases can be handled without calling out to Lua for performance; I assume that doing non-trivial key processing in Lua will be slow enough to be problematic.

I'm developing this on NixOS, and so have a default.nix that I'm using to build it.  I would expect it to work elsewhere without too much trouble, but haven't given it any thought.
