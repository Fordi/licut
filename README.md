Release notes
28-Jan-2011

Currently, licut is tested on Ubuntu and works with Cricut Cake.
To enable additional debugging messages, run with --verbose=1
To just check communication with the Cricut unit, run licut
without any options.

To cut an Inkscape SVG file, run licut [options] svg-file

This is very early alpha - check back for updates

Binary tarballs are provided for x86-linux and for arm-linux
For other platforms, check out from subversion and build using
make -C src TARGET=<toolchain-prefix>
