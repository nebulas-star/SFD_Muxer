#ifndef __MPRG_BLOCK_PRINT_H__
#define __MPRG_BLOCK_PRINT_H__

#include "SFD_Muxer_Error.h"

unsigned long long int SCR_made(unsigned long long int block_num, unsigned long long int mux_rate)
{
    double a, c, d;
    unsigned long long int e;
    if (block_num ==0)
        e = 0;
    else
    {
        a = (0x800 * block_num) - 0x8;
        c = (90001 / 50) * a;
        d = c / mux_rate;
        e = d + 0.5;
    }
    return e;
}
void SCR_block_print(FILE *out_file, unsigned long long int block_num, unsigned long long int mux_rate, int ansi_codepage)
{
    unsigned long long int SCR;
    SCR = SCR_made(block_num, mux_rate);
    if (SCR > 0x1FFFFFFFF)
        error(201, 0, ansi_codepage);
    unsigned long long int a, b, c, d, e;
    a =  (SCR >> 29) | 0x21;
    b =  (SCR >> 22) & 0xFF;
    c = ((SCR >> 14) & 0xFE) | 0x01;
    d =  (SCR >>  7) & 0xFF;
    e = ((SCR <<  1) & 0xFE) | 0x01;
    char SCR_block[5] = {a, b, c, d, e};
    fwrite(SCR_block, 1, 5, out_file);
}
void rate_block_print(FILE *out_file, unsigned long long int mux_rate)
{
    unsigned long long int a, b, c;
    a = (mux_rate >> 15) | 0x80;
    b = (mux_rate >> 7) & 0xFF;
    c = ((mux_rate << 1) & 0xFE) | 0x01;
    char mux_rate_block[3] = {a, b, c};
    fwrite(mux_rate_block, 1, 3, out_file);
}
void length_block_print(FILE *out_file, unsigned long int length)
{
    unsigned long int a, b;
    a = length >> 8;
    b = length & 0xFF;
    char length_block[2] = {a, b};
    fwrite(length_block, 1, 2, out_file);
}
void bound_blocks_print(FILE *out_file, unsigned int video_bound, unsigned int audio_bound)
{
    unsigned long int a, b;
    a = (audio_bound << 2) | 0x02;
    b = video_bound | 0x20;
    char bound_block[2] = {a, b};
    fwrite(bound_block, 1, 2, out_file);
}
void reserved_byte(FILE *out_file, unsigned int byte_num)
{
    int i = 0;
    char reserved[1] = {0xFF};
    if (byte_num != 0)
        for (i = 0; i < byte_num; i++)
            fwrite(reserved, 1, 1, out_file);
}
void STD_buffer_block_print(FILE *out_file, int STD_buffer_scale, unsigned int STD_buffer_size)
{
    unsigned int i = 0x40 | ((STD_buffer_scale << 5) | (STD_buffer_size >> 8));
    unsigned int j = STD_buffer_size & 0xFF;
    char STD_bufffer_block[2] = {i, j};
    fwrite(STD_bufffer_block, 1, 2, out_file);
}
void PTS_DTS_block_print(FILE *out_file, char TS_mark, unsigned long long xTS)
{
    unsigned long long int a = 0, b, c, d, e;
    if (TS_mark == 0x01)
        a = (xTS >> 29) | 0x11;
    else if (TS_mark == 0x02)
        a = (xTS >> 29) | 0x21;
    else if (TS_mark == 0x03)
        a = (xTS >> 29) | 0x31;
    b =  (xTS >> 22) & 0xFF;
    c = ((xTS >> 14) & 0xFE) | 0x01;
    d =  (xTS >>  7) & 0xFF;
    e = ((xTS <<  1) & 0xFE) | 0x01;
    char PTS_DTS_block[5] = {a, b, c, d, e};
    fwrite(PTS_DTS_block, 1, 5, out_file);
}

void pack_head_print(FILE *out_file, unsigned long long int block_num, unsigned long long int mux_rate, int ansi_codepage)
{
    char pack_start_code[4] = {0x00, 0x00, 0x01, 0xBA};
    fwrite(pack_start_code, 1, 4, out_file);
    SCR_block_print(out_file, block_num, mux_rate, ansi_codepage);
    rate_block_print(out_file, mux_rate);
}
void system_head_print(FILE *out_file, unsigned long long int mux_rate, unsigned int video_bound, 
                                                                       unsigned int audio_bound, unsigned int audio_ID_start_offset)
{
    int i = 0;
    char system_header_start_code[4] = {0x00, 0x00, 0x01, 0xBB};
    fwrite(system_header_start_code, 1, 4, out_file);
    unsigned long int header_length = 6 + (3 * (video_bound + audio_bound));
    length_block_print(out_file, header_length);
    rate_block_print(out_file, mux_rate);
    bound_blocks_print(out_file, video_bound, audio_bound);
    reserved_byte(out_file, 1);
    if (audio_bound != 0)
    {
        char audio_stream_block[3] = {0xC0, 0xC0, 0x04};
        for (i = 0; i < audio_bound; i++)
        {
            audio_stream_block[0] = 0xC0 + i + audio_ID_start_offset;
            fwrite(audio_stream_block, 1, 3, out_file);
        }
    }
    if (video_bound != 0)
    {
        char video_stream_block[3] = {0xE0, 0xE0, 0x2E};
        for (i = 0; i < video_bound; i++)
        {
            video_stream_block[0] = 0xE0 + i;
            fwrite(video_stream_block, 1, 3, out_file);
        }
    }
}
void packet_head_print(FILE *out_file, char stream_id, unsigned int packet_length, unsigned int stuffing_byte_length,
                                                                 char STD_buffer_scale, unsigned int STD_buffer_size,
                                                                 char DTS_type, unsigned long long PTS, unsigned long long DTS)
{
    char packet_start_code[4] = {0x00, 0x00, 0x01, stream_id};
    fwrite(packet_start_code, 1, 4, out_file);
    length_block_print(out_file, packet_length);
    reserved_byte(out_file, stuffing_byte_length);
    if (DTS_type == 0x00) //audio_stream
    {
        STD_buffer_block_print(out_file, STD_buffer_scale, STD_buffer_size);
        PTS_DTS_block_print(out_file, 0x02, PTS);
    }
    else if (DTS_type == 0x01)//vidoe_stream, I/P picture
    {
        STD_buffer_block_print(out_file, STD_buffer_scale, STD_buffer_size);
        PTS_DTS_block_print(out_file, 0x03, PTS);
        PTS_DTS_block_print(out_file, 0x01, DTS);
    }
    else if (DTS_type == 0x03)//vidoe_stream, B picture
    {
        reserved_byte(out_file, 5);
        STD_buffer_block_print(out_file, STD_buffer_scale, STD_buffer_size);
        PTS_DTS_block_print(out_file, 0x02, PTS);
    }
    else if (DTS_type == 0x04)//vidoe_stream, don't have picture head
    {
        reserved_byte(out_file, 9);
        STD_buffer_block_print(out_file, STD_buffer_scale, STD_buffer_size);
        char mark_bits[1] = {0x0F};
        fwrite(mark_bits, 1, 1, out_file);
    }
}
void padding_stream_print(FILE *out_file, unsigned int packet_length)
{
    char stream_start_block[4] = {0x00, 0x00, 0x01, 0xBE};
    fwrite(stream_start_block, 1, 4, out_file);
    length_block_print(out_file, packet_length);
    char mark_bits[1] = {0x0F};
    fwrite(mark_bits, 1, 1, out_file);
    reserved_byte(out_file, (packet_length - 1));
}

#endif