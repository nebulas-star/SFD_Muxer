#SFD Muxer

Open source CRI Sofdec Muxer.
��Դ��CRI Sofdec��������

##Introduction
CRI Sofdec file format (SFD) is a type of video middleware format which developed by CRI Middleware Co., Ltd.
Now(2021), CRI Middleware has stopped supporting this format.
This tool is built as a limited alternative to CRI Sofdec SDK, used for made community localization or community DLC for some classic video games.

####���
CRI Sofdec�ļ���ʽ��������չ��ΪSFD����CRI Middleware��˾����������Ϸ����Ƶ�м����ʽ����ǰCRI Middleware��ֹ֧ͣ�ָø�ʽ��
������ּ���ṩCRI Sofdec SDK���������Ʒ��������Ƶ��Ϸ���������ػ�������DLC����ʹ�á�

##How to Use
Usage:
    `SFD_Muxer [-v [video]]�� [-a [audio]]�� [-o [output]] (-y) (-sfd [sample sfd]) (-s [sofdec stream verson]) (-as [audio offset]) (-l [language])`
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

####ʹ�÷���
����:
    `SFD_Muxer [-v [video]]�� [-a [audio]]�� [-o [output]] (-y) (-sfd [sample sfd]) (-s [sofdec stream verson]) (-as [audio offset]) (-l [language])`
˵��:

    -v:   ��Ƶ������(mpeg-1 video, ���ó���16��)
    -a:   ��Ƶ�����(SFA/AC-3(Dolby Digital, DD) audio, ���ó���32��)
    -o:   ����ļ�����
    -y:   Ĭ�ϸ��������
    -sfd: ����SFD�ļ����ṩ����ļ���������ʹ�ô˲�����������"-s"��"-as"������Ч��
    -s:   Sofdec��Ϣ���汾����ASCIIָʾ���ļ��е�һ��˽��������ƫַ0x2C����
    -as:  ��Ƶ����ʼƫַ��
          �ò���ָʾ������ļ��е�һ����Ƶ����"stream_ID"��
    -l:   ��ʾ���ԡ� (e = Ӣ��[default], c= ����)
          ����Դ�����UTF-8����Windows��ʹ������ģʽʱ��Ҫ��ִ��"chcp 65001"���ն�ת��ΪUTF-8����ҳ��

ʾ��:
    `SFD_Muxer -v AT_OP0D.m1v -a AT_OP0D.sfa -a AT_OP0D.ac3 -o AT_OP0D.sfd`


##TODO:
MPEG-2 video support
AIX audio surropt

####��ʵ��:
MPEG-2��Ƶ֧��
AIX��Ƶ֧��