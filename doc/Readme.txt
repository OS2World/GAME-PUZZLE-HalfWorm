HalfWorm for OS/2
=================
Version 0.9


Overview
--------
HalfWorm is a two-player action game for OS/2 Presentation Manager.
Two worms compete on a game board: each player controls a worm that
rotates and fires bullets. Eat apples to gain special abilities.
The last worm standing wins.

Originally written by Jan M. Danielsson in 1997. This version has been
ported to OpenWatcom C by the OS2World community in 2026.


How to Play
-----------
Start the game from the Game menu or press Ctrl+N (F2).

Each player controls a worm:
  - Rotate counterclockwise
  - Rotate clockwise
  - Fire weapon

The worm always moves forward. Play a round until only one worm is left.

Eat apples scattered on the board to gain special powers. Some apples
give advantages (speed boost, better weapons); others have negative
effects. Watch out!

The game board wraps at the edges. A worm dies when it collides with
the other worm, its own body, or a wall segment.


Controls
--------
Default game keys (can be changed with Options > Keys):

                            Player 1     Player 2
  Rotate counterclockwise   A            Left arrow
  Rotate clockwise          D            Right arrow
  Fire                      W            Up arrow

Keyboard shortcuts:
  Ctrl+N / F2    New Game
  Ctrl+P         Pause / resume the game
  Ctrl+Q         Quit the current game (back to the title screen)
  Ctrl+X / F3    Exit application
  Ctrl+D         Toggle double window (4x4 pixel elements)
  Ctrl+B         Toggle background run (keep running without the focus;
                 when off the game pauses while the window is not active)
  Ctrl+F         Toggle frame controls (title bar / menu)
  F1             Help

Help
----
Online help is available in English, Spanish, Dutch, German, French and
Italian. Use the Help menu (Help index, General help, Using help) or press
F1 on a menu item or in a dialog. The help follows the language chosen in
Options > Language; if a help file is missing the English one is used.


Sound
-----
The sound effects are the WAV files in the sounds folder (mono, 16-bit,
22050 Hz); keep it next to HalfWorm.exe. You can replace them with your own
files of the same names and format. They are synthesized by tools\gen_sounds.py.


Requirements
------------
- OS/2 Warp 4 or ArcaOS 5.x
- DIVE (Direct Interface Video Extensions) - included in OS/2
- MMPM/2 (Multimedia) - for sound effects
- 8-bit (256 colour) display mode recommended


File List
---------
HalfWorm.exe    Main executable
HalfWorm.ini    Settings file (created on first run)
sounds\*.wav     Sound effects (keep the sounds folder next to HalfWorm.exe)
help\HalfWorm_en.hlp, _es, _nl, _de, _fr, _it - online help (keep the help folder
                next to HalfWorm.exe)
doc\Readme.txt  This file
doc\Changelog.txt  Version history
doc\LICENSE.txt    BSD 3-Clause License


Disclaimer
----------
This software is provided "as is" without warranty of any kind.
See doc\LICENSE.txt for the full BSD 3-Clause license text.


Author
------
Original code: Jan M. Danielsson (1997)
OS2World OpenWatcom port: OS2World community (2026)
https://www.os2world.com
