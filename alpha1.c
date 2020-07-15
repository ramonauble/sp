/*
   my son mk 1 /|\ alpha
   by ramona a -w-
   4-19-20
   ><>|^*^|<><
   --___--
     |||___-π-___...
     |||---|||---|||
    / π \       / π \
    \___/       \___/
*/

//pragma for bit level operations
//--------------------------------------------------------------------------------
#define clearRegBit(reg, bit) (_SFR_BYTE(reg) &= ~_BV(bit))
#define setRegBit(reg, bit) (_SFR_BYTE(reg) |= _BV(bit))

//BEGIN declarations
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
const float sFreq = 31372.54902; //sampling frequency in hz - as determined by (clockFreq/510) (phase correct PWM, no prescaler)
                                 //as the clock frequency is 16mhz, 16,000,000/510 ~= 31372.5490196
                                 //this is the rate at which the ISR for timer 2 executes
                                 //once every 510 clock cycles, giving ~31372.549 ISR calls per second

//tuning words:
//  -/+ 4 octaves around middle a (440hz - 919) - calculated as oscFreq/sFreq
//  the accumulators themselves are unsigned integers, with a range of 0-65535
//  every time the ISR is called (31372.549 times per second), the tuning word is added to the phase accumulator
//  the tuning word itself is essentially a phase measurement, representing a definite portion of a single waveform cycle
//  thus, the size of the tuning word relative to the max size of the accumulator determines how quickly the acc. overflows
//  the rate at which the "phase" is accumulated is constant (sFreq), so changing the tuning word changes the speed of accumulator overflow (hz)
//  this is how the oscillators are cycle-timed - per the picycle -w-
//--------------------------------------------------------------------------------------------------------
volatile unsigned int tuningWords[] { //0-95
  57, 60, 64, 68, 72, 76, 81, 86, 91, 96, 102, 108,
  114, 121, 128, 136, 144, 153, 162, 172, 182, 193, 204, 216,
  229, 243, 257, 273, 289, 306, 324, 344, 364, 386, 409, 433,
  459, 486, 515, 546, 579, 613, 649, 688, 729, 772, 818, 867,
  919, 973, 1031, 1093, 1158, 1226, 1299, 1377, 1459, 1545, 1637, 1735, //middle a - 919
  1838, 1947, 2063, 2186, 2316, 2453, 2599, 2754, 2918, 3091, 3275, 3470,
  3676, 3895, 4126, 4372, 4632, 4907, 5199, 5508, 5836, 6183, 6550, 6940,
  7353, 7790, 8253, 8744, 9264, 9815, 10398, 11017, 11672, 12366, 13101, 13880
};


//I/O pin declarations
//--------------------------------------------------------------------------------------------------------
byte shiftSerOut = 6;     //shift out pin for all shift register operations
byte shiftLatch = 9;      //shift latch pin (normally low; set high->low after full 32bit shift to latch outputs)
byte shiftClock = 10;     //shift clock pin (normally low; set high->low to shift in next bit

byte matrixRow1 = 2;      //switch matrix pins
byte matrixRow2 = 4;      //connected to matrix rows 1-5 in turn
byte matrixRow3 = 7;
byte matrixRow4 = 8;
byte matrixRow5 = 12;
byte matrixByte = 255;    //determines which matrix columns are ON/OFF (for column selection during read cycle)

byte outputRight = 11;    //right channel output pin (pwm, timer2 OC2B (chip pin 17))
byte outputLeft = 3;      //left channel output pin (pwm, timer2 OC2A (chip pin 5))

byte paramPins[] = {      //pin declarations for analog inputs
  A0, A1, A2, A3, A4, A5
};


//BPM declarations:
//  bpm is taken as qnps (quarter notes per second) - currently with a range of 0 to 1023
//  note that there are 4x16th notes per quarter note, and 3x1/8th note triplets per quarter note
//  therefore a timing resolution of -12/+11 "ticks" per 16th note allows for representation of both duplets and triplets on the sequencer
//  16th notes: 24 ticks | 8th note triplets: 32 ticks | and other variations
//------------------------------------------------------------------------------------
//  so... 120bpm = 120qnpm = 2qnps = 8sixteenth_nps = 192tps (ticks per second)
//    as bpm changes, per params3_T1[5], tps changes, effectively changing the size of the tick "groups" in samples
//  as there are exactly sFreq samples in one second, (sFreg/tps) divides the second into tps "groups" of sptCounterMax samples
//  sptCounter is incremented once per sample, and is reset on overflow (when it reaches sptCounterMax)
//  each overflow of sptCounter corresponds to a single "tick" of the sequencer - thus triggering a tick advance
//  all this together relates the bpm to the rate at which the sequencer is incremented
//    and thus, the sequencer is given a heartbeat <3
//--------------------------------------------------------------------------------------------------------
volatile float bpm = 120.0;                             //init to 120bpm
volatile float tps = ((bpm / 60.0) * 4 * 24);           //ticks per second - to relate the sequencer to the clock - 16th note resolution
volatile unsigned int sptCounterMax = int(sFreq / tps); //truncates fractional component - samples per tick
volatile unsigned int sptCounter = 0;                   //current sample - only counts tick on overflow - samples per trig


//parameter delcarations
//--------------------------------------------------------------------------------------------------------
//track 1 (rightmost)
//--------------------------------------------------------------------------------
unsigned int params3_T1[] = {    //track 1 parameter page 3 (rightmost)
  48, 48, 48, 48, 48, 48
};
unsigned int params2_T1[] = {    //track 1 parameter page 2
  48, 48, 48, 48, 48, 48
};
unsigned int params1_T1[] = {    //track 1 parameter page 1 (leftmost)
  48, 48, 48, 48, 48, 48
};
//track 2 (middle)
//--------------------------------------------------------------------------------
unsigned int params3_T2[] = {    //track 2 parameter page 3
  48, 48, 48, 48, 48, 48
};
unsigned int params2_T2[] = {    //track 2 parameter page 2
  48, 48, 48, 48, 48, 48
};
unsigned int params1_T2[] = {    //track 2 parameter page 1
  48, 48, 48, 48, 48, 48
};
//track 3 (leftmost)
//--------------------------------------------------------------------------------
unsigned int params3_T3[] = {    //track 3 parameter page 3
  48, 48, 48, 48, 48, 48
};
unsigned int params2_T3[] = {    //track 3 parameter page 2
  48, 48, 48, 48, 48, 48
};
unsigned int params1_T3[] = {    //track 3 parameter page 1
  48, 48, 48, 48, 48, 48
};


//parameter input variables:
//  top 2 MSB's of pgChange [7-6] are not used
//  bottom 6 bits [5-0] represent change state for each param pot [1-6] (buffer indeces [0-5]) (left to right)
//  a state of 1 for a given pot means the corresponding parameter buffer value WILL NOT be assigned to the associated param
//  a state of 0 for a given pot means the corresponding parameter buffer value WILL be assigned to the associated param
//--------------------------------------------------------------------------------
//  initially, all are 0, as sp (currently) boots to track 1, page 3 - params already "caught up" w/ input buffer at start
//  bits 0-5 are set (pgChange == Bxx111111) on EITHER of the following two events:
//  1. param page change
//  2. track select change
//--------------------------------------------------------------------------------
//  each bit (flag) is cleared individually for the associated param if EITHER of the following conditions are met:
//  1. if the sign bit (bit 15, MSB) of (paramsBuff[n] - params(page)_T(track)[n]) != (prevParamsBuff[n] - params(page)_T(track)[n])
//    which is to say... if the parameter input for the selected track/page PASSES whatever the value was previously
//  2. if the most recently read value (paramsBuff[n]) is EQUAL TO the value of the parameter being modified (params(page)_T(track)[n])
//    really just catching the outlier cases... in case the parameter control resumes from one of the L/R bounaries
//--------------------------------------------------------------------------------
//  when the bit is cleared, params(page)_T(track)[n] is updated from the buffer
//  it then proceeds to be updated EVERY LOOP, until the corresponding bit flag is set again
//  after a track or param page change, ALL bit flags are set (Bxx111111)
//  thus, the parameters play "catch up" - to prevent sudden jumps in value when changing the selected track/page
//--------------------------------------------------------------------------------------------------------
unsigned int paramsBuff[] = {
  0, 0, 0, 0, 0, 0
};
unsigned int prevParamsBuff[] = {
  0, 0, 0, 0, 0, 0
};
byte pgChange = B00000000;


//sequencer trackers
//--------------------------------------------------------------------------------------------------------
//track 1
//--------------------------------------------------------------------------------
byte seqTrackerT1[] = {               //for led display of active step (when seq playing)
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};
byte *seqTrackerT1_P = seqTrackerT1;  //pointer to start of tracker array
//track 2
//--------------------------------------------------------------------------------
byte seqTrackerT2[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};
byte *seqTrackerT2_P = seqTrackerT2;
//track 3
//--------------------------------------------------------------------------------
byte seqTrackerT3[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};
byte *seqTrackerT3_P = seqTrackerT3;

//sequencer pages
//--------------------------------------------------------------------------------------------------------
//track 1
//--------------------------------------------------------------------------------
byte seqPagesT1[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};
//track 2
//--------------------------------------------------------------------------------
byte seqPagesT2[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};
//track 3
//--------------------------------------------------------------------------------
byte seqPagesT3[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

//sequencer notes
//--------------------------------------------------------------------------------------------------------
//track 1
//--------------------------------------------------------------------------------
char seqNotesT1[] = {     //holds the note stored on each step - as a signed 8 bit value [-128, 127]
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};
//track 2
//--------------------------------------------------------------------------------
char seqNotesT2[] = {     //this is essentially a semitone offset from A-440
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};
//track 3
//--------------------------------------------------------------------------------
char seqNotesT3[] = {     //which is used to address the tuning word lookup table (to set the pitch of each oscillator)
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

//sequencer offsets
//--------------------------------------------------------------------------------------------------------
//track 1
//--------------------------------------------------------------------------------
byte seqOffsT1[] = {  //tick offset for each step - default is center (tracks 1-3)
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
//track 2
//--------------------------------------------------------------------------------
byte seqOffsT2[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
//track 3
//--------------------------------------------------------------------------------
byte seqOffsT3[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//sequencer variables
//--------------------------------------------------------------------------------------------------------
volatile bool seqPlay = 0;  //sequencer state: 1 for playing | 0 for stopped (default) | changed by play/stop buttons
byte page = 1;              //currently selected seq page

//track 1
//--------------------------------------------------------------------------------
byte seqPageA_T1 = 0;       //indeces for sequencer page addressing - since each page is actually 2 bytes within the array
byte seqPageB_T1 = 1;       //when a new page is selected, both indeces need to be changed, to properly address the sequencer bytes
byte seqStepT1 = 0;         //currently active step of the sequencer (0-63) - 0 when not playing
volatile unsigned int seqTicksT1 = 0;   //present tick position of sequencer
volatile unsigned int maxTicksT1 = 384; //determines sequence length before reset (max steps == maxTicks/24)
volatile byte stepIndexT1 = 0;          //0-23 - current inter-step index (0 when stopped)
volatile byte seqLimT1 = 1;             //last group before reset - starting from 0
volatile byte seqLengthT1 = 16;         //length of sequence in steps
//track 2
//--------------------------------------------------------------------------------
byte seqPageA_T2 = 0;
byte seqPageB_T2 = 1;
byte seqStepT2 = 0;
volatile unsigned int seqTicksT2 = 0;
volatile unsigned int maxTicksT2 = 384;
volatile byte stepIndexT2 = 0;
volatile byte seqLimT2 = 1;
volatile byte seqLengthT2 = 16;
//track 3
//--------------------------------------------------------------------------------
byte seqPageA_T3 = 0;
byte seqPageB_T3 = 1;
byte seqStepT3 = 0;
volatile unsigned int seqTicksT3 = 0;
volatile unsigned int maxTicksT3 = 384;
volatile byte stepIndexT3 = 0;
volatile byte seqLimT3 = 1;
volatile byte seqLengthT3 = 16;


//input detection variables:
//  passed to input processing function updateUI(keyVal) only if pressed (input read as 0)
//--------------------------------------------------------------------------------------------------------
byte keyVal = 0;       //numerical value of current key being read from the matrix (top->down, left->right)
byte matrixState[] = { //instantaneous states of all buttons - 5x8 grid - 0-38 (last not used)
  B00000000, //0-7
  B00000000, //8-15
  B00000000, //16-23
  B00000000, //24-31
  B00000000  //32-38 (last bit (39) not used)
};
byte matrixStateB[] = { //buffer for matrix state - for input filtering (edge detection) - updateUI called only once per press
  B00000000, //0-7
  B00000000, //8-15
  B00000000, //16-23
  B00000000, //24-31
  B00000000  //32-38 (last bit (39) not used)
};

//note entry variables
//--------------------------------------------------------------------------------------------------------
char noteVal = 3; //note value offset used to calculate pitch from tuningRef - init to 3 since keyboard starts on C4
char noteValT1 = seqNotesT1[0];
char noteValT2 = seqNotesT2[0];
char noteValT3 = seqNotesT3[0];


//stereo phase accumulators
//--------------------------------------------------------------------------------------------------------
//track 1
//--------------------------------------------------------------------------------
volatile unsigned int phaseAccT1_L; //stereo phase accumulators for each track
volatile unsigned int phaseAccT1_R;
volatile unsigned int tWordT1_L = tuningWords[48 + noteValT1]; //independent tuning words for each accumulator
volatile unsigned int tWordT1_R = tuningWords[48 + noteValT1]; //each tuning word is calculated as (2^16) * (freqTn/sFreq)
volatile byte phaseT1_L;                                       //PWM phase (T1) (volatile for ISR access)
volatile byte phaseT1_R;                                       //OCR2A & OCR2B, respectively
byte ampT1_L = 255;        //instantaneous L osc amplitude - init to ON
byte ampT1_R = 255;        //instantaneous R osc amplitude - init to ON
byte pulseWidthT1_L = 128; //pulse width of L waveform - state change in algorithms (init to half width)
byte pulseWidthT1_R = 128; //pulse width of R waveform - state change in algorithms (init to half width)
//track 2
//--------------------------------------------------------------------------------
volatile unsigned int phaseAccT2_L;                            //unsigned int is a 16 bit type
volatile unsigned int phaseAccT2_R;                            //so the accumulator resolution is 2 bytes
volatile unsigned int tWordT2_L = tuningWords[48 + noteValT2]; //the tuning word is essentially a fraction of the full
volatile unsigned int tWordT2_R = tuningWords[48 + noteValT2]; //accumulator resolution
volatile byte phaseT2_L;  
volatile byte phaseT2_R;  
byte ampT2_L = 255; 
byte ampT2_R = 255; 
byte pulseWidthT2_L = 128; 
byte pulseWidthT2_R = 128; 
//track 3
//--------------------------------------------------------------------------------
volatile unsigned int phaseAccT3_L; 
volatile unsigned int phaseAccT3_R; 
volatile unsigned int tWordT3_L = tuningWords[48 + noteValT3]; 
volatile unsigned int tWordT3_R = tuningWords[48 + noteValT3]; 
volatile byte phaseT3_L;  
volatile byte phaseT3_R;  
byte ampT3_L = 255;
byte ampT3_R = 255;
byte pulseWidthT3_L = 128;
byte pulseWidthT3_R = 128;


//envelope variables
//--------------------------------------------------------------------------------------------------------
//track 1
//--------------------------------------------------------------------------------
volatile byte env_valT1 = 0;             //byte which represents the instantaneous state of the envelope for T1 - bitwise &ed w/ volT1 in ISR
volatile bool env_trigT1 = 0;            //1 when envelope starts and while running; set to 0 at end of env (env_ValT1 == 0)
volatile unsigned int env_cntT1 = 0;
volatile unsigned int env_speedT1 = 255; //init to 3/4 max speed
//track 2
//--------------------------------------------------------------------------------
volatile byte env_valT2 = 0;             
volatile bool env_trigT2 = 0;
volatile unsigned int env_cntT2 = 0;
volatile unsigned int env_speedT2 = 255; 
//track 3
//--------------------------------------------------------------------------------
volatile byte env_valT3 = 0; 
volatile bool env_trigT3 = 0;
volatile unsigned int env_cntT3 = 0;
volatile unsigned int env_speedT3 = 255;


//track mute variables
//--------------------------------------------------------------------------------------------------------
bool muteT1 = 1;      //flags to represent the individual mute states of each track
bool muteT2 = 1;      //1 is normal volume (as determined by rest of output function); 0 is muted
bool muteT3 = 1;      //all tracks init to unmuted (muteValTn = 255);
byte muteValT1 = 255; //all mute vals B11111111 - for binary (output on/off) bitwise AND operation between muteValTn & the value of
byte muteValT2 = 255; //OCR2A/B just before assignment (last in order of operations)
byte muteValT3 = 255;


//output filters (not currently used)
//--------------------------------------------------------------------------------------------------------
volatile int outputFilter_L[] {
  0, 0
};
volatile int outputFilter_R[] {
  0, 0
};


//LED state variables
//--------------------------------------------------------------------------------------------------------
byte ledState[] = {  //instantaneous states of all LEDs
  B10000011,         //pag 1-4 | trk 1-3 | bpm 1) (SR 3) (bit 0 is LSB - bpm)
  B11111111,         //seq leds 9-16 (SR 2)
  B11111111          //seq leds 1-8 (SR 1) (bit 7 is MSB - seq 1)
};


//misc. variables
//--------------------------------------------------------------------------------------------------------
byte fn = 0;  //instantaneous state of function key (for accessing alternate functions)
char octaveOffset = 0;  //offset in +/- |
byte paramsPage = 3; //active (selected) parameter page
volatile byte trackSelect = 1; //active (selected) track (for param assignment)
bool T1_hasSwitched = 1;  //track 1 is starting track, so flag instantly set
bool T2_hasSwitched = 0;  //T2 and T3 flags init to 0; set to 1 when corresponding track switched to
bool T3_hasSwitched = 0;  //only set once - really only to ensure proper pot behavior
bool ledOn = 1;

//END declarations
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------


//BEGIN setup()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void setup() {
  clearRegBit (TIMSK0, TOIE0);  //disables timer 0 overflow interrupt
  clearRegBit (TIMSK1, TOIE1);  //disables timer 0 overflow interrupt
  setRegBit (TIMSK2, TOIE2);    //enables timer 2 overflow interrupt

  setRegBit (TCCR2B, CS20);     //prescaler to 1
  clearRegBit (TCCR2B, CS21);   //this means that in phase correct mode, the timer will reset every 510 clock cycles
  clearRegBit (TCCR2B, CS22);   //16.000.000/510 = 31372.54902hz - this is how the sampling frequency is derived
                                //since the interrupt is executed at the end of every full timer cycle

  clearRegBit (TCCR2A, COM2A0); //configures both OC2A & OC2B to clear compare match
  setRegBit (TCCR2A, COM2A1);   //when TCNT2 reaches OCR2A/B when counting up, output A/B is cleared
  clearRegBit (TCCR2A, COM2B0); //when TCNT2 reaches OCR2A/B when counting down, output A/B is set
  setRegBit (TCCR2A, COM2B1);

  setRegBit (TCCR2A, WGM20);    //configures phase correct pwm for both OCR2A & OCR2B
  clearRegBit (TCCR2A, WGM21);
  clearRegBit (TCCR2B, WGM22);

  pinMode(shiftSerOut, OUTPUT); //serial output pin
  pinMode(shiftLatch, OUTPUT);  //shift register latch pin
  pinMode(shiftClock, OUTPUT);  //shift register clock pin

  pinMode(matrixRow1, INPUT_PULLUP); //configures matrix input pins with internal pullup resistors
  pinMode(matrixRow2, INPUT_PULLUP); //active low - if input HIGH, pin not sinked to ground
  pinMode(matrixRow3, INPUT_PULLUP); //when input is LOW sp knows that pin is being sinked (pressed)
  pinMode(matrixRow4, INPUT_PULLUP); //changes state of matrix cell to reflect detected input
  pinMode(matrixRow5, INPUT_PULLUP);

  pinMode(outputLeft, OUTPUT);  //OCR2A & OCR2B outputs
  pinMode(outputRight, OUTPUT); //serve as L and R mix outputs

  for (int p = 0; p <= 5; p++) {  //initializes analog inputs 0-5 for parameter entry
    pinMode(paramPins[p], INPUT);
  }

  Serial.end(); //force end all serial com - for speed
}
//END setup()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------


//BEGIN loop()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void loop() {
  for (int pLoop = 0; pLoop <= 5; pLoop++) {
    //read instantaneous level of active parameter pot (as determined by pLoop)
    //assign pot value read by ADC to corresponding location (0-5) in the parameter buffer
    //--------------------------------------------------------------------------------
    int paramRead = analogRead(paramPins[pLoop]); //the analog voltage read directly from the input
    prevParamsBuff[pLoop] = paramsBuff[pLoop];    //saves previous value for sign change detection
    paramsBuff[pLoop] = (1024 - paramRead);       //compensates for inverted pot read (1023 is fully counterclockwise, and 0 is fully clockwise)
    int paramsAvg = paramsBuff[pLoop];            //assign newly acquired parameter value to variable

    //if active pgChange bit == 1 (no assign):
    //  compare active param value (as determined by pLoop) of currently selected track and page to those stored in parameter buffer
    //  clear pgChange flag of corresponding pot (0-5) when the parameter value either PASSES (sign change of difference) or EQUALS the buffer value
    //if active pgChange bit == 0 (assign):
    //  update active param value (as determined by pLoop) of the currently selected track and page (assign paramsAvg to location)
    //--------------------------------------------------------------------------------
    //page 3 - rightmost
    //------------------------------------------------------------
    if (paramsPage == 3) {
      if (bitRead(pgChange, (5 - pLoop))) {
        if (trackSelect == 1) {
          if ((bitRead(int(prevParamsBuff[pLoop] - params3_T1[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params3_T1[pLoop]), 15)) || (paramsAvg == params3_T1[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) { //fn not pressed - normal behavior
              bitClear(pgChange, (5 - pLoop));
            }
          }
        } else if (trackSelect == 2) {
          if ((bitRead(int(prevParamsBuff[pLoop] - params3_T2[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params3_T2[pLoop]), 15)) || (paramsAvg == params3_T2[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        } else {
          if ((bitRead(int(prevParamsBuff[pLoop] - params3_T3[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params3_T3[pLoop]), 15)) || (paramsAvg == params3_T3[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        }
      } else {
        if (!fn) {
          if (trackSelect == 1) {
            params3_T1[pLoop] = paramsAvg;
          } else if (trackSelect == 2) {
            params3_T2[pLoop] = paramsAvg;
          } else {
            params3_T3[pLoop] = paramsAvg;
          }
        }
      }
    //page 2 - middle
    //------------------------------------------------------------
    } else if (paramsPage == 2) {
      if (bitRead(pgChange, (5 - pLoop))) {
        if (trackSelect == 1) {
          if ((bitRead(int(prevParamsBuff[pLoop] - params2_T1[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params2_T1[pLoop]), 15)) || (paramsAvg == params2_T1[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        } else if (trackSelect == 2) {
          if ((bitRead(int(prevParamsBuff[pLoop] - params2_T2[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params2_T2[pLoop]), 15)) || (paramsAvg == params2_T2[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        } else {
          if ((bitRead(int(prevParamsBuff[pLoop] - params2_T3[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params2_T3[pLoop]), 15)) || (paramsAvg == params2_T3[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        }
      } else {
        if (!fn) {
          if (trackSelect == 1) {
            params2_T1[pLoop] = paramsAvg;
          } else if (trackSelect == 2) {
            params2_T2[pLoop] = paramsAvg;
          } else {
            params2_T3[pLoop] = paramsAvg;
          }
        }
      }
    //page 1 - leftmost
    //------------------------------------------------------------
    } else {
      if (bitRead(pgChange, (5 - pLoop))) {
        if (trackSelect == 1) {
          if ((bitRead(int(prevParamsBuff[pLoop] - params1_T1[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params1_T1[pLoop]), 15)) || (paramsAvg == params1_T1[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        } else if (trackSelect == 2) {
          if ((bitRead(int(prevParamsBuff[pLoop] - params1_T2[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params1_T2[pLoop]), 15)) || (paramsAvg == params1_T2[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        } else {
          if ((bitRead(int(prevParamsBuff[pLoop] - params1_T3[pLoop]), 15) != bitRead(int(paramsBuff[pLoop] - params1_T3[pLoop]), 15)) || (paramsAvg == params1_T3[pLoop])) { //only updates when caught up w/ pot - on sign change
            if (!fn) {
              bitClear(pgChange, (5 - pLoop));
            }
          }
        }
      } else {
        if (!fn) {
          if (trackSelect == 1) {
            params1_T1[pLoop] = paramsAvg;
          } else if (trackSelect == 2) {
            params1_T2[pLoop] = paramsAvg;
          } else {
            params1_T3[pLoop] = paramsAvg;
          }
        }
      }
    }
  }

  //port mapping for I/O:
  //------------------------------------------------------------
  //sr out - digital output 6 - port D, bit 6
  //sr clk - digital output 9 - port B, bit 1
  //sr ltc - digital output 10 - port B, bit 2 :3
  //2, 4, 7, 8, 12 - matrix rows 1-5 pin mapping
  //2 -  PD2 | 4 - PD4 | 7 - PD7 | 8 - PB0 | 12 - PB4

  //update BPM
  //--------------------------------------------------------------------------------
  bpm = params3_T1[5] + 10;         //gets new bpm as a function of the instantaneous state of params3_T1[5] (rightmost, track 1, page 3)
  tps = ((bpm / 60.0) * 4 * 24);    //calculates the number of sequencer "ticks" in one second, as a function of bpm
  sptCounterMax = int(sFreq / tps); //calculates the maximum counter value to relate the sequencer tick frequency to the sampling (interrupt) frequency

  //get input from all switches & call input handling function updateUI as needed (for each detected press)
  //--------------------------------------------------------------------------------
  for (int colCount = 0; colCount <= 7; colCount++) {

    //matrixByte
    //  bits 7-0 are cleared in turn (as determined by colCount), to select the active switch matrix column to be read
    //  shiftByte then shifts the column byte in once per colCount loop (8 times in total), to read every switch in the matrix
    //ledState[1-3]
    //  set elsewhere (LED states are changed in several locations throughout the program, for varying reasons)
    //  instantaneous LED states are shifted back in after each matrixByte shift, to ensure LEDs are always current between matrix reads
    //------------------------------------------------------------
    bitWrite(matrixByte, (7 - colCount), 0);                         //clear bit to check correct column
    shiftByte(matrixByte, shiftSerOut, shiftClock, shiftLatch);      //reg for matrix column config
    shiftByte(ledState[0], shiftSerOut, shiftClock, shiftLatch); //leds 17-24 (pag | trk | bpm)
    shiftByte(ledState[1], shiftSerOut, shiftClock, shiftLatch); //leds 9-16 (seq 16-9)
    shiftByte(ledState[2], shiftSerOut, shiftClock, shiftLatch); //leds 1-8 (seq 1-8)
    bitSet(PORTB, 1);                                            //latch shift registers after all bytes shifted in

    //checks each matrix row in turn by performing 5 consecutive digital reads on matrix rows 1-5 (pins 2, 4, 7, 8 and 12)
    //on first pass - pressed:
    //  sets matrix & matrix buffer states simultaneously for active key value (as determined by matrix row and colum  (matrixByte state)) - the matrix buffer prevents duplicate presses from being detected
    //  calls updateUI with the instantaneous key value when a press is detected (as determined by matrix row and column (matrixByte state)) - this is the input handling operation
    //on second pass - releasesd:
    //  clears matrix & matrix buffer states simultaneously for active key value (as determined by matrix row and colum  (matrixByte state))
    //------------------------------------------------------------
    //matrix row 1 (pin 2)
    //----------------------------------------
    if (!digitalRead(matrixRow1)) {                     //if matrix pin is sinked to ground (voltage 0, logic 1 - button pressed)
      if (!bitRead(matrixStateB[0], (7 - colCount))) {    //if matrix buffer state for current row and column was previously NOT pressed (logic 0)
        bitSet(matrixState[0], (7 - colCount));             //set the button state to pressed (logic 1)
        bitSet(matrixStateB[0], (7 - colCount));            //set the buffer state to pressed (logic 1) - to avoid repeat presses
        updateUI(keyVal);                                   //call updateUI to handle button press
      }
    } else {                                            //if matrix pin is not sinked to ground (voltage 1, logic 0 - button released)
      bitClear(matrixState[0], (7 - colCount));           //clear the button state to NOT pressed (logic 0)
      bitClear(matrixStateB[0], (7 - colCount));          //clear the buffer state to NOT pressed (logic 0) - to prepare for next press
    }
    keyVal++;                                             //keyVal++ after each read, to keep track of current switch being read
    //matrix row 2 (pin 4)
    //----------------------------------------
    if (!digitalRead(matrixRow2)) {
      if (!bitRead(matrixStateB[1], (7 - colCount))) {
        bitSet(matrixState[1], (7 - colCount));
        bitSet(matrixStateB[1], (7 - colCount));
        updateUI(keyVal);
      }
    } else if (keyVal == 16) {  //fn not pressed
      if (bitRead(matrixStateB[1], (7 - colCount))) { //if fn was previously pressed
        bitClear(matrixState[1], (7 - colCount));
        bitClear(matrixStateB[1], (7 - colCount));
        updateUI(keyVal);
      }
    } else {
      bitClear(matrixState[1], (7 - colCount));
      bitClear(matrixStateB[1], (7 - colCount));
    }
    keyVal++;
    //matrix row 3 (pin 7)
    //----------------------------------------
    if (!digitalRead(matrixRow3)) {
      if (!bitRead(matrixStateB[2], (7 - colCount))) {
        bitSet(matrixState[2], (7 - colCount));
        bitSet(matrixStateB[2], (7 - colCount));
        updateUI(keyVal);
      }
    } else {
      bitClear(matrixState[2], (7 - colCount));
      bitClear(matrixStateB[2], (7 - colCount));
    }
    keyVal++;
    //matrix row 4 (pin 8)
    //----------------------------------------
    if (!digitalRead(matrixRow4)) {
      if (!bitRead(matrixStateB[3], (7 - colCount))) {
        bitSet(matrixState[3], (7 - colCount));
        bitSet(matrixStateB[3], (7 - colCount));
        updateUI(keyVal);
      }
    } else {
      bitClear(matrixState[3], (7 - colCount));
      bitClear(matrixStateB[3], (7 - colCount));
    }
    keyVal++;
    //matrix row 5 (pin 12)
    //----------------------------------------
    if (!digitalRead(matrixRow5)) {
      if (!bitRead(matrixStateB[4], (7 - colCount))) {
        bitSet(matrixState[4], (7 - colCount));
        bitSet(matrixStateB[4], (7 - colCount));
        updateUI(keyVal);
      }
    } else {
      bitClear(matrixState[4], (7 - colCount));
      bitClear(matrixStateB[4], (7 - colCount));
    }
    keyVal++;

    //advance to next matrix column
    //------------------------------------------------------------
    bitWrite(matrixByte, (7 - colCount), 1); //set bit in prep for next column read
  }

  //return to start of button matrix read cycle (key 0, sequencer button 1)
  //--------------------------------------------------------------
  keyVal = 0;

  //update sequencer tracker:
  //  sets instantaneous state of sequencer LED bytes
  //  as a function of both the current tracker state (for the active track) & the state of the selected sequencer track page (1-4)
  //--------------------------------------------------------------
  if (trackSelect == 1) {
    ledState[2] = (seqPagesT1[seqPageA_T1] | seqTrackerT1[seqPageA_T1]);
    ledState[1] = (seqPagesT1[seqPageB_T1] | seqTrackerT1[seqPageB_T1]);
  } else if (trackSelect == 2) {
    ledState[2] = (seqPagesT2[seqPageA_T2] | seqTrackerT2[seqPageA_T2]);
    ledState[1] = (seqPagesT2[seqPageB_T2] | seqTrackerT2[seqPageB_T2]);
  } else {
    ledState[2] = (seqPagesT3[seqPageA_T3] | seqTrackerT3[seqPageA_T3]);
    ledState[1] = (seqPagesT3[seqPageB_T3] | seqTrackerT3[seqPageB_T3]);
  }
}
//END loop()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------


//BEGIN shiftByte()
//--------------------------------------------------------------------------------------------------------
//shifts one byte out on the given serial output pin, LSB first
//ex. shiftOut(B10001000, 6, 10, 9) would shift out the given byte on pin 6
//using clock pin 10 and latch pin 9 as a part of the shift operation
//--------------------------------------------------------------------------------------------------------
void shiftByte(byte n, byte serOut, byte srClk, byte srLatch) {
  clearRegBit(PORTB, 1);  //take clock pin low to ensure good starting state
  clearRegBit(PORTB, 2);  //take latch pin low to prepare for shift

  for (int shiftCnt = 0; shiftCnt <= 7; shiftCnt++) {
    if (bitRead(n, shiftCnt)) { //if 1
      setRegBit(PORTD, 6);
    } else {
      clearRegBit(PORTD, 6);
    }
    digitalWrite(srClk, HIGH); //pulse clock pin to shift in bit
    digitalWrite(srClk, LOW);  //can't be port manip - too fast for for shift registers
  }
}
//END shiftByte()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------


//BEGIN updateUI()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void updateUI(byte keyVal) {
  switch (keyVal) {
    case 16: //FN key changed state
      if (!fn) { //if fn was pressed (rising edge)
        fn = 1;
        if (trackSelect == 1) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 1);
        } else if (trackSelect == 2) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 2);
        } else {
          bitClear(ledState[0], 2);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 3);
        }
      } else {  //if fn was released (falling edge)
        fn = 0;
        if (paramsPage == 3) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 1);
        } else if (paramsPage == 2) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 2);
        } else {
          bitClear(ledState[0], 2);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 3);
        }
      }
      break;
    case 17: //seq stop
      seqPlay = 0; //stop sequencer
      seqTrackerT1[seqStepT1 >> 3] = 0; //clear current T1 tracker state
      seqTrackerT2[seqStepT2 >> 3] = 0; //clear current T2 tracker state
      seqTrackerT3[seqStepT3 >> 3] = 0; //clear current T3 tracker state
      if (trackSelect == 1) {
        ledState[2] = (seqPagesT1[seqPageA_T1] | seqTrackerT1[seqPageA_T1]);
        ledState[1] = (seqPagesT1[seqPageB_T1] | seqTrackerT1[seqPageB_T1]);
      } else if (trackSelect == 2) {
        ledState[2] = (seqPagesT2[seqPageA_T2] | seqTrackerT2[seqPageA_T2]);
        ledState[1] = (seqPagesT2[seqPageB_T2] | seqTrackerT2[seqPageB_T2]);
      } else {
        ledState[2] = (seqPagesT3[seqPageA_T3] | seqTrackerT3[seqPageA_T3]);
        ledState[1] = (seqPagesT3[seqPageB_T3] | seqTrackerT3[seqPageB_T3]);
      }
      seqStepT1 = 0;  //reset step T1 counter - restarts sequencer by step
      seqStepT2 = 0;  //reset step T2 counter - restarts sequencer by step
      seqStepT3 = 0;  //reset step T3 counter - restarts sequencer by step
      break;
    case 18: //seq play
      seqStepT1 = 0;
      seqTicksT1 = 0;
      stepIndexT1 = 0;
      seqTrackerT1[0] = B10000000; //reset current T1 tracker state

      seqStepT2 = 0;
      seqTicksT2 = 0;
      stepIndexT2 = 0;
      seqTrackerT2[0] = B10000000; //reset current T2 tracker state

      seqStepT3 = 0;
      seqTicksT3 = 0;
      stepIndexT3 = 0;
      seqTrackerT3[0] = B10000000; //reset current T3 tracker state

      sptCounter = 0;
      seqPlay = 1; //start sequencer
      break;
    case 19:
      if (!fn) { //shift seq 1 page (16 steps) to the right (fn == 0)
        if (page == 1) { //page 0 to 1
          page = 2;
          bitClear(ledState[0], 7);
          bitSet(ledState[0], 6);
          seqPageA_T1 = 2;
          seqPageB_T1 = 3;
          seqPageA_T2 = 2;
          seqPageB_T2 = 3;
          seqPageA_T3 = 2;
          seqPageB_T3 = 3;
        } else if (page == 2) { //page 1 to 2
          page = 3;
          bitClear(ledState[0], 6);
          bitSet(ledState[0], 5);
          seqPageA_T1 = 4;
          seqPageB_T1 = 5;
          seqPageA_T2 = 4;
          seqPageB_T2 = 5;
          seqPageA_T3 = 4;
          seqPageB_T3 = 5;
        } else if (page == 3) { //page 2 to 3
          page = 4;
          bitClear(ledState[0], 5);
          bitSet(ledState[0], 4);
          seqPageA_T1 = 6;
          seqPageB_T1 = 7;
          seqPageA_T2 = 6;
          seqPageB_T2 = 7;
          seqPageA_T3 = 6;
          seqPageB_T3 = 7;
        } else {                //page 3 to 0
          page = 1;
          bitClear(ledState[0], 4);
          bitSet(ledState[0], 7);
          seqPageA_T1 = 0;
          seqPageB_T1 = 1;
          seqPageA_T2 = 0;
          seqPageB_T2 = 1;
          seqPageA_T3 = 0;
          seqPageB_T3 = 1;
        }
      } else {  //shift keyboard 1 octave (12 st) to the right (fn == 1)
        if (octaveOffset < 2) {
          octaveOffset++;
        }
      }
      break;
    case 20:
      if (fn == 0) {  //shift seq 1 page (16 steps) to the left (fn == 0)
        if (page == 1) {        //page 1 to 4
          page = 4;
          bitClear(ledState[0], 7);
          bitSet(ledState[0], 4);
          seqPageA_T1 = 6;
          seqPageB_T1 = 7;
          seqPageA_T2 = 6;
          seqPageB_T2 = 7;
          seqPageA_T3 = 6;
          seqPageB_T3 = 7;
        } else if (page == 2) { //page 2 to 1
          page = 1;
          bitClear(ledState[0], 6);
          bitSet(ledState[0], 7);
          seqPageA_T1 = 0;
          seqPageB_T1 = 1;
          seqPageA_T2 = 0;
          seqPageB_T2 = 1;
          seqPageA_T3 = 0;
          seqPageB_T3 = 1;
        } else if (page == 3) { //page 3 to 2
          page = 2;
          bitClear(ledState[0], 5);
          bitSet(ledState[0], 6);
          seqPageA_T1 = 2;
          seqPageB_T1 = 3;
          seqPageA_T2 = 2;
          seqPageB_T2 = 3;
          seqPageA_T3 = 2;
          seqPageB_T3 = 3;
        } else {                //page 4 to 3
          page = 3;
          bitClear(ledState[0], 4);
          bitSet(ledState[0], 5);
          seqPageA_T1 = 4;
          seqPageB_T1 = 5;
          seqPageA_T2 = 4;
          seqPageB_T2 = 5;
          seqPageA_T3 = 4;
          seqPageB_T3 = 5;
        }
      } else {  //shift keyboard 1 octave (12 st) to the left (fn == 1)
        if (octaveOffset > -3) {
          octaveOffset--;
        }
      }
      break;
    case 21: //param select (fn == 0) OR track select (fn == 1) - RIGHTMOST
      if (fn == 0) {
        if (paramsPage != 3) {
          pgChange = B11111111;
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 1);
          paramsPage = 3;
        }
      } else {
        trackSelect = 1;
        bitClear(ledState[0], 3);
        bitClear(ledState[0], 2);
        bitSet(ledState[0], 1);
        pgChange = B11111111;
      }
      break;
    case 22:  //param select (fn == 0) OR track select (fn == 1) - MIDDLE
      if (fn == 0) {
        if (paramsPage != 2) {
          pgChange = B11111111;
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 2);
          paramsPage = 2;
        }
      } else {
        trackSelect = 2;
        bitClear(ledState[0], 3);
        bitClear(ledState[0], 1);
        bitSet(ledState[0], 2);
        pgChange = B11111111;
      }
      break;
    case 23:  //param select (fn == 0) OR track select (fn == 1) - LEFTMOST
      if (fn == 0) {
        if (paramsPage != 1) {
          pgChange = B11111111;
          bitClear(ledState[0], 1);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 3);
          paramsPage = 1;
        }
      } else {
        trackSelect = 3;
        bitClear(ledState[0], 2);
        bitClear(ledState[0], 1);
        bitSet(ledState[0], 3);
        pgChange = B11111111;
      }
      break;
    default: //EITHER sequencer buttons [0-15] OR keyboard buttons [24-39]
      if (keyVal >= 0 && keyVal < 16) { //sequencer buttons pressed
        if (keyVal < 8) { //seq 1-8
          if (bitRead(ledState[2], (7 - (keyVal % 8)))) {
            bitClear(ledState[2], (7 - (keyVal % 8)));
            if (trackSelect == 1) {
              bitClear(seqPagesT1[seqPageA_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitClear(seqPagesT2[seqPageA_T2], (7 - (keyVal % 8)));
            } else {
              bitClear(seqPagesT3[seqPageA_T3], (7 - (keyVal % 8)));
            }

          } else {
            bitSet(ledState[2], (7 - (keyVal % 8)));
            if (trackSelect == 1) {
              bitSet(seqPagesT1[seqPageA_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitSet(seqPagesT2[seqPageA_T2], (7 - (keyVal % 8)));
            } else {
              bitSet(seqPagesT3[seqPageA_T3], (7 - (keyVal % 8)));
            }
          }
        } else { //seq 9-16
          if (bitRead(ledState[1], (7 - (keyVal % 8)))) {
            bitClear(ledState[1], (7 - (keyVal % 8)));
            if (trackSelect == 1) {
              bitClear(seqPagesT1[seqPageB_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitClear(seqPagesT2[seqPageB_T2], (7 - (keyVal % 8)));
            } else {
              bitClear(seqPagesT3[seqPageB_T3], (7 - (keyVal % 8)));
            }
          } else {
            bitSet(ledState[1], (7 - (keyVal % 8)));
            if (trackSelect == 1) {
              bitSet(seqPagesT1[seqPageB_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitSet(seqPagesT2[seqPageB_T2], (7 - (keyVal % 8)));
            } else {
              bitSet(seqPagesT3[seqPageB_T3], (7 - (keyVal % 8)));
            }
          }
        }
      } else { //can only be keyboard input
        if (keyVal < 34) { //bottom keyboard row
          noteVal = 3 + (33 - keyVal) + (12 * octaveOffset); //offset of 3 to account for C start
          if ((keyVal == 31) && (fn == 1)) { //if bottom c pressed while fn held
            if (muteValT1 == 0) {
              muteValT1 = 255;
            } else {
              muteValT1 = 0;
            }
          } else if ((keyVal == 32) && (fn == 1)) { //if bottom c pressed while fn held
            if (muteValT2 == 0) {
              muteValT2 = 255;
            } else {
              muteValT2 = 0;
            }
          } else if ((keyVal == 33) && (fn == 1)) { //if bottom c pressed while fn held
            if (muteValT3 == 0) {
              muteValT3 = 255;
            } else {
              muteValT3 = 0;
            }
          } else if ((keyVal == 27) && (fn == 1)) { //f# - seq set to 16 step length
            seqPlay = 0; //stop sequencer
            switch (trackSelect) {
              case 1: //track 1
                seqLengthT1 = 16; //change sequencer length
                maxTicksT1 = 384;
                seqLimT1 = 1;
                if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                  seqStepT1 = 0;  //reset sequencer
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000; //clear current tracker state
                }
                break;
              case 2: //track 2
                seqLengthT2 = 16; //change sequencer length
                maxTicksT2 = 384;
                seqLimT2 = 1;
                if (seqTicksT2 > maxTicksT2) { //protects against overflow of tracker on length change
                  seqStepT2 = 0;  //reset sequencer
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000; //clear current tracker state
                }
                break;
              case 3: //track 3
                seqLengthT3 = 16; //change sequencer length
                maxTicksT3 = 384;
                seqLimT3 = 1;
                if (seqTicksT3 > maxTicksT3) { //protects against overflow of tracker on length change
                  seqStepT3 = 0;  //reset sequencer
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000; //clear current tracker state
                }
                break;
            }
            seqPlay = 1; //restart sequencer
          } else if ((keyVal == 26) && (fn == 1)) { //g - seq set to 32 step length
            seqPlay = 0; //stop sequencer
            switch (trackSelect) {
              case 1: //track 1
                seqLengthT1 = 32; //change sequencer length
                maxTicksT1 = 768;
                seqLimT1 = 3;
                if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                  seqStepT1 = 0;  //reset sequencer
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000; //clear current tracker state
                }
                break;
              case 2: //track 1
                seqLengthT2 = 32; //change sequencer length
                maxTicksT2 = 768;
                seqLimT2 = 3;
                if (seqTicksT2 > maxTicksT2) { //protects against overflow of tracker on length change
                  seqStepT2 = 0;  //reset sequencer
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000; //clear current tracker state
                }
                break;
              case 3: //track 1
                seqLengthT3 = 32; //change sequencer length
                maxTicksT3 = 768;
                seqLimT3 = 3;
                if (seqTicksT3 > maxTicksT3) { //protects against overflow of tracker on length change
                  seqStepT3 = 0;  //reset sequencer
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000; //clear current tracker state
                }
                break;
            }
            seqPlay = 1; //restart sequencer
          } else if ((keyVal == 25) && (fn == 1)) { //g# - seq set to 48 step length
            seqPlay = 0; //stop sequencer
            switch (trackSelect) {
              case 1: //track 1
                seqLengthT1 = 48; //change sequencer length
                maxTicksT1 = 1152;
                seqLimT1 = 5;
                if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                  seqStepT1 = 0;  //reset sequencer
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000; //clear current tracker state
                }
                break;
              case 2: //track 1
                seqLengthT2 = 48; //change sequencer length
                maxTicksT2 = 1152;
                seqLimT2 = 5;
                if (seqTicksT2 > maxTicksT2) { //protects against overflow of tracker on length change
                  seqStepT2 = 0;  //reset sequencer
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000; //clear current tracker state
                }
                break;
              case 3: //track 1
                seqLengthT3 = 48; //change sequencer length
                maxTicksT3 = 1152;
                seqLimT3 = 5;
                if (seqTicksT3 > maxTicksT3) { //protects against overflow of tracker on length change
                  seqStepT3 = 0;  //reset sequencer
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000; //clear current tracker state
                }
                break;
            }
            seqPlay = 1; //restart sequencer
          } else if ((keyVal == 24) && (fn == 1)) { //a - seq set to 64 step length
            seqPlay = 0; //stop sequencer
            switch (trackSelect) {
              case 1: //track 1
                seqLengthT1 = 64; //change sequencer length
                maxTicksT1 = 1536;
                seqLimT1 = 7;
                if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                  seqStepT1 = 0;  //reset sequencer
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000; //clear current tracker state
                }
                break;
              case 2: //track 1
                seqLengthT2 = 64; //change sequencer length
                maxTicksT2 = 1536;
                seqLimT2 = 7;
                if (seqTicksT2 > maxTicksT2) { //protects against overflow of tracker on length change
                  seqStepT2 = 0;  //reset sequencer
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000; //clear current tracker state
                }
                break;
              case 3: //track 1
                seqLengthT3 = 64; //change sequencer length
                maxTicksT3 = 1536;
                seqLimT3 = 7;
                if (seqTicksT3 > maxTicksT3) { //protects against overflow of tracker on length change
                  seqStepT3 = 0;  //reset sequencer
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000; //clear current tracker state
                }
                break;
            }
            seqPlay = 1; //restart sequencer
          } else if ((keyVal == 29) && (fn == 1)) { //f - seq length decremented by 1 step (24 ticks); min. length 1 (24 ticks)
            switch (trackSelect) {
              case 1: //track 1
                if (maxTicksT1 > 24) { //don't do anything if step length is 1
                  seqPlay = 0; //stop sequencer
                  seqLengthT1 = seqLengthT1 - 1;
                  maxTicksT1 = maxTicksT1 - 24;
                  seqLimT1 = (maxTicksT1 - 1) / 192; //integer division - truncates fractional component
                  if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                    seqStepT1 = 0;  //reset sequencer
                    seqTicksT1 = 0;
                    stepIndexT1 = 0;
                    seqTrackerT1[0] = B10000000; //clear current tracker state
                  }
                  seqPlay = 1; //restart sequencer
                }
                break;
              case 2: //track 1
                if (maxTicksT2 > 24) { //don't do anything if step length is 1
                  seqPlay = 0; //stop sequencer
                  seqLengthT2 = seqLengthT2 - 1;
                  maxTicksT2 = maxTicksT2 - 24;
                  seqLimT2 = (maxTicksT2 - 1) / 192; //integer division - truncates fractional component
                  if (seqTicksT2 > maxTicksT2) { //protects against overflow of tracker on length change
                    seqStepT2 = 0;  //reset sequencer
                    seqTicksT2 = 0;
                    stepIndexT2 = 0;
                    seqTrackerT2[0] = B10000000; //clear current tracker state
                  }
                  seqPlay = 1; //restart sequencer
                }
                break;
              case 3: //track 1
                if (maxTicksT3 > 24) { //don't do anything if step length is 1
                  seqPlay = 0; //stop sequencer
                  seqLengthT3 = seqLengthT3 - 1;
                  maxTicksT3 = maxTicksT3 - 24;
                  seqLimT3 = (maxTicksT3 - 1) / 192; //integer division - truncates fractional component
                  if (seqTicksT3 > maxTicksT3) { //protects against overflow of tracker on length change
                    seqStepT3 = 0;  //reset sequencer
                    seqTicksT3 = 0;
                    stepIndexT3 = 0;
                    seqTrackerT3[0] = B10000000; //clear current tracker state
                  }
                  seqPlay = 1; //restart sequencer
                }
                break;
            }
          } else if ((keyVal == 28) && (fn == 1)) { //e - seq length incremented by 1 step (24 ticks); max. length 64 (1536 ticks)
            switch (trackSelect) {
              case 1: //track 1
                if (maxTicksT1 < 1536) { //don't do anything if step length is 64
                  seqPlay = 0; //stop sequencer
                  seqLengthT1 = seqLengthT1 + 1;
                  maxTicksT1 = maxTicksT1 + 24;
                  seqLimT1 = (maxTicksT1 - 1) / 192; //integer division - truncates fractional component
                  if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                    seqStepT1 = 0;  //reset sequencer
                    seqTicksT1 = 0;
                    stepIndexT1 = 0;
                    seqTrackerT1[0] = B10000000; //clear current tracker state
                  }
                  seqPlay = 1; //restart sequencer
                }
                break;
              case 2: //track 1
                if (maxTicksT2 < 1536) { //don't do anything if step length is 1
                  seqPlay = 0; //stop sequencer
                  seqLengthT2 = seqLengthT2 + 1;
                  maxTicksT2 = maxTicksT2 + 24;
                  seqLimT2 = (maxTicksT2 - 1) / 192; //integer division - truncates fractional component
                  if (seqTicksT2 > maxTicksT2) { //protects against overflow of tracker on length change
                    seqStepT2 = 0;  //reset sequencer
                    seqTicksT2 = 0;
                    stepIndexT2 = 0;
                    seqTrackerT2[0] = B10000000; //clear current tracker state
                  }
                  seqPlay = 1; //restart sequencer
                }
                break;
              case 3: //track 1
                if (maxTicksT3 < 1536) { //don't do anything if step length is 1
                  seqPlay = 0; //stop sequencer
                  seqLengthT3 = seqLengthT3 + 1;
                  maxTicksT3 = maxTicksT3 + 24;
                  seqLimT3 = (maxTicksT3 - 1) / 192; //integer division - truncates fractional component
                  if (seqTicksT3 > maxTicksT3) { //protects against overflow of tracker on length change
                    seqStepT3 = 0;  //reset sequencer
                    seqTicksT3 = 0;
                    stepIndexT3 = 0;
                    seqTrackerT3[0] = B10000000; //clear current tracker state
                  }
                  seqPlay = 1; //restart sequencer
                }
                break;
            }
          }
        } else { //can only be top keyboard row
          noteVal = 3 + (keyVal - 24) + (12 * octaveOffset);
        }
        switch (trackSelect) {
          case 1:
            if (!fn) { //only trigger note if fn not held
              env_valT1 = 0; //reset envelope (silence note before tword change)
              noteValT1 = noteVal; //set note val of active track (T1) to most recently received key note value
              tWordT1_L = tuningWords[noteValT1 + 48]; //looks up new tuning word in tword LUT (addressed by noteVal)
              tWordT1_R = tWordT1_L;
              if (muteValT1 != 0) { //only resets envelope level if not muted - else key is effectively not trigged
                env_valT1 = 255; //reset envelope
              }
              env_cntT1 = 0;  //resets envelope counter
              env_trigT1 = 1; //triggers envelope in next ISR cycle - sound now ready :3
              for (int chkSeqs = 0; chkSeqs < 16; chkSeqs++) {
                if (bitRead(matrixState[chkSeqs % 5], 7 - (chkSeqs / 5))) { //checks state of each seq. button and assigns note if pressed
                  seqNotesT1[chkSeqs + (16 * (page - 1))] = noteValT1; //assigns read note value to pressed seq butt. (page depending)
                  bitSet(seqPagesT1[2 * (page - 1) + (chkSeqs >> 3)], (7 - chkSeqs % 8)); //turns seq step back on
                  bitSet(ledState[2 - (chkSeqs >> 3)], (7 - chkSeqs % 8)); //turns led back on
                }
              }
            }
            break;
          case 2:
            if  (!fn) {
              env_valT2 = 0; //reset envelope
              noteValT2 = noteVal;
              tWordT2_L = tuningWords[noteValT2 + 48];
              tWordT2_R = tWordT2_L;
              if (muteValT2 != 0) { //only resets envelope level if not muted
                env_valT2 = 255; //reset envelope
              }
              env_cntT2 = 0;
              env_trigT2 = 1;
              for (int chkSeqs = 0; chkSeqs < 16; chkSeqs++) {
                if (bitRead(matrixState[chkSeqs % 5], 7 - (chkSeqs / 5))) {
                  seqNotesT2[chkSeqs + (16 * (page - 1))] = noteValT2;
                  bitSet(seqPagesT2[2 * (page - 1) + (chkSeqs >> 3)], (7 - chkSeqs % 8));
                  bitSet(ledState[2 - (chkSeqs >> 3)], (7 - chkSeqs % 8));
                }
              }
            }
            break;
          case 3:
            if (!fn) {
              env_valT3 = 0; //reset envelope
              noteValT3 = noteVal;
              tWordT3_L = tuningWords[noteValT3 + 48];
              tWordT3_R = tWordT3_L;
              if (muteValT3 != 0) { //only resets envelope level if not muted
                env_valT3 = 255; //reset envelope
              }
              env_cntT3 = 0;
              env_trigT3 = 1;
              for (int chkSeqs = 0; chkSeqs < 16; chkSeqs++) {
                if (bitRead(matrixState[chkSeqs % 5], 7 - (chkSeqs / 5))) {
                  seqNotesT3[chkSeqs + (16 * (page - 1))] = noteValT3;
                  bitSet(seqPagesT3[2 * (page - 1) + (chkSeqs >> 3)], (7 - chkSeqs % 8));
                  bitSet(ledState[2 - (chkSeqs >> 3)], (7 - chkSeqs % 8));
                }
              }
            }
            break;
        }
      }
      break;
  }
}
//END updateUI()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------

//BEGIN ISR()
//--------------------------------------------------------------------------------------------------------
//interrupt service routine - executes sFreq times per second
//all DSP happens here - oscillators, envelopes, mixer & output
//sequencers also happen here
//--------------------------------------------------------------------------------------------------------
ISR(TIMER2_OVF_vect) {
  if (sptCounter < sptCounterMax) { //counts to sptCounterMax, then advances sequencer by 1 tick & resets sptCounter
    sptCounter++;
  } else {                          //sptCounter overflow - trigger sequencer tick advance & reset sptCounter
    if (((seqTicksT1 % 96) == 0) && (!bitRead(ledState[0], 0))) {     //if start of quarter note - divisible by 96 ticks
      bitSet(ledState[0], 0);                                         //turn on BPM LED
    } else if (bitRead(ledState[0], 0) && ((seqTicksT1 % 96) > 12)) { //if bpm led on & not first 16th note
      bitClear(ledState[0], 0);                                       //turn off - doesn't turn on again til next quarter note
    }
    sptCounter = 0; //reset sptCounter (to prepare for next tick)

    //track 1 sequencer
    //--------------------------------------------------------------------------------
    if ((stepIndexT1 < seqOffsT1[seqStepT1]) && seqPlay) { //if the current stepIndex is less than the tick offset for the current step & the seq is playing
      stepIndexT1++;  //increment stepIndex (inter-step placement)
      seqTicksT1++;   //increment seqTicks (advance to next tick)
    } else if (stepIndexT1 == seqOffsT1[seqStepT1]) { //if the current stepIndex is equal to the tick offset for the current step
      if (seqPlay) { //if sequencer playing
        if (bitRead(*(seqPagesT1 + (seqStepT1 >> 3)), (7 - (seqStepT1 % 8)))) {
          noteValT1 = *(seqNotesT1 + seqStepT1);
          tWordT1_L = *(tuningWords + (noteValT1 + 48));
          tWordT1_R = tWordT1_L;
          if (muteValT1 != 0) { //only resets envelope level if not muted
            env_valT1 = 255; //reset envelope
          }
          env_cntT1 = 0;
          env_trigT1 = 1;
        }
      }
      stepIndexT1++;
      seqTicksT1++;
    } else if ((stepIndexT1 < 23) && seqPlay) {
      stepIndexT1++;
      seqTicksT1++;
    } else { //can only be tick 23 - do last calc, then reset index to 0; also increment step
      if (seqPlay) {
        if (((seqLengthT1 - 1) - seqStepT1) == 0) { //if last step of last group
          seqTrackerT1[seqStepT1 >> 3] = 0; //clear current tracker byte
          seqTrackerT1[0] = B10000000; //prep next tracker byte
          seqTicksT1 = 0;
          seqStepT1 = 0; //wrap around
        } else if ((seqStepT1 % 8) == 7) { //if last bit
          seqTrackerT1[seqStepT1 >> 3] = 0; //clear current tracker byte
          seqTrackerT1[(seqStepT1 >> 3) + 1] = B10000000; //prep next tracker byte
          seqTicksT1++;
          seqStepT1++; //advance to next step
        } else {                  //else - bit shift (tracker still within byte (seqStep >> 3))
          seqTrackerT1[seqStepT1 >> 3] = seqTrackerT1[seqStepT1 >> 3] >> 1;
          seqTicksT1++;
          seqStepT1++; //advance to next step
        }
        stepIndexT1 = 0;
      } else {  //23rd tick but sequencer stopped - anything need done here?
        stepIndexT1 = 0;
        if (seqTicksT1 < maxTicksT1) {
          seqTicksT1++;
        } else {
          seqTicksT1 = 0;
        }
      }
    }
    //track 2 sequencer
    //--------------------------------------------------------------------------------
    if ((stepIndexT2 < seqOffsT2[seqStepT1]) && seqPlay) { //if the current stepIndex is less than the tick offset for the current step & the seq is playing
      stepIndexT2++;  //increment stepIndex (inter-step placement)
      seqTicksT2++;   //increment seqTicks (advance to next tick)
    } else if (stepIndexT2 == seqOffsT2[seqStepT1]) { //if the current stepIndex is equal to the tick offset for the current step
      if (seqPlay) { //if sequencer playing
        if (bitRead(*(seqPagesT2 + (seqStepT2 >> 3)), (7 - (seqStepT2 % 8)))) {
          noteValT2 = *(seqNotesT2 + seqStepT2);
          tWordT2_L = *(tuningWords + (noteValT2 + 48));
          tWordT2_R = tWordT2_L;
          if (muteValT2 != 0) { //only resets envelope level if not muted
            env_valT2 = 255; //reset envelope
          }
          env_cntT2 = 0;
          env_trigT2 = 1;
        }
      }
      stepIndexT2++;
      seqTicksT2++;
    } else if ((stepIndexT2 < 23) && seqPlay) {
      stepIndexT2++;
      seqTicksT2++;
    } else { //can only be tick 23 - do last calc, then reset index to 0; also increment step
      if (seqPlay) {
        if (((seqLengthT2 - 1) - seqStepT2) == 0) { //if last step of last group
          seqTrackerT2[seqStepT2 >> 3] = 0; //clear current tracker byte
          seqTrackerT2[0] = B10000000; //prep next tracker byte
          seqTicksT2 = 0;
          seqStepT2 = 0; //wrap around
        } else if ((seqStepT2 % 8) == 7) { //if last bit
          seqTrackerT2[seqStepT2 >> 3] = 0; //clear current tracker byte
          seqTrackerT2[(seqStepT2 >> 3) + 1] = B10000000; //prep next tracker byte
          seqTicksT2++;
          seqStepT2++; //advance to next step
        } else {                  //else - bit shift (tracker still within byte (seqStep >> 3))
          seqTrackerT2[seqStepT2 >> 3] = seqTrackerT2[seqStepT2 >> 3] >> 1;
          seqTicksT2++;
          seqStepT2++; //advance to next step
        }
        stepIndexT2 = 0;
      } else {  //23rd tick but sequencer stopped - anything need done here?
        stepIndexT2 = 0;
        if (seqTicksT2 < maxTicksT2) {
          seqTicksT2++;
        } else {
          seqTicksT2 = 0;
        }
      }
    }
    //track 3 sequencer
    //--------------------------------------------------------------------------------
    if ((stepIndexT3 < seqOffsT3[seqStepT3]) && seqPlay) { //if the current stepIndex is less than the tick offset for the current step & the seq is playing
      stepIndexT3++;  //increment stepIndex (inter-step placement)
      seqTicksT3++;   //increment seqTicks (advance to next tick)
    } else if (stepIndexT3 == seqOffsT3[seqStepT3]) { //if the current stepIndex is equal to the tick offset for the current step
      if (seqPlay) { //if sequencer playing
        if (bitRead(*(seqPagesT3 + (seqStepT3 >> 3)), (7 - (seqStepT3 % 8)))) {
          noteValT3 = *(seqNotesT3 + seqStepT3);
          tWordT3_L = *(tuningWords + (noteValT3 + 48));
          tWordT3_R = tWordT3_L;
          if (muteValT3 != 0) { //only resets envelope level if not muted
            env_valT3 = 255; //reset envelope
          }
          env_cntT3 = 0;
          env_trigT3 = 1;
        }
      }
      stepIndexT3++;
      seqTicksT3++;
    } else if ((stepIndexT3 < 23) && seqPlay) {
      stepIndexT3++;
      seqTicksT3++;
    } else { //can only be tick 23 - do last calc, then reset index to 0; also increment step
      if (seqPlay) {
        if (((seqLengthT3 - 1) - seqStepT3) == 0) { //if last step of last group
          seqTrackerT3[seqStepT3 >> 3] = 0; //clear current tracker byte
          seqTrackerT3[0] = B10000000; //prep next tracker byte
          seqTicksT3 = 0;
          seqStepT3 = 0; //wrap around
        } else if ((seqStepT3 % 8) == 7) { //if last bit
          seqTrackerT3[seqStepT3 >> 3] = 0; //clear current tracker byte
          seqTrackerT3[(seqStepT3 >> 3) + 1] = B10000000; //prep next tracker byte
          seqTicksT3++;
          seqStepT3++; //advance to next step
        } else {                  //else - bit shift (tracker still within byte (seqStep >> 3))
          seqTrackerT3[seqStepT3 >> 3] = seqTrackerT3[seqStepT3 >> 3] >> 1;
          seqTicksT3++;
          seqStepT3++; //advance to next step
        }
        stepIndexT3 = 0;
      } else {  //23rd tick but sequencer stopped - anything need done here?
        stepIndexT3 = 0;
        if (seqTicksT3 < maxTicksT3) {
          seqTicksT3++;
        } else {
          seqTicksT3 = 0;
        }
      }
    }
  }

  //track 1 PWM oscillator
  //--------------------------------------------------------------------------------
  pulseWidthT1_L = (*(params3_T1) >> 2) + (*(params3_T1 + 1) >> 4);
  pulseWidthT1_R = (*(params3_T1) >> 2) - (*(params3_T1 + 1) >> 4);
  phaseAccT1_L = phaseAccT1_L + tWordT1_L;  //increment phase accumulator
  phaseAccT1_R = phaseAccT1_R + tWordT1_R;
  phaseT1_L = phaseAccT1_L >> 8; //(top 8 bits)
  phaseT1_R = phaseAccT1_R >> 8;
  if (phaseT1_L < pulseWidthT1_L) {
    ampT1_L = 255;
  } else {
    ampT1_L = 0;
  }
  if (phaseT1_R < pulseWidthT1_R) {
    ampT1_R = 255;
  } else {
    ampT1_R = 0;
  }
  //track 2 PWM oscillator
  //--------------------------------------------------------------------------------
  pulseWidthT2_L = *(params3_T2) >> 2;
  pulseWidthT2_R = *(params3_T2) >> 2;
  phaseAccT2_L = phaseAccT2_L + tWordT2_L;
  phaseAccT2_R = phaseAccT2_R + tWordT2_R;
  phaseT2_L = phaseAccT2_L >> 8;
  phaseT2_R = phaseAccT2_R >> 8;
  if (phaseT2_L < pulseWidthT2_L) {
    ampT2_L = 255;
  } else {
    ampT2_L = 0;
  }
  if (phaseT2_R < pulseWidthT2_R) {
    ampT2_R = 255;
  } else {
    ampT2_R = 0;
  }
  //track 3 PWM oscillator
  //--------------------------------------------------------------------------------
  pulseWidthT3_L = *(params3_T3) >> 2;
  pulseWidthT3_R = *(params3_T3) >> 2;
  phaseAccT3_L = phaseAccT3_L + tWordT3_L;
  phaseAccT3_R = phaseAccT3_R + tWordT3_R;
  phaseT3_L = phaseAccT3_L >> 8;
  phaseT3_R = phaseAccT3_R >> 8;
  if (phaseT3_L < pulseWidthT3_L) {
    ampT3_L = 255;
  } else {
    ampT3_L = 0;
  }
  if (phaseT3_R < pulseWidthT3_R) {
    ampT3_R = 255;
  } else {
    ampT3_R = 0;
  }

  //track 1 envelope
  //--------------------------------------------------------------------------------
  env_speedT1 = (*(params2_T1 + 2)) >> 2;
  if (env_trigT1) { //update envelope val
    if ((env_cntT1 < env_speedT1) && ((env_valT1) != 0)) {
      env_cntT1++;
    } else if (env_cntT1 >= env_speedT1) {
      env_cntT1 = 0;
      if (env_valT1 == 0) {
        env_trigT1 = 0;
      } else {
        env_valT1--;
      }
    }
  }
  //track 2 envelope
  //--------------------------------------------------------------------------------
  env_speedT2 = (*(params2_T2 + 2)) >> 2;
  if (env_trigT2) { //update envelope val
    if ((env_cntT2 < env_speedT2) && ((env_valT2) != 0)) {
      env_cntT2++;
    } else if (env_cntT2 >= env_speedT2) {
      env_cntT2 = 0;
      if (env_valT2 == 0) {
        env_trigT2 = 0;
      } else {
        env_valT2--;
      }
    }
  }
  //track 3 envelope
  //--------------------------------------------------------------------------------
  env_speedT3 = (*(params2_T3 + 2)) >> 2;
  if (env_trigT3) { //update envelope val
    if ((env_cntT3 < env_speedT3) && ((env_valT3) != 0)) {
      env_cntT3++;
    } else if (env_cntT3 >= env_speedT3) { //speed set by param determines rate of decrement
      env_cntT3 = 0;
      if (env_valT3 == 0) {
        env_trigT3 = 0;
      } else {
        env_valT3--;
      }
    }
  }

  //output stage - stereo mix
  //--------------------------------------------------------------------------------
  OCR2A = (((ampT1_L & (env_valT1)) + (ampT2_L & (env_valT2)) + (ampT3_L & (env_valT3))) >> 2);
  OCR2B = (((ampT1_R & (env_valT1)) + (ampT2_R & (env_valT2)) + (ampT3_R & (env_valT3))) >> 2);
}
