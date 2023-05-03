# dspadpcm
This repository contains source files for dspadpcm, a now discontinued audio encoding tool for Dolphin (GCN), Revolution (Wii), and Cafe (Wii U)

## What is this for?
This is a tool for converting audio files over into a readable format for GCN. Also, it is used on Wii and Wii U for some applications. This tool would be later discontinued on Nintendo Switch in the form of more traditional and modern formats. However, this still serves it's purpose for making audio files on GCN.

## How do I get right to the building?
There are 2 files for "building". There is `buildscript`, and `makefile`. When you run make, make also runs the `buildscript` file. However, the `makefile` included uses commands provided by a terminal, so you will need some sort of linux terminal (perhaps WSL?) to actually build this.

Since this version of dspadpcm is built for Revolution, you will need a directory for the Revolution SDK. There are also additional things you will need, but they are too lengthy to explain here. I suggest reading the file code to gain more information than you possibly could from this README.

# Resources
The code for dspadpcm contains contextual information for what happens with it. However, there is also an online guide for this as well. See here for official documentation: http://web.archive.org/web/20230421013408/http://hcs64.com/files/DSPADPCM.us.pdf
