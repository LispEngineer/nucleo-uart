/*
 * midi.c
 *
 *  Created on: Sep 8, 2024
 *  Updated on: 2024-09-08
 *      Author: Douglas P. Fields, Jr.
 *   Copyright: 2024, Douglas P. Fields, Jr.
 *     License: Apache 2.0
 *
 * Parses incoming MIDI messages into a persistent buffer, one
 * byte at a time. Handles running status.
 *
 * For notes on MIDI: see MIDI.md
 */

#include <stdint.h>
#include "midi.h"

/** Initializes a new MIDI data stream to start values.
 */
void midi_stream_init(midi_stream *ms) {
  ms->last_status = 0;
  ms->received_data1 = 0;
}

// FIXME: Fix all these magic numbers

/** Receives a byte on a MIDI stream.
 * Returns true if we received a full message.
 * Puts the message in the specified location, if one is fully received.
 */
int midi_stream_receive(midi_stream *ms, uint8_t b, midi_message *msg) {

  if (b & 0x80) {
    // Status byte

    switch (b & 0xF0) {

    // Channel Voice/Mode status bytes ///////////////////////////////////////
    case 0x80: // Note off (2 bytes)
    case 0x90: // Note on (2 bytes)
    case 0xA0: // Poly aftertouch (2 bytes)
    case 0xB0: // Control change (2 bytes) & Channel Mode (1st byte 120-127)
    case 0xC0: // Program Change (1 byte)
    case 0xD0: // Channel aftertouch (1 byte)
    case 0xE0: // Pitch bend (2 bytes)
      ms->last_status = b;
      ms->received_data1 = 0;
      return 0; // Expecting more later

    // System status bytes ///////////////////////////////////////////////////
    case 0xF0: // System: Exclusive, Common, Real-Time
      // Real time messages do not interfere with running status
      if (b >= 0xF8) {
        // Real-time message does not change running status and has zero data bytes
        msg->type = b;
        return 1;
      }
      switch (b) {
      case 0xF0: // Start of SysEx - we ignore all data bytes from here
        ms->last_status = b;
        return 0;
      case 0xF1: // MIDI Time Code Quarter Frame - (1 byte)
      case 0xF2: // Song Position Pointer - (2 bytes)
      case 0xF3: // Song select (1 byte)
        ms->last_status = b;
        ms->received_data1 = 0;
        return 0; // Expecting more later
      case 0xF4: // Undefined
      case 0xF5: // Undefined
        ms->last_status = MIDI_NONE;
        // We probably don't need to return anything
        return 0; // Error
      case 0xF6: // Tune request (0 bytes)
        ms->last_status = MIDI_NONE; // End running status
        msg->type = b;
        return 1;
      case 0xF7: // SysEx EOX (end of eXclusive) - we are ignoring this for now
        ms->last_status = MIDI_NONE;
        return 0;
      }
    }

    // No fall through - should be unreachable
    return 0;
  }

  //////////////////////////////////////////////////////////////////////////////////
  // Data byte Handling

  switch (ms->last_status) {

  case 0x80: // Note off (2 bytes)
  case 0x90: // Note on (2 bytes)
  case 0xA0: // Poly aftertouch (2 bytes)
    if (ms->received_data1) {
      msg->type = ms->last_status;
      msg->note = ms->data1;
      msg->velocity = b;
      msg->channel = ms->last_status & 0x0F;
      // Running status remains
      ms->received_data1 = 0;
      return 1;
    }
    // Store data 1 for next time
    ms->data1 = b;
    ms->received_data1 = 1;
    return 0;

  case 0xB0: // Control change (2 bytes) & Channel Mode (1st byte 120-127)
    if (ms->received_data1) {
      if (ms->data1 >= 120) {
        // Channel mode message
        msg->type = ms->data1;
        msg->data1 = b;
        msg->data2 = b;
        ms->received_data1 = 0;
        return 1;
      }
      // Regular Control Change
      msg->type = ms->last_status;
      msg->control = ms->data1;
      msg->cc_value = b;
      ms->received_data1 = 0;
      return 1;
    }
    ms->data1 = b;
    ms->received_data1 = 1;
    return 0;

  case 0xC0: // Program Change (1 byte)
    msg->type = ms->received_data1;
    msg->program = b;
    return 1;

  case 0xD0: // Channel aftertouch (1 byte)
    msg->type = ms->received_data1;
    msg->pressure = b;
    return 1;

  case 0xE0: // Pitch bend (2 bytes)
    if (ms->received_data1) {
      msg->type = ms->last_status;
      msg->msb = b;
      msg->lsb = ms->data1;
      ms->received_data1 = 0;
      return 1;
    }
    ms->received_data1 = 1;
    ms->data1 = b;
    return 0;

  // System ////////////////////////////////////////////////////////////////////////

  case 0xF0: // SysEx - we're ignoring data in SysEx
    return 0;
  case 0xF1: // MIDI Time Code Quarter Frame - (1 byte)
    msg->type = ms->last_status;
    msg->tcqf_message_type = ((b & 0x70) >> 4);
    msg->tcqf_value = (b & 0x0F);
    ms->last_status = MIDI_NONE; // No running status
    return 1;
  case 0xF2: // Song Position Pointer - (2 bytes)
    if (ms->received_data1) {
      msg->type = ms->last_status;
      msg->lsb = ms->data1;
      msg->msb = b;
      ms->last_status = MIDI_NONE;
      return 1;
    }
    ms->received_data1 = 1;
    ms->data1 = b;
    return 0;
  case 0xF3: // Song select (1 byte)
    msg->type = ms->last_status;
    msg->data1 = b; // Song number
    ms->last_status = MIDI_NONE;
    return 1;
  case 0xF4: // Undefined - should never get here
  case 0xF5: // Undefined - should never get here
    ms->last_status = MIDI_NONE; // End running status
    return 0; // Error
  case 0xF6: // Tune request (0 bytes) - should never get here
    ms->last_status = MIDI_NONE; // End running status
    return 0;
  case 0xF7: // SysEx EOX (end of eXclusive) - we are ignoring this for now
    ms->last_status = MIDI_NONE;
    return 0;
  default:
    // TODO: Detect when we get here
    return 0;
  }

  // We should never get here
  return 0;
}
