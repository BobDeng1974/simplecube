/*
 * Copyright (c) 2012 Arvin Schnell <arvin.schnell@gmail.com>
 * Copyright (c) 2012 Rob Clark <rob@ti.com>
 * Copyright (c) 2015 Tomi Valkeinen <tomi.valkeinen@ti.com>
 * Copyright (c) 2018 Andrew F. Davis <afd@ti.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstdio>
#include <cstdlib>

#include <GLES2/gl2.h>

#define unlikely(x) __builtin_expect(!!(x), 0)

#define FAIL_IF(x, fmt, ...) \
	if (unlikely(x)) { \
		fprintf(stderr, "%s:%d: %s:\n" fmt "\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, ##__VA_ARGS__); \
		abort(); \
	}

class GlScene
{
public:
	GlScene();

	GlScene(const GlScene& other) = delete;
	GlScene& operator=(const GlScene& other) = delete;

	void print_info(void);

	void set_viewport(uint32_t width, uint32_t height);

	void draw(uint32_t framenum);

	void write_buffer(uint8_t *buffer, uint32_t width, uint32_t height);

private:
	GLint m_modelviewmatrix, m_modelviewprojectionmatrix, m_normalmatrix;

	uint32_t m_width;
	uint32_t m_height;
};
