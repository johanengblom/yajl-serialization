/*
 * Copyright (c) 2015, Johan Engblom <johan.engblom@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "yajl/yajl_parse.h"
#include "yajl/yajl_gen.h"

static int reformat_null(void * ctx)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_null(g);
}

static int reformat_boolean(void * ctx, int boolean)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_bool(g, boolean);
}

static int reformat_number(void * ctx, const char * s, size_t l)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_number(g, s, l);
}

static int reformat_string(void * ctx, const unsigned char * stringVal,
                           size_t stringLen)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
}

static int reformat_map_key(void * ctx, const unsigned char * stringVal,
                            size_t stringLen)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_string(g, stringVal, stringLen);
}

static int reformat_start_map(void * ctx)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_map_open(g);
}


static int reformat_end_map(void * ctx)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_map_close(g);
}

static int reformat_start_array(void * ctx)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_array_open(g);
}

static int reformat_end_array(void * ctx)
{
    yajl_gen g = (yajl_gen) ctx;
    return yajl_gen_status_ok == yajl_gen_array_close(g);
}

static yajl_callbacks callbacks = {
    reformat_null,
    reformat_boolean,
    NULL,
    NULL,
    reformat_number,
    reformat_string,
    reformat_start_map,
    reformat_map_key,
    reformat_end_map,
    reformat_start_array,
    reformat_end_array
};

int run_reformat(unsigned char* buffer, unsigned char* outbuffer)
{
    yajl_handle hand;
    /* generator config */
    yajl_gen g;
    yajl_status stat;
    size_t rd = strlen((char *)buffer);
    int retval = 0;

    g = yajl_gen_alloc(NULL);
    yajl_gen_config(g, yajl_gen_beautify, 1);
    yajl_gen_config(g, yajl_gen_validate_utf8, 1);

    /* ok.  let's read and parse */
    hand = yajl_alloc(&callbacks, NULL, (void *) g);
    /* and let's allow comments by default */
    yajl_config(hand, yajl_allow_comments, 1);

    stat = yajl_parse(hand, buffer, rd);

    if (stat != yajl_status_ok) return retval;

    {
    	const unsigned char* buf;
    	size_t len;
    	yajl_gen_get_buf(g, &buf, &len);
    	if (buf != NULL)
    		strcpy((char *)outbuffer, (char *)buf);
    	yajl_gen_clear(g);
    }


    stat = yajl_complete_parse(hand);

    if (stat != yajl_status_ok) {
        unsigned char * str = yajl_get_error(hand, 1, buffer, rd);
        fprintf(stderr, "%s", (const char *) str);
        yajl_free_error(hand, str);
        retval = 1;
    }

    yajl_gen_free(g);
    yajl_free(hand);

    return retval;
}

int jsonVerify(unsigned char *buffer)
{
    yajl_status stat;
    size_t rd;
    yajl_handle hand;
    unsigned char fileData[65536];
    int retval = yajl_status_error;

    /* allocate a parser */
    if (buffer == NULL)
    	return retval;

    rd = strlen((char *)buffer);
    hand = yajl_alloc(NULL, NULL, NULL);
    yajl_config(hand, yajl_allow_comments, 1);
    yajl_config(hand, yajl_dont_validate_strings, 1);
    /* read file data, pass to parser */
    stat = yajl_parse(hand, buffer, rd);

    if (stat == yajl_status_ok)
    {
    	retval = yajl_status_ok;
    	/* parse any remaining buffered data */
    	stat = yajl_complete_parse(hand);

    	if (stat != yajl_status_ok)
    	{
    		unsigned char * str = yajl_get_error(hand, 1, fileData, rd);
    		fprintf(stderr, "%s", (const char *) str);
    		yajl_free_error(hand, str);
    		retval = 1;
    	}
    }

    yajl_free(hand);
    fileData[0] = 0; //NULl plug
    sprintf((char *)&fileData[0],"JSON is %s\n", retval ? "invalid" : "valid" );
    printf("jsonVerify %s\n", (char *)fileData);

    return retval;
}

