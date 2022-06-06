/*Licensed under MIT or Public Domain. See bottom of file for details.

micro_ply.h
	Written by Nick Klingensmith, @koujaku on Twitter, @maluoi on GitHub

	This is a small ASCII .ply loader and converter. It's intended to be as
	short as possible, while still being easily readable! The idea is that it
	can be trivially embedded in another single header library or single file
	project without too much trouble.
	
Example usage:

	// micro_ply.h follows stb library conventions, so MICRO_PLY_IMPL must 
	// be defined before include in only one file in your project!
	#define MICRO_PLY_IMPL
	#include "micro_ply.h"

	// This is the vertex layout we'll be converting the PLY to.
	typedef struct skg_vert_t {
		float   pos [3];
		float   norm[3];
		float   uv  [2];
		uint8_t col [4];
	} skg_vert_t;

	// Read the data from file, that's still on you, sorry :)
	void  *data;
	size_t size;
	FILE  *fp;
	if (fopen_s(&fp, filename, "rb") != 0 || fp == nullptr) {
		return false;
	}
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	data = malloc(size);
	fread (data, size, 1, fp);
	fclose(fp);

	// Parse the data using ply_read
	ply_file_t file;
	if (!ply_read(data, size, &file))
		return false;

	// Describe the way the contents of the PLY file map to our own vertex 
	// format. If the property can't be found in the file, the default value
	// will be assigned.
	float     fzero = 0;
	uint8_t   white = 255;
	ply_map_t map_verts[] = {
		{ PLY_PROP_POSITION_X,  ply_prop_decimal, sizeof(float), 0,  &fzero },
		{ PLY_PROP_POSITION_Y,  ply_prop_decimal, sizeof(float), 4,  &fzero },
		{ PLY_PROP_POSITION_Z,  ply_prop_decimal, sizeof(float), 8,  &fzero },
		{ PLY_PROP_NORMAL_X,    ply_prop_decimal, sizeof(float), 12, &fzero },
		{ PLY_PROP_NORMAL_Y,    ply_prop_decimal, sizeof(float), 16, &fzero },
		{ PLY_PROP_NORMAL_Z,    ply_prop_decimal, sizeof(float), 20, &fzero },
		{ PLY_PROP_TEXCOORD_X,  ply_prop_decimal, sizeof(float), 24, &fzero },
		{ PLY_PROP_TEXCOORD_Y,  ply_prop_decimal, sizeof(float), 28, &fzero },
		{ PLY_PROP_COLOR_R,     ply_prop_uint,    sizeof(uint8_t), 32, &white },
		{ PLY_PROP_COLOR_G,     ply_prop_uint,    sizeof(uint8_t), 33, &white },
		{ PLY_PROP_COLOR_B,     ply_prop_uint,    sizeof(uint8_t), 34, &white },
		{ PLY_PROP_COLOR_A,     ply_prop_uint,    sizeof(uint8_t), 35, &white }, };
	ply_convert(&file, PLY_ELEMENT_VERTICES, map_verts, _countof(map_verts), sizeof(skg_vert_t), (void **)out_verts, out_vert_count);

	// Properties defined as lists in the PLY format will get triangulated 
	// during conversion, so you don't need to worry about quads or n-gons in 
	// the geometry.
	uint32_t  izero = 0;
	ply_map_t map_inds[] = { { PLY_PROP_INDICES, ply_prop_uint, sizeof(uint32_t), 0, &izero } };
	ply_convert(&file, PLY_ELEMENT_FACES, map_inds, _countof(map_inds), sizeof(uint32_t), (void **)out_indices, out_ind_count);

	// You gotta free the memory manually!
	ply_free(&file);
	free(data);
*/

#pragma once

#include <stdint.h>

#define PLY_PROP_POSITION_X "x"
#define PLY_PROP_POSITION_Y "y"
#define PLY_PROP_POSITION_Z "z"
#define PLY_PROP_NORMAL_X "nx"
#define PLY_PROP_NORMAL_Y "ny"
#define PLY_PROP_NORMAL_Z "nz"
#define PLY_PROP_TEXCOORD_X "s"
#define PLY_PROP_TEXCOORD_Y "t"
#define PLY_PROP_COLOR_R "red"
#define PLY_PROP_COLOR_G "green"
#define PLY_PROP_COLOR_B "blue"
#define PLY_PROP_COLOR_A "alpha"
#define PLY_PROP_INDICES "vertex_index"
#define PLY_ELEMENT_VERTICES "vertex"
#define PLY_ELEMENT_FACES "face"

///////////////////////////////////////////

typedef enum ply_prop_ {
	ply_prop_int = 1,
	ply_prop_uint,
	ply_prop_decimal,
} ply_prop_;

typedef struct ply_prop_t {
	uint8_t  bytes;
	uint8_t  type; // follows ply_prop_
	uint8_t  list_bytes;
	uint8_t  list_type;
	uint16_t offset;
	char     name[32];
} ply_prop_t;

typedef struct ply_element_t {
	char        name[64];
	int32_t     count;
	ply_prop_t *properties; 
	int32_t     property_count; 
	void       *data;
	int32_t     data_stride; 
	void       *list_data;
} ply_element_t;

typedef struct ply_file_t {
	ply_element_t *elements;
	int32_t        count;
} ply_file_t;

typedef struct ply_map_t {
	const char *name;
	uint8_t     to_type;
	uint8_t     to_size;
	uint16_t    to_offset;
	const void *default_val;
} ply_map_t;

///////////////////////////////////////////

bool ply_read   (const void *data, size_t data_size, ply_file_t *out_file);
void ply_free   (ply_file_t *file);
void ply_convert(const ply_file_t *file, const char *element_name, const ply_map_t *to_format, int32_t to_format_count, int32_t format_stride, void **out_data, int32_t *out_count);

///////////////////////////////////////////

#ifdef MICRO_PLY_IMPL

#include <stdlib.h>
#include <string.h>

///////////////////////////////////////////

void _ply_convert(uint8_t *dest, uint8_t dest_size, uint8_t dest_type, const uint8_t *src, uint8_t src_size, uint8_t src_type) {
	if (dest_size == src_size && src_type == dest_type) memcpy(dest, src, dest_size);
	else if (src_type == ply_prop_decimal) {
		double val = src_size == 4
			? *(float  *)src
			: *(double *)src;
		if (dest_size == 4) *(float  *)dest = (float)val;
		else                *(double *)dest = val;
	} else {
		int64_t val=0;
		if (src_type == ply_prop_int) {
			switch (src_size) {
			case 1: val = *(int8_t  *)src; break;
			case 2: val = *(int16_t *)src; break;
			case 4: val = *(int32_t *)src; break;
			case 8: val = *(int64_t *)src; break;}
		} else {
			switch (src_size) {
			case 1: val = *(uint8_t  *)src; break;
			case 2: val = *(uint16_t *)src; break;
			case 4: val = *(uint32_t *)src; break;
			case 8: val = *(uint64_t *)src; break;}
		}
		if (dest_type == ply_prop_int) {
			switch (dest_size) {
			case 1: *(int8_t  *)dest = (int8_t )val; break;
			case 2: *(int16_t *)dest = (int16_t)val; break;
			case 4: *(int32_t *)dest = (int32_t)val; break;}
		} else {
			switch (dest_size) {
			case 1: *(uint8_t *)dest = (uint8_t )val; break;
			case 2: *(uint16_t*)dest = (uint16_t)val; break;
			case 4: *(uint32_t*)dest = (uint32_t)val; break;}
		}
	}
}

///////////////////////////////////////////

bool ply_read(const void *file_data, size_t data_size, ply_file_t *out_file) {
	// Support function, if string starts with
	bool (*starts_with)(const char *, const char *) = [](const char *str, const char *prefix) {
		while (*prefix) {
			if (*prefix++ != *str++)
				return false;
		}
		return true;
	};
	// Support function, gets the following whitespace separated word
	void (*get_word)(char *start, char *out, size_t out_size) = [](char *start, char *out, size_t out_size) {
		size_t count = 0;
		while (*start != ' ' && *start != '\t' && *start != '\n' && *start != '\r' && *start != '\0' && count+1 < out_size) {
			out[count] = *start++;
			count++;
		}
		out[count] = '\0';
	};
	void (*type_info)(const char *, uint8_t *, uint8_t *) = [](const char *str, uint8_t *type, uint8_t *bytes) {
		if      (strcmp(str, "char"  ) == 0) { *bytes = 1; *type = ply_prop_int;     }
		else if (strcmp(str, "uchar" ) == 0) { *bytes = 1; *type = ply_prop_uint;    }
		else if (strcmp(str, "short" ) == 0) { *bytes = 2; *type = ply_prop_int;     }
		else if (strcmp(str, "ushort") == 0) { *bytes = 2; *type = ply_prop_uint;    }
		else if (strcmp(str, "int"   ) == 0) { *bytes = 4; *type = ply_prop_int;     }
		else if (strcmp(str, "uint"  ) == 0) { *bytes = 4; *type = ply_prop_uint;    }
		else if (strcmp(str, "float" ) == 0) { *bytes = 4; *type = ply_prop_decimal; }
		else if (strcmp(str, "double") == 0) { *bytes = 8; *type = ply_prop_decimal; }
	};

	// Check file signature
	char *file = (char*)file_data;
	if (!starts_with(file, "ply"))
		return false;

	// File data
	int32_t format     = 0;
	out_file->count    = 0;
	out_file->elements = nullptr;

	// Read the header
	char *line = strchr(file, '\n');
	char  word[128];
	while(true) {
		if (!line) return false;
		line += 1;

		if        (starts_with(line, "format ")) {
			get_word(line + sizeof("format"), word, sizeof(word));
			if      (strcmp(word, "ascii"               ) == 0) format = 0;
			else if (strcmp(word, "binary_little_endian") == 0) format = 1;
			else if (strcmp(word, "binary_big_endian"   ) == 0) format = 2;
		} else if (starts_with(line, "comment ")) {
		} else if (starts_with(line, "element ")) {
			ply_element_t el = {};
			get_word(line + sizeof("element"), el.name, sizeof(el.name));
			get_word(line + sizeof("element ") + strlen(el.name), word, sizeof(word));
			el.count = atoi(word);

			out_file->count   += 1;
			out_file->elements = (ply_element_t*)realloc(out_file->elements, sizeof(ply_element_t) * (out_file->count));
			out_file->elements[out_file->count - 1] = el;
		} else if (starts_with(line, "property ")) {
			ply_prop_t     prop = {};
			get_word(line + sizeof("property"), word, sizeof(word));
			
			if (strcmp(word, "list"  ) == 0) {
				size_t off = sizeof("property ") + strlen(word);
				get_word(line + off, word, sizeof(word));
				type_info(word, &prop.type, &prop.bytes);
				off += strlen(word) + 1;
				get_word(line + off, word, sizeof(word));
				type_info(word, &prop.list_type, &prop.list_bytes);
				off += strlen(word) + 1;
				get_word(line + off, prop.name, sizeof(prop.name));
			} else { 
				type_info(word, &prop.type, &prop.bytes);
				get_word(line + sizeof("property ") + strlen(word), prop.name, sizeof(prop.name));
			}

			ply_element_t *el   = &out_file->elements[out_file->count-1];
			prop.offset = (uint16_t)el->data_stride;
			el->data_stride    += prop.bytes;
			el->property_count += 1;
			el->properties      = (ply_prop_t*)realloc(el->properties, sizeof(ply_prop_t) * el->property_count);
			el->properties[el->property_count-1] = prop;
		} else if (starts_with(line, "end_header")) {
			line = strchr(line, '\n')+1;
			break;
		}
		line = strchr(line, '\n');
	}

	// Parse the data
	for (int32_t i = 0; i < out_file->count; i++) {
		ply_element_t *el = &out_file->elements[i];
		el->data = malloc(el->data_stride*el->count);
		uint8_t *data = (uint8_t*)el->data;

		// If it's a list type
		if (el->property_count == 1 && el->properties[0].list_type != 0) {
			int32_t list_cap   = el->count * 4;
			int32_t list_count = 0;
			el->list_data = malloc(el->properties[0].list_bytes * list_cap);
			uint8_t *list_data = (uint8_t*)el->list_data;

			for (int32_t e = 0; e < el->count; e++) {
				size_t off = 0;
				get_word(line + off, word, sizeof(word));
				off += strlen(word) + 1;
				int32_t count = atoi(word);
				_ply_convert(data, el->properties[0].bytes, el->properties[0].type, (uint8_t*)&count, sizeof(double), ply_prop_int);
				for (int32_t c = 0; c < count; c++) {
					get_word(line + off, word, sizeof(word));
					off += strlen(word) + 1;
					if (el->properties[0].type == ply_prop_decimal) {
						double  val = atof(word);
						_ply_convert(list_data, el->properties[0].list_bytes, el->properties[0].list_type, (uint8_t*)&val, sizeof(double), ply_prop_decimal);
					} else if (el->properties[0].type == ply_prop_int) {
						int64_t val = atol(word);
						_ply_convert(list_data, el->properties[0].list_bytes, el->properties[0].list_type, (uint8_t*)&val, sizeof(int64_t), ply_prop_int);
					} else {
						uint64_t val = atol(word);
						_ply_convert(list_data, el->properties[0].list_bytes, el->properties[0].list_type, (uint8_t*)&val, sizeof(uint64_t), ply_prop_uint);
					}
					list_data  += el->properties[0].list_bytes;
					list_count += 1;
					if (list_count >= list_cap) {
						list_cap = (int32_t)(list_cap * 1.25f);
						el->list_data = realloc(el->list_data, el->properties[0].list_bytes * list_cap);
						list_data = ((uint8_t*)el->list_data) + (list_count * el->properties[0].list_bytes);
					}
				}
				line = strchr(line, '\n') + 1;
				data += el->properties[0].bytes;
			}
		} else {
			for (int32_t e = 0; e < el->count; e++) {
				size_t off = 0;
				for (int32_t p = 0; p < el->property_count; p++) {
					get_word(line + off, word, sizeof(word));
					off += strlen(word) + 1;
					if (el->properties[p].type == ply_prop_decimal) {
						double  val = atof(word);
						_ply_convert(data+el->properties[p].offset, el->properties[p].bytes, el->properties[p].type, (uint8_t*)&val, sizeof(double), ply_prop_decimal);
					} else if (el->properties[p].type == ply_prop_int) {
						int64_t val = atol(word);
						_ply_convert(data+el->properties[p].offset, el->properties[p].bytes, el->properties[p].type, (uint8_t*)&val, sizeof(int64_t), ply_prop_int);
					} else {
						uint64_t val = atol(word);
						_ply_convert(data+el->properties[p].offset, el->properties[p].bytes, el->properties[p].type, (uint8_t*)&val, sizeof(uint64_t), ply_prop_uint);
					}
				}
				line = strchr(line, '\n') + 1;
				data += el->data_stride;
			}
		}
	}

	return true;
}

///////////////////////////////////////////

void ply_convert(const ply_file_t *file, const char *element_name, const ply_map_t *to_format, int32_t format_count, int32_t format_stride, void **out_data, int32_t *out_count) {
	*out_data  = nullptr;
	*out_count = 0;

	// Find the elements we want to convert by name
	const ply_element_t *elements = nullptr;
	for (int32_t i = 0; i < file->count; i++) {
		if (strcmp(file->elements[i].name, element_name) == 0) {
			elements = &file->elements[i];
			break;
		}
	}
	if (elements == nullptr)
		return;

	if (elements->list_data == nullptr) {
		// Map the element properties to the out format
		int32_t *map  = (int32_t*)malloc(sizeof(int32_t) * format_count);
		for (int32_t i = 0; i < format_count; i++) {
			map[i] = -1;
			for (int32_t p = 0; p < elements->property_count; p++) {
				if (strcmp(elements->properties[p].name, to_format[i].name) == 0) {
					map[i] = p;
					break;
				}
			}
		}

		// Now convert and copy each item
		*out_data  = malloc(elements->count * format_stride);
		*out_count = elements->count;
		uint8_t *src  = (uint8_t*)elements->data;
		uint8_t *dest = (uint8_t*)*out_data;
		for (int32_t i = 0; i < elements->count; i++) {
			for (int32_t f = 0; f < format_count; f++) {
				if (map[f] == -1) {
					memcpy(dest+to_format[f].to_offset, to_format[f].default_val, to_format[f].to_size);
				} else {
					ply_prop_t *prop = &elements->properties[map[f]];
					_ply_convert(
						dest+to_format[f].to_offset, to_format[f].to_size, to_format[f].to_type,
						src+prop->offset,            prop->bytes,          prop->type);
				}
			}
			src  += elements->data_stride;
			dest += format_stride;
		}

		free(map);
	} else {
		uint8_t src_size     = elements->properties[0].bytes;
		uint8_t src_type     = elements->properties[0].type;
		uint8_t src_ind_size = elements->properties[0].list_bytes;
		uint8_t src_ind_type = elements->properties[0].list_type;
		// Count how many total indices there will be
		int32_t count = 0;
		uint8_t *src = (uint8_t *)elements->data;
		for (int32_t i = 0; i < elements->count; i++) {
			int32_t ct = 0;
			_ply_convert((uint8_t *)&ct, sizeof(int32_t), ply_prop_int, src, src_size, src_type);
			count += 3 + (ct-3)*3;
			src   += src_size;
		}

		*out_data  = malloc(count * format_stride);
		*out_count = count;
		src              = (uint8_t*)elements->data;
		uint8_t *src_ind = (uint8_t*)elements->list_data;
		uint8_t *dest    = (uint8_t*)*out_data;
		for (int32_t i = 0; i < elements->count; i++) {
			int32_t ct = 0;
			_ply_convert((uint8_t *)&ct, sizeof(int32_t), ply_prop_int, src, src_size, src_type);
			for (int32_t x = 0; x < ct-2; x++) {
				_ply_convert(
					dest+(x*3*to_format[0].to_size), to_format[0].to_size, to_format[0].to_type,
					src_ind, src_ind_size, src_ind_type);
				_ply_convert(
					dest+((x*3+1)*to_format[0].to_size), to_format[0].to_size, to_format[0].to_type,
					src_ind+((x+1)*src_ind_size), src_ind_size, src_ind_type);
				_ply_convert(
					dest+((x*3+2)*to_format[0].to_size), to_format[0].to_size, to_format[0].to_type,
					src_ind+((x+2)*src_ind_size), src_ind_size, src_ind_type);
			}
			src_ind += src_ind_size * ct;
			dest    += format_stride * (3 + (ct-3)*3);
			src     += src_size;
		}
	}
}

///////////////////////////////////////////

void ply_free(ply_file_t *file) {
	for (int32_t i = 0; i < file->count; i++) {
		free(file->elements[i].data);
		free(file->elements[i].list_data);
		free(file->elements[i].properties);
	}
	free(file->elements);
}

#endif

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Nick Klingensmith
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/