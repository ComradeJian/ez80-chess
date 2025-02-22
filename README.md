# eZ80 Chess

A compact chess engine designed specifically for the TI-84 Plus CE calculator (which runs on the eZ80 processor). The engine employs efficient memory management and optimized algorithms to deliver strong chess play within hardware constraints.

## Building

If you would like to build this project, be sure you have the latest LLVM [CE C Toolchain](https://github.com/CE-Programming/toolchain/releases/latest) installed.

Then simply clone or download the repository from above and run the following commands:

```
make
```

The files below will be compiled and should be sent to the calculator:

|                 |              |
| --------------- | ------------ |
| bin/Chess.8xp   | Main Program |