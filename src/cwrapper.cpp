#include "psdocument.hpp"
#include "psdtypes.hpp"

#include <filesystem>
#include <string>
#include <locale>
#include <codecvt>

using namespace psdw;

extern "C"
{
	DllExport PSDocument* psd_new(int w, int h)
	{
		return new PSDocument(w, h);
	}

	DllExport void psd_delete(PSDocument* psd)
	{
		delete psd;
	}

	DllExport bool set_resolution(
		PSDocument* psd,
		double resolution)
	{
		PSDStatus response = psd->set_resolution(resolution);
		return response == PSDStatus::Success ? true : false;
	}

	DllExport bool set_profile(
		PSDocument* psd,
		const wchar_t* icc_profile)
	{
		PSDStatus response = psd->set_profile(icc_profile);
		return response == PSDStatus::Success ? true : false;
	}

	DllExport bool add_guide(
		PSDocument* psd,
		int position,
		PSDOrientation orientation)
	{
		PSDStatus response = psd->add_guide(position, orientation);
		return response == PSDStatus::Success ? true : false;
	}

	DllExport bool add_layer(
		PSDocument* psd,
		const unsigned char* img,
		bool rgba,
		PSDRect rect,
		const wchar_t* layer_name)
	{
		std::filesystem::path wstr(layer_name); // BODGE
		std::string str(wstr.string());

		psdw::PSDChannelOrder channel_order =
			rgba ? PSDChannelOrder::RGBA : PSDChannelOrder::BGRA;

		PSDStatus response = psd->add_layer(img, rect, str, channel_order);
		return response == PSDStatus::Success ? true : false;
	}

	DllExport bool save(
		PSDocument* psd,
		const wchar_t* filename,
		bool overwrite)
	{
		PSDStatus response = psd->save(filename, overwrite);
		return response == PSDStatus::Success ? true : false;
	}
}