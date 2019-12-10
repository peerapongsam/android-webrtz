/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <assert.h>
#include <stdio.h>

#include <memory>
#include <vector>

#include "modules/audio_coding/neteq/tools/packet.h"
#include "modules/audio_coding/neteq/tools/rtp_file_source.h"
#include "rtc_base/flags.h"

// Define command line flags.
DEFINE_int(red, 117, "RTP payload type for RED");
DEFINE_int(audio_level, -1, "Extension ID for audio level (RFC 6464); "
                            "-1 not to print audio level");
DEFINE_int(abs_send_time, -1, "Extension ID for absolute sender time; "
                             "-1 not to print absolute send time");
DEFINE_bool(help, false, "Print this message");

int main(int argc, char* argv[]) {
  std::string program_name = argv[0];
  std::string usage =
      "Tool for parsing an RTP dump file to text output.\n"
      "Run " +
      program_name +
      " --help for usage.\n"
      "Example usage:\n" +
      program_name + " input.rtp output.txt\n\n" +
      "Output is sent to stdout if no output file is given. " +
      "Note that this tool can read files with or without payloads.\n";
  if (rtc::FlagList::SetFlagsFromCommandLine(&argc, argv, true) ||
      FLAG_help || (argc != 2 && argc != 3)) {
    printf("%s", usage.c_str());
    if (FLAG_help) {
      rtc::FlagList::Print(nullptr, false);
      return 0;
    }
    return 1;
  }

  RTC_CHECK(FLAG_red >= 0 && FLAG_red <= 127);  // Payload type
  RTC_CHECK(FLAG_audio_level == -1 ||  // Default
      (FLAG_audio_level > 0 && FLAG_audio_level <= 255));  // Extension ID
  RTC_CHECK(FLAG_abs_send_time == -1 ||  // Default
      (FLAG_abs_send_time > 0 && FLAG_abs_send_time <= 255));  // Extension ID

  printf("Input file: %s\n", argv[1]);
  std::unique_ptr<webrtz::test::RtpFileSource> file_source(
      webrtz::test::RtpFileSource::Create(argv[1]));
  assert(file_source.get());
  // Set RTP extension IDs.
  bool print_audio_level = false;
  if (FLAG_audio_level != -1) {
    print_audio_level = true;
    file_source->RegisterRtpHeaderExtension(webrtz::kRtpExtensionAudioLevel,
                                            FLAG_audio_level);
  }
  bool print_abs_send_time = false;
  if (FLAG_abs_send_time != -1) {
    print_abs_send_time = true;
    file_source->RegisterRtpHeaderExtension(
        webrtz::kRtpExtensionAbsoluteSendTime, FLAG_abs_send_time);
  }

  FILE* out_file;
  if (argc == 3) {
    out_file = fopen(argv[2], "wt");
    if (!out_file) {
      printf("Cannot open output file %s\n", argv[2]);
      return -1;
    }
    printf("Output file: %s\n\n", argv[2]);
  } else {
    out_file = stdout;
  }

  // Print file header.
  fprintf(out_file, "SeqNo  TimeStamp   SendTime  Size    PT  M       SSRC");
  if (print_audio_level) {
    fprintf(out_file, " AuLvl (V)");
  }
  if (print_abs_send_time) {
    fprintf(out_file, " AbsSendTime");
  }
  fprintf(out_file, "\n");

  uint32_t max_abs_send_time = 0;
  int cycles = -1;
  std::unique_ptr<webrtz::test::Packet> packet;
  while (true) {
    packet = file_source->NextPacket();
    if (!packet.get()) {
      // End of file reached.
      break;
    }
    // Write packet data to file. Use virtual_packet_length_bytes so that the
    // correct packet sizes are printed also for RTP header-only dumps.
    fprintf(out_file,
            "%5u %10u %10u %5i %5i %2i %#08X",
            packet->header().sequenceNumber,
            packet->header().timestamp,
            static_cast<unsigned int>(packet->time_ms()),
            static_cast<int>(packet->virtual_packet_length_bytes()),
            packet->header().payloadType,
            packet->header().markerBit,
            packet->header().ssrc);
    if (print_audio_level && packet->header().extension.hasAudioLevel) {
      fprintf(out_file,
              " %5u (%1i)",
              packet->header().extension.audioLevel,
              packet->header().extension.voiceActivity);
    }
    if (print_abs_send_time && packet->header().extension.hasAbsoluteSendTime) {
      if (cycles == -1) {
        // Initialize.
        max_abs_send_time = packet->header().extension.absoluteSendTime;
        cycles = 0;
      }
      // Abs sender time is 24 bit 6.18 fixed point. Shift by 8 to normalize to
      // 32 bits (unsigned). Calculate the difference between this packet's
      // send time and the maximum observed. Cast to signed 32-bit to get the
      // desired wrap-around behavior.
      if (static_cast<int32_t>(
              (packet->header().extension.absoluteSendTime << 8) -
              (max_abs_send_time << 8)) >= 0) {
        // The difference is non-negative, meaning that this packet is newer
        // than the previously observed maximum absolute send time.
        if (packet->header().extension.absoluteSendTime < max_abs_send_time) {
          // Wrap detected.
          cycles++;
        }
        max_abs_send_time = packet->header().extension.absoluteSendTime;
      }
      // Abs sender time is 24 bit 6.18 fixed point. Divide by 2^18 to convert
      // to floating point representation.
      double send_time_seconds =
          static_cast<double>(packet->header().extension.absoluteSendTime) /
              262144 +
          64.0 * cycles;
      fprintf(out_file, " %11f", send_time_seconds);
    }
    fprintf(out_file, "\n");

    if (packet->header().payloadType == FLAG_red) {
      std::list<webrtz::RTPHeader*> red_headers;
      packet->ExtractRedHeaders(&red_headers);
      while (!red_headers.empty()) {
        webrtz::RTPHeader* red = red_headers.front();
        assert(red);
        fprintf(out_file,
                "* %5u %10u %10u %5i\n",
                red->sequenceNumber,
                red->timestamp,
                static_cast<unsigned int>(packet->time_ms()),
                red->payloadType);
        red_headers.pop_front();
        delete red;
      }
    }
  }

  fclose(out_file);

  return 0;
}
