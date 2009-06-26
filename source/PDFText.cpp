/*
 * Copyright 2001-2009, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Philippe Houdoin
 *		Simon Gauvin
 *		Michael Pfeiffer
 */

#include <stdio.h>
#include <string.h>
#include <math.h>

#include <Debug.h>
#include <StorageKit.h>
#include <TranslationKit.h>
#include <support/UTF8.h>

#include "PDFWriter.h"
#include "Link.h"
#include "Bookmark.h"
#include "DrawShape.h"
#include "XReferences.h"
#include "Log.h"
#include "Report.h"
#include "pdflib.h"


typedef struct {
	uint16 from;
	uint16 to;
	int16  length;
	uint16 *unicodes;
} unicode_to_encoding;

typedef struct {
	uint16 unicode;
	uint16 cid;
} unicode_to_cid;

typedef struct {
	uint16         length;
	unicode_to_cid *table;
} cid_table;

#ifdef UNICODE5_FROM
#	error check code!
#endif

#define ELEMS(v, e) sizeof(v) / sizeof(e)

// Adobe Glyph List
#include "enc_range.h"
#include "unicode0.h"
#include "unicode1.h"
#include "unicode2.h"
#include "unicode3.h"
#include "unicode4.h"


static unicode_to_encoding encodings[] = {
	{UNICODE0_FROM, UNICODE0_TO, ELEMS(unicode0, uint16), unicode0},
	{UNICODE1_FROM, UNICODE1_TO, ELEMS(unicode1, uint16), unicode1},
	{UNICODE2_FROM, UNICODE2_TO, ELEMS(unicode2, uint16), unicode2},
	{UNICODE3_FROM, UNICODE3_TO, ELEMS(unicode3, uint16), unicode3},
	{UNICODE4_FROM, UNICODE4_TO, ELEMS(unicode4, uint16), unicode4}
};

// unicode to cid
#include "japanese.h"
#include "gb1.h"
#include "cns1.h"
#include "korean.h"


static cid_table cid_tables[] = {
	{ELEMS(japanese, unicode_to_cid), japanese},
	{ELEMS(CNS1,     unicode_to_cid), CNS1},
	{ELEMS(GB1,      unicode_to_cid), GB1},
	{ELEMS(korean,   unicode_to_cid), korean}
};

static const char* encoding_names[] = {
	"macroman",
	// TrueType
	"ttenc0",
	"ttenc1",
	"ttenc2",
	"ttenc3",
	"ttenc4",
	// Type 1
	"t1enc0",
	"t1enc1",
	"t1enc2",
	"t1enc3",
	"t1enc4",
	// CJK
	"UniJIS-UCS2-H",
	"UniCNS-UCS2-H",
	"UniGB-UCS2-H",
	"UniKS-UCS2-H"
};


//	#pragma mark -


static bool
find_encoding(uint16 unicode, uint8 &encoding, uint16 &index)
{
	for (unsigned int i = 0; i < ELEMS(encodings, unicode_to_encoding); i++) {
		if (encodings[i].from <= unicode && unicode <= encodings[i].to) {
			int16 bottom = 0;
			int16 top = encodings[i].length-1;
			uint16* codes = encodings[i].unicodes;
			while (top >= bottom) {
			    int16 m = (top + bottom) / 2;
				if (unicode < codes[m]) {
					top = m-1;
				} else if (unicode > codes[m]) {
					bottom = m+1;
				} else {
					index = m;
					encoding = i;
					return true;
				}
			}
		    return false;
		}
	}
	return false;
}


static bool
find_in_cid_tables(uint16 unicode, font_encoding &encoding, uint16 &index,
	font_encoding* order)
{
	for (unsigned int i = 0; i < ELEMS(cid_tables, cid_table); i++) {
		encoding = order[i];
		if (encoding == invalid_encoding) break;
		int index = encoding - first_cjk_encoding;
		int32 bottom = 0;
		int32 top = cid_tables[index].length-1;
		unicode_to_cid *table = cid_tables[index].table;
		while (top >= bottom) {
		    int32 m = (top + bottom) / 2;
			if (unicode < table[m].unicode) {
				top = m-1;
			} else if (unicode > table[m].unicode) {
				bottom = m+1;
			} else {
				index = table[m].cid;
				return true;
			}
		}
	}
	return false;
}


void
PDFWriter::MakeUserDefinedEncoding(uint16 unicode, uint8 &enc, uint8 &index)
{
	if (fUserDefinedEncodings.Get(unicode, enc, index)) {
		BString s("user");
		s << (int)enc;
		PDF_encoding_set_char(fPdf, s.String(), (int)index, NULL, (int)unicode);
	}
}


void
PDFWriter::RecordFont(const char* family, const char* style, float size)
{
	const int32 n = fUsedFonts.CountItems();
	for (int32 i = 0; i < n; i ++) {
		if (fUsedFonts.ItemAt(i)->Equals(family, style, size)) return;
	}

	UsedFont* font;
	font = new UsedFont(family, style, size);
	fUsedFonts.AddItem(font);

	REPORT(kInfo, -1, "Used font: \"%s\" \"%s\" %f", family, style, size);
}


void
PDFWriter::GetFontName(BFont *font, char *fontname)
{
	font_family family;
	font_style  style;

	font->GetFamilyAndStyle(&family, &style);
	strcat(strcat(strcpy(fontname, family), "-"), style);

	RecordFont(family, style, font->Size());
}


void
PDFWriter::GetFontName(BFont *font, char *fontname, bool &embed,
	font_encoding encoding)
{
	GetFontName(font, fontname);

	switch (encoding) {
		case japanese_encoding:
			strcpy(fontname, "HeiseiMin-W3"); return;
		case chinese_cns1_encoding:
			strcpy(fontname, "MHei-Medium"); return;
		case chinese_gb1_encoding:
			strcpy(fontname, "STSong-Light"); return;
		case korean_encoding:
			strcpy(fontname, "HYGoThic-Medium"); return;
		default:;
	}
}


int
PDFWriter::FindFont(char* fontName, bool embed, font_encoding encoding)
{
	static Font* cache = NULL;
	if (cache && cache->encoding == encoding
		&& strcmp(cache->name.String(), fontName) == 0)
		return cache->font;

	REPORT(kDebug, fPage, "FindFont %s", fontName);
	Font *f = NULL;
	const int n = fFontCache.CountItems();
	for (int i = 0; i < n; i++) {
		f = fFontCache.ItemAt(i);
		if (f->encoding == encoding && strcmp(f->name.String(), fontName) == 0) {
			cache = f;
			return f->font;
		}
	}

	if (embed) embed = EmbedFont(fontName);

	BString s;
	const char* encoding_name;
	if (encoding < user_defined_encoding_start) {
		encoding_name = encoding_names[encoding];
	} else {
		s = "user";
		s << (int)(encoding - user_defined_encoding_start);
		encoding_name = s.String();
	}
	REPORT(kDebug, fPage, "Create new font, %sembed, encoding %s",
		embed ? "" : "do not ", encoding_name);
	int font = PDF_findfont(fPdf, fontName, encoding_name, embed);
	if (font != -1) {
		REPORT(kDebug, fPage, "font created");
		cache = new Font(fontName, font, encoding);
		fFontCache.AddItem(cache);
	} else {
		REPORT(kError, fPage, "Could not create font '%s': %s", fontName,
			PDF_get_errmsg(fPdf));
	}
	return font;
}


void
PDFWriter::ToUtf8(uint32 encoding, const char *string, BString &utf8)
{
	int32 len = strlen(string);
	int32 srcLen = len, destLen = 255;
	int32 state = 0;
	char buffer[256];
	int32 srcStart = 0;

	do {
		convert_to_utf8(encoding, &string[srcStart], &srcLen, buffer, &destLen,
			&state);
		srcStart += srcLen;
		len -= srcLen;
		srcLen = len;

		utf8.Append(buffer, destLen);
		destLen = 255;
	} while (len > 0);
};


void
PDFWriter::ToUnicode(const char *string, BString &unicode)
{
	int32 len = strlen(string);
	int32 srcLen = len, destLen = 255;
	int32 state = 0;
	char buffer[256];
	int32 srcStart = 0;
	int i = 0;

	unicode = "";
	if (len == 0) return;

	do {
		convert_from_utf8(B_UNICODE_CONVERSION, &string[srcStart], &srcLen,
			buffer, &destLen, &state);
		srcStart += srcLen;
		len -= srcLen;
		srcLen = len;

		char *b = unicode.LockBuffer(i + destLen);
		memcpy(&b[i], buffer, destLen);
		unicode.UnlockBuffer(i + destLen);
		i += destLen;
		destLen = 255;
	} while (len > 0);
}


void
PDFWriter::ToPDFUnicode(const char *string, BString &unicode)
{
	// PDFlib requires BOM at begin and two 0 at end of string
	char marker[3] = { 0xfe, 0xff, 0}; // byte order marker
	BString s;
	ToUnicode(string, s);
	unicode << marker;
	int32 len = s.Length()+2;
	char* buf = unicode.LockBuffer(len + 2);
		// reserve space for two additional '\0'
	memcpy(&buf[2], s.String(), s.Length());
	buf[len] = buf[len+1] = 0;
	unicode.UnlockBuffer(len + 2);
}


uint16
PDFWriter::CodePointSize(const char* s)
{
	uint16 i = 1;
	for (s++; !BeginsChar(*s); s++) i++;
	return i;
}


void
PDFWriter::RecordDests(const char* s)
{
	::RecordDests record(fXRefDests, &fTextLine, fPage);
	fXRefs->Matches(s, &record, true);
}


void
PDFWriter::DrawChar(uint16 unicode, const char* utf8, int16 size)
{
	// try to convert from utf8 to MacRoman encoding schema...
	int32 srcLen  = size;
	int32 destLen = 1;
	char dest[3] = "\0\0";
	int32 state = 0;
	bool embed = true;
	font_encoding encoding = macroman_encoding;
	char fontName[B_FONT_FAMILY_LENGTH+B_FONT_STYLE_LENGTH+1];

	if (convert_from_utf8(B_MAC_ROMAN_CONVERSION, utf8, &srcLen, dest, &destLen,
			&state, 0) != B_OK || dest[0] == 0) {
		// could not convert to MacRoman
		font_encoding fenc;
		uint16 index = 0;
		uint8 enc;

		GetFontName(&fState->beFont, fontName);
		embed = EmbedFont(fontName);

		REPORT(kDebug, -1, "find_encoding unicode %d\n", (int)unicode);
		if (find_encoding(unicode, enc, index)) {
			// is code point in the Adobe Glyph List?
			// Note if rendering the glyphs only would be desired, we could
			// always use the second method below (MakeUserDefinedEncoding),
			// but extracting text from the generated PDF would be almost
			// impossible (OCR!)
			REPORT(kDebug, -1, "encoding for %x -> %d %d", unicode, (int)enc,
				(int)index);
			// use one of the user pre-defined encodings
			if (fState->beFont.FileFormat() == B_TRUETYPE_WINDOWS) {
				encoding = font_encoding(enc + tt_encoding0);
			} else {
				encoding = font_encoding(enc + t1_encoding0);
			}
			*dest = index;
		} else if (embed) {
			// if the font is embedded, create a user defined encoding at runtime
			uint8 index;
			MakeUserDefinedEncoding(unicode, enc, index);
			*dest = index;
			encoding = font_encoding(user_defined_encoding_start + enc);
		} else if (find_in_cid_tables(unicode, fenc, index, fFontSearchOrder)) {
			// font is not embedded use one of the CJK fonts for substitution
			REPORT(kDebug, -1, "cid table %d index = %d", (int)fenc, (int)index);
			dest[0] = unicode / 256;
			dest[1] = unicode % 256;
			destLen = 2;
			encoding = fenc;
			embed = false;
		} else {
			static bool found = false;
			REPORT(kDebug, -1, "encoding for %x not found!", (int)unicode);
			if (!found) {
				found = true;
				REPORT(kError, fPage, "Could not find an encoding for character "
					"with unicode %d! Message is not repeated for other unicode "
					"values.", (int)unicode);
			}
			*dest = 0; // paint a box (is 0 a box in MacRoman) or
			return; // simply skip character
		}
	} else {
		REPORT(kDebug, -1, "macroman srcLen=%d destLen=%d dest= %d %d!", srcLen,
			destLen, (int)dest[0], (int)dest[1]);
	}

	// Note we have to build the user defined encoding before it is used in
	// PDF_find_font!
	if (!MakesPDF()) return;

	int		font;

	GetFontName(&fState->beFont, fontName, embed, encoding);
	font = FindFont(fontName, embed, encoding);
	if (font < 0) {
		REPORT(kWarning, fPage, "**** PDF_findfont(%s) failed, back to default "
			"font", fontName);
		font = PDF_findfont(fPdf, "Helvetica", "macroman", 0);
	}

	fState->font = font;

	uint16 face = fState->beFont.Face();
	PDF_set_parameter(fPdf, "underline", (face & B_UNDERSCORE_FACE) != 0
		? "true" : "false");
	PDF_set_parameter(fPdf, "strikeout", (face & B_STRIKEOUT_FACE) != 0
		? "true" : "false");
	PDF_set_value(fPdf, "textrendering", (face & B_OUTLINED_FACE) != 0 ? 1 : 0);

	PDF_setfont(fPdf, fState->font, scale(fState->beFont.Size()));

	const float x = tx(fState->penX);
	const float y = ty(fState->penY);
	const float rotation = fState->beFont.Rotation();
	const bool rotate = rotation != 0.0;

	if (rotate) {
		PDF_save(fPdf);
		PDF_translate(fPdf, x, y);
		PDF_rotate(fPdf, rotation);
	    PDF_set_text_pos(fPdf, 0, 0);
	} else
	    PDF_set_text_pos(fPdf, x, y);

	PDF_show2(fPdf, dest, destLen);

	if (rotate) {
		PDF_restore(fPdf);
	}
}


void
PDFWriter::ClipChar(BFont* font, const char* unicode, const char* utf8,
	int16 size, float width)
{
	BShape glyph;
	bool hasGlyph[1];
	font->GetHasGlyphs(utf8, 1, hasGlyph);
	if (hasGlyph[0]) {
		BShape *glyphs[1];
		glyphs[0] = &glyph;
		font->GetGlyphShapes(utf8, 1, glyphs);
	} else {
		REPORT(kWarning, fPage, "glyph for %*.*s not found!", size, size, utf8);
		// create a rectangle instead
		font_height height;
		fState->beFont.GetHeight(&height);
		BRect r(0, 0, width, height.ascent);
		float w = r.Width() < r.Height() ? r.Width()*0.1 : r.Height()*0.1;
		BRect o = r; o.InsetBy(w, w);
		w *= 2.0;
		BRect i = r; i.InsetBy(w, w);

		o.OffsetBy(0, -height.ascent);
		i.OffsetBy(0, -height.ascent);

		glyph.MoveTo(BPoint(o.left,  o.top));
		glyph.LineTo(BPoint(o.right, o.top));
		glyph.LineTo(BPoint(o.right, o.bottom));
		glyph.LineTo(BPoint(o.left,  o.bottom));
		glyph.Close();

		glyph.MoveTo(BPoint(i.left,  i.top));
		glyph.LineTo(BPoint(i.left,  i.bottom));
		glyph.LineTo(BPoint(i.right, i.bottom));
		glyph.LineTo(BPoint(i.right, i.top));
		glyph.Close();
	}

	BPoint p(fState->penX, fState->penY);
	PushInternalState(); SetOrigin(p);
	{
		DrawShape iterator(this, false);
		iterator.Iterate(&glyph);
	}
	PopInternalState();
}


void
PDFWriter::DrawString(char *string, float escapementNoSpace,
	float escapementSpace)
{
	REPORT(kDebug, fPage, "DrawString string=\"%s\", escapementNoSpace=%f, "
		"escapementSpace=%f, at %f, %f", string, escapementNoSpace,
		escapementSpace, fState->penX, fState->penY);

	if (IsDrawing()) {
		// text color is always the high color and not the pattern!
		SetColor(fState->foregroundColor);
	}
	// convert string to UTF8
	BString utf8;
	if (fState->beFont.Encoding() == B_UNICODE_UTF8) {
		utf8 = string;
	} else {
		ToUtf8(fState->beFont.Encoding()-1, string, utf8);
	}

	// convert string in UTF8 to unicode UCS2
	BString unicode;
	ToUnicode(utf8.String(), unicode);
	// need font object to calculate width of utf8 code point
	BFont font = fState->beFont;
	font.SetEncoding(B_UNICODE_UTF8);
	// constants to calculate position of next character
	const double rotation = DEGREE2RAD(fState->beFont.Rotation());
	const bool rotate = rotation != 0.0;
	const double cos1 = rotate ? cos(rotation) : 1;
	const double sin1 = rotate ? -sin(rotation) : 0;

	BPoint start(fState->penX, fState->penY);

	BeginTransparency();
	// If !MakesPDF() all the effort below just for the bounding box!
	// draw each character
	const char *c = utf8.String();
	const unsigned char *u = (unsigned char*)unicode.String();
	for (int i = 0; i < unicode.Length(); i += 2) {
		int s = CodePointSize((char*)c);

		float w = font.StringWidth(c, s);

		if (MakesPDF() && IsClipping()) {
			ClipChar(&font, (char*)u, c, s, w);
		} else {
			DrawChar(u[0]*256+u[1], c, s);
		}

		// position of next character
		if (*(unsigned char*)c <= 0x20) { // should test if c is a white-space!
			w += escapementSpace;
		} else {
			w += escapementNoSpace;
		}

		fState->penX += w * cos1;
		fState->penY += w * sin1;

		// next character
		c += s; u += 2;
	}
	EndTransparency();

	// text line processing (for non rotated text only!)
	BPoint end(fState->penX, fState->penY);
	BRect bounds;
	font_height height;

	font.GetHeight(&height);

	bounds.left = start.x;
	bounds.right = end.x;
	bounds.top = start.y - height.ascent;
	bounds.bottom = end.y   + height.descent;

	TextSegment* segment = new TextSegment(utf8.String(), start, escapementSpace,
		escapementNoSpace, &bounds, &font, pdfSystem());

	fTextLine.Add(segment);
}


bool
PDFWriter::EmbedFont(const char* name)
{
	static FontFile* cache = NULL;
	if (cache && strcmp(cache->Name(), name) == 0) return cache->Embed();

	const int n = fFonts->Length();
	for (int i = 0; i < n; i++) {
		FontFile* f = fFonts->At(i);
		if (strcmp(f->Name(), name) == 0) {
			cache = f;
			return f->Embed();
		}
	}
	return false;
}
