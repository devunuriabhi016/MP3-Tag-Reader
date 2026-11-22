# MP3 Tag Reader and Editor in C

A small command line tool that reads and edits the ID3 tag of an MP3 file. I made it in C to practice binary file handling, command line arguments, structures of bytes, and working with several `.c` and `.h` files in one project.

You can view the tag details of a song, and change its title, artist, album, year or genre. The music data is never touched, only the tag at the start of the file.

## What you can do

| Command | What it does |
|---|---|
| `./a.out -v song.mp3` | Show the tag details |
| `./a.out -e song.mp3 -t "New title"` | Edit the title |
| `./a.out -e song.mp3 -a "New artist"` | Edit the artist |
| `./a.out -e song.mp3 -A "New album"` | Edit the album |
| `./a.out -e song.mp3 -y 2024` | Edit the year |
| `./a.out -e song.mp3 -g "Pop"` | Edit the genre |
| `./a.out -h` | Show the help |

Running the program with no options also shows the help.

## Example

```
$ ./a.out -v song.mp3

-----------------------------
       MP3 TAG DETAILS
-----------------------------
Title   : Example Song
Artist  : Example Artist
Album   : Example Album
Year    : 2004
Genre   : Pop
Comment : Downloaded from example.com
-----------------------------

$ ./a.out -e song.mp3 -t "Better Title"
TIT2 found
Old Name : Example Song
New Name : Better Title
New Frame Size : 13
Tag edited successfully
File updated successfully
```

## How to run

You need `gcc` installed.

```bash
git clone https://github.com/devunuriabhi016/MP3-Tag-Reader.git
cd MP3-Tag-Reader
gcc main.c view.c edit.c -o a.out
./a.out -v song.mp3
```

Editing changes the file you give it, so keep a copy of your song before you try `-e`.

## How this project works, in simple points

1. An MP3 file has two parts: a small **tag** at the start (title, artist, album, year, genre) and then the **music**.

   ```
   [ tag: title, artist, album ... ][ music data ........................ ]
   ```

2. You run the program in the terminal and tell it what to do: `-v` to look at the tag, `-e` to change it.
3. `main.c` reads what you typed and sends the work to `view.c` or `edit.c`.
4. **To view:** the program opens the song, reads only the tag, picks out the title, artist, album, year, genre and comment, and prints them neatly. It never reads the music.
5. **To edit:** the program makes a new file, copies the tag piece by piece, swaps the one piece you asked to change for the new value, and then copies the rest of the song exactly as it was.
6. The tag starts with its own size, so players know where the music begins. When your new text is longer or shorter, the program updates that size so the song still plays.
7. At the end the new file takes the place of the old one. If anything goes wrong on the way, the program says why and your song is left unchanged.

## How an MP3 tag is stored

An ID3 tag sits at the very start of the file. It has a 10 byte header followed by frames.

```
 Header (10 bytes)
 ┌─────┬─────────┬───────┬────────────────────┐
 │ ID3 │ version │ flags │ tag size (4 bytes) │
 └─────┴─────────┴───────┴────────────────────┘

 Each frame
 ┌──────────────┬───────────────┬───────────┬──────────────────────────┐
 │ frame ID     │ frame size    │ flags     │ data                     │
 │ 4 chars      │ 4 bytes       │ 2 bytes   │ encoding byte + text     │
 │ e.g. TIT2    │               │           │                          │
 └──────────────┴───────────────┴───────────┴──────────────────────────┘
```

| Frame ID | Meaning | Option |
|---|---|---|
| `TIT2` | Title | `-t` |
| `TPE1` | Artist | `-a` |
| `TALB` | Album | `-A` |
| `TYER` (v2.3) or `TDRC` (v2.4) | Year | `-y` |
| `TCON` | Genre | `-g` |
| `COMM` | Comment | view only |

The tag size in the header uses only 7 bits of every byte (called syncsafe). In ID3v2.3 the frame sizes are normal big endian numbers, and in ID3v2.4 they are syncsafe too. The program handles both.

## How it works

**Viewing (`-v`)**
1. Open the file in binary mode and read the 10 byte header.
2. Check for `ID3` and read the tag size.
3. Read frame after frame until the end of the tag or the padding.
4. Keep the frames we need and skip the rest.
5. Print the details.

**Editing (`-e`)**
1. Read and check the header, and check the option.
2. Copy the header and every frame before the target frame to a new file, `newmp3file.mp3`.
3. Write the new frame with the new text and the new frame size.
4. Copy everything after the old frame, including the rest of the tag and all the audio.
5. Write the new tag size back into the header.
6. Replace the original file with the new one.

If anything goes wrong, the temporary file is deleted and your original file is left as it was.

## What it checks

- The file must open, and must start with an ID3 tag
- Only ID3v2.3 and ID3v2.4 are supported
- The edit option must be one of `-t`, `-a`, `-A`, `-y`, `-g`
- The year must be exactly 4 digits
- The new value cannot be empty
- Frame sizes are checked so a broken tag cannot read past its end
- UTF-16 text is shown as plain characters when viewing

## Files in this project

- `main.c` reads the command line and calls the right function
- `view.c` reads and prints the tag
- `edit.c` changes one frame and rewrites the file
- `view.h` has the function declaration and the helpers for reading and writing tag sizes
- `edit.h` has the function declaration for `edit`

## What I learned

- Reading and writing binary files with `fopen`, `fread`, `fwrite`, `fseek` and `ftell`
- Command line arguments with `argc` and `argv`
- Building numbers from bytes using shifts and masks
- Allocating and freeing memory with `malloc` and `free`
- Handling every error path, so no half written file is left behind
- Splitting one program into several `.c` and `.h` files

## Limits and what I want to improve

- Only edits frames that already exist. If the tag has no `TALB`, for example, it says the frame was not found instead of adding one
- Comments can be viewed but not edited
- ID3v1 and ID3v2.2 tags are not supported
- New text is written as plain text, so keep to simple characters
- Add a Makefile
- Add an option to create a backup before editing
