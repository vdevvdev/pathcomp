# Path Compression Algorithm

Minimal C algorithm for compressing network routes or sequences using bitmasks.

## Features
- Inserts paths (e.g., [0,1,2]) with weights (e.g., latency).
- Compresses shared prefixes.
- Includes tests for correctness.
- Licensed under GPL v2.

## Compilation
```bash
gcc -o pathcomp pathcomp.c
