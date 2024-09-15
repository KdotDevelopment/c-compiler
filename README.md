# What Can It Do?
It can currently parse and create code for return expressions.\
For example: return 9 % 3 * 4 + 20;\
This will be converted to each individual math operation in x86 assembly with zero optimizations.\
It is returned via a unix syscall.
