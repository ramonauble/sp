   sp mk 1	/|\ alpha
   by ramona a -w-
   4-19-20
   ><>|^*^|<><
   _______
   --___--
     |||___-π-___...
     |||---|||---|||
    / π \       / π \
    \___/       \___/
	

general description
------------------------------------------------------------------------------
soulplotter is an 8 bit synthesizer & sequencer, based on atmega328p
comprised of 3 identical monophonic digital synth voices (tracks)
each track has its own 1-64 step sequence, with independent lengths per track
	independently running - sequences of differing lengths can generate polymetric rhythms
	24 tick/sixteenth note structure allows for microtiming, tuplets and polyrhythmic sequences
	sequencer steps programmed via built-in chromatic keyboard
	each step represents 1 sixteenth note - yielding 4 quarter notes (16 steps) per page 
	there are 4 identical pages - yielding 4 pages of 4 quarter notes (64 steps) in total

this project has a twofold purpose;
1: for my own sake
	i love synthesizers with my whole entire heart - this project was borne out of passion
2: for the sake of others
	this project is intended in part as practical education, to communicate the miniutae of how digital synthesis works "under the hood"
	i take deliberate care in my approach to inline annotation & commenting, to make my code both unambiguous & approachable
	post-alpha i will be packaging the fundamentals of my codebase into an arduino library