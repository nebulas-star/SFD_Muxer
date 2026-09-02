// SPDX-FileCopyrightText: 2026 Nebulas Astra <https://github.com/nebulas-star>
// SPDX-License-Identifier: MIT

// Substring searching function with Knuth–Morris–Pratt algorithm
// Rewrite from 1977 original paper "Fast Pattern Matching in Strings" (DOI: 10.1137/0206024) Part 2 "Programming the algorithm" 

#ifndef __MEMSEARCH_LIB__
#define __MEMSEARCH_LIB__

#ifdef __cplusplus
extern "C" {
#endif

static void compute_table_next(unsigned char* pattern, int m, int *next)
{
    int j = 0;
    int t = -1;
    next[0] = -1;
    while (j < (m - 1)){
        while (t > -1 && pattern[j] != pattern[t])
            t = next[t];
        t = t + 1;
        j = j + 1;
        if (pattern[j] == pattern[t])
            next[j] = next[t];
        else
            next[j] = t;
    }
}

int memsearch(unsigned char* text, int n, int offset, unsigned char* pattern, int m)
{
    int next[m];
    compute_table_next(pattern, m, next);

    int j = 0;
    int k = offset;
    while (j < m && k < n){
        while (j > -1 && text[k] != pattern[j])
            j = next[j];
        k = k + 1;
        j = j + 1;
    }

    if (j == m)
        return k - j;
    else
        return -1;
}

#ifdef __cplusplus
}
#endif

#endif // __MEMSEARCH_LIB__