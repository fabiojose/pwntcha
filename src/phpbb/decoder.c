/*
 * phpbb.c: decode phpBB captchas
 * $Id: decoder.c 2318 2008-04-26 08:41:43Z sam $
 *
 * Copyright: (c) 2005 Sam Hocevar <sam@zoy.org>
 *  This program is free software. It comes without any warranty, to
 *  the extent permitted by applicable law. You can redistribute it
 *  and/or modify it under the terms of the Do What The Fuck You Want
 *  To Public License, Version 2, as published by Sam Hocevar. See
 *  http://sam.zoy.org/wtfpl/COPYING for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "config.h"
#include "common.h"

/* Main function */
char *decode_phpbb(struct image *img)
{
    static struct font *font = NULL;
    char *result;
    struct image *tmp1, *tmp2;
    int x, y, i = 0;
    int r, g, b;
    int xmin, xmax, ymin, ymax, cur, offset = -1;
    int distmin, distx, disty, distch;

    if(!font)
    {
        font = font_load_fixed(DECODER, "font.png",
                               "ABCDEFGHIJKLMNOPQRSTUVWXYZ123456789");
        if(!font)
            exit(-1);
    }

    /* phpBB captchas have 6 characters */
    result = malloc(7 * sizeof(char));
    strcpy(result, "      ");

    tmp1 = image_dup(img);
    tmp2 = image_new(img->width, img->height);

    filter_smooth(tmp1);
    filter_threshold(tmp1, 128);

    for(x = 0; x < img->width; x++)
        for(y = 0; y < img->height; y++)
        {
            getpixel(tmp1, x, y, &r, &g, &b);
            if(r == 0 && offset == -1)
                offset = x;
            getpixel(img, x, y, &r, &g, &b);
            setpixel(tmp2, x, y, 255, g, 255);
        }

    for(cur = 0; cur < 6; cur++)
    {
        /* Try to find 1st letter */
        distmin = INT_MAX;
        for(i = 0; i < font->size; i++)
        {
            int localmin = INT_MAX, localx, localy;
            xmin = font->glyphs[i].xmin;
            ymin = font->glyphs[i].ymin;
            xmax = font->glyphs[i].xmax;
            ymax = font->glyphs[i].ymax;
            for(y = 0; y < img->height - (ymax - ymin); y++)
            {
                x = offset - 3;
                if(cur == 0)
                    x -= 10;
                if(x < 0)
                    x = 0;
                for(; x < offset + 3; x++)
                {
                    int z, t, dist;
                    dist = 0;
                    for(t = 0; t < ymax - ymin; t++)
                        for(z = 0; z < xmax - xmin; z++)
                        {
                            int r2;
                            getgray(font->img, xmin + z, ymin + t, &r);
                            getgray(tmp1, x + z, y + t, &r2);
                            if(r > r2)
                                dist += r - r2;
                            else
                                dist += (r2 - r) * 3 / 4;
                        }
                    if(dist < localmin)
                    {
                        localmin = dist;
                        localx = x;
                        localy = y;
                    }
                }
            }
            if(localmin < distmin)
            {
                distmin = localmin;
                distx = localx;
                disty = localy;
                distch = i;
            }
        }

        /* Print min glyph (debug) */
        xmin = font->glyphs[distch].xmin;
        ymin = font->glyphs[distch].ymin;
        xmax = font->glyphs[distch].xmax;
        ymax = font->glyphs[distch].ymax;
        for(y = 0; y < ymax - ymin; y++)
            for(x = 0; x < xmax - xmin; x++)
            {
                int r2;
                getpixel(font->img, xmin + x, ymin + y, &r2, &g, &b);
                if(r2 > 128)
                    continue;
                getpixel(tmp2, distx + x, disty + y, &r, &g, &b);
                setpixel(tmp2, distx + x, disty + y, r2, g, b);
            }

        offset = distx + xmax - xmin;
        result[cur] = font->glyphs[distch].c;
    }

    image_free(tmp1);
    image_free(tmp2);

    return result;
}

