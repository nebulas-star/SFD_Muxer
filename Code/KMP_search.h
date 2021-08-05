#ifndef __KMP_SEARCH_H__
#define __KMP_SEARCH_H__

void get_next_array_val(unsigned char* sample_string, int sample_str_length, int *next)
{
    int j = 0;
    int k = -1;
    next[0] = -1;
    while (j < sample_str_length - 1)
    {
        if (k == -1 || sample_string[j] == sample_string[k])
        {
            ++j;
            ++k;
            if (sample_string[j] != sample_string[k])
                next[j] = k;
            else
                next[j] = next[k];
        }
        else
            k = next[k];
    }
}
int string_searching(unsigned char *original_string, int original_str_length, unsigned char *sample_string, int sample_str_length, int search_start)
{
    int i = search_start;
    int j = 0;
    int next[sample_str_length];
    get_next_array_val(sample_string, sample_str_length, next);
    while (i < original_str_length && j < sample_str_length)
    {
        if (j == -1 || original_string[i] == sample_string[j])
        {
            i++;
            j++;
        }
        else
            j = next[j];
    }
    if (j == sample_str_length)
        return i - j;
    else
        return -1;
}

#endif