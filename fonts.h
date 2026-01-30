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

inline int font_char_width(const fontinfo_t *font, char c, int mono) {
    if (!font || !font->data)
        return 0;
    if (c < font->base)
        return 0;
    const int font_rows = (font->height + 7) / 8;
    const int char_offs = ((font->width * font_rows) + 1) * (c - font->base);
    return mono ? font->width : (int)font->data[char_offs];
}

// ------------------------------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------------------------------

#endif /* FONTS_H */
