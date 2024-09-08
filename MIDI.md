# Random Notes on MIDI

* Author: [Douglas P. Fields, Jr.](mailto:symbolics@lisp.engineer)
* Copyright 2024, Douglas P. Fields Jr.
* License: [Apache 2.0](https://www.apache.org/licenses/LICENSE-2.0.txt)
* Started: 2024-09-08

Documentation:
* [MIDI 1.0](https://midi.org/midi-1-0)
* [MIDI 1.0 Core Specifications](https://midi.org/midi-1-0-core-specifications)
* [MIDI 1.0 MPE](https://midi.org/midi-1-0-detailed-specification)

Some code on MIDI:
* [nanomidi](https://github.com/olemb/nanomidi)
* [C++ MIDI file parser](https://cm-gitlab.stanford.edu/craig/midifile/tree/04fd8bc23ddeb69d8f34a7adf259e5e1f2e26055)
* [midiparser](https://gitlab.com/jacobvosmaer/midiparser)

# High level

* Each byte is a status byte or data byte
  * Status has the high bit set
* Each Message is a channel or system message
* Channel messages have 4 bits for a channel 0-15 (usually called 1-16)
  * Voice or Mode
* System messages have no channels: Common, Real-time, Exclusive (SysEx)
  * Real-time are status bytes only, and can be sent anytime (!)
* Each Message can be:
  * Status
  * Status Data
  * Status Data Data
  * SysexStatus Data ... EOX
    * Any other status byte ends EXCEPT RealTime
* New status messages can cancel any incomplete previous message
  * Except RealTime status

# Running Status

* For Voice & Mode messages (ones with a Channel)
* Remains in that status until a different status byte is received
* So you can have a complete message with only data bytes
* Ends with any other status message EXCEPT a RealTime

# Channel Modes

* Omni, Poly, Mono
  * Poly disables Mono & vice-versa
  * Four possible modes: Omni On/Off, Poly/Mono - for EACH channel
* Receiver operates only under one Channel Mode a time - for EACH channel
* "It is recommended that at power-up, the basic channel should be set to 1, 
  and the mode set to Omni On/Poly (Mode 1)."
  * MIDI 1.0 spec
  
# Channel Voice Messages

Leading nibble:
* 8 - Note-off
* 9 - Note-on
* A - Poly key pressure
* B - control change ("CC")
* C - program change
* D - channel pressure
* E - pitch bend

Misc notes:
* Middle C = 60 (decimal)
* Send velocity 64 if not velocity sensitive ("mf")
* Velocity 0 is note off (in a note on message)

## Note On/Off

* First data byte is key (0-127)
* Second data byte is velocity (0-127)

# Questions


