#include <stdio.h>
#include <string.h>
#include "view.h"
#include "edit.h"

static void print_help(void)
{
    printf("MP3 Tag Reader and Editor\n");
    printf("\n");
    printf("Usage:\n");
    printf("./a.out -v <mp3_file>\n");
    printf("./a.out -e <mp3_file> -t <new_title>\n");
    printf("./a.out -e <mp3_file> -a <new_artist>\n");
    printf("./a.out -e <mp3_file> -A <new_album>\n");
    printf("./a.out -e <mp3_file> -y <new_year>\n");
    printf("./a.out -e <mp3_file> -g <new_genre>\n");
    printf("\n");
    printf("Options:\n");
    printf("-v    View MP3 tag information\n");
    printf("-e    Edit MP3 tag information\n");
    printf("-t    Edit Title\n");
    printf("-a    Edit Artist\n");
    printf("-A    Edit Album\n");
    printf("-y    Edit Year\n");
    printf("-g    Edit Genre\n");
    printf("-h    Display help\n");
}

int main(int argc, char *argv[])
{
    
    if(argc < 2)
    {
        print_help();
        return 0;
    }

    if(strcmp(argv[1], "-v") == 0)
    {
        if(argc != 3)
        {
            printf("Usage: ./a.out -v <mp3 file>\n");
            return 0;
        }

        view(argv[2]);
    }
    else if(strcmp(argv[1], "-e") == 0)
    {
        if(argc != 5)
        {
            printf("Usage: ./a.out -e <mp3 file> <-t/-a/-A/-y/-g> <new value>\n");
            return 0;
        }

        edit(argv[2], argv[3], argv[4]);
    }
    else if(strcmp(argv[1], "-h") == 0)
    {
        print_help();
    }
    else
    {
        print_help();
    }

    return 0;
}
