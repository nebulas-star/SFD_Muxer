#ifndef __FILE_FEATURE_READ_H__
#define __FILE_FEATURE_READ_H__

#include "SFD_Muxer_Error.h"

unsigned int sfa_rate_made(unsigned int sampling_rate, unsigned int channel_count, int ansi_codepage)
{
    double a = 1097;
    double b = 48000;
    double sample = a / b;
    int rate_out;
    if (channel_count == 2)
        rate_out = (sampling_rate * sample + 0.5);
    else if (channel_count == 1 && sampling_rate == 24000)
        rate_out = 0x142;
    else
        error(903, 0, ansi_codepage);//don't know how to get rate basic for 1 channel's sfa file.
    return rate_out;
}
unsigned long int rate_read(unsigned int i, unsigned int j, unsigned long int k)
{
    unsigned long int head_mark_bit = 0x800000;
    unsigned long int a, b, c, d, e;
    a = i << 16;
    b = j << 8;
    c = a | b | k;
    d = c ^ head_mark_bit;
    e = d >> 1;
    return e;
}
unsigned int audio_bound_read(unsigned int i)
{
    unsigned long int l = i >> 2;
    return l;
}
unsigned int video_bound_read(unsigned int i)
{
    unsigned int l = i & 0x1F;
    return l;
}
unsigned long int sample_rate_read(unsigned int i, unsigned int j, unsigned int k, unsigned long int l)
{
    unsigned long int a, b, c, d;
    a = i << 24;
    b = j << 16;
    c = k << 8;
    d = a | b | c | l;
    return d;
}
float DTS_basic_read(int i)
{
    int picture_rate = i & 0x0F;
    float DTS_basic = 0;
    if (picture_rate == 0x01)
        DTS_basic = 3753.75;
    else if (picture_rate == 0x02)
        DTS_basic = 3750;
    else if (picture_rate == 0x03)
        DTS_basic = 3600;
    else if (picture_rate == 0x04)
        DTS_basic = 3003;
    else if (picture_rate == 0x05)
        DTS_basic = 3000;
    else if (picture_rate == 0x06)
        DTS_basic = 1800;
    else if (picture_rate == 0x07)
        DTS_basic = 1501.5;
    else if (picture_rate == 0x08)
        DTS_basic = 1500;
    else
        ;
    return DTS_basic;
}
unsigned int temporal_reference_read(unsigned int i, unsigned int j)
{
    unsigned int a = (i >> 6) | (j >> 6);
    return a;
}
unsigned int picture_coding_type_read(unsigned i)
{
    unsigned int a = (i >> 3) & 0x07;
    return a;
}

#endif