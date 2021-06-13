# Dodecahedron implementation of chess rules

This directory contains the chess rules implementation taken from
[Dodecahedron](https://github.com/alex65536/dodecahedron/) chess engine. This implementation
is used as a baseline for the tests. So, the output of the routines in SoFCheck (like move
generating, move making, etc.) are compared with the output of Dodecahedron.

Dodecahedron implementation of chess rules is considered correct by me, and it there were some
bugs, it's more probably that they are in a different part of code, so it would be easily caught
and fixed.

By the way, the code in this directory is licensed under GPLv3 (or any later version), see `LICENSE`
for details.
