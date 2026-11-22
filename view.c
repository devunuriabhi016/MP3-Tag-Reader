#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "view.h"


static void decode_text(const unsigned char *src, unsigned int len,
                        unsigned char encoding, char *dest, size_t dest_size)
{
    size_t out = 0;
    unsigned int i = 0;

    if(encoding == 1 || encoding == 2)
    {
        int big = (encoding == 2);

        /* Byte order mark */
        if(len >= 2 && src[0] == 0xFF && src[1] == 0xFE)
        {
            big = 0;
            i = 2;
        }
        else if(len >= 2 && src[0] == 0xFE && src[1] == 0xFF)
        {
            big = 1;
            i = 2;
        }

        for(; i + 1 < len && out + 1 < dest_size; i += 2)
        {
            unsigned char lo = big ? src[i + 1] : src[i];
            unsigned char hi = big ? src[i] : src[i + 1];

            if(lo == 0 && hi == 0)
            {
                break;
            }

            dest[out++] = hi ? '?' : (char)lo;
        }
    }
    else
    {
        for(; i < len && out + 1 < dest_size; i++)
        {
            if(src[i] == 0)
            {
                break;
            }

            dest[out++] = (char)src[i];
        }
    }

    dest[out] = '\0';
}


static void read_frame(FILE *ptr, unsigned int frame_size,
                       char *dest, size_t dest_size, int is_comment)
{
    unsigned char *data;
    unsigned char encoding;
    unsigned int len;

    dest[0] = '\0';

    if(frame_size < 1)
    {
        return;
    }

    data = malloc(frame_size);

    if(data == NULL)
    {
        fseek(ptr, frame_size, SEEK_CUR);
        return;
    }

    if(fread(data, 1, frame_size, ptr) != frame_size)
    {
        free(data);
        return;
    }

    encoding = data[0];
    len = frame_size - 1;

    if(is_comment)
    {
        unsigned int i = 4;     /* skip encoding byte + 3 language bytes */

        if(frame_size < 4)
        {
            free(data);
            return;
        }

        /* skip the short description */
        if(encoding == 1 || encoding == 2)
        {
            while(i + 1 < frame_size && !(data[i] == 0 && data[i + 1] == 0))
            {
                i += 2;
            }
            i += 2;
        }
        else
        {
            while(i < frame_size && data[i] != 0)
            {
                i++;
            }
            i += 1;
        }

        if(i < frame_size)
        {
            decode_text(data + i, frame_size - i, encoding, dest, dest_size);
        }
    }
    else
    {
        decode_text(data + 1, len, encoding, dest, dest_size);
    }

    free(data);
}

void view(char *filename)
{
    FILE *ptr = fopen(filename, "rb");

    if(ptr == NULL)
    {
        printf("File cannot be opened\n");
        return;
    }

    /* Read ID3 header */
    unsigned char header[10];

    /* Check ID3 */
    if(fread(header, 1, 10, ptr) != 10 ||
       header[0] != 'I' ||
       header[1] != 'D' ||
       header[2] != '3')
    {
        printf("ID3 tag not found\n");
        fclose(ptr);
        return;
    }

    int version = header[3];

    if(version != 3 && version != 4)
    {
        printf("Unsupported ID3 version (only ID3v2.3 and ID3v2.4)\n");
        fclose(ptr);
        return;
    }

    if(header[5] & 0x40)
    {
        printf("Extended ID3 header is not supported\n");
        fclose(ptr);
        return;
    }

    int syncsafe = (version == 4);

    /* Tag size is stored in bytes 6 to 9 of the header */
    long tag_end = 10 + (long)id3_read_size(&header[6], 1);

    char title[100] = "";
    char artist[100] = "";
    char album[100] = "";
    char year[20] = "";
    char genre[100] = "";
    char comment[100] = "";

    /* Read frames until the end of the tag (or until the padding) */
    while(ftell(ptr) + 10 <= tag_end)
    {
        char frame_id[5];
        unsigned char size[4];
        unsigned char flags[2];

        /* Read 4 bytes: TIT2 */
        if(fread(frame_id, 1, 4, ptr) != 4)
        {
            break;
        }

        /* Add NULL character */
        frame_id[4] = '\0';

        /* Zero bytes mean the padding has started */
        if(frame_id[0] == '\0')
        {
            break;
        }

        /* Read 4 bytes: frame size */
        if(fread(size, 1, 4, ptr) != 4)
        {
            break;
        }

        unsigned int frame_size = id3_read_size(size, syncsafe);

        /* Read flags */
        if(fread(flags, 1, 2, ptr) != 2)
        {
            break;
        }

        /* A frame can never go past the end of the tag */
        if(ftell(ptr) + (long)frame_size > tag_end)
        {
            break;
        }

        if(strcmp(frame_id, "TIT2") == 0)
        {
            read_frame(ptr, frame_size, title, sizeof(title), 0);
        }
        else if(strcmp(frame_id, "TPE1") == 0)
        {
            read_frame(ptr, frame_size, artist, sizeof(artist), 0);
        }
        else if(strcmp(frame_id, "TALB") == 0)
        {
            read_frame(ptr, frame_size, album, sizeof(album), 0);
        }
        else if(strcmp(frame_id, "TYER") == 0)
        {
            read_frame(ptr, frame_size, year, sizeof(year), 0);
        }
        else if(strcmp(frame_id, "TDRC") == 0)
        {
            /* ID3v2.4 year: may look like 2004-05-01, keep only the year */
            read_frame(ptr, frame_size, year, sizeof(year), 0);
            year[4] = '\0';
        }
        else if(strcmp(frame_id, "TCON") == 0)
        {
            read_frame(ptr, frame_size, genre, sizeof(genre), 0);
        }
        else if(strcmp(frame_id, "COMM") == 0)
        {
            read_frame(ptr, frame_size, comment, sizeof(comment), 1);
        }
        else
        {
            /*
             * This is a frame we don't need.
             * Skip its data.
             */
            fseek(ptr, frame_size, SEEK_CUR);
        }
    }

    printf("\n-----------------------------\n");
    printf("       MP3 TAG DETAILS\n");
    printf("-----------------------------\n");

    printf("Title   : %s\n", title);
    printf("Artist  : %s\n", artist);
    printf("Album   : %s\n", album);
    printf("Year    : %s\n", year);
    printf("Genre   : %s\n", genre);
    printf("Comment : %s\n", comment);

    printf("-----------------------------\n");

    fclose(ptr);
}
