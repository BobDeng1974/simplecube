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

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include <EGL/egl.h>

#include "cube-gles2.h"

#define WIDTH 800
#define HEIGHT 800

using namespace std;

/*
 * writes RGB or RGBA data to a TGA file
 * returns true if successful
 */
bool Image_WriteTGA(const char *name, uint8_t *data, int width, int height, int bpp, bool upsidedown)
{
	FILE *handle = fopen(name, "wb");
	if (!handle)
	{
		printf("Error opening %s: %s\n", name, strerror(errno));
		return false;
	}

#define TARGAHEADERSIZE 18
	uint8_t header[TARGAHEADERSIZE];
	memset(&header, 0, TARGAHEADERSIZE);
	header[2] = 2; // uncompressed type
	header[12] = width & 255;
	header[13] = width >> 8;
	header[14] = height & 255;
	header[15] = height >> 8;
	header[16] = bpp; // pixel size
	if (upsidedown)
		header[17] = 0x20; //upside-down attribute

	// swap red and blue bytes
	int bytes = bpp / 8;
	int size = width * height * bytes;
	for (int i = 0; i < size; i += bytes)
	{
		int temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}

	fwrite(&header, 1, TARGAHEADERSIZE, handle);
	fwrite(data, 1, size, handle);
	fclose(handle);

	return true;
}

static void screenshot(int frame, GlScene &scene)
{
	// Allocate buffer
	uint8_t *buffer = new uint8_t[WIDTH * HEIGHT * 4];

	// Write scene to buffer
	scene.write_buffer(buffer, WIDTH, HEIGHT);

	// Build a file name
	char tganame[16];
	snprintf(tganame, sizeof(tganame), "frame%04i.tga", frame);

	// Write the file
	if (Image_WriteTGA(tganame, buffer, WIDTH, HEIGHT, 32, false))
		printf("Wrote %s\n", tganame);
	else
		printf("Couldn't create a TGA file\n");

	free(buffer);
}

static void print_egl_config(EGLDisplay dpy, EGLConfig cfg)
{
	auto getconf = [dpy, cfg](EGLint a) { EGLint v = -1; eglGetConfigAttrib(dpy, cfg, a, &v); return v;};

	printf("EGL Config %d: color buf %d/%d/%d/%d = %d, depth %d, stencil %d, samples %d\n",
	       getconf(EGL_CONFIG_ID),
	       getconf(EGL_ALPHA_SIZE),
	       getconf(EGL_RED_SIZE),
	       getconf(EGL_GREEN_SIZE),
	       getconf(EGL_BLUE_SIZE),
	       getconf(EGL_BUFFER_SIZE),
	       getconf(EGL_DEPTH_SIZE),
	       getconf(EGL_STENCIL_SIZE),
	       getconf(EGL_SAMPLES));
}

int main(int argc, char *argv[])
{
	// TODO: set these from command line
	bool s_verbose = 1;
	unsigned s_num_frames = 2;

	/* This is the default but set it anyway */
	if (!eglBindAPI(EGL_OPENGL_ES_API))
	{
		printf("Failed to bind API EGL_OPENGL_ES_API\n");
		return -1;
	}

	EGLDisplay m_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (!m_display)
	{
		printf("Failed to get egl display\n");
		return -1;
	}

	EGLint major, minor;
	if (!eglInitialize(m_display, &major, &minor))
	{
		printf("Failed to initialize\n");
		return -1;
	}

	if (s_verbose)
	{
		printf("Using display %p with EGL version %d.%d\n", m_display, major, minor);

		printf("EGL_VENDOR:      %s\n", eglQueryString(m_display, EGL_VENDOR));
		printf("EGL_VERSION:     %s\n", eglQueryString(m_display, EGL_VERSION));
		printf("EGL_EXTENSIONS:  %s\n", eglQueryString(m_display, EGL_EXTENSIONS));
		printf("EGL_CLIENT_APIS: %s\n", eglQueryString(m_display, EGL_CLIENT_APIS));
	}

	if (s_verbose)
	{
		EGLint numConfigs;
		if (!eglGetConfigs(m_display, nullptr, 0, &numConfigs))
		{
			printf("Failed to get number of configs");
			return -1;
		}

		EGLConfig configs[numConfigs];
		if (!eglGetConfigs(m_display, configs, numConfigs, &numConfigs))
		{
			printf("Failed to get config");
			return -1;
		}

		printf("Available configs:\n");

		for (int i = 0; i < numConfigs; ++i)
			print_egl_config(m_display, configs[i]);
	}

	EGLConfig m_config;
	EGLint n;
	static const EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
//		EGL_SAMPLE_BUFFERS, 1,
//		EGL_SAMPLES, 4,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};
	if (!eglChooseConfig(m_display, config_attribs, &m_config, 1, &n) || n != 1)
	{
		printf("Failed to choose config\n");
		return -1;
	}

	if (s_verbose)
	{
		printf("Chosen config: ");
		print_egl_config(m_display, m_config);
	}

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	EGLContext m_context = eglCreateContext(m_display, m_config, EGL_NO_CONTEXT, context_attribs);
	if (!m_context)
	{
		printf("failed to create context\n");
		return -1;
	}

	static const EGLint pbuff_attribs[] = {
		EGL_WIDTH, WIDTH,
		EGL_HEIGHT, HEIGHT,
		EGL_NONE
	};
	EGLSurface esurface = eglCreatePbufferSurface(m_display, m_config, pbuff_attribs);
	if (esurface == EGL_NO_SURFACE)
	{
		printf("Failed to create EGL surface\n");
		return -1;
	}

	if (!eglMakeCurrent(m_display, esurface, esurface, m_context))
	{
		printf("Failed to set context\n");
		return -1;
	}

	GlScene scene;
	scene.print_info();
	scene.set_viewport(WIDTH, HEIGHT);

	for (size_t framenum = 1; framenum <= s_num_frames; framenum++)
	{
		scene.draw(framenum);

		screenshot(framenum, scene);
	}

	eglMakeCurrent(m_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	eglDestroySurface(m_display, esurface);

	eglTerminate(m_display);

	return 0;
}
