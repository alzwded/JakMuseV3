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

Nothing yet. Please stand by.

Roadmap
=======

- specs (more or less done)
- composer
  + GUI (in progress)
  + back-end
- instrument & score parser (i.e. the I or I/O)
- synthesizer
  + blocks...
  + mixer...
  + connections...
  + clock...
- documentation (convert specs to docs)
- modify "static" parameters to also be connectable to input sources (filter frequencies, gain, ADSR); or just do this from the get-go
