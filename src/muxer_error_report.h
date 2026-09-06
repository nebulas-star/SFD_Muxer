// SPDX-FileCopyrightText: 2021-2026 Nebulas Astra <https://github.com/nebulas-star>
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

#define     E__NOT_CONFORM_ADX              0x80001100      // 不合法的ADX：使用了未知的音频编码器
#define     E__NOT_CONFORM_SFA              0x80001101      // 不符合SFA约束的ADX输入
#define     E__NOT_CONFORM_A52              0x80001102      // 不合法的AC-3
#define     E__NOT_CONFORM_AHX              0x80001103      // 不合法的AHX
#define     E__NOT_CONFORM_AIX              0x80001104      // 不合法的AIX

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
        case E__OPT_NON_NESSARY:
            char* ness_opt = va_arg(args, char*);
            printf("[ERROR] The necessary option \"%s\" is missing.", ness_opt);
            break;
        case E__UNSUPPORTED_VIDEO_FORMAT:
            char* unsp_vfile = va_arg(args, char*);
            printf("[ERROR] The specified video stream \"%s\" is not MPEG-1/2 video format.", unsp_vfile);
            break;
        case E__UNSUPPORTED_AUDIO_FORMAT:
            char* unsp_afile = va_arg(args, char*);
            printf("[ERROR] The specified audio stream \"%s\" is not supported audio format.", unsp_afile);
            break;
        case E__NOT_CONFORM_ADX:
            char* ncadx = va_arg(args, char*);
            printf("[ERROR] The specified CRI ADX stream \"%s\" used unsupported audio encoder.", ncadx);
            break;
        case E__NOT_CONFORM_SFA:
            char* ncsfa = va_arg(args, char*);
            printf("[ERROR] The specified CRI ADX stream \"%s\" does not meet the requirements of Sofdec Audio.", ncsfa);
            break;
        case E__NOT_CONFORM_A52:
            char* nca52 = va_arg(args, char*);
            printf("[ERROR] The specified audio stream \"%s\" is not standard AC-3 audio format.", nca52);
            break;
//        case E__NOT_CONFORM_AHX:
//            break;
        case E__NOT_CONFORM_AIX:
            char* ncaix = va_arg(args, char*);
            printf("[ERROR] The specified audio stream \"%s\" is not standard CRI AIX format, or have more than one segment.", ncaix);
            break;
        case E__EXCEED_INPUT_STREAM_LIMIT:
            printf("[ERROR] Input stream is too large, and exceeds the capacity of the container to record.");
            break;
        case E__EXCEED_REPLY_TIME_LIMIT:
            printf("[ERROR] Input stream is too long, and the maximum length of the DTS/PTS field exceeds the container constraint.");
            break;
        case E__TODO:
            char* todo_function = va_arg(args, char*);
            printf("[ERROR] Function \"%s\" has not yet been implemented.", todo_function);
            break;
        default:
            printf("[ERROR] Internal error: 0x%08llx.", error_code);
    }
    va_end(args);

    exit(error_code);
}

#endif // __MUXER_ERROR_REPORT__