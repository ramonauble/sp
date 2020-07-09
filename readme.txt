/*
	soulplotter alpha 1
	by ramona a
	6-19-20
	<><|^-^|><>
*/

> general description
	> soulplotter is a synthesizer & sequencer, based entirely on atmega328p
		> presently comprised of 3 identical monophonic synth voices (tracks)
		> each voice can be individually sequenced, with its own set of 18 parameters
			> giving a total of 54 parameters - 6 input pots x 3 pages x 3 tracks
		> the sequence length can be modified with fn shortcuts in realtime (while playing)
			> currently 6 shortcuts - jump to 16, 32, 48 & 64 steps, and -/+ 1 step
			> i plan to make the jumps programmable, eg, 12, 16, 18 & 20 steps - for
			  rythmic variation of melodies during performance
			> currently all tracks share the same length, but i will be changing to individual
			  lengths soon
	> this project has a threefold purpose;
		> 1: for my own sake
			> i love synthesizers with my whole entire heart - this project was borne out of passion
			> it has also given me an understanding of the fundamentals of digital synthesis
				> digital oscillators - wavetable, algorithmic & the phase accumulator
				> digital envelopes - algorithmically generated & sampled shapes
				> digital sequencing - utilizing cycle-timed interrupts to divide the clock
				  into a pulse, as a function of bpm & clock frequency
		> 2: for the sake of others
			> this project is intended in part as practical education, specifically for teaching
			  the miniutae of how digital synthesis actually works "under the hood"
			> i take great care in my approach to commenting & program structure, to make it as
			  comprehensible and approachable as i can
			> i intend to package the fundamentals of my codebase into an arduino library, to
			  give other folks accessible tools for building their own synth projects
		> 3: demonstrating my programming ability to prospective employers
			> this one is fairly self explanatory -w-
------------------------------------------------------------------------------	
> physical description:
	> 39 momentary switches
		> 17 for seq note entry / playing / alt functions
			> 1 + 5/12 of an octave; arranged chromatically
			> fn: C1, C#1 & D1 | mute tracks 1-3, respectively
			> fn: E1, F1	| decrement/increment step length (-/+ 1 step)
			> fn: F#1, G1, G#1, A1 | jump to seq length - 16, 32, 48, 64 steps respectively
		> 16 for sequencer step entry
		> 03 for param page & track selection
			> !fn: param page select mode
			> fn: track select mode
		> 02 for changing seq page (L/R) & octave shift (-/+)
			> !fn: seq page select mode
			> fn: octave shift mode
		> 01 for function (changes behavior of other switches when held)		
	> 24 leds
		> 16 blu for sequencer
		> 04 grn for seq page display
		> 03 ylo for param page & track display
		> 01 red for bpm
	> 6 potentiometers
		> for parameter input
		> 6 x 3 pages (18 in total) parameters per track
			> active page 1-3 selected using track select buttons
		> 18 x 3 tracks for a total of 54 parameters
			> active track 1-3 selected using fn + track select buttons
	> 4 74HC595 shift registers
		> 3 used for LED control (1:1 output pin mapping)
			> 24 digital outputs in total - to match number of LEDs
			> simple on/off operation
		> 1 used for implementation of switch matrix
			> 8 digital outputs x 5 digital inputs (on 328p)
				> 8 columns of 5 switches, connected together by their cathodes
					> last cathode of each column terminates at corresponding SR output
				> 5 rows of 8 switches, connected together by their anodes
					> leftmost anode of each row terminates at 328p digital input pin
				> this gives a matrix capacity of 40 discrete switches
				> last switch not used (though may be later)
			> diodes are used on each of the 8 SR outputs, to simulate an open/drain configuration
				> cathode facing each output - only allows current in
				> when the output is ON, that pin CANNOT sink current - essentially "hi-z"
				> when the output is OFF, that pin CAN sink current - hence the "drain"
			> by scanning the matrix in the main loop, the instantaneous states of each switch
			  can be determined, quickly enough to circumvent bouncing/false reads (read cycle >5ms)
				> reads all rows first, then moves to next column
				> this is faster - it allows for 5 discrete reads to take place (on each input)
				  without shifting a new byte into the register
				> only shifts new byte when entire column read - when ready to move to next column
				> thus, one full read cycle entails 8 discrete byte shifts, and 40 discrete reads

			
			
			
			
			
			
			
			
			
			
			
			