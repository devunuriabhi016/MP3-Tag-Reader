#ifndef VIEW_H
#define VIEW_H

typedef struct
{
    char title[100];
    char artist[100];
    char album[100];
    char year[20];
    char genre[100];
    char comment[100];

} MP3Tag;

void view(char *filename);


static inline unsigned int id3_read_size(const unsigned char *b, int syncsafe)
{
    if(syncsafe)
    {
        return ((unsigned int)(b[0] & 0x7F) << 21) |
               ((unsigned int)(b[1] & 0x7F) << 14) |
               ((unsigned int)(b[2] & 0x7F) << 7)  |
               (unsigned int)(b[3] & 0x7F);
    }

    return ((unsigned int)b[0] << 24) |
           ((unsigned int)b[1] << 16) |
           ((unsigned int)b[2] << 8)  |
           (unsigned int)b[3];
}

static inline void id3_write_size(unsigned char *b, unsigned int value, int syncsafe)
{
    if(syncsafe)
    {
        b[0] = (value >> 21) & 0x7F;
        b[1] = (value >> 14) & 0x7F;
        b[2] = (value >> 7)  & 0x7F;
        b[3] = value & 0x7F;
    }
    else
    {
        b[0] = (value >> 24) & 0xFF;
        b[1] = (value >> 16) & 0xFF;
        b[2] = (value >> 8)  & 0xFF;
        b[3] = value & 0xFF;
    }
}

#endif
