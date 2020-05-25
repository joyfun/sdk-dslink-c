/*
 * File      : mimetype.c
 * This file is part of RT-Thread RTOS/WebNet Server
 * COPYRIGHT (C) 2011, Shanghai Real-Thread Technology Co., Ltd
 *
 * All rights reserved.
 *
 * Change Logs:
 * Date           Author       Notes
 * 2011-08-02     Bernard      the first version
 */

#include <string.h>
#include "util.h"
#define GET_ARRAY_LEN(array,len){len = (sizeof(array) / sizeof(array[0]));}


struct webnet_mime_entry
{
    const char* name;
    const char* type;
};

static const struct webnet_mime_entry mime_tables[] =
{
    { "any",	"application/binary" }, /* default. */
    { "html",	"text/html" },
    { "htm",	"text/html" },
    { "css",	"text/css" },
    { "txt",	"text/plain" },
    { "pdf",	"application/pdf" },
    { "gif",	"image/gif" },
    { "png",	"image/png" },
    { "jpeg",	"image/jpeg" },
    { "jpg",	"image/jpeg" },
    { "avi",	"video/x-msvideo" },
    { "mp3",	"audio/mpeg" },
    { "ogg",	"audio/x-oggvorbis" },
    { "wav",	"audio/x-wav" },
    { "class",	"application/octet-stream" },
    { "js",		"application/x-javascript" },
    { "tar",	"application/x-tar" },
    { "zip",	"application/zip" },
    { "xml",	"text/xml" }
    };

/**
 * get mime type according to URL
 */
const char* mime_get_type(const char* url)
{
    int index;

    index = 0;
    if (url == NULL) return mime_tables[0].type;
    int mimeLen;
    GET_ARRAY_LEN(mime_tables,mimeLen);
    while (mime_tables[index].name != NULL&&index<mimeLen)
    {
        if (is_end_with(url, mime_tables[index].name)==0)
            return mime_tables[index].type;
        index ++;
    }

    /* return text/html as default */
    return mime_tables[0].type;
}
