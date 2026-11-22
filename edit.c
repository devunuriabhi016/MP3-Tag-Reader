#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "view.h"
#include "edit.h"

#define TEMP_FILE "newmp3file.mp3"

void edit(char *filename, char *edit_tag, char *new_name)
{
    FILE *ptr = NULL;
    FILE *ptr1 = NULL;
    unsigned char *old_text = NULL;
    unsigned char *temp = NULL;
    int created = 0;        
    int success = 0;       

    unsigned char header[10];
    char target_id[5];
    int version;
    int syncsafe;
    long old_tag_size;
    long tag_end;

    ptr = fopen(filename, "rb");

    if(ptr == NULL)
    {
        printf("File cannot be opened\n");
        return;
    }


    if(fread(header, 1, 10, ptr) != 10 ||
       header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("Invalid MP3 file (ID3 tag not found)\n");
        goto done;
    }

    version = header[3];

    if(version != 3 && version != 4)
    {
        printf("Unsupported ID3 version (only ID3v2.3 and ID3v2.4)\n");
        goto done;
    }

    if(header[5] & 0x40)
    {
        printf("Extended ID3 header is not supported\n");
        goto done;
    }

    syncsafe = (version == 4);


    if(strcmp(edit_tag, "-t") == 0)
    {
        strcpy(target_id, "TIT2");
    }
    else if(strcmp(edit_tag, "-a") == 0)
    {
        strcpy(target_id, "TPE1");
    }
    else if(strcmp(edit_tag, "-A") == 0)
    {
        strcpy(target_id, "TALB");
    }
    else if(strcmp(edit_tag, "-y") == 0)
    {
        strcpy(target_id, syncsafe ? "TDRC" : "TYER");

        if(strlen(new_name) != 4 ||
           !isdigit((unsigned char)new_name[0]) ||
           !isdigit((unsigned char)new_name[1]) ||
           !isdigit((unsigned char)new_name[2]) ||
           !isdigit((unsigned char)new_name[3]))
        {
            printf("Year must be 4 digits\n");
            goto done;
        }
    }
    else if(strcmp(edit_tag, "-g") == 0)
    {
        strcpy(target_id, "TCON");
    }
    else
    {
        printf("Invalid edit option\n");
        goto done;
    }

    if(strlen(new_name) == 0)
    {
        printf("New value cannot be empty\n");
        goto done;
    }

    ptr1 = fopen(TEMP_FILE, "wb");

    if(ptr1 == NULL)
    {
        printf("New file cannot be created\n");
        goto done;
    }

    created = 1;


    fwrite(header, 1, 10, ptr1);

   

    old_tag_size = (long)id3_read_size(&header[6], 1);

    tag_end = 10 + old_tag_size;

    while(ftell(ptr) + 10 <= tag_end)
    {
        

        long frame_start = ftell(ptr);

        char frame_id[5];
        unsigned char size[4];
        unsigned char flags[2];
        unsigned int frame_size;

       

        if(fread(frame_id, 1, 4, ptr) != 4)
        {
            break;
        }

        frame_id[4] = '\0';

       

        if(frame_id[0] == '\0')
        {
            break;
        }

        

        if(fread(size, 1, 4, ptr) != 4)
        {
            break;
        }

        frame_size = id3_read_size(size, syncsafe);


        if(fread(flags, 1, 2, ptr) != 2)
        {
            break;
        }

       

        if(frame_start + 10 + (long)frame_size > tag_end)
        {
            printf("Invalid frame size\n");
            break;
        }

       

        if(strcmp(frame_id, target_id) == 0)
        {
            printf("%s found\n", target_id);

            if(frame_size > 0)
            {
                unsigned char encoding;

               

                if(fread(&encoding, 1, 1, ptr) != 1)
                {
                    printf("Cannot read encoding\n");
                    goto done;
                }

                old_text = malloc(frame_size);

                if(old_text == NULL)
                {
                    printf("Memory allocation failed\n");
                    goto done;
                }

                

                if(fread(old_text, 1, frame_size - 1, ptr) != frame_size - 1)
                {
                    printf("Cannot read old tag\n");
                    goto done;
                }

                old_text[frame_size - 1] = '\0';

                if(encoding == 0 || encoding == 3)
                {
                    printf("Old Name : %s\n", old_text);
                }
                else
                {
                    printf("Old Name : (UTF-16 text)\n");
                }

                free(old_text);
                old_text = NULL;
            }
            else
            {
                printf("Old Name : (empty)\n");
            }

           

            unsigned int new_frame_size = 1 + (unsigned int)strlen(new_name);

            printf("New Name : %s\n", new_name);
            printf("New Frame Size : %u\n", new_frame_size);

            long new_tag_size = old_tag_size - (long)frame_size + (long)new_frame_size;

            if(new_tag_size > 0x0FFFFFFF)
            {
                printf("New value is too long\n");
                goto done;
            }

            

            unsigned char newsize[4];

            id3_write_size(newsize, new_frame_size, syncsafe);

           

            unsigned char new_flags[2] = {0, 0};
            unsigned char new_encoding = syncsafe ? 3 : 0;

            fwrite(frame_id, 1, 4, ptr1);

            fwrite(newsize, 1, 4, ptr1);

            fwrite(new_flags, 1, 2, ptr1);

            

            fwrite(&new_encoding, 1, 1, ptr1);


            fwrite(new_name, 1, strlen(new_name), ptr1);

            

            fseek(ptr, frame_start + 10 + (long)frame_size, SEEK_SET);


            size_t n;
            char buffer[1024];

            while((n = fread(buffer, 1, sizeof(buffer), ptr)) > 0)
            {
                fwrite(buffer, 1, n, ptr1);
            }

           

            id3_write_size(&header[6], (unsigned int)new_tag_size, 1);

           

            fseek(ptr1, 0, SEEK_SET);

            fwrite(header, 1, 10, ptr1);

            if(fclose(ptr1) != 0)
            {
                ptr1 = NULL;
                printf("Error while writing the new file\n");
                goto done;
            }

            ptr1 = NULL;

            fclose(ptr);
            ptr = NULL;


            if(rename(TEMP_FILE, filename) != 0)
            {
                printf("Cannot replace the original file\n");
                goto done;
            }

            printf("Tag edited successfully\n");
            printf("File updated successfully\n");

            success = 1;
            goto done;
        }
        else
        {
          

            if(frame_size > 0)
            {
                temp = malloc(frame_size);

                if(temp == NULL)
                {
                    printf("Memory allocation failed\n");
                    goto done;
                }

                if(fread(temp, 1, frame_size, ptr) != frame_size)
                {
                    printf("Cannot read frame data\n");
                    goto done;
                }
            }

            fwrite(frame_id, 1, 4, ptr1);
            fwrite(size, 1, 4, ptr1);
            fwrite(flags, 1, 2, ptr1);

            if(frame_size > 0)
            {
                fwrite(temp, 1, frame_size, ptr1);
            }

            free(temp);
            temp = NULL;
        }
    }

    printf("%s frame not found\n", target_id);

done:
    free(old_text);
    free(temp);

    if(ptr != NULL)
    {
        fclose(ptr);
    }

    if(ptr1 != NULL)
    {
        fclose(ptr1);
    }



    if(created && !success)
    {
        remove(TEMP_FILE);
    }
}
