#SFD Muxer

Open source CRI Sofdec Muxer.
开源的CRI Sofdec混流器。

##Introduction
CRI Sofdec file format (SFD) is a type of video middleware format which developed by CRI Middleware Co., Ltd.
Now(2021), CRI Middleware has stopped supporting this format.
This tool is built as a limited alternative to CRI Sofdec SDK, used for made community localization or community DLC for some classic video games.

####简介
CRI Sofdec文件格式（典型扩展名为SFD）是CRI Middleware公司所开发的游戏用视频中间件格式，当前CRI Middleware已停止支持该格式。
本工具旨在提供CRI Sofdec SDK的有限替代品供经典视频游戏的社区本地化或社区DLC制作使用。

##How to Use
Usage:
    `SFD_Muxer [-v [video]]… [-a [audio]]… [-o [output]] (-y) (-sfd [sample sfd]) (-s [sofdec stream verson]) (-as [audio offset]) (-l [language])`
Option:

    -v:   input video filename(mpeg-1 video, less than 16)  
	-a:   input audio filename(SFA/AC-3(Dolby Digital, DD) audio, less than 32)
	-o:   output filename.
    -y:   default overwrite output file.
    -sfd: sample Sofdec file. Build with this file's parameter. If using this parameter, "-s" and "-as" will be not use.
    -s:   Sofdec stream verson. In the first "private_stream_2" start address + 0x2C, write in ASCII.
    -as:  The start offset value of the audio stream.
          This parameter demonstrate first audio stream's "stream_ID", with +0xC0.
    -l:   language. (e = english[default], c= chinese)
          The code is writing in UTF-8, so must using "chcp 65001" to change terminal codepage when using chinese in windows.

Sample:
    `SFD_Muxer -v AT_OP0D.m1v -a AT_OP0D.sfa -a AT_OP0D.ac3 -o AT_OP0D.sfd`

####使用方法
命令:
    `SFD_Muxer [-v [video]]… [-a [audio]]… [-o [output]] (-y) (-sfd [sample sfd]) (-s [sofdec stream verson]) (-as [audio offset]) (-l [language])`
说明:

    -v:   视频输入流(mpeg-1 video, 不得超过16个)
    -a:   音频输出流(SFA/AC-3(Dolby Digital, DD) audio, 不得超过32个)
    -o:   输出文件名。
    -y:   默认覆盖输出。
    -sfd: 样本SFD文件。提供输出文件参数。若使用此参数，则所有"-s"与"-as"参数无效。
    -s:   Sofdec信息流版本。以ASCII指示于文件中第一个私用流包的偏址0x2C处。
    -as:  音频流起始偏址。
          该参数指示了输出文件中第一个音频流的"stream_ID"。
    -l:   显示语言。 (e = 英语[default], c= 中文)
          由于源码基于UTF-8，在Windows下使用中文模式时需要先执行"chcp 65001"将终端转换为UTF-8代码页。

示例:
    `SFD_Muxer -v AT_OP0D.m1v -a AT_OP0D.sfa -a AT_OP0D.ac3 -o AT_OP0D.sfd`


##TODO:
MPEG-2 video support
AIX audio surropt

####待实现:
MPEG-2视频支持
AIX音频支持