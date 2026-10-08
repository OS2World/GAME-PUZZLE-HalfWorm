# HalfWorm for OS/2

A two-player action game for OS/2 Presentation Manager. Two worms compete
on a game board, eating apples for special powers. The last worm standing wins.

Version 0.9 - OpenWatcom port by OS2World community.

## LICENSE

BSD 3-Clause

## COMPILE TOOLS

OpenWatcom C (wcc386 / wlink / wrc / wmake)

### Build Instructions

Run on ArcaOS or OS/2 Warp 4 with OpenWatcom installed:

```
compile-wat.cmd
```

Or manually:

```
wmake -f makefile.wat
```

The executable is placed in `bin\HalfWorm.exe` and the online help files (built with `wipfc`) in `bin\help\HalfWorm_xx.hlp`; keep the `help` folder next to the exe.

## FOLDER STRUCTURE

```
src\        Source files (C, RC, DEF, H)
bin\        Build output (exe, obj, res, help\*.hlp)
doc\        Documentation (Readme, Changelog, License)
help\       IPF help sources, one per language (en, es, nl, de, fr, it)
legacy\     Original source archive
```

## AUTHORS

- Jan M. Danielsson (original, 1997)
- OS2World community (OpenWatcom port, 2026)

## LINKS

- https://www.os2world.com/games/index.php/native-games/puzzle/281-halfworm
