#ifndef __SOFDEC_BLOCK_PRINT_H__
#define __SOFDEC_BLOCK_PRINT_H__

void sofdec_padding_block_print(FILE *out_file, unsigned int byte_num)
{
    int i = 0;
    char reserved[1] = {0x00};
    if (byte_num != 0)
        for (i = 0; i < byte_num; i++)
            fwrite(reserved, 1, 1, out_file);
}
void sofdec_stream_message_block(FILE *out_file, int sofdec_version)
{
    char packet_head[20] = {0x00, 0x00, 0x01, 0xBF, 0x07, 0xEE};
    if (sofdec_version == 2)
        packet_head[6] = 0x08;
    fwrite(packet_head, 1, 20, out_file);
    char version_block[0x18] = "SofdecStream            ";
    if (sofdec_version == 2)
        version_block[0x0C] = 0x32;
    fwrite(version_block, 1, 0x18, out_file);
    char identity_block[8] = {0x02, 0xFF, 0x00, 0x00, 0X20, 0x21, 0x07, 0x14};
    if (sofdec_version == 2)
    {
        identity_block[1] = 0x02;
        identity_block[2] = 0x02;
        identity_block[3] = 0xFF;
    }
    fwrite(identity_block, 1, 8, out_file);
    if (sofdec_version == 1)
        sofdec_padding_block_print(out_file, 0x20);
    char muxer_id[0x20] = "SFD_Muxer Ver.0.24 by Nebulas   ";
    fwrite(muxer_id, 1, 0x20, out_file);
    if (sofdec_version == 2)
        sofdec_padding_block_print(out_file, 0x20);
}
void sofdec_ending_block_print(FILE *out_file)
{
    char end_block[4] = {0x00, 0x00, 0x01, 0xB9};
    fwrite(end_block, 1, 4, out_file);
    reserved_byte(out_file, 0x7FC);
}

#endif