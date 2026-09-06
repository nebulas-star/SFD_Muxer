// SPDX-FileCopyrightText: 2021 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#ifndef __MUXER_ERROR_REPORT__
#define __MUXER_ERROR_REPORT__

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define     E__HELP                         0x80000000

#define     E__OPT_NUM                      0x80000001      // 参数数量错误
#define     E__OPT_UNKNOWN                  0x80000011      // 未知参数
#define     E__OPT_WRONG_REP                0x80000021      // 不可重复声明的参数被声明多次
#define     E__OPT_NON_NESSARY              0x80000041      // 缺少必须的参数、如输出文件指定 
#define     E__OPT_WRONG_VALUE              0x80000081      // 参数值声明错误

#define     E__UNSUPPORTED_VIDEO_FORMAT     0x80001001      // 不支持的视频格式
#define     E__UNSUPPORTED_AUDIO_FORMAT     0x80001002      // 不支持的音频格式

#define     E__NOT_CONFORM_MPEG             0x80001100      // 不合法的MPEG视频流

#define     E__NOT_CONFORM_ADX              0x80001200      // 不合法的ADX：使用了未知的音频编码器
#define     E__NOT_CONFORM_SFA              0x80001201      // 不符合SFA约束的ADX输入
#define     E__NOT_CONFORM_A52              0x80001202      // 不合法的AC-3
#define     E__NOT_CONFORM_AHX              0x80001203      // 不合法的AHX
#define     E__NOT_CONFORM_AIX              0x80001204      // 不合法的AIX

#define     E__EXCEED_INPUT_STREAM_LIMIT    0x80002001      // 输入流数量超限，包括数据速率过大/视频超16/音频超32/超出sfd记述能力
#define     E__EXCEED_REPLY_TIME_LIMIT      0x80002002      // 时长超限，包括参考时钟/DTS/PTS爆表

#define     E__TODO                         0xFFFFFFFF      // 尚未实现

void muxer_error(size_t error_code, ...)
{   va_list args;
    va_start(args, error_code);

    switch(error_code){
        case E__HELP:
            printf("USAGE:\n");
            printf("    SFD_Muxer [-h]\n");
            printf("    SFD_Muxer [-y] [-v <video_input>]... [-a <audio_input>]... -o <output_file> [-M <sofdec_metadata_verson>] [-A <audio_id_off>]\n");
            printf("\n");
            printf("OPTIONS:\n");
            printf("    -h:   print help infomation.\n");
            printf("\n");
            printf("    -y:   overwrite output files\n");
            printf("    -v:   specific input video file name\n");
            printf("    -a:   specific input audio file name\n");
            printf("    -o:   specific output file name\n");
            printf("\n");
            printf("    -M:   sofdec metadata version. only 2 different version.\n");
            printf("    -A:   audio stream id offset. when need leave in blank first x audio stream, use this option with argument x.");
            printf("\n");
            break;
//  -d:   sofdec文件元数据转储，由sfd_info给出
//  -t:   *.tag输入
//  -x:   *.sfx输入
//  -T:   若无输入则添加一个占位流，以符合原始Sofdec Multiplexer行为
        case E__OPT_NUM:
            char* num_error_opt = va_arg(args, char*);
            printf("[ERROR] Option \"%s\" miss parameters.",  num_error_opt);
            break;
        case E__OPT_UNKNOWN:
            char* ukn_opt = va_arg(args, char*);
            printf("[ERROR] Undefined option \"%s\".", ukn_opt);
            break;
        case E__OPT_WRONG_REP:
            char* rep_opt = va_arg(args, char*);
            printf("[ERROR] The non-reusable option \"%s\" has been used multiple times.", rep_opt);
            break;
        case E__UNSUPPORTED_VIDEO_FORMAT:
            char* unsp_vfile = va_arg(args, char*);
            printf("[ERROR] The specified video stream \"%s\" is not an MPEG-1/2 video stream.", unsp_vfile);
            break;
        case E__EXCEED_REPLY_TIME_LIMIT:
            printf("[ERROR] Input stream is too long, and the maximum length of the DTS/PTS field exceeds the container constraint.");
            break;

/*
        case 011:
            printf("ERROR 011: No output file specified.");
            break;
        case 012:
            printf("ERROR 012: Multiple output files were specified.");
            break;
        case 020:
            printf("ERROR 020: The number of input video streams exceeds the upper limit of the container.");
            break;
        case 021:
            printf("ERROR 021: The number of input audio streams exceeds the upper limit of the container.");
            break;
        case 022:
            printf("ERROR 022: No SFA/AIX stream input is specified, but AC-3(Dolby Digital) stream input is specified.");
            break;
        case 030:
            printf("ERROR 030: The specified Sofdec stream version does not meet the constraints.");
            break;
        case 032:
            printf("ERROR 032: The specified start offset value of the audio stream does not meet the constraints.");
            break;
        case 110:
            char* error_file = va_arg(args, char*);
            printf("ERROR 110: The specified audio stream \"%s\" is not an SFA/AIX/AC-3 audio stream.", error_file);
            break;
        case 111:
            char* error_file = va_arg(args, char*);
            printf("ERROR 111: The specified SFA stream \"%s\" isn't conformed to constraint conditions.", error_file);
            break;
        case 112:
            char* error_file = va_arg(args, char*);
            printf("ERROR 112: The specified AIX stream \"%s\" isn't conformed to constraint conditions.", error_file);
            break;
        case 120:
            char* error_file = va_arg(args, char*);
            printf("ERROR 120: The specified sample Sofdec file \"%s\" isn't conformed to constraint conditions.", error_file);
            break;
        case 200:
            printf("ERROR 200: Too many input streams, and the length of the mux_rate field exceeds the container constraint.");
            break;
        case 201:
            printf("ERROR 201: Too many input streams, and the maximum length of the system_lock_reference field exceeds the container constraint.");
            break;
        case 300:
            printf("ERROR 300: The number of input video streams is different from the number of video streams in the sample Sofdec file.");
            break;
        case 301:
            printf("ERROR 301: The number of input audio streams is different from the number of audio streams in the sample Sofdec file.");
            break;
        case 900:
            printf("ERROR 900: This feature has not yet been implemented.");
            break;
        case 901:
            printf("ERROR 901: The function of muxing MPEG-2 video streams has not yet been implemented.");
            break;
        case 903:
            printf("ERROR 903: The parameters of the SFA audio stream are not in the predetermined parameter table.");
            break;

*/
        case E__TODO:
            char* todo_function = va_arg(args, char*);
            printf("[ERROR] Function \"%s\" has not yet been implemented.", todo_function);
            break;
        default:
            printf("[ERROR] Internal error: 0x%08llx.", error_code, error_code);
    }
    va_end(args);

    exit(error_code);
}

#endif // __MUXER_ERROR_REPORT__