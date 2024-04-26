# clowncd

A collection of libraries and tools for interacting with Compact Disc images.

`clowncd-cue-converter` converts a CUE file to a small binary header. Prefixing
this header to a raw Mega CD disc image (2352-bytes-per-sector BIN file) yields
a file which can be used with clownmdemu.

`clowncd-test` extracts a disc image to a CUE file, and assorted ISO and OGG
files.
