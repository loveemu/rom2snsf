ROM2SNSF: SNES ROM to SNSF Converter
====================================
[![Travis Build Status](https://travis-ci.com/loveemu/rom2snsf.svg?branch=master)](https://travis-ci.com/loveemu/rom2snsf) [![AppVeyor Build Status](https://ci.appveyor.com/api/projects/status/hny4sxbralmkawpn/branch/master?svg=true)](https://ci.appveyor.com/project/loveemu/rom2snsf/branch/master)

Program to turn a ROM (or any binary files) into a SNSF file. This is used mostly with manual rips.

Downloads
---------

- [Latest release](https://github.com/loveemu/rom2snsf/releases/latest)

Usage
-----

Syntax: `rom2snsf <SNES Files>`

### Options ###

`--help`
  : Show help

`-o [output.snsf]`
  : Specify output filename

`--load [offset]`
  : Load offset of SNES executable

`--lib [libname.snsflib]`
  : Specify snsflib library name

`--psfby [name]` (aka. `--snsfby`)
  : Set creator name of SNSF
