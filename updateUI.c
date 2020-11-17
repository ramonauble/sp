//BEGIN updateUI()
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
void updateUI(byte keyVal) {
  switch (keyVal) {
    //case 16 - FN - called on every state change (rising AND falling edge)
    //------------------------------------------------------------
    //if fn was previously not pressed, a press is registered
    //  fn is set to 1
    //  led states for active track display are set, to reflect active track on page LEDs (as determined by trackSelect)
    //if fn was previously pressed, a release is registered
    //  fn is set to 0
    //  led states for active track display are returned to original states, to reflect active page (as determined by paramsPage)
    //------------------------------------------------------------
    case 16:
      //if fn was pressed (rising edge)
      //-------------------------------------------------------
      if (!fn) {
        fn = 1;
        //track 1
        //---------------------------------------------
        if (trackSelect == 1) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 1);
          switch (paramsPage) {
            case 3:       //params page 3 (rightmost)
              ledState[2] = optionStates_T1[4];
              ledState[1] = optionStates_T1[5];
            break;
            case 2:       //params page 2 (middle)
              ledState[2] = optionStates_T1[2];
              ledState[1] = optionStates_T1[3];
            break;
            case 1:       //params page 1 (leftmost)
              ledState[2] = optionStates_T1[0];
              ledState[1] = optionStates_T1[1];
            break;
          }
        //track 2
        //---------------------------------------------
        } else if (trackSelect == 2) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 2);
          switch (paramsPage) {
            case 3:       //params page 3 (rightmost)
              ledState[2] = optionStates_T2[4];
              ledState[1] = optionStates_T2[5];
            break;
            case 2:       //params page 2 (middle)
              ledState[2] = optionStates_T2[2];
              ledState[1] = optionStates_T2[3];
            break;
            case 1:       //params page 1 (leftmost)
              ledState[2] = optionStates_T2[0];
              ledState[1] = optionStates_T2[1];
            break;
          }
        //track 3
        //---------------------------------------------
        } else {
          bitClear(ledState[0], 2);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 3);
          switch (paramsPage) {
            case 3:       //params page 3 (rightmost)
              ledState[2] = optionStates_T3[4];
              ledState[1] = optionStates_T3[5];
            break;
            case 2:       //params page 2 (middle)
              ledState[2] = optionStates_T3[2];
              ledState[1] = optionStates_T3[3];
            break;
            case 1:       //params page 1 (leftmost)
              ledState[2] = optionStates_T3[0];
              ledState[1] = optionStates_T3[1];
            break;
          }
        }
      //if fn was released (falling edge)
      //-------------------------------------------------------
      } else {
        fn = 0;
        //track 1
        //---------------------------------------------
        if (paramsPage == 3) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 1);
        //track 2
        //---------------------------------------------
        } else if (paramsPage == 2) {
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 2);
        //track 3
        //---------------------------------------------
        } else {
          bitClear(ledState[0], 2);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 3);
        }
      }
    break;

    //case 17 - stop sequencer - called when sequencer stop button pressed
    //------------------------------------------------------------
    //seqPlay set to 0 (stops sequencer in ISR)
    //trackers cleared for each sequence (T1-T3)
    //sequencer LED states recalculated to account for newly cleared trackers
    //seqStepTn step counters reset to stopped (0)
    //------------------------------------------------------------
    case 17:
      seqPlay = 0;            //stop sequencer
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
      seqStepT1 = 0;  //reset T1 step counter
      seqStepT2 = 0;  //reset T2 step counter
      seqStepT3 = 0;  //reset T3 step counter
    break;

    //case 18 - start sequencer - called when sequencer start button pressed
    //------------------------------------------------------------
    //active sequencer step set to init state (step 0) for all tracks
    //sequencer tick counters reset (tick 0) for all tracks
    //inter-step indeces reset for all tracks (index 0)
    //sequencer trackers initialized for all tracks - start on step 1
    //samples per tick counter reset (sptCounter = 0) - initialize sample counter
    //seqPlay set to 1 - start sequencer (start counting)
    //------------------------------------------------------------
    case 18: //seq play
      seqStepT1 = 0;
      seqStepT2 = 0;
      seqStepT3 = 0;

      seqTicksT1 = 0;
      seqTicksT2 = 0;
      seqTicksT3 = 0;

      stepIndexT1 = 0;
      stepIndexT2 = 0;
      stepIndexT3 = 0;

      seqTrackerT1[0] = B10000000; //reset current T1 tracker state
      seqTrackerT2[0] = B10000000; //reset current T2 tracker state
      seqTrackerT3[0] = B10000000; //reset current T3 tracker state

      sptCounter = 0;
      seqPlay = 1; //start sequencer
    break;

    //case 19 - sequencer page shift right (!fn) | keyboard octave shift +12st (fn)
    //------------------------------------------------------------
    //if fn IS NOT currently pressed - sequencer page shifted right by 1 page
    //  change active sequencer page to next page to the immediate right (wrap around to 1 if on page 4)
    //  update sequencer page LEDs to reflect newly active page (1-4)
    //if fn IS currently pressed - octave shift +12st
    //  if not already at max octave, increment octaveOffset by 1, shifting the keyboard up by 12 semitones
    //------------------------------------------------------------
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
        //--------------------------------------------------
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
        //--------------------------------------------------
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
        //--------------------------------------------------
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

    //case 20 - sequencer page shift left (!fn) | keyboard octave shift -12st (fn)
    //------------------------------------------------------------
    //if fn IS NOT currently pressed - sequencer page shifted left by 1 page
    //  change active sequencer page to next page to the immediate left (wrap around to 4 if on page 1)
    //  update sequencer page LEDs to reflect newly active page (1-4)
    //if fn IS currently pressed - octave shift -12st
    //  if not already at min octave, decrement octaveOffset by 1, shifting the keyboard down by 12 semitones
    //------------------------------------------------------------
    case 20:
      if (!fn) {  //shift seq 1 page (16 steps) to the left (fn == 0)
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
        //--------------------------------------------------
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
        //--------------------------------------------------
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
        //--------------------------------------------------
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

    //case 21 - switch to parameter page 3 (rightmost) (!fn) | switch to track 1 (rightmost) (fn)
    //------------------------------------------------------------
    //if fn IS NOT currently pressed - switch to parameter page 3     1 | 2 | [3]
    //  reset pgChange flags - for parameter catch-up after switch
    //  change parameter page/track LEDs to reflect newly active page (3)
    //  set paramsPage to 3 - to set global active parameter page
    //if fn IS currently pressed - switch to track 1              3 | 2 | [1]
    //  set trackSelect to 1 - to set global active track
    //  change parameter page/track LEDs to reflect newly active track (1)
    //  reset pgChange flags - for parameter catch-up after switch
    //------------------------------------------------------------
    case 21:
      if (!fn) {
        if (paramsPage != 3) {
          pgChange = B11111111;
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 1);
          paramsPage = 3;
        }
      //--------------------------------------------------
      } else {
        trackSelect = 1;
        bitClear(ledState[0], 3);
        bitClear(ledState[0], 2);
        bitSet(ledState[0], 1);
        pgChange = B11111111;
      }
    break;

    //case 22 - switch to parameter page 2 (middle) (!fn) | switch to track 2 (middle) (fn)
    //------------------------------------------------------------
    //if fn IS NOT currently pressed - switch to parameter page 2     1 | [2] | 3
    //  reset pgChange flags - for parameter catch-up after switch
    //  change parameter page/track LEDs to reflect newly active page (2)
    //  set paramsPage to 2 - to set global active parameter page
    //if fn IS currently pressed - switch to track 2              3 | [2] | 1
    //  set trackSelect to 2 - to set global active track
    //  change parameter page/track LEDs to reflect newly active track (2)
    //  reset pgChange flags - for parameter catch-up after switch
    //------------------------------------------------------------
    case 22:  //param select (fn == 0) OR track select (fn == 1) - MIDDLE
      if (!fn) {
        if (paramsPage != 2) {
          pgChange = B11111111;
          bitClear(ledState[0], 3);
          bitClear(ledState[0], 1);
          bitSet(ledState[0], 2);
          paramsPage = 2;
        }
      //--------------------------------------------------
      } else {
        trackSelect = 2;
        bitClear(ledState[0], 3);
        bitClear(ledState[0], 1);
        bitSet(ledState[0], 2);
        pgChange = B11111111;
      }
    break;

    //case 23 - switch to parameter page 1 (leftmost) (!fn) | switch to track 3 (leftmost) (fn)
    //------------------------------------------------------------
    //if fn IS NOT currently pressed - switch to parameter page 1     [1] | 2 | 3
    //  reset pgChange flags - for parameter catch-up after switch
    //  change parameter page/track LEDs to reflect newly active page (1)
    //  set paramsPage to 1 - to set global active parameter page
    //if fn IS currently pressed - switch to track 2              [3] | 2 | 1
    //  set trackSelect to 3 - to set global active track
    //  change parameter page/track LEDs to reflect newly active track (3)
    //  reset pgChange flags - for parameter catch-up after switch
    //------------------------------------------------------------
    case 23:  //param select (fn == 0) OR track select (fn == 1) - LEFTMOST
      if (!fn) {
        if (paramsPage != 1) {
          pgChange = B11111111;
          bitClear(ledState[0], 1);
          bitClear(ledState[0], 2);
          bitSet(ledState[0], 3);
          paramsPage = 1;
        }
      //--------------------------------------------------
      } else {
        trackSelect = 3;
        bitClear(ledState[0], 2);
        bitClear(ledState[0], 1);
        bitSet(ledState[0], 3);
        pgChange = B11111111;
      }
    break;

    //EITHER sequencer buttons [0-15] OR keyboard buttons [24-39]
    //------------------------------------------------------------
    default:
      //if keyVal is within sequencer button range [0-15] - sequencer button pressed
      //-------------------------------------------------------
      if (keyVal >= 0 && keyVal < 16) {
        //if keyVal is between [0-7] - first 8 sequencer buttons
        //--------------------------------------------------
        if (keyVal < 8) {
          //if led state is ON for pressed sequencer button
          //---------------------------------------------
          if (bitRead(ledState[2], (7 - (keyVal % 8)))) {
            bitClear(ledState[2], (7 - (keyVal % 8)));  //clear LED state
            //if track is 1, 2 or 3 - clears the sequencer step state only for the selected track
            //----------------------------------------
            if (trackSelect == 1) {
              bitClear(seqPagesT1[seqPageA_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitClear(seqPagesT2[seqPageA_T2], (7 - (keyVal % 8)));
            } else {
              bitClear(seqPagesT3[seqPageA_T3], (7 - (keyVal % 8)));
            }
          //if led state is OFF for pressed sequencer button
          //---------------------------------------------
          } else {
            bitSet(ledState[2], (7 - (keyVal % 8)));  //set LED state
            //if track is 1, 2 or 3 - sets the sequencer step state only for the selected track
            //----------------------------------------
            if (trackSelect == 1) {
              bitSet(seqPagesT1[seqPageA_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitSet(seqPagesT2[seqPageA_T2], (7 - (keyVal % 8)));
            } else {
              bitSet(seqPagesT3[seqPageA_T3], (7 - (keyVal % 8)));
            }
          }
        //if keyVal is between [8-15] - second 8 sequencer buttons
        //--------------------------------------------------
        } else {
          //if led state is ON for pressed sequencer button
          //---------------------------------------------
          if (bitRead(ledState[1], (7 - (keyVal % 8)))) {
            bitClear(ledState[1], (7 - (keyVal % 8)));  //clear LED state
            //if track is 1, 2 or 3 - clears the sequencer step state only for the selected track
            //----------------------------------------
            if (trackSelect == 1) {
              bitClear(seqPagesT1[seqPageB_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitClear(seqPagesT2[seqPageB_T2], (7 - (keyVal % 8)));
            } else {
              bitClear(seqPagesT3[seqPageB_T3], (7 - (keyVal % 8)));
            }
          //if led state is OFF for pressed sequencer button
          //---------------------------------------------
          } else {
            bitSet(ledState[1], (7 - (keyVal % 8)));  //set LED state
            //if track is 1, 2 or 3 - sets the sequencer step state only for the selected track
            //----------------------------------------
            if (trackSelect == 1) {
              bitSet(seqPagesT1[seqPageB_T1], (7 - (keyVal % 8)));
            } else if (trackSelect == 2) {
              bitSet(seqPagesT2[seqPageB_T2], (7 - (keyVal % 8)));
            } else {
              bitSet(seqPagesT3[seqPageB_T3], (7 - (keyVal % 8)));
            }
          }
        }
      } else {
        //bottom keyboard row - keyVal == [33-24], inclusive, left-> right
        //C | C# | D | D# | E | F | F# | G | G# | A
        //33| 32 | 31| 30 | 29| 28| 27 | 26| 25 | 24
        //-----------------------------------------------------------------
        if (keyVal < 34) {
          noteVal = 3 + (33 - keyVal) + (12 * octaveOffset); //offset of 3 to account for C start | left -> right - bottom C == 0 (normalized)
          //------------------------------------------------------------
          if ((keyVal == 31) && (fn == 1)) { //if bottom c pressed while fn held
            if (muteValT1 == 0) {
              muteValT1 = 255;
            } else {
              muteValT1 = 0;
            }
          //------------------------------------------------------------
          } else if ((keyVal == 32) && (fn == 1)) { //if bottom c pressed while fn held
            if (muteValT2 == 0) {
              muteValT2 = 255;
            } else {
              muteValT2 = 0;
            }
          //------------------------------------------------------------
          } else if ((keyVal == 33) && (fn == 1)) { //if bottom c pressed while fn held
            if (muteValT3 == 0) {
              muteValT3 = 255;
            } else {
              muteValT3 = 0;
            }

          //f# - seq set to 16 step length
          //---------------------------------------------
          } else if ((keyVal == 27) && (fn == 1)) {
            seqPlay = 0; //stop sequencer
            switch (trackSelect) {
              //track 1
              //----------------------------------------
              case 1:
                seqLengthT1 = 16; //set length to 16
                maxTicksT1 = 384;
                seqLimT1 = 1;
                if (seqTicksT1 > maxTicksT1) { //protects against overflow of tracker on length change
                  seqStepT1 = 0;  //reset sequencer
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000; //clear current tracker state
                }
              break;
              //track 2
              //----------------------------------------
              case 2:
                seqLengthT2 = 16;
                maxTicksT2 = 384;
                seqLimT2 = 1;
                if (seqTicksT2 > maxTicksT2) {
                  seqStepT2 = 0;
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000;
                }
              break;
              //track 2
              //----------------------------------------
              case 3:
                seqLengthT3 = 16;
                maxTicksT3 = 384;
                seqLimT3 = 1;
                if (seqTicksT3 > maxTicksT3) {
                  seqStepT3 = 0;
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000;
                }
              break;
            }
            seqPlay = 1; //restart sequencer

          //g - seq set to 32 step length
          //---------------------------------------------
          } else if ((keyVal == 26) && (fn == 1)) {
            seqPlay = 0;
            switch (trackSelect) {
              //track 1
              //----------------------------------------
              case 1:
                seqLengthT1 = 32; //set length to 32
                maxTicksT1 = 768;
                seqLimT1 = 3;
                if (seqTicksT1 > maxTicksT1) {
                  seqStepT1 = 0;
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000;
                }
              break;
              //track 2
              //----------------------------------------
              case 2:
                seqLengthT2 = 32;
                maxTicksT2 = 768;
                seqLimT2 = 3;
                if (seqTicksT2 > maxTicksT2) {
                  seqStepT2 = 0;
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000;
                }
              break;
              //track 3
              //----------------------------------------
              case 3:
                seqLengthT3 = 32;
                maxTicksT3 = 768;
                seqLimT3 = 3;
                if (seqTicksT3 > maxTicksT3) {
                  seqStepT3 = 0;
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000;
                }
              break;
            }
            seqPlay = 1;

          //g# - seq set to 48 step length
          //---------------------------------------------
          } else if ((keyVal == 25) && (fn == 1)) {
            seqPlay = 0;
            switch (trackSelect) {
              //track 2
              //----------------------------------------
              case 1:
                seqLengthT1 = 48; //set length to 48
                maxTicksT1 = 1152;
                seqLimT1 = 5;
                if (seqTicksT1 > maxTicksT1) {
                  seqStepT1 = 0;
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000;
                }
              break;
              //track 2
              //----------------------------------------
              case 2:
                seqLengthT2 = 48;
                maxTicksT2 = 1152;
                seqLimT2 = 5;
                if (seqTicksT2 > maxTicksT2) {
                  seqStepT2 = 0;
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000;
                }
              break;
              //track 3
              //----------------------------------------
              case 3:
                seqLengthT3 = 48;
                maxTicksT3 = 1152;
                seqLimT3 = 5;
                if (seqTicksT3 > maxTicksT3) {
                  seqStepT3 = 0;
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000;
                }
              break;
            }
            seqPlay = 1;

          //a - seq set to 64 step length
          //---------------------------------------------
          } else if ((keyVal == 24) && (fn == 1)) {
            seqPlay = 0;
            switch (trackSelect) {
              //track 1
              //----------------------------------------
              case 1:
                seqLengthT1 = 64; //set length to 64
                maxTicksT1 = 1536;
                seqLimT1 = 7;
                if (seqTicksT1 > maxTicksT1) {
                  seqStepT1 = 0;
                  seqTicksT1 = 0;
                  stepIndexT1 = 0;
                  seqTrackerT1[0] = B10000000; //clear current tracker state
                }
              break;
              //track 2
              //----------------------------------------
              case 2:
                seqLengthT2 = 64;
                maxTicksT2 = 1536;
                seqLimT2 = 7;
                if (seqTicksT2 > maxTicksT2) {
                  seqStepT2 = 0;
                  seqTicksT2 = 0;
                  stepIndexT2 = 0;
                  seqTrackerT2[0] = B10000000;
                }
              break;
              //track 3
              //----------------------------------------
              case 3:
                seqLengthT3 = 64;
                maxTicksT3 = 1536;
                seqLimT3 = 7;
                if (seqTicksT3 > maxTicksT3) {
                  seqStepT3 = 0;
                  seqTicksT3 = 0;
                  stepIndexT3 = 0;
                  seqTrackerT3[0] = B10000000;
                }
              break;
            }
            seqPlay = 1; //restart sequencer

          } else if ((keyVal == 29) && (fn == 1)) { //f - seq length decremented by 1 step (24 ticks); min. length 1 (24 ticks)
            switch (trackSelect) {
              //track 1
              //----------------------------------------
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
              //track 2
              //----------------------------------------
              case 2:
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
              //track 3
              //----------------------------------------
              case 3:
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
              //track 1
              //----------------------------------------
              case 1:
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
              //track 2
              //----------------------------------------
              case 2:
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
              //track 3
              //----------------------------------------
              case 3:
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

        //top keyboard row - keyVal == [34-38], inclusive, left-> right
        //A# | B | C | C# | D |
        //34 | 35| 36| 37 | 38|
        //-----------------------------------------------------------------
        } else {
          noteVal = 3 + (keyVal - 24) + (12 * octaveOffset);
        }

        //triggers note on keyboard press, for currently selected track
          //recalculates tuning word
          //triggers envelope
        //also programs sequencer step by reading the matrix state & setting the note if held
        //----------------------------------------------------------------------
        switch (trackSelect) {
          //track 1
          //-----------------------------------------------------------------
          case 1:
            if (!fn) {                //only trigger note if fn not held
              env_valT1 = 0;              //reset envelope (silence note before tword change)
              noteValT1 = noteVal;            //set note val of active track (T1) to most recently received key note value
              tWordT1_L = tuningWords[noteValT1 + 48];  //looks up new tuning word in tword LUT (addressed by noteVal)
              tWordT1_R = tWordT1_L;          //middle A is at index 48 (noteValTn == 0)
              if (muteValT1 != 0) {           //only resets envelope level if not muted
                env_valT1 = 255;              //reset envelope
              }
              env_cntT1 = 0;                //resets envelope counter
              env_trigT1 = 1;               //triggers envelope in next ISR cycle - sound now ready :3
              for (int chkSeqs = 0; chkSeqs < 16; chkSeqs++) {
                if (bitRead(matrixState[chkSeqs % 5], 7 - (chkSeqs / 5))) { //checks state of each seq. button and assigns note if pressed
                  seqNotesT1[chkSeqs + (16 * (page - 1))] = noteValT1;    //assigns read note value to pressed seq butt. (page depending)
                  bitSet(seqPagesT1[2 * (page - 1) + (chkSeqs >> 3)], (7 - chkSeqs % 8));//turns seq step back on
                  bitSet(ledState[2 - (chkSeqs >> 3)], (7 - chkSeqs % 8));         //turns led back on
                }
              }
            }
          break;
          //track 2
          //-----------------------------------------------------------------
          case 2:
            if  (!fn) {
              env_valT2 = 0;
              noteValT2 = noteVal;
              tWordT2_L = tuningWords[noteValT2 + 48];
              tWordT2_R = tWordT2_L;
              if (muteValT2 != 0) {
                env_valT2 = 255;
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
          //track 3
          //-----------------------------------------------------------------
          case 3:
            if (!fn) {
              env_valT3 = 0;
              noteValT3 = noteVal;
              tWordT3_L = tuningWords[noteValT3 + 48];
              tWordT3_R = tWordT3_L;
              if (muteValT3 != 0) {
                env_valT3 = 255;
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
