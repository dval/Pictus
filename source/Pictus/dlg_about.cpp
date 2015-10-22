#include "res_settings.h"
#include "dlg_about.h"
#include "version.h"
#include "libpng/png.h"

#include <boost/algorithm/string.hpp>

namespace App {
	using namespace Intl;

	bool SetAbout::PerformOnInitPage() {
		Caption(App::SIDSettingsAbout);

		std::string sep = "\r\n\r\n\r\n\r\n";

		std::stringstream license;
		license << "IJG jpeg library\r\n";
		license << "this software is based in part on the work of the Independent JPEG Group" << sep;

		license << "libpng\r\n";
		auto png = std::string(png_get_copyright(nullptr));
		license << boost::replace_all_copy(png, "    ", "\r\n") << sep;

		license << "zlib\r\n"
				<< "Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler"
				<< sep;

		license << "libtiff\r\n";
		license << "Copyright(c) 1988 - 1997 Sam Leffler\r\n"
				<< "Copyright(c) 1991 - 1997 Silicon Graphics, Inc.\r\n"
				<< "\r\n"
				<< "Permission to use, copy, modify, distribute, and sell this software and its documentation for any purpose is hereby granted without fee, provided that(i) the above copyright notices and this permission notice appear in all copies of the software and related documentation, and (ii)the names of Sam Leffler and Silicon Graphics may not be used in any advertising or publicity relating to the software without the specific, prior written permission of Sam Leffler and Silicon Graphics.\r\n\r\n"
				<< "THE SOFTWARE IS PROVIDED \"AS - IS\" AND WITHOUT WARRANTY OF ANY KIND,EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.\r\n\r\n"
				<< "IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE."
				<< sep;

		ControlText(IDC_EDIT_ABOUT_COPYRIGHT, license.str());

		std::stringstream version;
		version << "Version " << ToAString(CurrentVersion);

		ControlText(IDC_TEXT_ABOUT_VERSION, version.str());

		m_headerFont = CreateFont(128, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, \
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, \
			DEFAULT_PITCH | FF_SWISS, L"Microsoft Sans Serif");

		SendDlgItemMessage(Handle(), IDC_ABOUT_HEADER, WM_SETFONT, (WPARAM)m_headerFont, 0);

		return true;
	}

	SetAbout::SetAbout():
		App::SettingsPage(IDD_SET_ABOUT),
		m_headerFont(0)
	{}

	SetAbout::~SetAbout() {
		if (m_headerFont != 0) {
			DeleteObject(m_headerFont);
			m_headerFont = 0;
		}
	}

	void SetAbout::onWriteSettings(Reg::Settings&)
	{

	}

	void SetAbout::PerformUpdateFromSettings(const Reg::Settings& settings)
	{

	}

}
