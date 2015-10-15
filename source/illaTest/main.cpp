#include <UnitTest++/UnitTest++.h>

#include "illa/surfacemgr.h"
#include "illa/swsurface.h"

#include <boost/filesystem.hpp>
#include <boost/locale.hpp>

std::string g_datapath;

int realMain(std::string path) {
	std::locale::global(boost::locale::generator().generate(""));
	boost::filesystem::path::imbue(std::locale());
	g_datapath = path;
	Img::SurfaceFactory(new Img::FactorySurfaceSoftware);

	return UnitTest::RunAllTests();
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[]) {
	if(argc < 2) return 1;
	return realMain(WStringToUTF8(argv[1]));
}
#else
int main(int argc, char* argv[]) {
	if(argc < 2) return 1;
	return realMain(argv[1]);;
}
#endif
