.* HalfWorm help - en
:userdoc.

:h1 res=2100.About HalfWorm
:i1.About HalfWorm
:p.
HalfWorm is a two-player worm game for OS/2 and ArcaOS, written in 1997 by Jan M. Danielsson. This version is an Open Watcom build with menus and online help in six languages, keyboard shortcuts and saved settings.
:p.
Two worms crawl around a board, grow by eating apples and shoot at each other. The last worm standing wins.
:p.
More information&colon.
:p.
:link reftype=hd res=2101.How to play:elink.
.br
:link reftype=hd res=2102.Controls:elink.
.br
:link reftype=hd res=2103.Apples and power-ups:elink.
.br
:link reftype=hd res=2104.Game menu:elink.
.br
:link reftype=hd res=2105.Options menu:elink.
.br
:link reftype=hd res=2106.Help menu:elink.
.br
:link reftype=hd res=2107.Copyright:elink.


:h1 res=2101.How to play
:i1.How to play
:ul compact.
:li.Choose New Game from the Game menu (Ctrl+N or F2) to start a round. Both worms start at the same time.
:li.Each player turns the worm counterclockwise or clockwise and fires missiles. The worm always moves forward.
:li.The board wraps around&colon. a worm that leaves one edge comes back in on the opposite edge.
:li.Eat apples to grow longer and to get special abilities. Some apples help you, others make you an easier target. See Apples and power-ups.
:li.A worm dies when it hits a worm (itself or the other one) or is hit by a missile. The last worm alive wins the round; if both die together the game is a draw. The status bar shows the result of the previous game.
:li.Pause Game (Ctrl+P) freezes the game until you choose it again. Quit Game (Ctrl+Q) abandons the current round and returns to the title screen; the round is not counted.
:eul.

:h1 res=2102.Controls
:i1.Controls
:p.
Each player uses three keys. The defaults are shown below; change them with Options - Keys. Keys are read directly from the keyboard, so several keys can be held at the same time.
:p.
:dl break=all.
:dt.Player 1 - A
:dd.Rotate counterclockwise.
:dt.Player 1 - D
:dd.Rotate clockwise.
:dt.Player 1 - W
:dd.Fire.
:dt.Player 2 - Left arrow
:dd.Rotate counterclockwise.
:dt.Player 2 - Right arrow
:dd.Rotate clockwise.
:dt.Player 2 - Up arrow
:dd.Fire.
:dt.Ctrl+N or F2
:dd.New game.
:dt.Ctrl+P
:dd.Pause / resume.
:dt.Ctrl+Q
:dd.Quit the current round.
:dt.Ctrl+X or F3
:dd.Exit HalfWorm.
:dt.Ctrl+D
:dd.Double window size on / off.
:dt.Ctrl+B
:dd.Background Run on / off.
:dt.Ctrl+F
:dd.Frame Controls - hide / show the title bar and menu.
:dt.F1
:dd.Help.
:edl.

:h1 res=2103.Apples and power-ups
:i1.Apples and power-ups
:p.
Apples appear from time to time on the board. What an apple does is a surprise, and the effect stays with the worm; the message in the status bar tells you what happened.
:p.
:dl break=all.
:dt.Speed
:dd.The worm becomes slower or faster.
:dt.Fire rate
:dd.You can shoot less or more often.
:dt.Missile speed
:dd.Your missiles travel slower or faster.
:dt.Launcher
:dd.You get an upgraded launcher (more missiles at a time) or lose one.
:dt.Explosions
:dd.Missile explosions become bigger or smaller.
:dt.Bouncing
:dd.Missiles bounce off the edges more or less often.
:dt.Homing missiles
:dd.Missiles steer towards the other worm.
:dt.Warping missiles
:dd.Missiles wrap around the board like the worms.
:dt.Drunken missiles
:dd.Missiles wander unpredictably.
:dt.Auto fire
:dd.The worm keeps firing by itself.
:dt.Pacifist
:dd.You cannot fire until the effect is cancelled.
:dt.Wriggle
:dd.The worm wriggles and is harder to steer.
:dt.Fatal attraction
:dd.The worm is drawn towards its foe.
:dt.Reset
:dd.All effects are cleared and the worm is as new.
:dt.Infested
:dd.An apple full of worms - it has no effect.
:edl.

:h1 res=2104.Game menu
:i1.Game menu
:dl break=all.
:dt.New Game (Ctrl+N, F2)
:dd.Starts a new round.
:dt.Pause Game (Ctrl+P)
:dd.Stops the game until you choose it again.
:dt.Quit Game (Ctrl+Q)
:dd.Abandons the current round and returns to the title screen. Only available during a game.
:dt.Exit (Ctrl+X, F3)
:dd.Closes HalfWorm. The settings are saved if Save settings on exit is on.
:edl.

:h1 res=2105.Options menu
:i1.Options menu
:dl break=all.
:dt.Board Size...
:dd.Sets the width and height of the board in pixels. Maximize for desktop and Maximize for double size fit the board to the screen.
:dt.Keys...
:dd.Opens the key mapping dialog for both players.
:dt.Double window (Ctrl+D)
:dd.Shows the game at twice the size.
:dt.Sound effects
:dd.Enabled turns the sound on or off; Volume sets the level.
:dt.Language
:dd.Changes the language of the menus and of this help. Available&colon. English, Espanol, Nederlands, Deutsch, Francais, Italiano.
:dt.Background Run (Ctrl+B)
:dd.The game keeps running while another window is active.
:dt.Frame Controls (Ctrl+F)
:dd.Hides or shows the title bar and the menu. Press Ctrl+F again to bring them back.
:dt.Save settings on exit
:dd.Saves the options, the keys and the language to HalfWorm.ini when you exit. On by default.
:edl.

:h1 res=2106.Help menu
:i1.Help menu
:dl break=all.
:dt.Help index
:dd.Shows the index of this help.
:dt.General help
:dd.Shows the introduction.
:dt.Using help
:dd.Explains how to use the help window.
:dt.About...
:dd.Shows the version and copyright. F1 shows help for the selected menu item.
:edl.

:h1 res=2107.Copyright
:i1.Copyright
:p.
HalfWorm for OS/2, version 0.9
:p.
Copyright (C) 1997 Jan M. Danielsson. Open Watcom build and extensions, 2026, OS2World.
:p.
Released as open source under the BSD 3-Clause License. This software comes with no warranty.
:p.
Sound effects and the DIVE video interface require MMPM/2 and a DIVE capable display driver.

:h1 res=2108.Board size
:i1.Board size
:p.
Sets the size of the playing board in pixels. Width and Height are the two values. Maximize for desktop makes the board as large as the screen allows; Maximize for double size does the same when the window is shown at double size. OK applies the new size, Cancel keeps the old one. A new size starts a new round.

:h1 res=2109.Key mapping
:i1.Key mapping
:p.
Choose the three keys of each player&colon. counterclockwise rotation (CCW), clockwise rotation (CW) and fire. Click a button and press the key you want to use. Defaults restores the standard keys. OK saves the keys, Cancel discards the changes.

:h1 res=2110.Sound effects volume
:i1.Sound effects volume
:p.
Sets the volume of the sound effects from 0 to 100. OK applies the new volume, Cancel keeps the old one.

:euserdoc.
