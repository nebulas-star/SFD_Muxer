# SFD Muxer

[English](/Readme.md)

开源的 CRI Sofdec 混流器。

## 简介
CRI Sofdec 文件格式（典型扩展名为SFD）是 CRI Middleware 公司所开发的游戏用视频中间件格式，当前 CRI Middleware 已停止支持该格式。

本工具旨在提供 CRI Sofdec SDK 的有限替代品供经典视频游戏的社区本地化或社区DLC制作使用。

## 使用方法
命令:

    SFD_Muxer (-y) [-v [video]]… [-a [audio]]… [-o [output]] (-sfd [sample sfd]) (-s [sofdec stream verson]) (-as [audio offset]) (-l [language])

说明:

    -y:   默认覆盖输出。
    -v:   视频输入流 (mpeg-1 video, 不得超过16个)
    -a:   音频输出流 (SFA/Dolby Digital AC-3/Dobly Digital AIX audio, 不得超过32个)
    -o:   输出文件名。
    -sfd: 样本 SFD 文件。提供输出文件参数。若使用此参数，则所有 "-s" 与 "-as" 参数无效。
    -s:   Sofdec 信息流版本。以 ASCII 指示于文件中第一个私用流包的偏址 0x2C 处。
    -as:  音频流起始偏址。
          该参数指示了输出文件中第一个音频流的 "stream_ID"。
    -l:   显示语言。 (e = 英语[default], c= 中文)
          由于源码基于 UTF-8，在 Windows 下使用中文模式时需要先执行 "chcp 65001" 将终端转换为 UTF-8 代码页。

示例:

    SFD_Muxer -v AT_OP0D.m1v -a AT_OP0D.sfa -a AT_OP0D.ac3 -o AT_OP0D.sfd

## Q&A
1. 什么是SFA？

    SFA是 [CRI ADX文件](https://wiki.multimedia.cx/index.php/CRI_ADX_file) 的一种，其特征是ADX头填充终止信息 "(c)CRI" 一定位于文件中0x119-0x11F处。

2. 什么是 Dobly Digital AIX？

    Dobly Digital AIX 是CRI AIX音频文件的一种。CRI AIX文件是混流了多路ADX的容器格式。

    Dobly Digital AIX 需要满足以下约束：

    1. Track num = 3 且 Phrase num = 1；
    2. 每个子ADX均为双声道，且各声道布置符合下表：
       | AIX track  | channel       | DoblyDIgital channel          |
       | ---------- | ------------- | ----------------------------- |
       | 1st track  | Left channel  | Front left channel            |
       | 1st track  | Right channel | Front right channel           |
       | 2nd track  | Left channel  | Back left channel             |
       | 2nd track  | Right channel | Back right channel            |
       | 3rd track  | Left channel  | Front center channel          |
       | 3rd track  | Right channel | Low-frequency effects channel |

3. "ERROR 2XX"文本中的"XXX"字段是什么意思？

    这些字段均定义于ISO/IEC 11172-1(MPEG-1 Part 1)。若有需要，请查阅该文档。

## 待实现:
    MPEG-2 视频支持
    更好的 SFA/AIX 音频支持