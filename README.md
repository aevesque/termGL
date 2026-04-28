
# termGL

A minimal graphics library in C for the Linux terminal.

Uses ANSI sequences and a special unicode character to display RGB pixels.

## Dependencies

- Make (optionnal, this is a single file library)
- a C compiler (with libc)

that's it!
## Documentation

see `include/termGL.h`.

#### Warning

- This is work in progress ; Nothing is final. The API is subject to changes breaking compatibility with older versions.  
- No boundary checking is done when setting/getting pixel values. Respecting the Window/Image size set on creation is the user's responsability.
