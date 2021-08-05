#ifndef __SFD_MUXER_ERROR_H__
#define __SFD_MUXER_ERROR_H__

void error(int error_code, char *error_file, int ansi_codepage)
{
    if      (ansi_codepage == 1252)
    {
        if      (error_code == 000)
            printf("ERROR 000: Parameters quantity isn't conformed to constraint conditions.");
        else if (error_code == 001)
            printf("ERROR 001: Undefined parameter \"%s\" exist in the command.", error_file);
        else if (error_code == 010)
            printf("ERROR 010: No input video stream specified.");
        else if (error_code == 011)
            printf("ERROR 011: No output file specified.");
        else if (error_code == 012)
            printf("ERROR 012: Multiple output files were specified.");
        else if (error_code == 020)
            printf("ERROR 020: The number of input video streams exceeds the upper limit of the container.");
        else if (error_code == 021)
            printf("ERROR 021: The number of input audio streams exceeds the upper limit of the container.");
        else if (error_code == 022)
            printf("ERROR 022: No SFA/AIX stream input is specified, but AC-3(Dolby Digital) stream input is specified.");
        else if (error_code == 030)
            printf("ERROR 030: The specified Sofdec stream version does not meet the constraints.");
        else if (error_code == 031)
            printf("ERROR 031: The specified number of Sofdec stream version is greater than 1.");
        else if (error_code == 032)
            printf("ERROR 032: The specified start offset value of the audio stream does not meet the constraints.");
        else if (error_code == 033)
            printf("ERROR 033: The specified number of start offset value of the audio stream is greater than 1.");
        else if (error_code == 034)
            printf("ERROR 034: The specified number of sample Sofdec file is greater than 1.");
        else if (error_code == 100)
            printf("ERROR 100: The specified video stream \"%s\" is not an MPEG-1/2 video stream.", error_file);
        else if (error_code == 110)
            printf("ERROR 110: The specified audio stream \"%s\" is not an SFA/AIX/AC-3 audio stream.", error_file);
        else if (error_code == 111)
            printf("ERROR 111: The specified SFA stream \"%s\" isn't conformed to constraint conditions.", error_file);
        else if (error_code == 112)
            printf("ERROR 112: The specified AIX stream \"%s\" isn't conformed to constraint conditions.", error_file);
        else if (error_code == 120)
            printf("ERROR 120: The specified sample Sofdec file \"%s\" isn't conformed to constraint conditions.", error_file);
        else if (error_code == 200)
            printf("ERROR 200: Too many input streams, and the length of the mux_rate field exceeds the container constraint.");
        else if (error_code == 201)
            printf("ERROR 201: Too many input streams, and the maximum length of the system_lock_reference field exceeds the container constraint.");
        else if (error_code == 202)
            printf("ERROR 202: The input stream is too long, and the maximum length of the DTS/PTS field exceeds the container constraint.");
        else if (error_code == 300)
            printf("ERROR 300: The number of input video streams is different from the number of video streams in the sample Sofdec file.");
        else if (error_code == 301)
            printf("ERROR 301: The number of input audio streams is different from the number of audio streams in the sample Sofdec file.");
        else if (error_code == 900)
            printf("ERROR 900: This feature has not yet been implemented.");
        else if (error_code == 901)
            printf("ERROR 901: The function of muxing MPEG-2 video streams has not yet been implemented.");
        else if (error_code == 902)
            printf("ERROR 902: The function of muxing AIX audio streams has not yet been implemented.");
        else if (error_code == 903)
            printf("ERROR 903: The parameters of the SFA audio stream are not in the predetermined parameter table.");
    }
    else if (ansi_codepage == 936)
    {
        if      (error_code == 000)
            printf("ERROR 000: 参数数量不合法。");
        else if (error_code == 001)
            printf("ERROR 001: 命令中存在未定义参数%s。", error_file);
        else if (error_code == 010)
            printf("ERROR 010: 未指定任何视频输入流。");
        else if (error_code == 011)
            printf("ERROR 011: 未指定任何输出文件。");
        else if (error_code == 012)
            printf("ERROR 012: 指定了多于一个输出文件。");
        else if (error_code == 020)
            printf("ERROR 020: 欲混流的视频流数超出容器上限。");
        else if (error_code == 021)
            printf("ERROR 021: 欲混流的音频流数超出容器上限。");
        else if (error_code == 022)
            printf("ERROR 022: 未指定任何SFA/AIX流输入，却指定了AC-3流输入。");
        else if (error_code == 030)
            printf("ERROR 030: 指定的Sofdec信息流版本不合法。");
        else if (error_code == 031)
            printf("ERROR 031: 指定的Sofdec信息流版本数量大于1。");
        else if (error_code == 032)
            printf("ERROR 032: 指定的音频流标记偏移值不合法。");
        else if (error_code == 033)
            printf("ERROR 033: 指定的音频流标记偏移值数量大于1。");
        else if (error_code == 034)
            printf("ERROR 034: 指定的SFD样本流数量大于1。");
        else if (error_code == 100)
            printf("ERROR 100: 指定的视频流\"%s\"不是合法的MPEG-1/2基本视频流。", error_file);
        else if (error_code == 110)
            printf("ERROR 110: 指定的音频流\"%s\"不是合法的SFA/AIX/AC-3音频流。", error_file);
        else if (error_code == 111)
            printf("ERROR 111: 指定的SFA流\"%s\"不合法。", error_file);
        else if (error_code == 112)
            printf("ERROR 112: 指定的AIX流\"%s\"不符合约束。", error_file);
        else if (error_code == 120)
            printf("ERROR 120: 指定的SFD样本流\"%s\"不合法。", error_file);
        else if (error_code == 200)
            printf("ERROR 200: 输入流过多，mux_rate字段长度超出容器约束。");
        else if (error_code == 201)
            printf("ERROR 201: 输入流过多，system_lock_reference字段最大值长度超出容器约束。");
        else if (error_code == 202)
            printf("ERROR 202: 输入流过长，DTS/PTS字段最大值长度超出容器约束。");
        else if (error_code == 300)
            printf("ERROR 300: 输入视频流数与样本SFD视频流数不相同。");
        else if (error_code == 301)
            printf("ERROR 301: 输入音频流数与样本SFD音频流数不相同。");
        else if (error_code == 900)
            printf("ERROR 900: 功能尚未实现。");
        else if (error_code == 901)
            printf("ERROR 901: 混流MPEG-2视频流的功能暂未实现。");
        else if (error_code == 902)
            printf("ERROR 902: 混流AIX音频流的功能暂未实现。");
        else if (error_code == 903)
            printf("ERROR 903: 该SFA音频流的参数不在预先确定的参数表中。");
    }
    exit (0);
}

#endif