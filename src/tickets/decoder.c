/*
 * tickets.c: decode tickets.com captchas
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

#define FONTS 8

/* Main function */
char *decode_tickets(struct image *img)
{
    static struct font *fonts[FONTS];
    char *result;
    struct image *tmp;
    int x, y, r, g, b, cur, i, j, f;
    int curx, bestx, besty;

    for(i = 0; i < FONTS; i++)
    {
        if(!fonts[i])
        {
            char buf[BUFSIZ];
            sprintf(buf, "font%i.png", i + 1);
            fonts[i] = font_load_variable(DECODER, buf, "0123456789");
            if(!fonts[i])
                exit(-1);
        }
    }

    /* tickets.com captchas have 7 characters */
    result = malloc(8 * sizeof(char));
    strcpy(result, "       ");

    /* captcha is jpeg; threshold the image */
    tmp = image_dup(img);
    filter_threshold(tmp, 127);

    /* Guess all glyphs */
    curx = 50;
    for(cur = 0; cur < 7; cur++)
    {
        for(f = 0; f < FONTS; f++)
        {
            for(x = curx - 5; x < curx + 10; x++)
            {
                for(y = 5; y < 15; y++)
                {
                    for(i = 0; i < fonts[f]->size; i++)
                    {
                        int xmin, xmax, ymin, ymax;
                        int t, z;

                        xmin = fonts[f]->glyphs[i].xmin;
                        ymin = fonts[f]->glyphs[i].ymin;
                        xmax = fonts[f]->glyphs[i].xmax;
                        ymax = fonts[f]->glyphs[i].ymax;
                        for(t = 0; t < ymax - ymin; t++)
                        {
                            for(z = 0; z < xmax - xmin; z++)
                            {
                                int r, r2;
                                getgray(fonts[f]->img, xmin + z, ymin + t, &r);
                                getgray(tmp, x + z, y + t, &r2);
                                if(r < 127 && r2 > 127)
                                    goto char_failed;
                            }
                        }
                        goto char_ok;
                    char_failed:
                        continue;
                    }
                }
            }
        }
        result[cur] = '?';
        curx += 10; /* XXX: totally random */
        continue;
    char_ok:
        result[cur] = fonts[f]->glyphs[i].c;
        curx = x + fonts[f]->glyphs[i].xmax - fonts[f]->glyphs[i].xmin;
    }

    image_free(tmp);

    return result;
}

