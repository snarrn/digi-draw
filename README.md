# Digi Draw
> __NOTE: This project is in its initial stages of development. Come back later for a more complete version.__

This repository serves as a guide to creating a Digi Draw, which is a small device consisting of a Raspberry Pi Pico, a small display, a couple of sensors, and some 3D-printed parts.

## Table of Contents
* [Features](#features)
* [Development Roadmap](#development-roadmap)
* [Contribution](#contribution)
* [License](#license)

## Features
Main features will be described here.

## Development Roadmap
The following tasks outline the development of the project.

### ~~Initial Project Setup~~
* ~~Create project file stucture~~
* ~~Configure and test build with pico-sdk~~
* ~~Test flashing the project to the `Pico`~~

### I/O Classes
* ~~Create a Display class to drive Waveshare's LCD~~
* Create a font and add Display.text()
* Create an Encoder class for encoder input

### App Selection
* Create an `App` base class
* Create a `DigiDraw` class that
    * Initializes I/O objects 
    * Displays an App Select screen with icons
    * Runs apps when they are selected

### File System
* Incorporate a file system for reading/writing files

### Documentation
* Reference CAD files for 3D-printed parts
* Write a full manual that describes the project's
    * Set-up procedure
    * Software (including how to add your own apps)
    * Hardware (including 3D printed parts)
* Update the README so it provides an overview of the project

## Contribution
This project's software and documentation were developed by Nathan Snarr with the use of generative AI. The CAD models were developed by Jacob Snarr. Project collaboration with people outside of Nathan Snarr's personal contacts is likely to be limited.

## License
This project is licensed under the Creative Commons Attribution 4.0
International License (CC BY 4.0). See [LICENSE.txt](LICENSE.txt)
for details.

The PIXY font data contained in
[pixy_font_5x5.h](include/pixy_font_5x5.h) is derived from the
PIXY font by 2DFUNS and is licensed separately under the
Creative Commons Attribution 3.0 License (CC BY 3.0).
See [pixy_font_5x5.h](include/pixy_font_5x5.h) and
[CC-BY-3.0.txt](licenses/CC-BY-3.0.txt) for details.