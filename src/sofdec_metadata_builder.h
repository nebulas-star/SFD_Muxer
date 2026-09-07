// SPDX-FileCopyrightText: 2026 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#ifndef __SOFDEC_METADATA_BUILDER__
#define __SOFDEC_METADATA_BUILDER__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>   // POSIX

static void memmove_u8le (char* str1, uint8_t  str2) {
    memmove(str1, &str2, 1);
}
static void memmove_u16le(char* str1, uint16_t str2) {

    uint8_t cache[2] = { str2       & 0xFF,
                        (str2 >> 8) & 0xFF};
    memmove(str1, &cache, 2);
}
static void memmove_u32le(char* str1, uint32_t str2) {
    uint8_t cache[4] = { str2        & 0xFF,
                        (str2 >> 8)  & 0xFF,
                        (str2 >> 16) & 0xFF,
                        (str2 >> 24) & 0xFF};
    memmove(str1, &cache, 4);
}
static void memmove_u8be (char* str1, uint8_t  str2) {
    memmove(str1, &str2, 1);
}
static void memmove_u16be(char* str1, uint16_t str2) {

    uint8_t cache[2] = { (str2 >> 8) & 0xFF,
                          str2       & 0xFF};
    memmove(str1, &cache, 2);
}
static void memmove_u32be(char* str1, uint32_t str2) {
    uint8_t cache[4] = {(str2 >> 24) & 0xFF,
                        (str2 >> 16) & 0xFF,
                        (str2 >> 8)  & 0xFF,
                         str2        & 0xFF};
    memmove(str1, &cache, 4);
}

#include "mpeg1_pack_builder.h"

#define MUXER_INFO_STRING   "SFD_Muxer Ver.0.5.1 by Nebulas"
#define SFD_MUX_LIB_INTER_MAJOR_VERSION 0x02    // 0x02 for all known offical lib 
#define SFD_MUX_LIB_INTER_MINOR_VERSION 0xFF

static void build_short_filename(char* origin_filename, char* short_filename) {
    char* last_slash = strrchr(origin_filename, '/');
    char* last_backslash = strrchr(origin_filename, '\\');
    char* filename_start = origin_filename;
    
    if (last_slash > filename_start) filename_start = last_slash + 1;
    if (last_backslash > filename_start) filename_start = last_backslash + 1;

    char* dot = strrchr(filename_start, '.');
    char* ext_start = NULL;
    size_t filename_len;
    if (dot == NULL)
        filename_len = strlen(filename_start);
    else if (strlen(dot) == 1)
        filename_len = strlen(filename_start) - 1;
    else {
        filename_len = dot - filename_start;
        ext_start = dot + 1;
    }


    char file_name[9] = "        ";
    char ext_name[4] = "   ";
    char default_ext[4] = "sfd";
    size_t ext_len;
    if (ext_start != NULL){
        ext_len = strlen(ext_start);
        if (ext_len > 3)
            ext_len = 3;
    } else {
        ext_start = default_ext;
        ext_len = 3;
    }

    if (filename_len <= 8){
        memcpy(file_name, filename_start, filename_len);
        memcpy(ext_name, ext_start, ext_len);
    } else {
        if( *filename_start == '.')
            filename_start = filename_start + 1;
        memcpy(file_name, filename_start, 6);
        for (int i = 0; i < 6; i++) {
            if (islower(file_name[i]))
                file_name[i] = toupper(file_name[i]);
            else if ( !isupper(file_name[i]) && !isdigit(file_name[i]) )
                file_name[i] = '_';
        }
        file_name[6] = '~';
        file_name[7] = '1';

        memcpy(ext_name, ext_start, ext_len);
        for (int i = 0; i < ext_len; i++) {
            if (islower(ext_name[i]))
                file_name[i] = toupper(file_name[i]);
            else if ( !isupper(ext_name[i]) && !isdigit(ext_name[i]) )
                file_name[i] = '_';
        }
    }
    snprintf(short_filename, 13, "%s.%s", file_name, ext_name);
}
static void build_sofdec_timestamp(time_t raw_timestamp, char* sofdec_style_timestamp){
    struct tm* timeinfo = localtime(&raw_timestamp);
    strftime(sofdec_style_timestamp, 13, "%Y%m%d%H%M", timeinfo);
}

enum video_codec {
    CRI_SOFDEC_VIDEO,   // Sofdec/Video     *.sfv   // unused in this version
    MPEG1_VIDEO,        // MPEG1/Video      *.m1v
    MPEG_VIDEO,         // MPEG/Video       *.mpv   // Deprecated?
    MPEG2_VIDEO         // MPEG2/Video      *.m2v
};
enum audio_codec {
    CRI_SOFDEC_AUDIO,   // Sofdec/Audio     *.sfd
    MPEG_AUDIO,         // MPEG/Audio               // Deprecated, may support in early dev version?
    DOBLY_DIGITAL,      // Dobly Digital    *.ac3
    CRI_AHX,            // AHX              *.ahx
    CRI_AIX             // AIX              *.aix
};

typedef struct{
    char* file_path;

    uint8_t     stream_id;
    uint8_t     codec_type;
    uint16_t    frame_width;
    uint16_t    frame_hight;
    uint8_t     frame_rate;

// for calculate playback time
    uint32_t    total_frames_count;
// for caculate DTS and PTS
    void*       frame_map;
// for cacilate average bitrate for metadata v2
    uint32_t    file_size;

// only use in "Sofdec Craft" output
    uint8_t  YUV_Conversion_mode;
    uint8_t  picture_type;
    bool     fixed_bitrate_flag;
    bool     fixed_SHC_flag;
    uint8_t  advanced_feature;
    uint8_t  FX_type;
    uint8_t  GOP_N;
    uint8_t  GOP_M;
} video_stream_info;
typedef struct{
    char* file_path;

    uint8_t     stream_id;
    uint8_t     codec_type;
    uint8_t     mepg_audio_layer;  // Deprecated
    uint8_t     audio_channels;
    uint32_t    audio_sample_rate; // Hz

// for calculate playback time
    uint32_t    total_sample_count;

// for metadata v2
    uint8_t     subaudio_channel_count;
    uint8_t     subaudio_streams_count;
    uint32_t    total_bitrate;
} audio_stream_info;
typedef struct{
    uint8_t total_stream_num;
    uint8_t video_stream_num;
    uint8_t audio_stream_num;
    uint8_t private_stream_num;

    video_stream_info** video_stream_layer;
    audio_stream_info** audio_stream_layer;

    uint32_t bitrate_of_system_stream;
    uint32_t longest_audio_playbk_time;     // ms
    uint32_t longest_video_playbk_time;     // ms
    uint32_t maximum_video_frame;

// extend in 2002~2005 version:
    uint32_t maximum_video_picture_size;
    uint32_t average_video_picture_size;

// for metadata version 2
    uint32_t maximum_video_stream_bitrate;
} stream_info;

void video_stream_metadata_v1_build(char* metadata_block, video_stream_info stream_info){
    char short_source_name[13];
    build_short_filename(stream_info.file_path, short_source_name);
    memmove      (metadata_block,        short_source_name, 12);

    char update_time[13];
    struct stat fileStat; stat(stream_info.file_path, &fileStat);
    build_sofdec_timestamp(fileStat.st_mtime, update_time);
    memmove      (metadata_block + 0xC,  update_time, 12);

    memmove_u8le (metadata_block + 0x18, stream_info.stream_id);
    memmove_u8le (metadata_block + 0x19, stream_info.codec_type);
    memmove_u8le (metadata_block + 0x1A, 0xFF);
    memmove_u8le (metadata_block + 0x1B, 0xFF);

    uint32_t frame_info = ((stream_info.frame_width & 0xFFF) << 20) | ((stream_info.frame_hight & 0xFFF) << 8) | (stream_info.frame_rate & 0xFF);
    memmove_u32be(metadata_block + 0x1C, frame_info);

    // "Sofdec Craft" output extend metadata byte ignore
}
void audio_stream_metadata_v1_build(char* metadata_block, audio_stream_info stream_info){
    char short_source_name[13];
    build_short_filename(stream_info.file_path, short_source_name);
    memmove      (metadata_block,        short_source_name, 12);

    char update_time[13];
    struct stat fileStat; stat(stream_info.file_path, &fileStat);
    build_sofdec_timestamp(fileStat.st_mtime, update_time);
    memmove      (metadata_block + 0xC,  update_time, 12);

    memmove_u8le (metadata_block + 0x18, stream_info.stream_id);
    memmove_u8le (metadata_block + 0x19, stream_info.codec_type);
    memmove_u8le (metadata_block + 0x1A, 0x00);
    memmove_u8le (metadata_block + 0x1B, stream_info.audio_channels);
    memmove_u32le(metadata_block + 0x1C, stream_info.audio_sample_rate);
}
void sofdec_metadata_v1_build(char* metadata_block, char* output_filename, stream_info input_stream_info){

    memmove      (metadata_block,         "SofdecStream            ", 24);
    memmove_u8le (metadata_block + 0x18, SFD_MUX_LIB_INTER_MAJOR_VERSION);
    memmove_u8le (metadata_block + 0x19, SFD_MUX_LIB_INTER_MINOR_VERSION);

    char short_output_name[13];
    build_short_filename(output_filename, short_output_name);
    time_t rawtime; time(&rawtime);
    char output_time[13];
    build_sofdec_timestamp(rawtime, output_time);
    memmove      (metadata_block + 0x20, short_output_name, 12);
    memmove      (metadata_block + 0x2C, output_time, 12);

    int infostrlen = strlen(MUXER_INFO_STRING);
    if ( infostrlen < 0x20 ){
        memmove(metadata_block + 0x40, MUXER_INFO_STRING, infostrlen);
        memset (metadata_block + 0x40 + infostrlen, 0x20, 0x20 - infostrlen);
    } else {
        memmove(metadata_block + 0x40, MUXER_INFO_STRING, 0x20);
    }

    uint32_t header_size                = 0x00000800;
    uint8_t  pack_creation_type         = 0;
    uint16_t packet_length_field_size   = 2;  
    uint32_t packet_size                = 0x00000800;
    memmove_u32le(metadata_block + 0x60, header_size);
    memmove_u8le (metadata_block + 0x64, pack_creation_type);
    memmove_u16le(metadata_block + 0x68, packet_length_field_size);
    memmove_u32le(metadata_block + 0x6C, packet_size);

    memmove_u8le (metadata_block + 0x90, input_stream_info.total_stream_num);
    memmove_u8le (metadata_block + 0x91, input_stream_info.video_stream_num);
    memmove_u8le (metadata_block + 0x92, input_stream_info.audio_stream_num);
    memmove_u8le (metadata_block + 0x93, input_stream_info.private_stream_num);
    memmove_u32le(metadata_block + 0x94, input_stream_info.bitrate_of_system_stream);
    memmove_u32le(metadata_block + 0x98, input_stream_info.longest_audio_playbk_time);
    memmove_u32le(metadata_block + 0x9C, input_stream_info.longest_video_playbk_time);
    memmove_u32le(metadata_block + 0xA0, input_stream_info.maximum_video_frame);
    memmove_u32le(metadata_block + 0xA4, input_stream_info.maximum_video_picture_size);
    memmove_u32le(metadata_block + 0xA8, input_stream_info.average_video_picture_size);

    // Commit string ignore

    int count = 0;

    if (input_stream_info.private_stream_num != 0){
        memmove     (metadata_block + 0x160, "<SFM_P2>.TMP", 12);
        memmove     (metadata_block + 0x16C, output_time, 12);
        memmove_u8le(metadata_block + 0x178, 0xBF);
        count++;
    }
    for (int i = 0; i < input_stream_info.video_stream_num; i++){
        video_stream_metadata_v1_build(metadata_block + 0x160 + 0x40 * count, *input_stream_info.video_stream_layer[i]);
        count++;
    }
    for (int i = 0; i < input_stream_info.audio_stream_num; i++){
        audio_stream_metadata_v1_build(metadata_block + 0x160 + 0x40 * count, *input_stream_info.audio_stream_layer[i]);
        count++;
    }
}

void audio_stream_metadata_v2_build(char *metadata_block, audio_stream_info stream_info){
    memmove_u8be (metadata_block, stream_info.stream_id);

    uint8_t substream_channel[2];
    if (stream_info.subaudio_streams_count > 0x0F){
        substream_channel[0] = (stream_info.subaudio_channel_count & 0x0F) << 4 | 0xF0;
    } else {
        substream_channel[0] = (stream_info.subaudio_channel_count & 0x0F) << 4 | (stream_info.subaudio_streams_count & 0x0F);
    }
    if (stream_info.audio_channels > 0x0F){
        substream_channel[1] = 0xF0;
    } else {
        substream_channel[1] = (stream_info.audio_channels & 0x0F) << 4;
    }
    memmove(metadata_block + 0x1, substream_channel, 2);
    memmove_u16be(metadata_block + 0x3, stream_info.audio_sample_rate);

    if (stream_info.codec_type == CRI_SOFDEC_AUDIO){
        memmove_u32be(metadata_block + 0x6, stream_info.total_sample_count);}

    memmove_u32be(metadata_block + 0xA, stream_info.total_bitrate);
}
void video_stream_metadata_v2_build(char *metadata_block, video_stream_info stream_info){
    memmove_u8be (metadata_block,     stream_info.stream_id);
    memmove_u16be(metadata_block + 2, stream_info.frame_width);
    memmove_u16be(metadata_block + 4, stream_info.frame_hight);
    memmove_u16be(metadata_block + 8, stream_info.total_frames_count);
    uint16_t frame_rate_ms = 1000 * picture_rate[stream_info.frame_rate];
    memmove_u16be(metadata_block + 10, frame_rate_ms);
    // other parts unknown
}
void sofdec_metadata_v2_build(char* metadata_block, stream_info input_stream_info){
    memmove      (metadata_block,         "SofdecStream2           ", 24);
    memmove_u8be (metadata_block + 0x18, 0x02);
    memmove_u8be (metadata_block + 0x19, 0x02);     // 0x02 for "Sofdec Multiplexer", 0x03 for "Sofdec Craft"
    memmove_u8be (metadata_block + 0x1A, SFD_MUX_LIB_INTER_MAJOR_VERSION);
    memmove_u8be (metadata_block + 0x1B, SFD_MUX_LIB_INTER_MINOR_VERSION);

    int infostrlen = strlen(MUXER_INFO_STRING);
    if ( infostrlen < 0x20 ){
        memmove(metadata_block + 0x40, MUXER_INFO_STRING, infostrlen);
    } else {
        memmove(metadata_block + 0x40, MUXER_INFO_STRING, 0x20);}

    memmove_u8be (metadata_block + 0xA0, input_stream_info.total_stream_num);
    memmove_u8be (metadata_block + 0xA1, input_stream_info.audio_stream_num);
    memmove_u8be (metadata_block + 0xA2, input_stream_info.video_stream_num);
    memmove_u8be (metadata_block + 0xA3, input_stream_info.private_stream_num);
    memmove_u32be(metadata_block + 0xA4, input_stream_info.maximum_video_stream_bitrate);
    memmove_u32be(metadata_block + 0xA8, input_stream_info.longest_audio_playbk_time);
    memmove_u32be(metadata_block + 0xAC, input_stream_info.longest_video_playbk_time);
    memmove_u32be(metadata_block + 0xB0, input_stream_info.maximum_video_frame);
    memmove_u32le(metadata_block + 0xB4, input_stream_info.maximum_video_picture_size);
    memmove_u32le(metadata_block + 0xB8, input_stream_info.average_video_picture_size);

    // unknown metadata: 0xBC
    // "Sofdec Craft" output extend metadata: 0xC4

    for (int i = 0; i < input_stream_info.audio_stream_num; i++){
        audio_stream_metadata_v1_build(metadata_block + 0x1A0 + 0x10 * i, *input_stream_info.audio_stream_layer[i]);}    
    for (int i = 0; i < input_stream_info.video_stream_num; i++){
        video_stream_metadata_v1_build(metadata_block + 0x3A0 + 0x40 * i, *input_stream_info.video_stream_layer[i]);}
    if (input_stream_info.private_stream_num != 0){
        memmove_u8be(metadata_block + 0x7C0, 0xBF);}
}

#endif // __SOFDEC_METADATA_BUILDER__