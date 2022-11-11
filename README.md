# lightning-simulation

## Overview
This C++ SFML 2.5.1 application is an experiment project in simulating air pressure and (TODO) lightning strikes. In physics model used in this application, lightning strike goes through areas with small resistance, and in turn ionizes air (lowers resistance) and creates areas of high pressure. Different resistance air masses are mixed by wind.

## Graphics

View: pressure view

### Cell colors
* Red - high pressure
* Blue - low pressure
* Gray - ground

## Controls
### Draw modes
* 1 - pressure brush
* 2 - ground brush
* 3 - lightning (unused)

### Brush size
* Mouse wheel up - brush size up (max radius 10)
* Mouse wheel down - brush size down (min radius 1)

Brush shape: square

Default brush radius: 5

### Pressure brush
* Lmb - add pressure
* Rmb - reduce pressure

### Ground brush
* Lmb - add ground
* Rmb - remove ground

## TODO
* Lightning simulation
* Circle brush
* Air velocity view
