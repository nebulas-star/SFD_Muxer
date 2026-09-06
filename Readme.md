# SFD Muxer
Open source CRI Sofdec Muxer.

## Introduction
CRI Sofdec file format is a type of video middleware format which developed by CRI Middleware Co., Ltd.

This tool is built as a limited alternative to CRI Sofdec SDK, used for made community localization or community DLC for some classic video games.

## How to Use

USAGE:
```
    SFD_Muxer [-h]
    SFD_Muxer [-y] [-v <video_input>]... [-a <audio_input>]... -o <output_file> [-M <sofdec_metadata_verson>] [-A <audio_id_off>]
```

OPTIONS:
```
    -h:   print help infomation.

    -y:   overwrite output files
    -v:   specific input video file name
    -a:   specific input audio file name
    -o:   specific output file name

    -M:   sofdec metadata version. only 2 different version.
    -A:   audio stream id offset. when need leave in blank first x audio stream, use this option with argument x.
```

Sample:
```
    SFD_Muxer -v video.m1v -a audio.sfa -a dobly.ac3 -o output.sfd
```

Supporting input formats:
- Video:
- - CRI Sofdec Video (a version of MPEG-1 video)
- - MPEG-1 Video（ISO/IEC 11172-2）
- - MPEG-2 Video（ISO/IEC 13818-2）
- Audio:
- - CRI Sofdec Audio (a version of [CRI ADX file](https://wiki.multimedia.cx/index.php/CRI_ADX_file) with CRI ADX APDCM)
- - Dolby AC-3 Audio（ATSC A/52）
- - CRI AIX Audio Container


## TODO:
    Supporting CRI AHX Audio input
    Supporting CRITAGS extra metadata input
