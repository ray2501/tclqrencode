/* A lot of code from libqrencode qrenc.c this file, add copyright header */

/**
 * qrencode - QR Code encoder
 *
 * QR Code encoding tool
 * Copyright (C) 2006-2013 Kentaro Fukuchi <kentaro@fukuchi.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <errno.h>

#include "tqrencode.h"
#include "qrencode.h"

#define INCHES_PER_METER (100.0/2.54)

static int casesensitive = 1;
static int eightbit = 0;
static int version = 0;
static int size = 3;
static int margin = 4;
static int dpi = 72;
static int structured = 0;
static int rle = 0;
static int micro = 0;
static QRecLevel level = QR_ECLEVEL_L;
static QRencodeMode hint = QR_MODE_8;
static unsigned char fg_color[4] = {0, 0, 0, 255};
static unsigned char bg_color[4] = {255, 255, 255, 255};

enum imageType {
	PNG_TYPE,
	PNG32_TYPE,
	EPS_TYPE,
	SVG_TYPE,
	XPM_TYPE,
	ANSI_TYPE,
	ANSI256_TYPE,
	ASCII_TYPE,
	ASCIIi_TYPE,
	UTF8_TYPE,
	ANSIUTF8_TYPE,
	UTF8i_TYPE,
	ANSIUTF8i_TYPE
};

static enum imageType image_type = PNG_TYPE;

static int color_set(unsigned char color[4], const char *value)
{
	int len = strlen(value);
	int i, count;
	unsigned int col[4];
	if(len == 6) {
		count = sscanf(value, "%02x%02x%02x%n", &col[0], &col[1], &col[2], &len);
		if(count < 3 || len != 6) {
			return -1;
		}
		for(i = 0; i < 3; i++) {
			color[i] = col[i];
		}
		color[3] = 255;
	} else if(len == 8) {
		count = sscanf(value, "%02x%02x%02x%02x%n", &col[0], &col[1], &col[2], &col[3], &len);
		if(count < 4 || len != 8) {
			return -1;
		}
		for(i = 0; i < 4; i++) {
			color[i] = col[i];
		}
	} else {
		return -1;
	}
	return 0;
}


static FILE *openFile(const char *outfile)
{
	FILE *fp;

	if(outfile == NULL || (outfile[0] == '-' && outfile[1] == '\0')) {
		fp = stdout;
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create file: %s\n", outfile);
			perror(NULL);
			return NULL;
		}
	}

	return fp;
}


static void fillRow(unsigned char *row, int size, const unsigned char color[])
{
	int i;

	for(i = 0; i < size; i++) {
		memcpy(row, color, 4);
		row += 4;
	}
}


static int writePNG(const QRcode *qrcode, const char *outfile, enum imageType type)
{
	static FILE *fp; // avoid clobbering by setjmp.
	png_structp png_ptr;
	png_infop info_ptr;
	png_colorp palette = NULL;
	png_byte alpha_values[2];
	unsigned char *row, *p, *q;
	int x, y, xx, yy, bit;
	int realwidth;

	realwidth = (qrcode->width + margin * 2) * size;
	if(type == PNG_TYPE) {
		row = (unsigned char *)malloc((realwidth + 7) / 8);
	} else if(type == PNG32_TYPE) {
		row = (unsigned char *)malloc(realwidth * 4);
	} else {
		fprintf(stderr, "Internal error.\n");
		return 1;
	}
	if(row == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 1;
	}

	if(outfile[0] == '-' && outfile[1] == '\0') {
		fp = stdout;
	} else {
		fp = fopen(outfile, "wb");
		if(fp == NULL) {
			fprintf(stderr, "Failed to create file: %s\n", outfile);
			perror(NULL);
			return 1;
		}
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG writer.\n");
		return 1;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		fprintf(stderr, "Failed to initialize PNG write.\n");
		return 1;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fprintf(stderr, "Failed to write PNG image.\n");
		return 1;
	}

	if(type == PNG_TYPE) {
		palette = (png_colorp) malloc(sizeof(png_color) * 2);
		if(palette == NULL) {
			fprintf(stderr, "Failed to allocate memory.\n");
			return 1;
		}
		palette[0].red   = fg_color[0];
		palette[0].green = fg_color[1];
		palette[0].blue  = fg_color[2];
		palette[1].red   = bg_color[0];
		palette[1].green = bg_color[1];
		palette[1].blue  = bg_color[2];
		alpha_values[0] = fg_color[3];
		alpha_values[1] = bg_color[3];
		png_set_PLTE(png_ptr, info_ptr, palette, 2);
		png_set_tRNS(png_ptr, info_ptr, alpha_values, 2, NULL);
	}

	png_init_io(png_ptr, fp);
	if(type == PNG_TYPE) {
		png_set_IHDR(png_ptr, info_ptr,
				realwidth, realwidth,
				1,
				PNG_COLOR_TYPE_PALETTE,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	} else {
		png_set_IHDR(png_ptr, info_ptr,
				realwidth, realwidth,
				8,
				PNG_COLOR_TYPE_RGB_ALPHA,
				PNG_INTERLACE_NONE,
				PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	}
	png_set_pHYs(png_ptr, info_ptr,
			dpi * INCHES_PER_METER,
			dpi * INCHES_PER_METER,
			PNG_RESOLUTION_METER);
	png_write_info(png_ptr, info_ptr);

	if(type == PNG_TYPE) {
	/* top margin */
		memset(row, 0xff, (realwidth + 7) / 8);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for(y = 0; y < qrcode->width; y++) {
			memset(row, 0xff, (realwidth + 7) / 8);
			q = row;
			q += margin * size / 8;
			bit = 7 - (margin * size % 8);
			for(x = 0; x < qrcode->width; x++) {
				for(xx = 0; xx < size; xx++) {
					*q ^= (*p & 1) << bit;
					bit--;
					if(bit < 0) {
						q++;
						bit = 7;
					}
				}
				p++;
			}
			for(yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		memset(row, 0xff, (realwidth + 7) / 8);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	} else {
	/* top margin */
		fillRow(row, realwidth, bg_color);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}

		/* data */
		p = qrcode->data;
		for(y = 0; y < qrcode->width; y++) {
			fillRow(row, realwidth, bg_color);
			for(x = 0; x < qrcode->width; x++) {
				for(xx = 0; xx < size; xx++) {
					if(*p & 1) {
						memcpy(&row[((margin + x) * size + xx) * 4], fg_color, 4);
					}
				}
				p++;
			}
			for(yy = 0; yy < size; yy++) {
				png_write_row(png_ptr, row);
			}
		}
		/* bottom margin */
		fillRow(row, realwidth, bg_color);
		for(y = 0; y < margin * size; y++) {
			png_write_row(png_ptr, row);
		}
	}

	png_write_end(png_ptr, info_ptr);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(fp);
	free(row);
	free(palette);

	return 0;
}


static int writeEPS(const QRcode *qrcode, const char *outfile)
{
	FILE *fp;
	unsigned char *row, *p;
	int x, y, yy;
	int realwidth;

	fp = openFile(outfile);
	if(fp==NULL) {
		return 1;
	}

	realwidth = (qrcode->width + margin * 2) * size;
	/* EPS file header */
	fprintf(fp, "%%!PS-Adobe-2.0 EPSF-1.2\n"
				"%%%%BoundingBox: 0 0 %d %d\n"
				"%%%%Pages: 1 1\n"
				"%%%%EndComments\n", realwidth, realwidth);
	/* draw point */
	fprintf(fp, "/p { "
				"moveto "
				"0 1 rlineto "
				"1 0 rlineto "
				"0 -1 rlineto "
				"fill "
				"} bind def\n");
	/* set color */
	fprintf(fp, "gsave\n");
	fprintf(fp, "%f %f %f setrgbcolor\n",
			(float)bg_color[0] / 255,
			(float)bg_color[1] / 255,
			(float)bg_color[2] / 255);
	fprintf(fp, "%d %d scale\n", realwidth, realwidth);
	fprintf(fp, "0 0 p\ngrestore\n");
	fprintf(fp, "%f %f %f setrgbcolor\n",
			(float)fg_color[0] / 255,
			(float)fg_color[1] / 255,
			(float)fg_color[2] / 255);
	fprintf(fp, "%d %d scale\n", size, size);

	/* data */
	p = qrcode->data;
	for(y = 0; y < qrcode->width; y++) {
		row = (p+(y*qrcode->width));
		yy = (margin + qrcode->width - y - 1);

		for(x = 0; x < qrcode->width; x++) {
			if(*(row+x)&0x1) {
				fprintf(fp, "%d %d p ", margin + x,  yy);
			}
		}
	}

	fprintf(fp, "\n%%%%EOF\n");
	fclose(fp);

	return 0;
}


#ifdef _MSC_VER
int snprintf(char* str, size_t size, const char* format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = c99_vsnprintf(str, size, format, ap);
    va_end(ap);

    return count;
}

int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}
#endif


static void writeSVG_drawModules(FILE *fp, int x, int y, int width, const char* col, float opacity)
{
    if(fg_color[3] != 255) {
        fprintf(fp, "\t\t\t<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"1\" "\
                "fill=\"#%s\" fill-opacity=\"%f\"/>\n",
                x, y, width, col, opacity );
    } else {
        fprintf(fp, "\t\t\t<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"1\" "\
                "fill=\"#%s\"/>\n",
                x, y, width, col );
    }
}


static int writeSVG(const QRcode *qrcode, const char *outfile)
{
	FILE *fp;
	unsigned char *row, *p;
	int x, y, x0, pen;
	int symwidth, realwidth;
	float scale;
	char fg[7], bg[7];
	float fg_opacity;
	float bg_opacity;

	fp = openFile(outfile);
	if(fp==NULL) {
		return 1;
	}

	scale = dpi * INCHES_PER_METER / 100.0;

	symwidth = qrcode->width + margin * 2;
	realwidth = symwidth * size;

	snprintf(fg, 7, "%02x%02x%02x", fg_color[0], fg_color[1],  fg_color[2]);
	snprintf(bg, 7, "%02x%02x%02x", bg_color[0], bg_color[1],  bg_color[2]);
	fg_opacity = (float)fg_color[3] / 255;
	bg_opacity = (float)bg_color[3] / 255;

	/* XML declaration */
	fputs( "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n", fp );

	/* DTD
	   No document type specified because "while a DTD is provided in [the SVG]
	   specification, the use of DTDs for validating XML documents is known to
	   be problematic. In particular, DTDs do not handle namespaces gracefully.
	   It is *not* recommended that a DOCTYPE declaration be included in SVG
	   documents."
	   http://www.w3.org/TR/2003/REC-SVG11-20030114/intro.html#Namespace
	*/

	/* Vanity remark */
	fprintf(fp, "<!-- Created with qrencode %s (https://fukuchi.org/works/qrencode/index.html) -->\n", QRcode_APIVersionString());

	/* SVG code start */
	fprintf(fp,
			"<svg width=\"%.2fcm\" height=\"%.2fcm\" viewBox=\"0 0 %d %d\""\
			" preserveAspectRatio=\"none\" version=\"1.1\""\
			" xmlns=\"http://www.w3.org/2000/svg\">\n",
			realwidth / scale, realwidth / scale, symwidth, symwidth
		   );

	/* Make named group */
	fputs("\t<g id=\"QRcode\">\n", fp);

	/* Make solid background */
	if(bg_color[3] != 255) {
		fprintf(fp, "\t\t<rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\" fill-opacity=\"%f\"/>\n", symwidth, symwidth, bg, bg_opacity);
	} else {
		fprintf(fp, "\t\t<rect x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" fill=\"#%s\"/>\n", symwidth, symwidth, bg);
	}

    /* Create new viewbox for QR data */
    fprintf(fp, "\t\t<g id=\"Pattern\" transform=\"translate(%d,%d)\">\n", margin, margin);

	/* Write data */
	p = qrcode->data;
	for(y = 0; y < qrcode->width; y++) {
		row = (p+(y*qrcode->width));

		if( !rle ) {
			/* no RLE */
			for(x = 0; x < qrcode->width; x++) {
				if(*(row+x)&0x1) {
					writeSVG_drawModules(fp, x, y, 1, fg, fg_opacity);
				}
			}
		} else {
			/* simple RLE */
			pen = 0;
			x0  = 0;
			for(x = 0; x < qrcode->width; x++) {
				if( !pen ) {
					pen = *(row+x)&0x1;
					x0 = x;
				} else if(!(*(row+x)&0x1)) {
					writeSVG_drawModules(fp, x0, y, x-x0, fg, fg_opacity);
					pen = 0;
				}
			}
			if( pen ) {
				writeSVG_drawModules(fp, x0, y, qrcode->width - x0, fg, fg_opacity);
			}
		}
	}

    /* Close QR data viewbox */
    fputs("\t\t</g>\n", fp);

	/* Close group */
	fputs("\t</g>\n", fp);

	/* Close SVG code */
	fputs("</svg>\n", fp);
	fclose(fp);

	return 0;
}


static int writeXPM(const QRcode *qrcode, const char *outfile)
{
	FILE *fp;
	int x, xx, y, yy, realwidth, realmargin;
	char *row;
	char fg[7], bg[7];
	unsigned char *p;

	fp = openFile(outfile);
	if(fp==NULL) {
		return 1;
	}

	realwidth = (qrcode->width + margin * 2) * size;
	realmargin = margin * size;

	row = malloc(realwidth + 1);
	if (!row ) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 1;
	}

	snprintf(fg, 7, "%02x%02x%02x", fg_color[0], fg_color[1],  fg_color[2]);
	snprintf(bg, 7, "%02x%02x%02x", bg_color[0], bg_color[1],  bg_color[2]);

	fputs("/* XPM */\n", fp);
	fputs("static const char *const qrcode_xpm[] = {\n", fp);
	fputs("/* width height ncolors chars_per_pixel */\n", fp);
	fprintf(fp, "\"%d %d 2 1\",\n", realwidth, realwidth);

	fputs("/* colors */\n", fp);
	fprintf(fp, "\"F c #%s\",\n", fg);
	fprintf(fp, "\"B c #%s\",\n", bg);

	fputs("/* pixels */\n", fp);
	memset(row, 'B', realwidth);
	row[realwidth] = '\0';

	for (y = 0; y < realmargin; y++) {
		fprintf(fp, "\"%s\",\n", row);
	}

	p = qrcode->data;
	for (y = 0; y < qrcode->width; y++) {
		for (yy = 0; yy < size; yy++) {
			fputs("\"", fp);

			for (x = 0; x < margin; x++) {
				for (xx = 0; xx < size; xx++) {
					fputs("B", fp);
				}
			}

			for (x = 0; x < qrcode->width; x++) {
				for (xx = 0; xx < size; xx++) {
					if (p[(y * qrcode->width) + x] & 0x1) {
						fputs("F", fp);
					} else {
						fputs("B", fp);
					}
				}
			}

			for (x = 0; x < margin; x++) {
				for (xx = 0; xx < size; xx++) {
					fputs("B", fp);
				}
			}

			fputs("\",\n", fp);
		}
	}

	for (y = 0; y < realmargin; y++) {
		fprintf(fp, "\"%s\"%s\n", row, y < (size - 1) ? "," : "};");
	}

	free(row);
	fclose(fp);

	return 0;
}


static void writeANSI_margin(FILE* fp, int realwidth,
                             char* buffer, const char* white, int white_s )
{
	int y;

	strncpy(buffer, white, white_s);
	memset(buffer + white_s, ' ', realwidth * 2);
	strcpy(buffer + white_s + realwidth * 2, "\033[0m\n"); // reset to default colors
	for(y = 0; y < margin; y++ ){
		fputs(buffer, fp);
	}
}


static int writeANSI(const QRcode *qrcode, const char *outfile)
{
	FILE *fp;
	unsigned char *row, *p;
	int x, y;
	int realwidth;
	int last;

	const char *white, *black;
	char *buffer;
	int white_s, black_s, buffer_s;

	if(image_type == ANSI256_TYPE){
		/* codes for 256 color compatible terminals */
		white = "\033[48;5;231m";
		white_s = 11;
		black = "\033[48;5;16m";
		black_s = 10;
	} else {
		white = "\033[47m";
		white_s = 5;
		black = "\033[40m";
		black_s = 5;
	}

	size = 1;

	fp = openFile(outfile);
	if(fp==NULL) {
		return 1;
	}

	realwidth = (qrcode->width + margin * 2) * size;
	buffer_s = (realwidth * white_s) * 2;
	buffer = (char *)malloc(buffer_s);
	if(buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 1;
	}

	/* top margin */
	writeANSI_margin(fp, realwidth, buffer, white, white_s);

	/* data */
	p = qrcode->data;
	for(y = 0; y < qrcode->width; y++) {
		row = (p+(y*qrcode->width));

		memset(buffer, 0, buffer_s);
		strncpy(buffer, white, white_s);
		for(x = 0; x < margin; x++ ){
			strncat(buffer, "  ", 2);
		}
		last = 0;

		for(x = 0; x < qrcode->width; x++) {
			if(*(row+x)&0x1) {
				if( last != 1 ){
					strncat(buffer, black, black_s);
					last = 1;
				}
			} else if( last != 0 ){
				strncat(buffer, white, white_s);
				last = 0;
			}
			strncat(buffer, "  ", 2);
		}

		if( last != 0 ){
			strncat(buffer, white, white_s);
		}
		for(x = 0; x < margin; x++ ){
			strncat(buffer, "  ", 2);
		}
		strncat(buffer, "\033[0m\n", 5);
		fputs(buffer, fp);
	}

	/* bottom margin */
	writeANSI_margin(fp, realwidth, buffer, white, white_s);

	fclose(fp);
	free(buffer);

	return 0;
}


static void writeUTF8_margin(FILE* fp, int realwidth, const char* white,
                             const char *reset, const char* full)
{
	int x, y;

	for (y = 0; y < margin/2; y++) {
		fputs(white, fp);
		for (x = 0; x < realwidth; x++)
			fputs(full, fp);
		fputs(reset, fp);
		fputc('\n', fp);
	}
}


static int writeUTF8(const QRcode *qrcode, const char *outfile, int use_ansi, int invert)
{
	FILE *fp;
	int x, y;
	int realwidth;
	const char *white, *reset;
	const char *empty, *lowhalf, *uphalf, *full;

	empty = " ";
	lowhalf = "\342\226\204";
	uphalf = "\342\226\200";
	full = "\342\226\210";

	if (invert) {
		const char *tmp;

		tmp = empty;
		empty = full;
		full = tmp;

		tmp = lowhalf;
		lowhalf = uphalf;
		uphalf = tmp;
	}

	if (use_ansi){
		white = "\033[40;37;1m";
		reset = "\033[0m";
	} else {
		white = "";
		reset = "";
	}

	fp = openFile(outfile);
	if(fp==NULL) {
		return 1;
	}

	realwidth = (qrcode->width + margin * 2);

	/* top margin */
	writeUTF8_margin(fp, realwidth, white, reset, full);

	/* data */
	for(y = 0; y < qrcode->width; y += 2) {
		unsigned char *row1, *row2;
		row1 = qrcode->data + y*qrcode->width;
		row2 = row1 + qrcode->width;

		fputs(white, fp);

		for (x = 0; x < margin; x++) {
			fputs(full, fp);
		}

		for (x = 0; x < qrcode->width; x++) {
			if(row1[x] & 1) {
				if(y < qrcode->width - 1 && row2[x] & 1) {
					fputs(empty, fp);
				} else {
					fputs(lowhalf, fp);
				}
			} else if(y < qrcode->width - 1 && row2[x] & 1) {
				fputs(uphalf, fp);
			} else {
				fputs(full, fp);
			}
		}

		for (x = 0; x < margin; x++)
			fputs(full, fp);

		fputs(reset, fp);
		fputc('\n', fp);
	}

	/* bottom margin */
	writeUTF8_margin(fp, realwidth, white, reset, full);

	fclose(fp);

	return 0;
}


static void writeASCII_margin(FILE* fp, int realwidth, char* buffer, int invert)
{
	int y, h;

	h = margin;

	memset(buffer, (invert?'#':' '), realwidth);
	buffer[realwidth] = '\n';
	buffer[realwidth + 1] = '\0';
	for(y = 0; y < h; y++ ){
		fputs(buffer, fp);
	}
}


static int writeASCII(const QRcode *qrcode, const char *outfile, int invert)
{
	FILE *fp;
	unsigned char *row;
	int x, y;
	int realwidth;
	char *buffer, *p;
	int buffer_s;
	char black = '#';
	char white = ' ';

	if(invert) {
		black = ' ';
		white = '#';
	}

	size = 1;

	fp = openFile(outfile);
	if(fp==NULL) {
		return 1;
	}

	realwidth = (qrcode->width + margin * 2) * 2;
	buffer_s = realwidth + 2;
	buffer = (char *)malloc( buffer_s );
	if(buffer == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 1;
	}

	/* top margin */
	writeASCII_margin(fp, realwidth, buffer, invert);

	/* data */
	for(y = 0; y < qrcode->width; y++) {
		row = qrcode->data+(y*qrcode->width);
		p = buffer;

		memset(p, white, margin * 2);
		p += margin * 2;

		for(x = 0; x < qrcode->width; x++) {
			if(row[x]&0x1) {
				*p++ = black;
				*p++ = black;
			} else {
				*p++ = white;
				*p++ = white;
			}
		}

		memset(p, white, margin * 2);
		p += margin * 2;
		*p++ = '\n';
		*p++ = '\0';
		fputs( buffer, fp );
	}

	/* bottom margin */
	writeASCII_margin(fp, realwidth, buffer, invert);

	fclose(fp);
	free(buffer);

	return 0;
}


static QRcode *encode(const unsigned char *intext, int length)
{
	QRcode *code;

	if(micro) {
		if(eightbit) {
			code = QRcode_encodeDataMQR(length, intext, version, level);
		} else {
			code = QRcode_encodeStringMQR((char *)intext, version, level, hint, casesensitive);
		}
	} else if(eightbit) {
		code = QRcode_encodeData(length, intext, version, level);
	} else {
		code = QRcode_encodeString((char *)intext, version, level, hint, casesensitive);
	}

	return code;
}


static int qrencode(const unsigned char *intext, int length, const char *outfile)
{
	QRcode *qrcode;

	qrcode = encode(intext, length);
	if(qrcode == NULL) {
		if(errno == ERANGE) {
			fprintf(stderr, "Failed to encode the input data: Input data too large\n");
		} else {
			perror("Failed to encode the input data");
		}
		return 1;
	}

	switch(image_type) {
		case PNG_TYPE:
		case PNG32_TYPE:
			writePNG(qrcode, outfile, image_type);
			break;
		case EPS_TYPE:
			writeEPS(qrcode, outfile);
			break;
		case SVG_TYPE:
			writeSVG(qrcode, outfile);
			break;
		case XPM_TYPE:
			writeXPM(qrcode, outfile);
			break;
		case ANSI_TYPE:
		case ANSI256_TYPE:
			writeANSI(qrcode, outfile);
			break;
		case ASCIIi_TYPE:
			writeASCII(qrcode, outfile,  1);
			break;
		case ASCII_TYPE:
			writeASCII(qrcode, outfile,  0);
			break;
		case UTF8_TYPE:
			writeUTF8(qrcode, outfile, 0, 0);
			break;
		case ANSIUTF8_TYPE:
			writeUTF8(qrcode, outfile, 1, 0);
			break;
		case UTF8i_TYPE:
			writeUTF8(qrcode, outfile, 0, 1);
			break;
		case ANSIUTF8i_TYPE:
			writeUTF8(qrcode, outfile, 1, 1);
			break;
		default:
			fprintf(stderr, "Unknown image type.\n");
			return 1;
	}

	QRcode_free(qrcode);
	return 0;
}


static QRcode_List *encodeStructured(const unsigned char *intext, int length)
{
	QRcode_List *list;

	if(eightbit) {
		list = QRcode_encodeDataStructured(length, intext, version, level);
	} else {
		list = QRcode_encodeStringStructured((char *)intext, version, level, hint, casesensitive);
	}

	return list;
}

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp  strnicmp
#endif


static int qrencodeStructured(const unsigned char *intext, int length, const char *outfile)
{
	QRcode_List *qrlist, *p;
	char filename[FILENAME_MAX];
	char *base, *q, *suffix = NULL;
	const char *type_suffix;
	int i = 1;
	size_t suffix_size;

	switch(image_type) {
		case PNG_TYPE:
		case PNG32_TYPE:
			type_suffix = ".png";
			break;
		case EPS_TYPE:
			type_suffix = ".eps";
			break;
		case SVG_TYPE:
			type_suffix = ".svg";
			break;
		case XPM_TYPE:
			type_suffix = ".xpm";
			break;
		case ANSI_TYPE:
		case ANSI256_TYPE:
		case ASCII_TYPE:
		case UTF8_TYPE:
		case ANSIUTF8_TYPE:
		case UTF8i_TYPE:
		case ANSIUTF8i_TYPE:
			type_suffix = ".txt";
			break;
		default:
			fprintf(stderr, "Unknown image type.\n");
			return 1;
	}

	if(outfile == NULL) {
		fprintf(stderr, "An output filename must be specified to store the structured images.\n");
		return 1;
	}
	base = strdup(outfile);
	if(base == NULL) {
		fprintf(stderr, "Failed to allocate memory.\n");
		return 1;
	}
	suffix_size = strlen(type_suffix);
	if(strlen(base) > suffix_size) {
		q = base + strlen(base) - suffix_size;
		if(strcasecmp(type_suffix, q) == 0) {
			suffix = strdup(q);
			*q = '\0';
		}
	}

	qrlist = encodeStructured(intext, length);
	if(qrlist == NULL) {
		if(errno == ERANGE) {
			fprintf(stderr, "Failed to encode the input data: Input data too large\n");
		} else {
			perror("Failed to encode the input data");
		}
		return 1;
	}

	for(p = qrlist; p != NULL; p = p->next) {
		if(p->code == NULL) {
			fprintf(stderr, "Failed to encode the input data.\n");
			return 1;
		}
		if(suffix) {
			snprintf(filename, FILENAME_MAX, "%s-%02d%s", base, i, suffix);
		} else {
			snprintf(filename, FILENAME_MAX, "%s-%02d", base, i);
		}

		switch(image_type) {
			case PNG_TYPE:
			case PNG32_TYPE:
				writePNG(p->code, filename, image_type);
				break;
			case EPS_TYPE:
				writeEPS(p->code, filename);
				break;
			case SVG_TYPE:
				writeSVG(p->code, filename);
				break;
			case XPM_TYPE:
				writeXPM(p->code, filename);
				break;
			case ANSI_TYPE:
			case ANSI256_TYPE:
				writeANSI(p->code, filename);
				break;
			case ASCIIi_TYPE:
				writeASCII(p->code, filename, 1);
				break;
			case ASCII_TYPE:
				writeASCII(p->code, filename, 0);
				break;
			case UTF8_TYPE:
				writeUTF8(p->code, filename, 0, 0);
				break;
			case ANSIUTF8_TYPE:
				writeUTF8(p->code, filename, 0, 0);
				break;
			case UTF8i_TYPE:
				writeUTF8(p->code, filename, 0, 1);
				break;
			case ANSIUTF8i_TYPE:
				writeUTF8(p->code, filename, 0, 1);
				break;

			default:
				fprintf(stderr, "Unknown image type.\n");
				return 1;
		}
		i++;
	}

	free(base);
	if(suffix) {
		free(suffix);
	}

	QRcode_List_free(qrlist);
	return 0;
}


int SETMODE_8BIT (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_eightbit;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "setup_8bit_mode");
        return TCL_ERROR;
    }    
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_eightbit) != TCL_OK) {
        return TCL_ERROR;
    }
    
    if(m_eightbit > 0)
      eightbit = 1;
    else
      eightbit = 0;
    
    
    return TCL_OK;    
}


int SETCASESENSITIVE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_casesensitive;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "casesensitive");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_casesensitive) != TCL_OK) {
        return TCL_ERROR;
    }
    
    if(m_casesensitive > 0)
      m_casesensitive = 1;
    else
      m_casesensitive = 0;
    
    casesensitive = m_casesensitive;

    return TCL_OK;    
}


int SETKANJI (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_hint;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "setup_kanji_mode");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_hint) != TCL_OK) {
        return TCL_ERROR;
    }
    
    if(m_hint > 0)
      hint = QR_MODE_KANJI;
    else
      hint = QR_MODE_8;
    
    return TCL_OK;    
}


int SETMICRO (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_micro;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "micro");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_micro) != TCL_OK) {
        return TCL_ERROR;
    }
    
    if(m_micro > 0)
      micro = 1;
    else
      micro = 0;
    
    micro = m_micro;
    
    return TCL_OK;    
}


int SETDPI (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_dpi;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "dpi");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_dpi) != TCL_OK) {
        return TCL_ERROR;
    }
    
    if(m_dpi <= 0)
        dpi = 720;
    else          
        dpi = m_dpi;
    
    return TCL_OK;    
}


int SETLEVEL (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_level;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "level");
        return TCL_ERROR;
    }    
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_level) != TCL_OK) {
        return TCL_ERROR;
    }
    
    if(m_level >= 0 && m_level <= 3)
        level = m_level;
    else
        level = 0;
    
    
    return TCL_OK;    
}


int SETSIZE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_size;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "size");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_size) != TCL_OK) {
        return TCL_ERROR;
    }

    if(m_size <= 0 ) {
        return TCL_ERROR;
    } 
        
    size = m_size;
    
    
    return TCL_OK;   
}



int SETSTRUCTURED (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_structured;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "structured");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_structured) != TCL_OK) {
        return TCL_ERROR;
    }

    if(m_structured > 0 ) {
        structured = 1;
    } else {
        structured = 0;      
    }    
    
    return TCL_OK;   
}


int SETFILETYPE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    char *filetype;
    Tcl_Size len;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "filetype");
        return TCL_ERROR;
    }    
    
    filetype = Tcl_GetStringFromObj(obj[1], &len);
    if(!filetype || len < 1) {
        return TCL_ERROR;
    }
        
    if(strcasecmp(filetype, "png") == 0) {
        image_type = PNG_TYPE;
    } else if(strcasecmp(filetype, "png32") == 0) {
        image_type = PNG32_TYPE;
    } else if(strcasecmp(filetype, "eps") == 0) {
        image_type = EPS_TYPE;
    } else if(strcasecmp(filetype, "svg") == 0) {
        image_type = SVG_TYPE;
    } else if(strcasecmp(filetype, "xpm") == 0) {
        image_type = XPM_TYPE;
    } else if(strcasecmp(filetype, "ansi") == 0) {
        image_type = ANSI_TYPE;
    } else if(strcasecmp(filetype, "ansi256") == 0) {
        image_type = ANSI256_TYPE;
    } else if(strcasecmp(filetype, "asciii") == 0) {
        image_type = ASCIIi_TYPE;
    } else if(strcasecmp(filetype, "ascii") == 0) {
        image_type = ASCII_TYPE;
    } else if(strcasecmp(filetype, "utf8") == 0) {
        image_type = UTF8_TYPE;
    } else if(strcasecmp(filetype, "ansiutf8") == 0) {
        image_type = ANSIUTF8_TYPE;
    } else {
        image_type = PNG_TYPE;
    }

    return TCL_OK;   
}


int SETVERSION (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    int m_version;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "version");
        return TCL_ERROR;
    }
    
    if(Tcl_GetIntFromObj(interp, obj[1], &m_version) != TCL_OK) {
        return TCL_ERROR;
    }
        
    if(!micro) {
        if(m_version < 0 && m_version > QRSPEC_VERSION_MAX)
            return TCL_ERROR;
    } else {
        if(m_version < 0 && m_version > MQRSPEC_VERSION_MAX)
            return TCL_ERROR;      
    }
        
    version = m_version;
    
    
    return TCL_OK;    
}



int SETFOREGROUND (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    const char *color = NULL;
    Tcl_Size len;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "foreground");
        return TCL_ERROR;
    }
    
    color = Tcl_GetStringFromObj(obj[1], &len);
    if(!color || len < 1) {
        return TCL_ERROR;
    }
        
    if(color_set(fg_color, color)) {
        return TCL_ERROR;
    }	
    
    return TCL_OK;    
}



int SETBACKGROUND (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    char *color;
    Tcl_Size len;
    
    if(objc != 2)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "background");
        return TCL_ERROR;
    }    
    
    color = Tcl_GetStringFromObj(obj[1], &len);
    if(!color || len < 1) {
        return TCL_ERROR;
    }
        
    if(color_set(bg_color, color)) {
        return TCL_ERROR;
    }	
    
    return TCL_OK;    
}



int QRENCODE (ClientData clientData, Tcl_Interp *interp, int objc, Tcl_Obj *const obj[])
{
    unsigned char *intext = NULL;  
    Tcl_Size len = 0;
    char *outfile = NULL;
    int length = 0;
    int result = 0;
    TCL_DECLARE_MUTEX(myMutex);
    
    if(objc != 3)
    {
        Tcl_WrongNumArgs(interp, 1, obj, "string filename");
        return TCL_ERROR;
    }  

    intext = (unsigned char *) Tcl_GetStringFromObj(obj[1], &len);
    if(!intext || len < 1) {
        return TCL_ERROR;
    }
    length = strlen((char *)intext);
    
    outfile = Tcl_GetStringFromObj(obj[2], &len);
    if(!outfile || len < 1) {
        return TCL_ERROR;
    }
    
    if(micro && version > MQRSPEC_VERSION_MAX) {
        return TCL_ERROR;
    } else if(!micro && version > QRSPEC_VERSION_MAX) {
        return TCL_ERROR;
    }    
    
    if(micro) {
        margin = 2;
    } else {
        margin = 4;
    }
    
    if(micro) {
        // Version must be specified to encode a Micro QR Code symbol
        if(version == 0) {
	  return TCL_ERROR;
	}
	
        // Micro QR Code does not support structured symbols
        if(structured) {
	  return TCL_ERROR;
	}	
    }
    
    Tcl_MutexLock(&myMutex);
    if(structured)
        result = qrencodeStructured(intext, length, outfile);
    else {  
        result = qrencode(intext, length, outfile);
    }
    Tcl_MutexUnlock(&myMutex);

    if(result > 0) {
       return TCL_ERROR;
    }

    return TCL_OK;  
}
