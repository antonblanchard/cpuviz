# cpuviz

A simple QT program that visualizes the CPU usage of every CPU in a Linux
system.

## Building

```
qmake-qt5 cpuviz.pro
make
```

## Running

cpuviz takes two arguments, the width and height of the array of CPUs.

For example, when running on a 1536 way POWER9 we can break it down into a
48x32 array:

```
./cpuviz 48 32
```
