#ifndef FONT_STUFF_H_   /* Include guard */
#define FONT_STUFF_H_

extern u32 * texture_mem;

extern u32 * texture_pointer;

int TTFLoadFont(char * path, void * from_memory, int size_from_memory);

void TTFUnloadFont();

void LoadTexture();

void DrawBackground2D(u32 rgba);

void DrawSprites2D(float x, float y, float layer, float dx, float dy, u32 rgba);

#endif // FONT_STUFF_H_