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

Eat apples scattered on the board to gain special powers. Some apples
give advantages (speed boost, better weapons); others have negative
effects. Watch out!

The game board wraps at the edges. A worm dies when it collides with
the other worm, its own body, or a wall segment.


Controls
--------
Default key assignment is done on first run. You will be prompted to
press keys for each action. Keys can be reassigned via Options > Keys.

Keyboard shortcuts:
  Ctrl+N / F2    New Game
  Ctrl+Q         Quit current game (restart)
  Ctrl+X / F3    Exit application
  Ctrl+D         Toggle double-size window
  Ctrl+B         Toggle background run
  Ctrl+F         Toggle frame controls (title bar / menu)
  F1             Help

Help
----
Online help is available in English, Spanish, Dutch, German, French and
Italian. Use the Help menu (Help index, General help, Using help) or press
F1 on a menu item or in a dialog. The help follows the language chosen in
Options > Language; if a help file is missing the English one is used.


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
