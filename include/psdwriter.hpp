#ifndef PSDWRITER_H
#define PSDWRITER_H

#include "psddata.hpp"
#include "psdtypes.hpp"

#include <string>
#include <filesystem>
#include <fstream>
#include <cstdint>
#include <vector>

namespace psdimpl
{
	class PSDWriter
	{
	public:
		PSDWriter(const PSDData& psd_data);

		psdw::PSDStatus write(const std::filesystem::path& filepath,
			bool overwrite);
		psdw::PSDStatus status() { return m_status; }

	private:
		constexpr bool little_endian();

		void write(const uint8_t& val);
		void write(const uint16_t& val);
		void write(const int16_t& val);
		void write(const uint32_t& val);
		void write(const double& val);
		void write_with_null(const double& val);
		void write(const std::vector<char>& val);
		void write(const std::vector<uint8_t>& val);
		void write(const std::vector<uint16_t>& val);
		void write(const std::string& val);
		void write(const psdimpl::PascalString& val);
		void write(const psdimpl::LayerRect& val);
		void write(const psdimpl::ChannelInfo& val);
		void write(const psdimpl::LayerBlendingRanges& val);
		void write(const psdimpl::AdditionalLayerInfo& val);
		void write(const psdimpl::AdditionalLayerInfoLuni& val);
		void write(const psdimpl::AdditionalLayerInfoLnsr& val);
		void write(const psdimpl::AdditionalLayerInfoLyid& val);
		void write(const psdimpl::AdditionalLayerInfoShmd& val);
		void write(const psdimpl::AdditionalLayerInfoCust& val);
		void write(const psdimpl::AdditionalLayerInfoFxrp& val);

		psdw::PSDStatus m_status;
		const PSDData& m_data;
		std::ofstream m_writer;
	};
}

#endif
