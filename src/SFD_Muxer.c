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
    int i;
    stream_info stream_info;
    stream_info.video_stream_num = 0;
    stream_info.audio_stream_num = 0;
    stream_info.private_stream_num = 0;

    char *video_file[16];
    char *audio_file[32];
    unsigned int default_overwrite_flag = 0;
    bool output_file_set = false;
    char *output_file;
    bool sofdec_version_set = false;
    unsigned int sofdec_version = 1;
    bool audio_start_offset_set = false;
    unsigned int audio_start_offset = 0;
    bool have_critags = false;

    int option;
    struct optparse options;
    optparse_init(&options, argv);
    while ((option = optparse(&options, ":A:M:Ta:d:ho:s:t:v:x:y")) != -1) {
        switch (option) {
        case 'y':
            default_overwrite_flag = 1;
            break;
        case 'v':
            video_file[stream_info.video_stream_num] = options.optarg;
            stream_info.video_stream_num++;
            break;
        case 'a':
            audio_file[stream_info.audio_stream_num] = options.optarg;
            stream_info.audio_stream_num++;
            break;
        case 'o':
            output_file = options.optarg;
            if (output_file_set) {muxer_error(E__OPT_WRONG_REP, (char* )&options.optopt);} else {output_file_set = true;}
            break;
        case 'M':
            sscanf(options.optarg, "%u", &sofdec_version); 
            if (sofdec_version_set) {muxer_error(E__OPT_WRONG_REP, (char* )&options.optopt);} else {sofdec_version_set = true;}
            if (sofdec_version < 1 || sofdec_version > 2){muxer_error(E__OPT_WRONG_VALUE, (char* )&options.optopt);}
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
        case 'h':
            muxer_error(E__HELP);
        case ':':
            muxer_error(E__OPT_NUM, (char* )&options.optopt);
        case '?':
            muxer_error(E__OPT_UNKNOWN, (char* )&options.optopt);
        }
    }
    if (output_file_set == false)
        muxer_error(E__OPT_NON_NESSARY, "o");
    if (stream_info.video_stream_num > 16 || stream_info.audio_stream_num > 32 || (audio_start_offset + stream_info.audio_stream_num) > 32)
        muxer_error(E__EXCEED_INPUT_STREAM_LIMIT);
    //overwrite?
    if (access(output_file, 00) == 0 && default_overwrite_flag == 0)
        overwrite_question(output_file);


 /********input classification********/
    stream_info.total_stream_num = stream_info.video_stream_num + stream_info.audio_stream_num + stream_info.private_stream_num;
    stream_info.video_stream_layer = (video_stream_info**)malloc(stream_info.video_stream_num * sizeof(void*));
    stream_info.audio_stream_layer = (audio_stream_info**)malloc(stream_info.audio_stream_num * sizeof(void*));
    for (i = 0; i < stream_info.video_stream_num; i++){
        stream_info.video_stream_layer[i] = (video_stream_info*)malloc(sizeof(video_stream_info));
        stream_info.video_stream_layer[i]->file_path = video_file[i];
        stream_info.video_stream_layer[i]->stream_id = video_stream_0 + i;
        video_format_check(stream_info.video_stream_layer[i]);
    }
    for (i = 0; i < stream_info.audio_stream_num; i++){
        stream_info.audio_stream_layer[i] = (audio_stream_info*)malloc(sizeof(audio_stream_info));
        stream_info.audio_stream_layer[i]->file_path = audio_file[i];
        stream_info.audio_stream_layer[i]->stream_id = audio_stream_0 + audio_start_offset + i;
        audio_format_check(stream_info.audio_stream_layer[i]);
    }
    double audio_bitrate_count = 0;
    for (i = 0; i < stream_info.audio_stream_num; i++){
        audio_bitrate_count += ((stream_info.audio_stream_layer[i]->total_bitrate / 8) * (2048.0 / 2016.0));}
    stream_info.bitrate_of_system_stream = audio_bitrate_count + stream_info.video_stream_num * 0xCAF8F4;
    uint32_t mux_rate = (stream_info.bitrate_of_system_stream + 49) / 50;
    if (mux_rate >= (2 << 22)){
        muxer_error(E__EXCEED_INPUT_STREAM_LIMIT);}

    stream_info.longest_audio_playbk_time = 0;
    stream_info.longest_video_playbk_time = 0;
    stream_info.maximum_video_frame = 0;
    stream_info.maximum_video_picture_size = 0;     // TODO
    stream_info.average_video_picture_size = 0;     // TODO
    stream_info.maximum_video_stream_bitrate = 0;
    uint32_t replay_time;
    uint32_t stream_rate;
    for (i = 0; i < stream_info.audio_stream_num; i++){
        replay_time = 1000.0 * (stream_info.audio_stream_layer[i]->total_sample_count / stream_info.audio_stream_layer[i]->audio_sample_rate);
        if (stream_info.longest_audio_playbk_time < replay_time){
            stream_info.longest_audio_playbk_time = replay_time;}
    }
    for (i = 0; i < stream_info.video_stream_num; i++){
        replay_time = 1000.0 * (stream_info.video_stream_layer[i]->total_frames_count / picture_rate[stream_info.video_stream_layer[i]->frame_rate]);
        if (stream_info.longest_video_playbk_time < replay_time){
            stream_info.longest_video_playbk_time = replay_time;}
        if (stream_info.maximum_video_frame < stream_info.video_stream_layer[i]->total_frames_count){
            stream_info.maximum_video_frame = stream_info.video_stream_layer[i]->total_frames_count;}
        stream_rate = fsize_d(stream_info.video_stream_layer[i]->file_path) / (replay_time / 1000.0);
        if (stream_info.maximum_video_stream_bitrate < stream_rate){
            stream_info.maximum_video_stream_bitrate = stream_rate;}
        }

/******** build output file ********/
    char *pack_cache;
    pack_cache = (char*)malloc(0x800);
    memset(pack_cache, 0xFF, 0x800);
    size_t pack_offset = 0;
    FILE* output = fopen(output_file, "wb");
    uint32_t pack_index = 0;

    // metadata
    if (stream_info.audio_stream_num != 0){
        pack_offset += mpeg1_pack_header_build(pack_cache, pack_index, stream_info.bitrate_of_system_stream, mux_rate);
        pack_offset += mpeg1_system_header_build(pack_cache + pack_offset, mux_rate, 0, stream_info.audio_stream_num, audio_stream_0 + audio_start_offset);
        mpeg1_padding_stream_packet_build(pack_cache + pack_offset, 0x800 - pack_offset);
        fwrite(pack_cache, 1, 0x800, output);
        pack_index++;
        pack_offset = 0;
    }
    if (stream_info.video_stream_num != 0){
        pack_offset += mpeg1_pack_header_build(pack_cache, pack_index, stream_info.bitrate_of_system_stream, mux_rate);
        pack_offset += mpeg1_system_header_build(pack_cache + pack_offset, mux_rate, 1, stream_info.video_stream_num, 0);
        mpeg1_padding_stream_packet_build(pack_cache + pack_offset, 0x800 - pack_offset);
        fwrite(pack_cache, 1, 0x800, output);
        pack_index++;
        pack_offset = 0;
    }
    // sofdec metadata
    memset(pack_cache, 0, 0x800);
    mpeg1_pack_header_build(pack_cache, pack_index, stream_info.bitrate_of_system_stream, mux_rate);
    mpeg1_packet_header_build(pack_cache + 12, private_stream_2, 0x800 - 12, 0, 0);
    if (sofdec_version == 2){
        pack_cache[0x12] = 0x08;
        sofdec_metadata_v2_build(pack_cache + 0x20, stream_info);
    } else {
        sofdec_metadata_v1_build(pack_cache + 0x20, output_file, stream_info);}
    fwrite(pack_cache, 1, 0x800, output);
    pack_index++;
    pack_offset = 0;

    if (have_critags){
// TODO: CRITAGS stream
    }

    // audio && video stream
    FILE* input_audio[32];
    for (i = 0; i < stream_info.audio_stream_num; i++){
        input_audio[i] = fopen(stream_info.audio_stream_layer[i]->file_path, "rb");
    }
    FILE* input_video[16];
    uint32_t frame_decode_index[16] = {0};
    video_frame_info* frame_info[16];
    for (i = 0; i < stream_info.video_stream_num; i++){
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
    while (end_stream != stream_info.total_stream_num){
        next_input = 0;
        for (i = 0; i < stream_info.total_stream_num; i++){
            if (stream_end[i] != true){
                if (next_subpack_DTS[next_input] > next_subpack_DTS[i]){
                    next_input = i;
                }
            }
        }
        if (next_input == 0){
        }
        subpack_count[next_input]++;
        if (next_input < stream_info.audio_stream_num){
            mpeg1_pack_header_build(pack_cache, pack_index, stream_info.bitrate_of_system_stream, mux_rate);
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
            next_input -= stream_info.audio_stream_num;
            mpeg1_pack_header_build(pack_cache, pack_index, stream_info.bitrate_of_system_stream, mux_rate);
            read_size = fread(pack_cache + 12 + 18, 1, 0x7E2, input_video[next_input]);
            if (read_size < 0x7E2){
                stream_end[next_input + stream_info.audio_stream_num] = true;
                end_stream++;
            }
            if (read_size == 0){
                continue;
            }
            if (frame_info[next_input]->next_frame == NULL ||
                frame_info[next_input]->next_frame->offset > (subpack_count[next_input + stream_info.audio_stream_num] * 0x7E2)){
                mpeg1_packet_header_build(pack_cache + 12, stream_info.video_stream_layer[next_input]->stream_id, read_size + 18, 0, 0);
            } else {
                size_t this_DTS = round(system_clock_frequency * ((frame_decode_index[next_input] + 1)              / picture_rate[stream_info.video_stream_layer[next_input]->frame_rate]) );
                size_t this_PTS = round(system_clock_frequency * (frame_info[next_input]->next_frame->present_index / picture_rate[stream_info.video_stream_layer[next_input]->frame_rate]) );
                if (frame_info[next_input]->next_frame->type == I_frame || frame_info[next_input]->next_frame->type == P_frame){
                    mpeg1_packet_header_build(pack_cache + 12, stream_info.video_stream_layer[next_input]->stream_id, read_size + 18, this_PTS, this_DTS);
                } else if (frame_info[next_input]->next_frame->type == B_frame){
                    mpeg1_packet_header_build(pack_cache + 12, stream_info.video_stream_layer[next_input]->stream_id, read_size + 18, this_PTS, 0);
                } else {
                    muxer_error(E__UNSUPPORTED_VIDEO_FORMAT, stream_info.video_stream_layer[next_input]->file_path);}
                do {
                    if (frame_info[next_input]->next_frame == NULL ||
                        frame_info[next_input]->next_frame->offset > subpack_count[next_input + stream_info.audio_stream_num] * 0x7E2){
                        next_subpack_DTS[next_input + stream_info.audio_stream_num] = round(system_clock_frequency * frame_decode_index[next_input] / picture_rate[stream_info.video_stream_layer[next_input]->frame_rate] );
                        break;
                    }
                    frame_info[next_input] = frame_info[next_input]->next_frame;
                    frame_decode_index[next_input]++;
                } while (1);
            }
            if (read_size <= 0x7DB) {
                mpeg1_padding_stream_packet_build(pack_cache + 12 + 18 + read_size, 0x800 - (12 + 18 + read_size));
            } else if (read_size < 0x7E1){
                memmove(pack_cache + 12 + 6 + (read_size - 0x7DB), pack_cache + 12 + 6, read_size + 12);
                memset(pack_cache + 12 + 6, 0xFF, read_size - 0x7DB);
            }
            fwrite(pack_cache, 1, 0x800, output);
            next_input += stream_info.audio_stream_num;
        }
        if (stream_end[next_input] == true){
            next_subpack_DTS[next_input] = MAX_ENC_VALUE;
        }
        pack_index++;
        pack_offset = 0;
        if (next_subpack_DTS[next_input] > MAX_ENC_VALUE){
            muxer_error(E__EXCEED_REPLY_TIME_LIMIT);}
    }
    mpeg1_end_block_build(pack_cache);
    fwrite(pack_cache, 1, 0x800, output);

    printf("\nMux complete.\n");
    return 0;
}

