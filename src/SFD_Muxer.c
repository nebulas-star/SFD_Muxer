// SPDX-FileCopyrightText: 2021-2026 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include <unistd.h>     // POSIX


#include "lib/memsearch.h"
// source: https://github.com/skeeto/optparse
// commit: a86877ed301d89a4eb64feb08f23af395aede2ed
#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "lib/optparse.h"

#include "muxer_error_report.h"
#include "metadata_reader.h"
#include "mpeg1_pack_builder.h"


void overwrite_question(char *file)
{
    printf("File \"%s\" already exists. Overwrite? [y/N]\n", file);
    char i;
    scanf("%c", &i);
    if(i != 'y' && i != 'Y'){
        printf("User termination.");
        exit(0);
    }
}

int main(int argc, char *argv[])
{
    unsigned int files_num = 0;
    unsigned int video_num = 0;
    char *video_file[16];
    unsigned int audio_num = 0;
    char *audio_file[32];
    unsigned int default_overwrite_flag = 0;


    bool output_file_set = false;
    char *output_file;
    bool sofdec_style_set = false;
    char *sofdec_style_file;
    bool sofdec_version_set = false;
    unsigned int sofdec_version = 1;
    bool audio_start_offset_set = false;
    unsigned int audio_start_offset = 0;


    int option;
    struct optparse options;
    optparse_init(&options, argv);
    while ((option = optparse(&options, ":A:M:Ta:d:ho:s:t:v:x:y")) != -1) {
        switch (option) {
        case 'y':
            default_overwrite_flag = 1;
            break;
        case 'v':
            video_file[video_num] = options.optarg;
            video_num++;
            break;
        case 'a':
            audio_file[audio_num] = options.optarg;
            audio_num++;
            break;
        case 'o':
            output_file = options.optarg;
            if (output_file_set) {muxer_error(E__OPT_WRONG_REP, (char* )&options.optopt);} else {output_file_set = true;}
            break;
        case 's':
            sofdec_style_file = options.optarg;
            if (sofdec_style_set) {muxer_error(E__OPT_WRONG_REP, (char* )&options.optopt);} else {sofdec_style_set = true;}
            break;
        case 'M':
            sscanf(options.optarg, "%u", &sofdec_version); 
            if (sofdec_version_set) {muxer_error(E__OPT_WRONG_REP, (char* )&options.optopt);} else {sofdec_version_set = true;}
            break;
        case 'A':
            sscanf(options.optarg, "%u", &audio_start_offset);
            if (audio_start_offset_set) {muxer_error(E__OPT_WRONG_REP, (char* )&options.optopt);} else {audio_start_offset_set = true;}
            break;

        case 'T':
        case 't':
        case 'x':
            muxer_error(E__TODO, "Multiplex CRITAGS extended data Stream");
            break;
        case 'd':
            muxer_error(E__TODO, "Multiplex with external Sofdec metadata dump");
            break;

        case 'h':
            muxer_error(E__HELP);
        case ':':
            muxer_error(E__OPT_NUM, (char* )&options.optopt);
        case '?':
            muxer_error(E__OPT_UNKNOWN, (char* )&options.optopt);
        }
    }
    files_num = video_num + audio_num;

// TODO: adding check: sofdec_style and other conflict opt

    if (output_file_set == false)
        muxer_error(011, 0);
    if (video_num > 16)
        muxer_error(020, 0);
    if (audio_num > 32)
        muxer_error(021, 0);
    if (sofdec_version > 2 || sofdec_version < 1)
        muxer_error(030, 0);
    if ((audio_start_offset + audio_num) > 32 )
        muxer_error(032, 0);


    int i;

    char audio_stream_start_index = audio_stream_0 + audio_start_offset;

    uint32_t bitrate = 0;
    uint32_t mux_rate = 0;

 /********input classification********/
    struct{
        video_stream_info** video_stream_layer;
        audio_stream_info** audio_stream_layer;
    } stream_info;
    stream_info.video_stream_layer = (video_stream_info**)malloc(video_num * sizeof(void*));
    stream_info.audio_stream_layer = (audio_stream_info**)malloc(audio_num * sizeof(void*));

    for (i = 0; i < video_num; i++){
        stream_info.video_stream_layer[i] = (video_stream_info*)malloc(sizeof(video_stream_info));
        stream_info.video_stream_layer[i]->file_path = video_file[i];
        stream_info.video_stream_layer[i]->stream_id = video_stream_0 + i;
        video_format_check(stream_info.video_stream_layer[i]);
    }
    for (i = 0; i < audio_num; i++){
        stream_info.audio_stream_layer[i] = (audio_stream_info*)malloc(sizeof(audio_stream_info));
        stream_info.audio_stream_layer[i]->file_path = audio_file[i];
        stream_info.audio_stream_layer[i]->stream_id = audio_stream_start_index + i;
        audio_format_check(stream_info.audio_stream_layer[i]);
    }

/*
    // If sample Sofdec, read parameter.
    // need rewrite.
    if (sofdec_style_set == 1)
    {
        int j;
        int video_bound, audio_bound;
        char file_style_cache[0x2000];

        FILE* input_cache = fopen(sofdec_style_file, "rb");
        fread(file_style_cache, 1, 0x2000, input_cache);
        if(  (file_style_cache[0x00] == 0x00 && file_style_cache[0x01] == 0x00 
                                             && file_style_cache[0x02] == 0x01 && file_style_cache[0x03] == 0xBA)
          && (file_style_cache[0x1020] == 0x53 && file_style_cache[0x1021] == 0x6F && file_style_cache[0x1022] == 0x66
                                               && file_style_cache[0x1023] == 0x64 && file_style_cache[0x1024] == 0x65
                                               && file_style_cache[0x1025] == 0x63 && file_style_cache[0x1026] == 0x53
                                               && file_style_cache[0x1027] == 0x74 && file_style_cache[0x1028] == 0x72
                                               && file_style_cache[0x1029] == 0x65 && file_style_cache[0x102A] == 0x61
                                                                                   && file_style_cache[0x102B] == 0x6D)
          )
        {
            mux_rate = rate_read(file_style_cache[0x09], file_style_cache[0x0A], file_style_cache[0x0B]);
            for (i = 0; i < 3; i++)
            {
                if (file_style_cache[(i * 0x800) + 0x0F] == 0xBB)
                {
                    j = audio_bound_read(file_style_cache[(i * 0x800) + 0x15]);
                    if (j == 0)
                        video_bound = video_bound_read(file_style_cache[(i * 0x800) + 0x16]);
                    else
                    {
                        audio_bound = j;
                        audio_start_offset = file_style_cache[(i * 0x800) + 0x18] - 0xC0;
                    }
                }
                else if(file_style_cache[(i * 0x800) + 0x0F] == 0xBF)
                {
                    if (file_style_cache[(i * 0x800) + 0x2C] == 0x32)
                        sofdec_version = 2;
                    if(i == 1)
                        i++;
                }
            }

            fclose(input_cache);
        }
        else
            muxer_error(120, sofdec_style_file);

        bitrate = 50 * mux_rate;

        if (video_bound != video_num)
            muxer_error(300, 0);
        if (audio_bound != audio_num)
            muxer_error(301, 0);
    } else
*/
    double audio_bitrate_count = 0;
    for (i = 0; i < audio_num; i++){
        audio_bitrate_count += ((stream_info.audio_stream_layer[i]->total_bitrate / 8) * (2048.0 / 2016.0));}
    bitrate = audio_bitrate_count + video_num * 0xCAF8F4;
    mux_rate = (bitrate + 49) / 50;
    if (mux_rate >= (2 << 22)){
        muxer_error(E__EXCEED_INPUT_STREAM_LIMIT);}


    //overwrite?
    if (access(output_file, 00) == 0 && default_overwrite_flag == 0)
        overwrite_question(output_file);


/******** build output file ********/
    char *pack_cache;
    pack_cache = (char*)malloc(0x800);
    memset(pack_cache, 0xFF, 0x800);
    size_t pack_offset = 0;
    FILE* output = fopen(output_file, "wb");
    uint32_t pack_index = 0;

    // metadata
    if (audio_num != 0){
        pack_offset += mpeg1_pack_header_build(pack_cache, pack_index, bitrate, mux_rate);
        pack_offset += mpeg1_system_header_build(pack_cache + pack_offset, mux_rate, 0, audio_num, audio_stream_start_index);
        mpeg1_padding_stream_packet_build(pack_cache + pack_offset, 0x800 - pack_offset);
        fwrite(pack_cache, 1, 0x800, output);
        pack_index++;
        pack_offset = 0;
    }
    if (video_num != 0){
        pack_offset += mpeg1_pack_header_build(pack_cache, pack_index, bitrate, mux_rate);
        pack_offset += mpeg1_system_header_build(pack_cache + pack_offset, mux_rate, 1, video_num, 0);
        mpeg1_padding_stream_packet_build(pack_cache + pack_offset, 0x800 - pack_offset);
        fwrite(pack_cache, 1, 0x800, output);
        pack_index++;
        pack_offset = 0;
    }

    pack_offset += mpeg1_pack_header_build(pack_cache, pack_index, bitrate, mux_rate);
// TODO: embed new Sofdec info pack build func
    mpeg1_padding_stream_packet_build(pack_cache + pack_offset, 0x800 - pack_offset);
    fwrite(pack_cache, 1, 0x800, output);
    pack_index++;
    pack_offset = 0;
    pack_offset += mpeg1_pack_header_build(pack_cache, pack_index, bitrate, mux_rate);
// TODO: CRITAGS stream
    mpeg1_padding_stream_packet_build(pack_cache + pack_offset, 0x800 - pack_offset);
    fwrite(pack_cache, 1, 0x800, output);
    pack_index++;
    pack_offset = 0;

    // audio && video stream
    FILE* input_audio[32];
    for (i = 0; i < audio_num; i++){
        input_audio[i] = fopen(stream_info.audio_stream_layer[i]->file_path, "rb");
    }
    FILE* input_video[16];
    uint32_t frame_decode_index[16] = {0};
    video_frame_info* frame_info[16];
    for (i = 0; i < video_num; i++){
        input_video[i] = fopen(stream_info.video_stream_layer[i]->file_path, "rb");
        frame_decode_index[i] = 0;
        frame_info[i] = (video_frame_info*)stream_info.video_stream_layer[i]->frame_map;
    }

    size_t subpack_count[48] = {0};
    size_t next_subpack_DTS[48] = {0};
    bool   stream_end[48] = {0};

    int next_input = 0;
    int read_size = 0;
    int end_stream = 0;
    while (end_stream != files_num){
        next_input = 0;
        for (i = 0; i < files_num; i++){
            if (stream_end[i] == false && next_subpack_DTS[next_input] > next_subpack_DTS[i]){
                next_input = i;}}
        subpack_count[next_input]++;
        if (next_input < audio_num){
            mpeg1_pack_header_build(pack_cache, pack_index, bitrate, mux_rate);
            read_size = fread(pack_cache + 12 + 13, 1, 0x7E0, input_audio[next_input]);
            if (read_size < 0x7E0){
                stream_end[next_input] = true;
                end_stream++;
            }
            if (read_size == 0){
                continue;
            }
            mpeg1_packet_header_build(pack_cache + 12, stream_info.audio_stream_layer[next_input]->stream_id, read_size + 13, next_subpack_DTS[next_input], 0);
            mpeg1_padding_stream_packet_build(pack_cache + 12 + 13 + read_size, 0x800 - 12 - 13 - read_size);
            fwrite(pack_cache, 1, 0x800, output);

            next_subpack_DTS[next_input] = subpack_count[next_input] * round(system_clock_frequency * 2016 / (stream_info.audio_stream_layer[next_input]->total_bitrate / 8) ) ;
        } else {
            next_input -= audio_num;
            mpeg1_pack_header_build(pack_cache, pack_index, bitrate, mux_rate);
            read_size = fread(pack_cache + 12 + 18, 1, 0x7E2, input_video[next_input]);
            if (read_size < 0x7E2){
                stream_end[next_input + audio_num] = true;
                end_stream++;
            }
            if (read_size == 0){
                continue;
            }
// 如果当前帧的下一帧没有在这个块开始的话
// 则这里什么都不更新
            if (frame_info[next_input]->next_frame == NULL ||
                frame_info[next_input]->next_frame->offset > (subpack_count[next_input + audio_num] * 0x7E2)){
                mpeg1_packet_header_build(pack_cache + 12, stream_info.video_stream_layer[next_input]->stream_id, read_size + 18, 0, 0);
                fwrite(pack_cache, 1, 0x800, output);
            } else {
                size_t this_DTS = round(system_clock_frequency * ((frame_decode_index[next_input] + 1)              / picture_rate[stream_info.video_stream_layer[next_input]->frame_rate]) );
                size_t this_PTS = round(system_clock_frequency * (frame_info[next_input]->next_frame->present_index / picture_rate[stream_info.video_stream_layer[next_input]->frame_rate]) );
                if (frame_info[next_input]->next_frame->type == I_frame || frame_info[next_input]->next_frame->type == P_frame){
                    mpeg1_packet_header_build(pack_cache + 12, stream_info.video_stream_layer[next_input]->stream_id, read_size + 18, this_PTS, this_DTS);
                } else if (frame_info[next_input]->next_frame->type == B_frame){
                    mpeg1_packet_header_build(pack_cache + 12, stream_info.video_stream_layer[next_input]->stream_id, read_size + 18, this_PTS, 0);
                } else {
                    muxer_error(E__UNSUPPORTED_VIDEO_FORMAT, stream_info.video_stream_layer[next_input]->file_path);}
                fwrite(pack_cache, 1, 0x800, output);

                do {
                    if (frame_info[next_input]->next_frame == NULL ||
                        frame_info[next_input]->next_frame->offset > subpack_count[next_input + audio_num] * 0x7E2){
                        next_subpack_DTS[next_input + audio_num] = round(system_clock_frequency * frame_decode_index[next_input] / picture_rate[stream_info.video_stream_layer[next_input]->frame_rate] );
                        break;
                    }
                    frame_info[next_input] = frame_info[next_input]->next_frame;
                    frame_decode_index[next_input]++;
                } while (1);
            }
    

            if (read_size <= 0x7DB) {
                mpeg1_padding_stream_packet_build(pack_cache + 12 + 18 + read_size, 0x800 - 12 - 18 - read_size);
            } else if (read_size < 0x7E1){
                memmove(pack_cache + 12 + 6 + (read_size - 0x7DB), pack_cache + 12 + 6, read_size + 12);
                memset(pack_cache + 12 + 6, 0xFF, read_size - 0x7DB);
            }
            next_input += audio_num;
        }
        pack_index++;
        pack_offset = 0;
        if (next_subpack_DTS[next_input] >= MAX_ENC_VALUE){
            muxer_error(E__EXCEED_REPLY_TIME_LIMIT);}
    }
    mpeg1_end_block_build(pack_cache);
    fwrite(pack_cache, 1, 0x800, output);

    printf("\nMux complete.\n");
    return 0;
}

