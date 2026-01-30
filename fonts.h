#ifndef FONTS_H
#define FONTS_H

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

typedef struct {
    const unsigned char *data;
    int width, height;
    char base;
} fontinfo_t;

extern const fontinfo_t font_noto_mono40x56;
extern const fontinfo_t font_noto_mono23x37;
extern const fontinfo_t font_noto_mono6x10;
extern const fontinfo_t font_noto_vari20x28;
extern const fontinfo_t font_icon_40x60;

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#endif /* FONTS_H */
