// SPDX-FileCopyrightText: 2025 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

#ifndef __FSIZE_LIB__
#define __FSIZE_LIB__

#include <stdio.h>

size_t fsize(FILE *fp){
    size_t n;
    fpos_t fpos;
    fgetpos(fp, &fpos);     //保存原位
    fseek(fp, 0, SEEK_END);
    n = ftell(fp);
    fsetpos(fp, &fpos);      //复位
    return n;
}

#endif // __FSIZE_LIB__