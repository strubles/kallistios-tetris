#ifndef COLORS_H
#define COLORS_H

#define RGB(rgb)      ((0xFF << 24) | ((rgb) & 0xFFFFFF)) // default opaque
#define RGBA(rgb, a)  (((a) << 24) | ((rgb) & 0xFFFFFF))

#define COLOR_RED       RGB(0xFF0000)
#define COLOR_ORANGE    RGB(0xFFAE5E)
#define COLOR_YELLOW    RGB(0xFFFF00)
#define COLOR_GREEN     RGB(0x00FF00)
#define COLOR_CYAN      RGB(0x00FFFF)
#define COLOR_BLUE      RGB(0x0000FF)
#define COLOR_PURPLE    RGB(0xFF00FF)
#define COLOR_WHITE     RGB(0xFFFFFF)
#define COLOR_BLACK     RGB(0x000000)

#endif // COLORS_H