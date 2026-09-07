// SPDX-FileCopyrightText: 2026 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#ifndef __METADATA_READER__
#define __METADATA_READER__

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib/fsize.h"
#include "lib/memsearch.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static uint32_t memread_u32be(uint8_t* src){
    uint32_t dst = (src[0] << 24) | 
                   (src[1] << 16) |
                   (src[2] << 8)  | 
                    src[3];
    return dst;
}

#include "sofdec_metadata_builder.h"
#include "muxer_error_report.h"

// ATSC A/52 (AC-3) 5.4.4.1 Table 5.18
static uint16_t a52_frmsizecod_bitrate[38] = {      //kbps
     32,  32,  40,  40,  48,  48,  56,  56,  64,  64,  80,  80,  96,  96, 112, 112,
    128, 128, 160, 160, 192, 192, 224, 224, 256, 256, 320, 320, 384, 384, 448, 448,
    512, 512, 576, 576, 640, 640
};
static uint16_t a52_frame_size_code_table[3][38] = {
    {     64,   64,   80,   80,   96,   96,  112,  112,  128,  128,  160,  160,  192,  192,  224,  224,
         256,  256,  320,  320,  384,  384,  448,  448,  512,  512,  640,  640,  768,  768,  896,  896,
        1024, 1024, 1152, 1152, 1280, 1280  },      // 48 kHz
    {     69,   70,   87,   88,  104,  105,  121,  122,  139,  140,  174,  175,  208,  209,  243,  244,
         278,  279,  348,  349,  417,  418,  487,  488,  557,  558,  696,  697,  835,  836,  975,  976,
        1114, 1115, 1253, 1254, 1393, 1394  },      // 44.1 kHz
    {     96,   96,  120,  120,  144,  144,  168,  168,  192,  192,  240,  240,  288,  288,  336,  336,
         384,  384,  480,  480,  576,  576,  672,  672,  768,  768,  960,  960, 1152, 1152, 1344, 1344,
        1536, 1536, 1728, 1728, 1920, 1920  }       // 32 kHz
};

typedef struct video_frame_info {
    size_t   offset;
    uint32_t present_index;
    uint8_t  type;
    
    struct video_frame_info* next_frame;
} video_frame_info;

#define STREAM_BUFFER_SIZE 0x1000000     //16 MiB

uint32_t video_frame_map(FILE* video_stream, video_stream_info* stream_info){
    fseek(video_stream, 0, SEEK_SET);

    stream_info->frame_map = (video_frame_info*)calloc(1, sizeof(video_frame_info));
    video_frame_info* this_frame = (video_frame_info*)stream_info->frame_map;
    uint32_t frame_count = 0;

    uint8_t mpeg_start_code_prefix[3] = {0x00, 0x00, 0x01};

    uint8_t* stream_buffer = (uint8_t*)calloc(STREAM_BUFFER_SIZE + 5, sizeof(uint8_t));
    fread(stream_buffer + 2, sizeof(uint8_t), 3, video_stream);
    size_t before_buffer = 0;
    size_t after_buffer = fsize(video_stream);
    uint32_t search_area_size = 0;

    int32_t next_start_code = 0;
    int32_t next_search_start = 0;
    uint16_t temporal_reference;
    uint16_t max_temporal_reference = 0;
    int32_t present_index_offset = -1;

    bool update_buffer = 1;
    do {
        if (update_buffer){
            if (after_buffer == 0){
                break;
            }
            next_search_start = 0;
            memmove(stream_buffer, stream_buffer + STREAM_BUFFER_SIZE, 5);
            search_area_size = fread(stream_buffer + 5, sizeof(uint8_t), STREAM_BUFFER_SIZE, video_stream);
            if (search_area_size < STREAM_BUFFER_SIZE){
                after_buffer = 0;
            } else {
                after_buffer -= search_area_size;
            }
            if (after_buffer == 0){
                search_area_size += 2;
            }
            update_buffer = 0;
        }

        next_start_code = memsearch(stream_buffer, search_area_size + 2, next_search_start, mpeg_start_code_prefix, 3);
        if (next_start_code == -1){
            before_buffer += search_area_size;
            update_buffer = 1;
            continue;
        }

        // ISO/IEC 11172-2 (MPEG-1 Video) 2.4.2
        if (stream_buffer[next_start_code + 3] == 0xB8){            // group_start_code
            present_index_offset = present_index_offset + max_temporal_reference + 1; 
            max_temporal_reference = 0;
        } else if (stream_buffer[next_start_code + 3] == 0x00){     // picture_start_code
            this_frame->offset = before_buffer + next_start_code - 2;
            temporal_reference = (((stream_buffer[next_start_code + 4] << 2) | (stream_buffer[next_start_code + 5] >> 6)) & 0x3FF);
            this_frame->present_index = present_index_offset + temporal_reference;
            max_temporal_reference = MAX(max_temporal_reference, temporal_reference);
            this_frame->type = (stream_buffer[next_start_code + 5] >> 3) & 0x07;
            this_frame->next_frame = (video_frame_info*)calloc(1, sizeof(video_frame_info));
            this_frame = this_frame->next_frame;
            frame_count++;
        } else if (stream_buffer[next_start_code + 3] == 0xB7){     // aquence_end_code
            break;
        }

        next_search_start = next_start_code + 4;
        if (next_search_start > search_area_size){
            before_buffer += search_area_size;
            update_buffer = 1;
        }
    } while (1);

    return frame_count;
}

uint8_t format_cache[0x120];

void video_format_check(video_stream_info* stream_info){
    FILE* input_video = fopen(stream_info->file_path, "rb");
    fread(format_cache, 1, 0x90, input_video);

    // ISO/IEC 11172-2 (MPEG-1 Video) 2.4.2.1
    uint8_t sequence_header_code[4] = {0x00, 0x00, 0x01, 0xB3};   
    if(memcmp(format_cache, sequence_header_code, 4)){
        muxer_error(E__UNSUPPORTED_VIDEO_FORMAT, stream_info->file_path);}

    // ISO/IEC 11172-2 (MPEG-1 Video) 2.4.2.3
    stream_info->frame_width = (format_cache[4] << 4) | ((format_cache[5] & 0xF0) >> 4);
    stream_info->frame_hight = (format_cache[5] & 0x0F) << 8 | format_cache[6];
    stream_info->frame_rate = format_cache[7] & 0x0F;
    
    uint8_t extension_data_offset = 0x0C;
    if ((format_cache[extension_data_offset] & 0x02) == 0x02){
        extension_data_offset += 0x40;}
    if ((format_cache[extension_data_offset] & 0x01) == 0x01){
        extension_data_offset += 0x40;}

    // ISO/IEC 13818-2 (MPEG-2 Video) 6.3.1 Figure 6-15
    uint8_t extension_start_code[4] = {0x00, 0x00, 0x01, 0xB5};
    if (!memcmp(format_cache + extension_data_offset, extension_start_code, 4)){                                      
        stream_info->codec_type = MPEG2_VIDEO; 
    } else {
        stream_info->codec_type = MPEG1_VIDEO;}

    stream_info->total_frames_count = video_frame_map(input_video, stream_info);

    fclose(input_video);
}

void audio_format_check(audio_stream_info* stream_info){
    uint8_t cri_adx_file_signatures[2] = {0x80, 0x00};
    uint8_t a52_syncinfo_syncword[2] = {0x0B, 0x77};
    uint8_t cri_aix_file_signatures[4] = {'A', 'I', 'X', 'F'};
    uint8_t copyright_str[6] = {'(', 'c', ')', 'C', 'R', 'I'};

    FILE *input_audio = fopen(stream_info->file_path, "rb");
    fread(format_cache, 1, 0x08, input_audio);      

    if (!memcmp(format_cache, cri_adx_file_signatures, 2)){             // CRI ADX format
        if (format_cache[4] == 0x03){                                       // standard ADX codec
            uint8_t sfa_data_offset[2] = {0x01, 0x1C};
            fread(format_cache + 8, 1, 0x118, input_audio);
            if (memcmp(format_cache + 0x02, sfa_data_offset, 2) || memcmp(format_cache + 0x11A, copyright_str, 6)){
                muxer_error(E__NOT_CONFORM_SFA, stream_info->file_path);                 // Sofdec Audio particular value
            }
            stream_info->codec_type = CRI_SOFDEC_AUDIO;
            stream_info->subaudio_channel_count = format_cache[7];
            stream_info->subaudio_streams_count = 1;
            stream_info->audio_sample_rate  = memread_u32be(format_cache + 0x08);
            stream_info->total_sample_count = memread_u32be(format_cache + 0x0C);
            stream_info->total_bitrate = 4.5 * stream_info->audio_sample_rate * stream_info->subaudio_channel_count;
            stream_info->audio_channels = stream_info->subaudio_channel_count;

        } else if (format_cache[4] == 0x11){              // standard AHX
//                 format_cache[4] == 0x10                // Dreamcast AHX
            muxer_error(E__TODO, "Multiplex CRI AHX audio stream");
        } else {
            muxer_error(E__NOT_CONFORM_ADX, stream_info->file_path);
        }
    } else if (!memcmp(format_cache, a52_syncinfo_syncword, 2)){        // ATSC A/52 (AC-3) 5.3.1
        stream_info->codec_type = DOBLY_DIGITAL;

        // ATSC A/52 (AC-3) 5.3.1
        uint8_t a52_syncinfo_fscode = (format_cache[4] >> 6) & 0x03;    
        switch (a52_syncinfo_fscode){                                       // ATSC A/52 (AC-3) 5.4.1.3
            case 0x00:
                stream_info->audio_sample_rate = 48000;
                break;
            case 0x01:
                stream_info->audio_sample_rate = 44100;
                break;
            case 0x02:
                stream_info->audio_sample_rate = 32000;
                break;
            default:
                muxer_error(E__NOT_CONFORM_A52, stream_info->file_path);
        }
        uint8_t a52_syncinfo_frmsizecod = format_cache[4] & 0x3F;
        size_t frame_size;
        if (a52_syncinfo_frmsizecod >= 38){
            muxer_error(E__NOT_CONFORM_A52, stream_info->file_path);
        }
        stream_info->total_bitrate = a52_frmsizecod_bitrate[a52_syncinfo_frmsizecod] * 1000;
        frame_size = 2 * a52_frame_size_code_table[a52_syncinfo_fscode][a52_syncinfo_frmsizecod];
        
        fseek(input_audio, frame_size, SEEK_SET);
        fread(format_cache + 8, 1, 2, input_audio);
        if (memcmp(format_cache, a52_syncinfo_syncword, 2)){
            muxer_error(E__NOT_CONFORM_A52, stream_info->file_path);
        }
        size_t file_size = fsize(input_audio);                              // ATSC A/52 (AC-3) 5.1: 6 coded audio blocks per frame, 256 new audio samples per channel
        stream_info->total_sample_count = (file_size / frame_size) * (256 * 6);

        // ATSC A/52 (AC-3) 5.3.2
        uint8_t a52_bsi_acmod = (format_cache[6] >> 5) & 0x07;
        uint8_t a52_bsi_lfeon;
        uint8_t lfeon_off_bit = 0;
        if((a52_bsi_acmod & 0x1) && (a52_bsi_acmod != 0x1)){
            lfeon_off_bit += 2;}
        if (a52_bsi_acmod & 0x4){
            lfeon_off_bit += 2;}
        if (a52_bsi_acmod == 0x2){
            lfeon_off_bit += 2;}
        if (lfeon_off_bit > 4){
            a52_bsi_lfeon = (format_cache[7] >> (7 - lfeon_off_bit + 5)) && 0x01;
        } else {
            a52_bsi_lfeon = (format_cache[6] >> (7 - lfeon_off_bit - 3)) && 0x01;
        }
        uint8_t audio_coding_mode[8] = {2, 1, 2, 3, 3, 4, 4, 5};    // ATSC A/52 (AC-3) 5.4.2.2
        stream_info->subaudio_channel_count = audio_coding_mode[a52_bsi_acmod] + a52_bsi_lfeon;
        stream_info->subaudio_streams_count = 1;
        stream_info->audio_channels = stream_info->subaudio_channel_count;

    } else if (!memcmp(format_cache, cri_aix_file_signatures, 4)){      // CRI AIX format

        uint32_t data_offset = memread_u32be(format_cache + 4);
        fseek(input_audio, data_offset + 2, SEEK_SET);
        fread(format_cache + 0x20, 1, 0x6, input_audio);
        if (memcmp(format_cache + 0x20, copyright_str, 6)){
            muxer_error(E__NOT_CONFORM_AIX, stream_info->file_path);
        }
        fseek(input_audio, 8, SEEK_SET);
        fread(format_cache + 8, 1, 0x40, input_audio);
        if (format_cache[0x18] != 1){
            muxer_error(E__NOT_CONFORM_AIX, stream_info->file_path);
        }
        stream_info->total_sample_count = memread_u32be(format_cache + 0x28);
        stream_info->audio_sample_rate  = memread_u32be(format_cache + 0x2C);

        uint8_t layer_count = format_cache[0x40];
        fread(format_cache, 1, 0x8 * layer_count, input_audio);
        uint32_t layer_simple_rate = memread_u32be(format_cache);
        if (layer_simple_rate != stream_info->audio_sample_rate){
            muxer_error(E__NOT_CONFORM_AIX, stream_info->file_path);}
        uint8_t layer_channel_count = format_cache[4];
        if (layer_channel_count > 4){
            muxer_error(E__NOT_CONFORM_AIX, stream_info->file_path);}
        for (int i = 1; i < layer_count; i++){
            if ((memread_u32be(format_cache + i * 8) != layer_channel_count) || (format_cache[4 + i * 8] != layer_channel_count)){
                muxer_error(E__NOT_CONFORM_AIX, stream_info->file_path);}}
        stream_info->subaudio_streams_count = layer_count;
        stream_info->subaudio_channel_count = layer_channel_count;
        stream_info->audio_channels = stream_info->subaudio_channel_count * stream_info->subaudio_streams_count;
        stream_info->total_bitrate = 4.5 * stream_info->audio_channels * stream_info->audio_sample_rate + 7680;

    } else {
        muxer_error(E__UNSUPPORTED_AUDIO_FORMAT, stream_info->file_path);
    }
}

#endif // __METADATA_READER__