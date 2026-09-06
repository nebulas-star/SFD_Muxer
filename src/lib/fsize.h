// SPDX-FileCopyrightText: 2025-2026 Nebulas Astra <https://github.com/nebulas-star>
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

size_t fsize_d(char *fp){
    FILE *file = fopen(fp, "rb");
    size_t n = fsize(file);
    fclose(file);
    return n;
}

#endif // __FSIZE_LIB__