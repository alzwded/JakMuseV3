# JakMuseV3
3rd iteration on my sequencer

Why version 3?
==============

[Version 2](http://github.com/alzwded/JakMuse) has some annoying limitations. I mean V2 does its job, but it has limitations. V3 is a major rewrite/overhaul which in its initial state will do the same thing as V2, but I can later extend V3 to support some more advanced features (like variable/input-based filter frequencies, variable LFO frequency, variable gain, etc)

Also, V3 can potentially work in real time more easily since it doesn't need to read all its input in advance (which was _one_ of the many annoying limitations of V2).

V3 is the future. It can do wave tables and doesn't affraid of anything.

Oh, and I'm actually writing a GUI for V3's note input. [V2's](http://github.com/alzwded/Jakkat) was a waste of time.

What's done
===========

Synth
-----

* you have the following blocks to play around with:
  + Generator – generates a periodic signal based on a wave table; supports Glide
  + Filter – a god block which does the following:
    - gain (<1.0 only)
    - ADSR envelope
    - low & high pass filtering
    - smooth mixing of input samples
  + Delay – helps keep nets in synch
  + Input – reads note notation data or raw PCM, to be used as inputs to everything else
  + Noise – generate some noise; supports a 16bit and an 8bit generator; can hold a sample for multiple ticks to generate brown-ish/pink-ish noise (I think? anyway, noise with less high frequencies)
* you can connect the blocks via the IN and RST parameters
* you can input notes via a common variation of letter notation
* you can input raw PCM data
* you can generate a .wav file with your work of art

GUI
---

* you can edit the NOTES section of a jakmuse input file (but not directly; you will need to do a `cat instances notes end_section > jakmuseinput` since the composer can only edit the notes part
* idem PCM data
* you can save and load files (yay!)

Documentation
-------------

The closest thing I have to documentation is [the original scratch spec](https://github.com/alzwded/JakMuseV3/blob/master/jakmusev3_6.txt) and [the interpreter source code](https://github.com/alzwded/JakMuseV3/blob/master/jakmuse/blocks_interpreter.cpp).

This will need to change and have formal documentation. This is not yet done.

Roadmap
=======

- specs (more or less done)
- composer (ready)
  + GUI (ready)
  + back-end (ready)
- instrument & score parser (i.e. the I or I/O) (done)
- synthesizer (done, off-line only)
  + blocks... (done)
  + mixer... (done, part of block)
  + connections... (done, part of block)
  + clock... (done, off-line only)
- documentation (convert specs to docs) TODO
- modify "static" parameters to also be connectable to input sources (filter frequencies, gain, ADSR); or just do this from the get-go (handled)
- linux support (i.e. Makefile + ifdef away windows specific stuff) TODO
