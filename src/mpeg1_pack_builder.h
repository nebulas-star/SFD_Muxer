// SPDX-FileCopyrightText: 2026 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#ifndef __MPEG1_PACK_BUILDER__
#define __MPEG1_PACK_BUILDER__

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "muxer_error_report.h"

#define MAX_ENC_VALUE 0x200000000   // 2^33

// ISO/IEC 11172-1 (MPEG-1 System) 2.4.2
#define system_clock_frequency 90000.0

// ISO/IEC 11172-1 (MPEG-1 System) 2.4.4.2 Table 1 
enum stream_id {
                        // 0xBC reserved
    private_stream_1 =     0xBD,
    padding_stream,     // 0xBE
    private_stream_2,   // 0xBF
    audio_stream_0,     // 0xC0
    audio_stream_1,
    audio_stream_2,
    audio_stream_3,
    audio_stream_4,
    audio_stream_5,
    audio_stream_6,
    audio_stream_7,
    audio_stream_8,
    audio_stream_9,
    audio_stream_10,
    audio_stream_11,
    audio_stream_12,
    audio_stream_13,
    audio_stream_14,
    audio_stream_15,
    audio_stream_16,
    audio_stream_17,
    audio_stream_18,
    audio_stream_19,
    audio_stream_20,
    audio_stream_21,
    audio_stream_22,
    audio_stream_23,
    audio_stream_24,
    audio_stream_25,
    audio_stream_26,
    audio_stream_27,
    audio_stream_28,
    audio_stream_29,
    audio_stream_30,
    audio_stream_31,    // 0xDF
    video_stream_0,     // 0xE0
    video_stream_1,
    video_stream_2,
    video_stream_3,
    video_stream_4,
    video_stream_5,
    video_stream_6,
    video_stream_7,
    video_stream_8,
    video_stream_9,
    video_stream_10,
    video_stream_11,
    video_stream_12,
    video_stream_13,
    video_stream_14,
    video_stream_15,    // 0xEF
};                      // 0xF0~0xFF reserved

// ISO/IEC 11172-2 (MPEG-1 Video) 2.4.3.2
// ISO/IEC 13818-2 (MPEG-2 Video) 6.3.3 Table 6-4   
double picture_rate[9] = {0, 24000.0/1001.0, 24, 25, 30000.0/1001.0, 30, 50, 60000.0/1001.0, 60};  // 0x0 forbidden, 0x9~0xF reserved

// ISO/IEC 11172-2 (MPEG-1 Video) 2.4.3.4
// ISO/IEC 13818-2 (MPEG-2 Video) 6.3.9 Table 6-12
enum picture_coding_type {
                // 0x0 forbidden
    I_frame = 1,    // intra-coded
    P_frame,        // predictive-coded
    B_frame,        // bidirectionally-predictive-coded 
    D_frame         // dc intra-coded   // Shall not be used
                // 0x5~0x7 reserved
};

static void mpeg_time_codeing(uint8_t* Dst, size_t time_stamp, uint8_t hi_bits){
    Dst[0] =((((time_stamp >> 30) & 0x07) << 1) | 0x01) | (hi_bits & 0xF0) ;
    Dst[1] =   (time_stamp >> 22) & 0xFF;
    Dst[2] = (((time_stamp >> 15) & 0x7F) << 1) | 0x01;
    Dst[3] =   (time_stamp >>  7) & 0xFF;
    Dst[4] =  ((time_stamp        & 0x7F) << 1) | 0x01;
}

void mpeg1_end_block_build(char* metadata_block){
    uint8_t MPEG_end_code[4] = {0x00, 0x00, 0x01, 0xB9};    // ISO/IEC 11172-1 (MPEG-1 System) 2.4.3.1
    memmove(metadata_block, MPEG_end_code, 4);
    memset(metadata_block + 4, 0xFF, 0x7FC);
}

// ISO/IEC 11172-1 (MPEG-1 System) 2.4.3.2
size_t mpeg1_pack_header_build(char* metadata_block, size_t pack_index, size_t bitrate, size_t mux_rate){
    uint8_t pack_start_code[4] = {0x00, 0x00, 0x01, 0xBA};
    memmove(metadata_block, pack_start_code, 4);

    size_t system_clock_reference = round(system_clock_frequency * (pack_index * 0x800 + 9) / bitrate);
    if (system_clock_reference >= MAX_ENC_VALUE){
        muxer_error(E__EXCEED_REPLY_TIME_LIMIT);}
    uint8_t pack_header_data[8] = {0};
    
    mpeg_time_codeing(pack_header_data, system_clock_reference, 0x20);
    pack_header_data[5] = ((mux_rate >> 15) & 0x7F)       | 0x80;
    pack_header_data[6] =  (mux_rate >>  7) & 0xFF;
    pack_header_data[7] = ((mux_rate        & 0x7F) << 1) | 0x01;
    memmove(metadata_block + 4, pack_header_data, 8);

    return 12;
}
size_t mpeg1_system_header_build(char* metadata_block, size_t rate_bound, bool is_video, uint8_t stream_num, uint8_t stream_start_index){
    uint8_t system_header_start_code[4] = {0x00, 0x00, 0x01, 0xBB};
    memmove(metadata_block, system_header_start_code, 4);

    uint16_t header_length = 6 + (3 * stream_num);
    uint8_t system_header_data[8] = {0};
    system_header_data[0] = (header_length >> 8) & 0xFF;
    system_header_data[1] =  header_length       & 0xFF;
    system_header_data[2] = ((rate_bound >> 15) & 0x7F)       | 0x80;
    system_header_data[3] =  (rate_bound >>  7) & 0xFF;
    system_header_data[4] = ((rate_bound        & 0x7F) << 1) | 0x01;
    if (is_video){
        system_header_data[5] = 0x02;
        system_header_data[6] = 0x20 | (stream_num & 0x1F);
        system_header_data[7] = 0xFF;
    } else {
        system_header_data[5] = 0x02 | ((stream_num & 0x3F) << 2);
        system_header_data[6] = 0x20;
        system_header_data[7] = 0xFF;
    }
    memmove(metadata_block + 4, system_header_data, 8);

    uint8_t stream_info[3];
    if (is_video){
        stream_info[0] = video_stream_0;
        stream_info[1] = 0xE0;
        stream_info[2] = 0x2E;
    } else {
        stream_info[0] = stream_start_index;
        stream_info[1] = 0xC0;
        stream_info[2] = 0x04;
    }
    for (int i = 0; i < stream_num; i++){
        memmove(metadata_block + 12 + i * 3, stream_info, 3);}

    return 12 + 3 * stream_num;
}

bool first_video_block = true;
// ISO/IEC 11172-1 (MPEG-1 System) 2.4.3.3
size_t mpeg1_packet_header_build(char* metadata_block, uint8_t substream_id, uint16_t packet_size, uint32_t presentation_time_stamp, uint32_t decoding_time_stamp){
    uint8_t packet_start_code_prefix[3] =  {0x00, 0x00, 0x01};
    uint16_t packet_length = packet_size - 6;
    memmove(metadata_block, packet_start_code_prefix, 3);
    metadata_block[3] = substream_id;
    metadata_block[4] = (packet_length >> 8) & 0xFF;
    metadata_block[5] =  packet_length       & 0xFF;

    if (substream_id == private_stream_2){
        return 6;
    } else if (audio_stream_0 <= substream_id && substream_id <= audio_stream_31 ){
        uint8_t audio_packet_info[7] = {0x40, 0x04};
        mpeg_time_codeing(audio_packet_info + 2, presentation_time_stamp, 0x20);
        memmove(metadata_block + 6, audio_packet_info, 7);
        return 13;
    } else if (video_stream_0 <= substream_id && substream_id <= video_stream_15 ){
        uint8_t video_buffer[2] = {0x60, 0x2E};
        uint8_t video_packet_info[12];
        if (presentation_time_stamp == 0 && !first_video_block){
            memset (video_packet_info, 0xFF, 9);
            memmove(video_packet_info + 9, video_buffer, 2);
            memset (video_packet_info + 11, 0x0F, 1);
        } else if (decoding_time_stamp == 0 && !first_video_block){
            memset (video_packet_info, 0xFF, 5);
            memmove(video_packet_info + 5, video_buffer, 2);
            mpeg_time_codeing(video_packet_info + 7, presentation_time_stamp, 0x20);
        } else {
            memmove(video_packet_info, video_buffer, 2);
            mpeg_time_codeing(video_packet_info + 2, presentation_time_stamp, 0x30);
            mpeg_time_codeing(video_packet_info + 7, decoding_time_stamp,     0x10);
            first_video_block = false;
        }
        memmove(metadata_block + 6, video_packet_info, 12);
        return 18;
    } else {     // substream_id == padding_stream
        memset(metadata_block + 6, 0x0F, 1);
        return 7;
    }
}

void mpeg1_padding_stream_packet_build(char* metadata_block, uint16_t packet_size){
    mpeg1_packet_header_build(metadata_block, padding_stream, packet_size, 0, 0);
    if (packet_size > 7){
        memset(metadata_block + 7, 0xFF, packet_size - 7);}    // ISO/IEC 11172-1 (MPEG-1 System) 2.4.4.3
}

#endif // __MPEG1_PACK_BUILDER__