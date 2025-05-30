execute fbterm (fb0 is the display adapter and term for terminal) : ./fbterm
arguments : 
-font large --> bigger font (currently broken)
-image "path-to-image.png" (only supports pngs for now)
-text "Hello framebuffer!" (only ascii and single-line)


TODO: add multiple fonts, multi-line support and potentially other image formats. 

to compile (requires aarch64-linux-gnu-gcc): 
aarch64-linux-gnu-gcc -o fbterm fbterm.c -lm | chmod +x fbterm

