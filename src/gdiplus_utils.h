/*
This source code file is licensed under the GNU GPL Version 2.0 Licence by the following copyright holder:
Crown Copyright Commonwealth of Australia (Geoscience Australia) 2015.
The GNU GPL 2.0 licence is available at: http://www.gnu.org/licenses/gpl-2.0.html. If you require a paper copy of the GNU GPL 2.0 Licence, please write to Free Software Foundation, Inc. 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

Author: Ross C. Brodie, Geoscience Australia.
*/

#ifndef _gdiplus_utils_H
#define _gdiplus_utils_H

#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include "colormap.h"
#include "stretch.h"

class cGDIplusHelper {

public:

	cGDIplusHelper() {};

	static ULONG_PTR start() {
		GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR token;
		GdiplusStartup(&token, &gdiplusStartupInput, NULL);
		return token;
	}

	static void stop(ULONG_PTR token) {
		GdiplusShutdown(token);
	}

	static std::wstring ws(const std::string& bstr)
	{
		size_t len = bstr.length() + 1;
		wchar_t* p = new wchar_t[len];
		mbstowcs_s(&len, p, len, bstr.c_str(), _TRUNCATE);
		std::wstring wstr(p);
		delete[]p;
		return wstr;
	}

	static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
	{
		UINT  num = 0;          // number of image encoders
		UINT  size = 0;         // size of the image encoder array in bytes

		GetImageEncodersSize(&num, &size);
		if (size == 0) return -1;  // Failure

		ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
		if (pImageCodecInfo == NULL) return -1;  // Failure

		GetImageEncoders(num, size, pImageCodecInfo);

		for (UINT j = 0; j < num; j++) {
			if (wcscmp((pImageCodecInfo[j]).MimeType, format) == 0) {
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}
		}

		free(pImageCodecInfo);
		return -1;  // Failure
	}

	static CLSID getencoderid(const std::string filename)
	{
		int status = -1;
		CLSID id;
		std::string ext = extractfileextension(filename);
		if (strcasecmp(ext, ".jpg") == 0) {
			status = GetEncoderClsid(L"image/jpeg", &id);
		}
		else if (strcasecmp(ext, ".png") == 0) {
			status = GetEncoderClsid(L"image/png", &id);
		}
		else if (strcasecmp(ext, ".bmp") == 0) {
			status = GetEncoderClsid(L"image/bmp", &id);
		}
		else if (strcasecmp(ext, ".emf") == 0) {
			status = GetEncoderClsid(L"image/emf", &id);
		}
		return id;
	}

	static Status saveimage(Bitmap* bm, const std::string& filename)
	{
		makedirectorydeep(extractfiledirectory(filename));
		CLSID id = getencoderid(filename);
		std::wstring wpath(ws(filename.c_str()));
		return bm->Save(wpath.c_str(), &id, NULL);
	}

	static Bitmap* colorbar(const cColorMap& cmap, const cStretch& stretch, const std::string title, std::vector<double> ticks) {

		REAL fontsize = 12;
		FontStyle fontstyle = FontStyleBold;
		Font font(L"Arial", fontsize, fontstyle, UnitPoint);
		StringFormat textformat;
		textformat.SetAlignment(StringAlignmentNear);
		textformat.SetLineAlignment(StringAlignmentCenter);

		REAL dpi = 300;
		int width = 400;
		int height = 700;
		int margin = 40;
		int ph1 = 64 + margin;
		int ph2 = ph1 + 128;
		int pvtop = margin;
		int pvbot = height - margin;
		int ticklength = 10;

		Bitmap* bm = new Bitmap(width, height, PixelFormat32bppARGB);
		bm->SetResolution(dpi, dpi);
		for (int i = 0; i < width; i++) {
			for (int j = 0; j < height; j++) {
				bm->SetPixel(i, j, Color(255, 255, 255, 255));
			}
		}

		Pen blackpen(Color::Black, 3);
		SolidBrush blackbrush(Color::Black);

		Graphics gr(bm);
		gr.SetPageUnit(UnitPixel);
		gr.SetTextRenderingHint(TextRenderingHintAntiAlias);

		for (int j = pvtop; j <= pvbot; j++) {
			int ind = 255 * (j - pvbot) / (pvtop - pvbot);
			for (int i = ph1; i <= ph2; i++) {
				bm->SetPixel(i, j, Color(255, cmap.r[ind], cmap.g[ind], cmap.b[ind]));
			}
		}
		gr.DrawRectangle(&blackpen, ph1, pvtop, ph2 - ph1, pvbot - pvtop);

		for (int i = 0; i < ticks.size(); i++) {
			if (ticks[i] < stretch.lowclip)continue;
			if (ticks[i] > stretch.highclip)continue;

			int ind = stretch.index(ticks[i]);
			int tickv = (int)(pvbot + (double)(pvtop - pvbot) * (double)ind / (double)stretch.nbins);

			std::string s = strprint("%5.3lf", ticks[i]);

			PointF p;
			p.X = (REAL)ph2 + 3;
			p.Y = (REAL)tickv;
			gr.DrawString(ws(s).c_str(), -1, &font, p, &textformat, &blackbrush);
			gr.DrawLine(&blackpen, ph2 - ticklength, tickv, ph2 + ticklength, tickv);
		}

		PointF p;
		p.X = (REAL)ph1 - 4;
		p.Y = (REAL)(pvtop + pvbot) / 2;

		textformat.SetAlignment(StringAlignmentCenter);
		textformat.SetLineAlignment(StringAlignmentFar);
		gr.TranslateTransform(p.X, p.Y);
		gr.RotateTransform(-90);
		gr.DrawString(ws(title).c_str(), -1, &font, PointF(0, 0), &textformat, &blackbrush);

		return bm;
	}



};

#endif
