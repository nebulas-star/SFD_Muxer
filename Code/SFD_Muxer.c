#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "SFD_Muxer_Error.h"
#include "MPEG_block_print.h"
#include "Sofdec_block_print.h"
#include "file_feature_read.h"
#include "KMP_search.h"

void overwrite_question(char *file, int ansi_codepage)
{
    if      (ansi_codepage == 1252)
        printf("File \"%s\" already exists. Overwrite? [y/N]", file);
    else if (ansi_codepage == 936)
        printf("指定的输出文件\"%s\"已存在，是否覆盖？[y/N]", file);
    char i;
    scanf("%c", &i);
    if(i != 'y' && i != 'Y')
        exit(0);
}

void mux_end_print(int ansi_codepage)
{
    if      (ansi_codepage == 1252)
        printf("\nMux complete.\n");
    else if (ansi_codepage == 936)
        printf("\n混流已完成。\n");
}

int main(int argc, char *argv[])
{
    unsigned int files_num = 0;
    unsigned int video_num = 0;
    char *video_file[16];
    unsigned int m1v_num = 0;
    char *m1v_file[16];
    float m1v_DTS_basic[16];
    unsigned int m2v_num = 0;
    char *m2v_file[16];
    unsigned int audio_num = 0;
    char *audio_file[32];
    unsigned int sfa_num = 0;
    char *sfa_file[32];
    unsigned int sfa_DTS_basic[32];
    unsigned int aix_num = 0;
    char *aix_file[32];
    unsigned int ac3_num = 0;
    char *ac3_file[32];
    unsigned int output_num = 0;
    char *output_file;
    unsigned int SFD_style_num = 0;
    char *SFD_style_file;
    unsigned int sofdec_version_num = 0;
    unsigned int sofdec_version = 1;
    unsigned int default_overwrite_flag = 0;
    unsigned int audio_ID_start_offset_num = 0;
    unsigned int audio_ID_start_offset = 0;
    int ansi_codepage = 1252;

    int i, j, k, l;
    FILE *input_cache;
    unsigned char file_style_cache[0x2000];

    FILE *output;
    unsigned long int mux_rate = 0;
    unsigned int video_bound = 0;
    unsigned int audio_bound = 0;
    unsigned int audio_ID_start = 0;
    unsigned long long int SCR_flag;

    FILE *inputs[48];
    char DTS_flag[48];
    float DTS_basic[48];
    unsigned long long int DTS_forecast[48];
    unsigned long long int picture_num_basic[48];
    unsigned long long int picture_num_current[48];
    unsigned long int picture_num_bigest[48];
    unsigned char picture_head[4] = {0x00, 0x00, 0x01, 0x00};
    unsigned int picture_coding_type;
    unsigned int temporal_reference;
    unsigned int read_flag;
    unsigned int last_picture_start = 0;
    unsigned long long int PTS_cache;

    //read from terminal
    for (i = 1; i < argc; i = i + 2)
    {
        if      (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "-V") == 0)
        {
            video_file[video_num] = argv[i + 1];
            video_num++;
        }
        else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-A") == 0)
        {
            audio_file[audio_num] = argv[i + 1];
            audio_num++;
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "-O") == 0)
        {
            output_file = argv[i + 1];
            output_num++;
        }
        else if (strcmp(argv[i], "-sfd") == 0 || strcmp(argv[i], "-SFD") == 0)
        {
            SFD_style_num++;
            SFD_style_file = argv[i + 1];
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "-S") == 0)
        {
            sofdec_version_num++;
            sscanf(argv[i + 1], "%u", &sofdec_version);
        }
        else if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "-Y") == 0)
        {
            default_overwrite_flag = 1;
            i--;
        }
        else if (strcmp(argv[i], "-as") == 0 || strcmp(argv[i], "-AS") == 0)
        {
            audio_ID_start_offset_num++;
            sscanf(argv[i + 1], "%u", &audio_ID_start_offset);

        }
        else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "-L") == 0)
        {
            if     (strcmp(argv[i+1], "e") == 0 || strcmp(argv[i+1], "E") == 0)
                ansi_codepage = 1252;
            else if(strcmp(argv[i+1], "c") == 0 || strcmp(argv[i+1], "C") == 0)
                ansi_codepage = 936;
            else
                printf("Warning: Language parameter isn't conformed to constraint conditions. The interface language is still in use English.");
        }
        else
            error(001, argv[i], ansi_codepage);
    }
    files_num = video_num + audio_num;
    if ((default_overwrite_flag == 0 && argc % 2 == 0) || argc < 5 || (default_overwrite_flag == 1 && argc % 2 == 1))
        error(000, 0, ansi_codepage);
    if (video_num == 0)
        error(010, 0, ansi_codepage);
    if (output_num == 0)
        error(011, 0, ansi_codepage);
    if (output_num > 1)
        error(012, 0, ansi_codepage);
    if (video_num > 16)
        error(020, 0, ansi_codepage);
    if (audio_num > 32)
        error(021, 0, ansi_codepage);
    if (sofdec_version > 2 || sofdec_version < 1)
        error(030, 0, ansi_codepage);
    if (sofdec_version_num > 1)
        error(031, 0, ansi_codepage);
    if ((audio_ID_start_offset + audio_num) > 32 )
        error(032, 0, ansi_codepage);
    if (audio_ID_start_offset_num > 1)
        error(033, 0, ansi_codepage);
    if (SFD_style_num > 1)
        error(034, 0, ansi_codepage);

    //input classification
    for (i = 0; i < video_num; i++)
    {
        input_cache = fopen(video_file[i], "rb");
        fread(file_style_cache, 1, 0x90, input_cache);
        if (file_style_cache[0] == 0x00 && file_style_cache[1] == 0x00
                                        && file_style_cache[2] == 0x01 && file_style_cache[3] == 0xB3)
        {
            if (  (file_style_cache[0x0C] == 0x00 && file_style_cache[0x0D] == 0x00 
                                                  && file_style_cache[0x0E] == 0x01 && file_style_cache[0x0F] == 0xB5)
               || (file_style_cache[0x4C] == 0x00 && file_style_cache[0x4D] == 0x00 
                                                  && file_style_cache[0x4E] == 0x01 && file_style_cache[0x4F] == 0xB5)
               || (file_style_cache[0x8C] == 0x00 && file_style_cache[0x8D] == 0x00 
                                                  && file_style_cache[0x8E] == 0x01 && file_style_cache[0x8F] == 0xB5)
               )
            {
                m2v_file[m2v_num] = video_file[i];
                m2v_num++;
            }
            else
            {
                m1v_file[m1v_num] = video_file[i];
                m1v_num++;
            }
            fclose(input_cache);
        }
        else
            error(100, video_file[i], ansi_codepage);
    }
    if (m2v_num > 0)//Now can't mux MPEG-2 Video.
        error(901, 0, ansi_codepage);
    for (i = 0; i < audio_num; i++)
    {
        input_cache = fopen(audio_file[i], "rb");
        fread(file_style_cache, 1, 0x1800, input_cache);
        if      (file_style_cache[0] == 0x80 && file_style_cache[1] == 0x00)
        {
            if(file_style_cache[0x11A] == 0x28 && file_style_cache[0x11B] == 0x63 && file_style_cache[0x11C] == 0x29
                                               && file_style_cache[0x11D] == 0x43 && file_style_cache[0x11E] == 0x52
                                                                                  && file_style_cache[0x11F] == 0x49)
            {
                sfa_file[sfa_num] = audio_file[i];
                sfa_num++;
            }
            else
                error(111, audio_file[i], ansi_codepage);
        }
        else if (file_style_cache[0] == 0x0B && file_style_cache[1] == 0x77)
        {
            ac3_file[ac3_num] = audio_file[i];
            ac3_num++;
        }
        else if (file_style_cache[0] == 0x41 && file_style_cache[1] == 0x49 
                                             && file_style_cache[2] == 0x58 && file_style_cache[3] == 0x46)
            error(902, 0, ansi_codepage);//Now can't mux AIX Video.
        else
            error(110, audio_file[i], ansi_codepage);
        fclose(input_cache);
    }
    if ((sfa_num + aix_num) == 0 && ac3_num > 0)
        error(022, 0, ansi_codepage);

    //If sample Sofdec, read parameter.
    if (SFD_style_num == 1)
    {
        input_cache =  fopen(SFD_style_file, "rb");
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
                        audio_ID_start = file_style_cache[(i * 0x800) + 0x18];
                        audio_ID_start_offset = audio_ID_start - 0xC0;
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
            error(120, SFD_style_file, ansi_codepage);
    if (video_bound != video_num)
        error(300, 0, ansi_codepage);
    if (audio_bound != audio_num)
        error(301, 0, ansi_codepage);
    }
    
    if (SFD_style_num == 0)
    {
        mux_rate++;
        mux_rate = mux_rate + m1v_num * 0x40F38;
        mux_rate = mux_rate + ac3_num * 0x471;
        if (sfa_num != 0)
            for (i = 0; i < sfa_num; i++)
            {
                input_cache = fopen(sfa_file[i], "rb");
                fread(file_style_cache, 1, 0x20, input_cache);
                j = sample_rate_read(file_style_cache[0x08], file_style_cache[0x09], file_style_cache[0x0A],
                                                                                     file_style_cache[0x0B]);
                k = file_style_cache[0x07];
                mux_rate = mux_rate + sfa_rate_made(j, k, ansi_codepage);
                fclose(input_cache);
            }
        if (mux_rate >= 0x3FFFFF)
            error(200, 0, ansi_codepage);
    }
    
    //calculate DTS basic
    if (sfa_num != 0)
        for (i = 0; i < sfa_num; i++)
        {
            input_cache = fopen(sfa_file[i], "rb");
            fread(file_style_cache, 1, 0x20, input_cache);
            j = sample_rate_read(file_style_cache[0x08], file_style_cache[0x09], file_style_cache[0x0A],
                                                                                 file_style_cache[0x0B]);
            k = file_style_cache[0x07];
            sfa_DTS_basic[i] = (322560000 / (j * k));
            fclose(input_cache);
        }
    for (i = 0; i < m1v_num; i++)
    {
        input_cache = fopen(m1v_file[i], "rb");
        fread(file_style_cache, 1, 0x07, input_cache);
        m1v_DTS_basic[i] = DTS_basic_read(file_style_cache[0x07]);
        fclose(input_cache);
    }

    //overwrite?
    j = access(output_file, 00);
    if (j == 0 && default_overwrite_flag == 0)
        overwrite_question(output_file, ansi_codepage);

    //write
    output = fopen(output_file, "wb");
    SCR_flag = 0;
    if (audio_num != 0)
    {
        pack_head_print(output, SCR_flag, mux_rate, ansi_codepage);
        system_head_print(output, mux_rate, 0, audio_num, audio_ID_start_offset);
        padding_stream_print(output, (0x07E2 - 3 * audio_num));
        SCR_flag++;
    }
    pack_head_print(output, SCR_flag, mux_rate, ansi_codepage);
    system_head_print(output, mux_rate, video_num, 0, 0);
    padding_stream_print(output, (0x07E2 - 3 * video_num));
    SCR_flag++;
    pack_head_print(output, SCR_flag, mux_rate, ansi_codepage);
    sofdec_stream_message_block(output, sofdec_version);
    SCR_flag++;

    i = 0;
    j = 0;
    for (; i < (sfa_num + j); i++)
    {
        inputs[i] = fopen(sfa_file[i - j], "rb");
        DTS_flag[i] = 0x01;
        DTS_basic[i] = sfa_DTS_basic[i - j];
    }
    j = i;
    for (; i < (ac3_num + j); i++)
    {
        inputs[i] = fopen(ac3_file[i - j], "rb");
        DTS_flag[i] = 0x01;
        DTS_basic[i] = 0xCA8;
    }
    j = i;
    for (; i < (m1v_num + j); i++)
    {
        inputs[i] = fopen(m1v_file[i - j], "rb");
        DTS_flag[i] = 0x02;
        DTS_basic[i] = m1v_DTS_basic[i - j];
    }

    while (SCR_flag != 0)
    {
        //compare DTS found the smallest
        j = 0;
        for (i = 1; i < files_num; i++)
            if (DTS_flag[i] != 0xFF)
                if (DTS_forecast[j] > DTS_forecast[i])
                    j = i;
        if (DTS_flag[j] == 0x01)
        {
            k = fread(file_style_cache, 1, 0x7E0, inputs[j]);
            if (k == 0)
            {
                DTS_flag[j] = 0xFF;
                DTS_forecast[j] = 0xFFFFFFFFF;
            }
            else
            {
                pack_head_print(output, SCR_flag, mux_rate, ansi_codepage);
                packet_head_print(output, (0xC0 + j + audio_ID_start_offset), (k + 0x07), 0, 0, 0x04, 0, DTS_forecast[j], 0);
                fwrite(file_style_cache, 1, k, output);
                padding_stream_print(output, 0x07E1 - k);
                DTS_forecast[j] = DTS_forecast[j] + DTS_basic[j]; //renewal DTS
                if (DTS_forecast[j] > 0x1FFFFFFFF)
                    error(202, 0, ansi_codepage);
                if (k < 0x7E0)
                {
                    DTS_flag[j] = 0xFF;
                    DTS_forecast[j] = 0xFFFFFFFFF;
                }
                SCR_flag++;
            }
        }
        else if (DTS_flag[j] == 0x02)
        {
            read_flag = 0;
            while (read_flag == 0)
            {
                k = fread(file_style_cache, 1, 0x7E2, inputs[j]);
                l = string_searching(file_style_cache, k, picture_head, 4, 0);
                if (k == 0)
                {
                    read_flag = 1;
                    DTS_flag[j] = 0xFF;
                    DTS_forecast[j] = 0xFFFFFFFFF;
                }
                else
                {
                    pack_head_print(output, SCR_flag, mux_rate, ansi_codepage);
                    if (l == -1 || l >= 0x7DA) //no picture_head
                        packet_head_print(output, (0xE0 + j - audio_num), (k + 0x0C), 0, 1, 0x2E, 4, 0, 0);
                    else
                    {
                        picture_coding_type = picture_coding_type_read(file_style_cache[l + 5]);
                        temporal_reference = temporal_reference_read(file_style_cache[l + 4], file_style_cache[l + 5]);
                        PTS_cache = ((picture_num_basic[j]) + temporal_reference) * DTS_basic[j];

                        if (picture_coding_type == 0x01 || picture_coding_type == 0x02)
                            packet_head_print(output, (0xE0 + j - audio_num), (k + 0x0C), 0, 1, 0x2E, 1, PTS_cache, DTS_forecast[j]);
                        if (picture_coding_type == 0x03)
                            packet_head_print(output, (0xE0 + j - audio_num), (k + 0x0C), 0, 1, 0x2E, 3, PTS_cache, DTS_forecast[j]);
                        read_flag = 1;
                        while (l != -1)//renewal DTS
                        {
                            last_picture_start = l;
                            picture_num_current[j]++;
                            temporal_reference = temporal_reference_read(file_style_cache[l + 4], file_style_cache[l + 5]);
                            if (picture_num_bigest[j] < temporal_reference)
                                picture_num_bigest[j] = temporal_reference;
                            if (temporal_reference == 0)
                            {
                                picture_num_basic[j] = picture_num_basic[j] + picture_num_bigest[j] + 1;
                                picture_num_current[j] = picture_num_basic[j];
                            }
                            DTS_forecast[j] = DTS_basic[j] * picture_num_current[j];
                            l = string_searching(file_style_cache, k, picture_head, 4, (last_picture_start + 1));
                        }
                        last_picture_start = 0;
                    }
                    fwrite(file_style_cache, 1, k, output);
                    if (k < 0x7E2)
                    {
                        DTS_flag[j] = 0xFF;
                        DTS_forecast[j] = 0xFFFFFFFFF;
                        if (0x7DC - k > 0)
                            padding_stream_print(output, 0x07DC - k);
                        else
                            reserved_byte(output, 0x07E2 - k);
                    }
                    SCR_flag++;
                }
            }
        }
        else
            SCR_flag = 0;//End loop
    }
    sofdec_ending_block_print(output);
    mux_end_print(ansi_codepage);
    return 0;
}

