#include "framework.h"
#include "pch.h"
#include "psdocument.hpp"
#include "psdtypes.hpp"

#include <locale>
#include <codecvt>
#include <iostream>

using namespace psdw;

extern "C"
{
	__declspec(dllexport) PSDocument* psd_new(int w, int h)
	{
		return new PSDocument(w, h);
	}

	__declspec(dllexport) void psd_delete(PSDocument* psd)
	{
		delete psd;
	}

	__declspec(dllexport) bool set_resolution(
		PSDocument* psd,
		double resolution)
	{
		PSDStatus response = psd->set_resolution(resolution);
		return response == PSDStatus::Success ? true : false;
	}

	__declspec(dllexport) bool set_profile(
		PSDocument* psd,
		wchar_t* icc_profile)
	{
		PSDStatus response = psd->set_profile(icc_profile);
		return response == PSDStatus::Success ? true : false;
	}

	__declspec(dllexport) bool add_layer(
		PSDocument* psd,
		const unsigned char* img,
		bool rgba,
		PSDRect rect,
		wchar_t* layer_name)
	{
		std::filesystem::path wstr(layer_name); // BODGE
		std::string str(wstr.string());

		psdw::PSDChannelOrder channel_order =
			rgba ? PSDChannelOrder::RGBA : PSDChannelOrder::BGRA;

		PSDStatus response = psd->add_layer(img, rect, str, channel_order);
		return response == PSDStatus::Success ? true : false;
	}

	__declspec(dllexport) bool save(
		PSDocument* psd,
		wchar_t* filename,
		bool overwrite)
	{
		PSDStatus response = psd->save(filename, overwrite);
		return response == PSDStatus::Success ? true : false;
	}
}