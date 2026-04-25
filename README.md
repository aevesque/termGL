
# termGL

A minimal graphics library in C for the Linux terminal.

Uses ANSI sequences and a special unicode character to display RGB pixels.

## Dependencies

- Make (optionnal, this is a single file library)
- a C compiler (with libc)

that's it!
## Documentation

see also: `innclude/termGL.h`.

#### Frames
Frames are a representation of the pixels to be displayed on screen.

`pixels` is an array of int. Each pixel has a color represented by a value in the array using the format is 0xRRGGBB.

Everytime the displayFrame() function is called, the Frame's pixels get translated to printable characters, stored in the preallocated frame buffer and sent to stdout using `write()`.

Every pixel of a frame gets drawn.

#### Images

Images are reusable "sub-frames" to be copied into Frames at specific, user-specified coordinates using `putImageInFrame()`.

Same as Frame, they contain a `pixels` array.

To facilitate ASCII art and preprocessor magic, Images can be created from strings using `strToNewImage()`.

#### Memory managament

You are encouraged to reuse Frames : their somewhat big internal buffer are allocated on creation to the appropriate size for any combination of pixels fitting into the user-specified size.

To avoid leaks, Frames and Images need to be disposed of before the end of the program by calling the appropriate function, either `destroyFrame()` or `destroyImage()`.

#### Warning

- No boundary checking is done when setting/getting pixel values. Respecting the Frame/Image size set on creation is the user's responsability.
