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


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "JSONParseWrapper.h"
#include "JSONReformat.h"
#include "yajl/yajl_parse.h"
#include "yajl/yajl_gen.h"
#include "yajl/yajl_tree.h"

/// The maximum size of json message allowed
#define JSONBUF_MAX (USHRT_MAX)

int currentMapkey = -1;
int arrayIndex = -1;
JSON_SPEC *currentSpec;
int dept = 0;
char key[UCHAR_MAX];

static int parse_bool(void * ctx, int boolVal)
{

    if (currentMapkey != -1)
    {
    	if (arrayIndex<0)
    	{
    		printf("parse_bool - boolVal %d\n", boolVal);
    		long long *ll = currentSpec[currentMapkey].value_ptr;
    		if (ll == NULL)
    			return yajl_status_error;

    		*ll = boolVal;
    	}
    	else
    	{
    		long long * gg = (long long *)currentSpec[currentMapkey].value_ptr;
    		gg[arrayIndex++] = boolVal;
    		long long * elems = (long long *)(currentSpec[currentMapkey].arrayelements);
    		if (elems == NULL)
    			return yajl_status_error;

    		*elems = arrayIndex;
    	}
    }
    return 1;
}

static int parse_integer(void * ctx, long long integerVal)
{

    if (currentMapkey != -1)
    {
    	if (arrayIndex<0)
    	{
    		printf("parse_integer integerVal = %lld , currentMapKey = %d -> %s\n", integerVal, currentMapkey,  currentSpec[currentMapkey].name);
        	if (currentSpec[currentMapkey].value_ptr != NULL)
        	{
        		long long *ll = currentSpec[currentMapkey].value_ptr;
        		if (ll==NULL)
        			return yajl_status_error;

        		*ll = integerVal;
        	}
    	}
    	else
    	{
    		long long * gg = (long long *)currentSpec[currentMapkey].value_ptr;
    		gg[arrayIndex++] = integerVal;
    		long long * elems = (long long *)(currentSpec[currentMapkey].arrayelements);
    		if (elems==NULL)
    			return yajl_status_error;

    		*elems = arrayIndex;
    	}
    }
    return 1;
}

static int parse_double(void * ctx, double dblVal)
{

    if (currentMapkey != -1)
    {
    	if (arrayIndex<0)
    	{
    		printf("parser_double dblVal %e\n", dblVal);
    		double *d = currentSpec[currentMapkey].value_ptr;
    		if (d==NULL)
    			return yajl_status_error;

    		*d = dblVal;
    	}
    	else
    	{
    		double * gg = (double *)currentSpec[currentMapkey].value_ptr;
    		gg[arrayIndex++] = dblVal;
    		long long * elems = (long long *)(currentSpec[currentMapkey].arrayelements);
    		if (elems==NULL)
    			return yajl_status_error;

    		*elems = arrayIndex;
    	}
    }
    return 1;
}

static int parse_string(void * ctx, const unsigned char * stringVal,
                           size_t stringLen)
{
	if (currentMapkey != -1)
	{
		memset(currentSpec[currentMapkey].value_ptr,0,stringLen+1);
		strncpy(currentSpec[currentMapkey].value_ptr, (char *)stringVal, stringLen);

		printf("parse_string - (%s , %.*s)\n", key, (int)stringLen, (char *)stringVal);
	}
	return 1;
}

static int parse_map_key(void * ctx, const unsigned char * stringVal, size_t stringLen)
{
	int i = 0;
	currentMapkey = -1;
	memset(key,0,UCHAR_MAX); // Reset keyvalue
	strncpy(key, (char *)stringVal, stringLen);
	while(currentSpec[i].name != NULL)
	{
		if (strlen(currentSpec[i].name) == stringLen)
		{
			int status = strncmp(currentSpec[i].name, (const char*)stringVal, stringLen);
			if (!status)
			{
				printf("parse_map_key %*.*s  == %s\n", (int)stringLen, (int)stringLen, stringVal, currentSpec[i].name);
				currentMapkey = i;
				break;
			}
		}
		i++;
	}
	return 1;
}

static int parse_start_map(void * ctx)
{
	dept++;
	printf("start map - dept %d\n", dept);

	return 1;
}

static int parse_end_map(void * ctx)
{
	puts("parse_end map");
	memset(key,0,UCHAR_MAX);
	dept--;
	return 1;
}

static int parse_start_array(void * ctx)
{
	puts("parse_start_array");
	arrayIndex = 0;
	return 1;
}

static int parse_end_array(void * ctx)
{
	puts("parse_end_array");
	arrayIndex = -1;
	return 1;
}

static yajl_callbacks parse_callbacks = {
    NULL,
    parse_bool,
    parse_integer,
    parse_double,
    NULL, //reformat_number,
    parse_string,
    parse_start_map,
    parse_map_key,
    parse_end_map,
    parse_start_array,
    parse_end_array
};

yajl_status doParse(JSON_SPEC *specification, const char* data)
{
    yajl_handle hand;
    yajl_gen handle;
    yajl_status stat;
    int dataLen = strlen(data);

    //if (yajl_status_ok == jsonVerify(data))
    //{
    	currentSpec = specification;

    	arrayIndex = -1;

    	handle = yajl_gen_alloc(NULL);
    	yajl_gen_config(handle, yajl_gen_beautify, 1);
    	yajl_gen_config(handle, yajl_gen_validate_utf8, 1);

    	/* ok.  let's read and parse */
    	hand = yajl_alloc(&parse_callbacks, NULL, (void *) handle);

    	/* and let's allow comments by default */
    	yajl_config(hand, yajl_allow_comments, 1);

    	stat = yajl_parse(hand, (const unsigned char*)data, dataLen);

    	yajl_free(hand);

    	yajl_gen_free(handle);
    //}
    //else
    //{
    //	stat = yajl_status_error;
    //}

    return stat;
}



yajl_gen_status yajl_gen_bool_wrap(yajl_gen g,char *name, int* value)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_bool(g,*value);
	return status;
}

yajl_gen_status yajl_gen_integer_wrap(yajl_gen g, char *name, long long int *value)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_integer(g,*value);

	return status;
}

yajl_gen_status yajl_gen_double_wrap(yajl_gen g, char *name, double *value)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_double(g, *value);
	return status;
}

yajl_gen_status yajl_gen_string_wrap(yajl_gen g, char *name, char *value)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_string(g,(unsigned char *)value, strlen(value));
	return status;
}

yajl_gen_status yajl_gen_intarray_wrap(yajl_gen g,char *name, long long int* value, long long *nbrElements)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_array_open(g);
	long long index = 0;
	while(index<*nbrElements)
	{
		status = yajl_gen_integer(g,value[index++]);
	}
	status = yajl_gen_array_close(g);
	return status;
}

yajl_gen_status yajl_gen_stringarray_wrap(yajl_gen g,char *name, char** value, long long *nbrElements)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_array_open(g);
	long long index = 0;
	while(index<*nbrElements)
	{
		status = yajl_gen_string(g, (unsigned char*)value[index], strlen(value[index]));
		index++;
	}
	status = yajl_gen_array_close(g);
	return status;
}

yajl_gen_status yajl_gen_map_open_wrap(yajl_gen g, char *name)
{
	yajl_gen_status status = yajl_gen_string(g, (unsigned char*)name, strlen(name));
	status = yajl_gen_map_open(g);
	return status;
}

yajl_gen_status yajl_gen_map_close_wrap(yajl_gen g)
{
	yajl_gen_status status = yajl_gen_map_close(g);
	return status;
}



int doGenParams(yajl_gen g, JSON_SPEC *specification)
{
	/* generator config */
	yajl_gen_status gen_status;

	int index=0;

	arrayIndex = -1;

	yajl_gen_map_open(g);
	while(specification[index].name != NULL)
	{
		switch(specification[index].type)
		{
		case (json_type_boolean) :
		gen_status = yajl_gen_bool_wrap(g,specification[index].name, (int *)specification[index].value_ptr);
		break;
		case (json_type_integer) :
		gen_status = yajl_gen_integer_wrap(g,specification[index].name, (long long int *)specification[index].value_ptr);
		break;
		case (json_type_double) :
		gen_status = yajl_gen_double_wrap(g,specification[index].name, (double *)specification[index].value_ptr);
		break;
		case (json_type_string) :
		gen_status = yajl_gen_string_wrap(g,specification[index].name, (char *)specification[index].value_ptr);
		break;
		case (json_type_intArray) :
		gen_status = yajl_gen_intarray_wrap(g,specification[index].name, (long long *)specification[index].value_ptr, specification[index].arrayelements);
		break;
		case (json_type_stringArray) :
				gen_status = yajl_gen_stringarray_wrap(g,specification[index].name, (char **)specification[index].value_ptr, specification[index].arrayelements);
		break;
		case (json_type_object) :
		{
			yajl_gen_string(g, (unsigned char*)specification[index].name, strlen(specification[index].name));
			gen_status = doGenParams(g,specification[index].value_ptr);
			break;
		}
		}

		index++;
	}
	yajl_gen_map_close(g);

	return gen_status;
}

int doGen(JSON_SPEC *specification, char *result, int beautify)
{
	yajl_gen g = yajl_gen_alloc(NULL);

	yajl_gen_config(g, yajl_gen_beautify, beautify);

	yajl_gen_config(g, yajl_gen_validate_utf8, 1);

	yajl_gen_status gen_status = doGenParams(g,specification);

	unsigned char** b = malloc(JSONBUF_MAX);
	size_t len;
	yajl_gen_get_buf(g, (const unsigned char**)b, &len);
	strcpy(result, (char *)b[0]);
	yajl_gen_clear(g);
	free(b);

	yajl_gen_free(g);

	return gen_status;
}
