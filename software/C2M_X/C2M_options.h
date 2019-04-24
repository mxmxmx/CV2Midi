/*
 *
 * compile options.
 *
 */

#ifndef C2M_OPTIONS_H_
#define C2M_OPTIONS_H_

/* ------------ default note number # (@ 0.0V) ------------------------------------------------------  */
#define NOTE_NUMBER_1 60 // --> S+H channel 1
#define NOTE_NUMBER_2 60 // --> S+H channel 2
#define NOTE_NUMBER_3 60 // --> S+H channel 3
#define NOTE_NUMBER_4 60 // --> S+H channel 4
#define NOTE_NUMBER_5 60 // --> S+H channel 5

/* ------------ default midi channel # (0 - 15) -----------------------------------------------------  */
#define MIDI_CHANNEL_1 1 // --> S+H channel 1
#define MIDI_CHANNEL_2 2 // --> S+H channel 2
#define MIDI_CHANNEL_3 3 // --> S+H channel 3
#define MIDI_CHANNEL_4 4 // --> S+H channel 4
#define MIDI_CHANNEL_5 5 // --> S+H channel 5

/* ------------ print debug messages to USB serial --------------------------------------------------  */
//#define PRINT_DEBUG

/* ------------ use serial_write(const void *buf, unsigned int count) -------------------------------  */
#define USE_TX_BUFFER

#endif
