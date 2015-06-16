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

#ifndef JSONPARSEWRAPPER_H_
#define JSONPARSEWRAPPER_H_

#include "yajl/yajl_parse.h"
#include "yajl/yajl_gen.h"
#include "yajl/yajl_tree.h"

/**
 * description of the different json data types
 */
typedef enum json_types {
	json_type_boolean,    //!< json_type_boolean
	json_type_integer,    //!< json_type_integer
	json_type_double,     //!< json_type_double
	json_type_string,     //!< json_type_string
	json_type_intArray,   //!< json_type_intArray
	json_type_stringArray,//!< json_type_stringArray
	json_type_object,     //!< json_type_object
}json_types;

/**
 * Description a JSON element
 */
typedef struct JSON_SPEC
{
	/** The name of the JSON element */
	char *name;
	/** The pointer to the value of the element */
	void *value_ptr;
	/** The element type */
	json_types type;
	/** Extra parameter describing the length of the array if the type is array */
	long long *arrayelements;
}JSON_SPEC;

/**
 * Generate boolean value
 * @param g
 * @param name
 * @param value
 * @return
 */
yajl_gen_status yajl_gen_bool_wrap(yajl_gen g,char *name, int* value);

/**
 * Generate an integer value
 * @param g
 * @param name
 * @param value
 * @return
 */
yajl_gen_status yajl_gen_integer_wrap(yajl_gen g,char *name, long long int* value);

/**
 * Generate a double value
 * @param g
 * @param name
 * @param value
 * @return
 */
yajl_gen_status yajl_gen_double_wrap(yajl_gen g,char *name, double* value);

/**
 * Generate a string value
 * @param g
 * @param name
 * @param value
 * @return
 */
yajl_gen_status yajl_gen_string_wrap(yajl_gen g,char *name, char* value);

/**
 * Generate an integer array
 * @param g
 * @param name
 * @param value
 * @param nbrElements
 * @return
 */
yajl_gen_status yajl_gen_intarray_wrap(yajl_gen g, char *name, long long int* value, long long* nbrElements);

/**
 * Generate a string array
 * @param g
 * @param name
 * @param value
 * @param nbrElements
 * @return
 */
yajl_gen_status yajl_gen_stringarray_wrap(yajl_gen g, char *name, char **value, long long* nbrElements);

/**
 * Generate object wrap
 * @param g
 * @param name
 * @param jsonStruct
 * @return
 */
yajl_gen_status yajl_gen_object_wrap(yajl_gen g, char *name, JSON_SPEC *jsonStruct);

/**
 * Parse a JSON message given the specification
 * @param specification
 * @param data
 * @return
 */
yajl_status doParse(JSON_SPEC *specification, const char* data);
/**
 * Generate a json formatted message from a specification
 * @param specification
 * @param result
 * @param beautify  if the output should be pretty or not:)
 * @return
 */
int doGen(JSON_SPEC *specification, char *result, int beautify);

/**
 * Generate parameters from specification
 * @param g
 * @param specification
 * @return
 */
int doGenParams(yajl_gen g, JSON_SPEC *specification);


#endif /* JSONPARSEWRAPPER_H_ */
