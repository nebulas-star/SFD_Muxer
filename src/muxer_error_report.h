// SPDX-FileCopyrightText: 2021 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#ifndef __MUXER_ERROR_REPORT__
#define __MUXER_ERROR_REPORT__

#include <stdarg.h>

void error(int error_code, ...)
{   va_list args;
    va_start(args, error_code);

    switch(error_code){
        case 000:
            printf("ERROR 000: Parameters quantity isn't conformed to constraint conditions.");
            break;
        case 001:
            char* error_file = va_arg(args, char*);
            printf("ERROR 001: Undefined parameter \"%s\" exist in the command.", error_file);
            break;
        case 010:
            printf("ERROR 010: No input video stream specified.");
            break;
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
        case 031:
            printf("ERROR 031: The specified number of Sofdec stream version is greater than 1.");
            break;
        case 032:
            printf("ERROR 032: The specified start offset value of the audio stream does not meet the constraints.");
            break;
        case 033:
            printf("ERROR 033: The specified number of start offset value of the audio stream is greater than 1.");
            break;
        case 034:
            printf("ERROR 034: The specified number of sample Sofdec file is greater than 1.");
            break;
        case 100:
            char* error_file = va_arg(args, char*);
            printf("ERROR 100: The specified video stream \"%s\" is not an MPEG-1/2 video stream.", error_file);
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
        case 202:
            printf("ERROR 202: The input stream is too long, and the maximum length of the DTS/PTS field exceeds the container constraint.");
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
        default:
            ;
    }
    va_end(args);

    exit(error_code);
}

#endif // __MUXER_ERROR_REPORT__