#ifndef PSDIMAGE_H
#define PSDIMAGE_H

#include "psdtypes.hpp"

#include <cstdint>
#include <vector>

namespace psdimpl
{
	struct PSDChannel
	{
		uint16_t compression{};
		std::vector<uint8_t> image_data{};
		std::vector<uint16_t> bytecounts{};
	};

	class PSDImage
	{
	public:
		virtual psdw::PSDStatus load(const unsigned char* img,
			psdimpl::ChannelOrder channel_order, int width, int height) = 0;
		virtual psdw::PSDStatus load(std::vector<PSDChannel> img, int channels,
			int width, int height) = 0;

		const std::vector<PSDChannel>& data() const { return m_image_data; }
		int channels() const { return m_channels; }
		int width() const { return m_width; }
		int height() const { return m_height; }

	protected:
		// Given a pixel coordinate, retrieve the index of the corresponding
		// element in the band-interleaved-by-pixel 1D array.
		size_t get_index(int channels, int width, int channel,
			int x, int y) const;

		std::vector<int> enumerate_channels(ChannelOrder channel_order) const;

		int m_channels{};
		int m_width{};
		int m_height{};
		std::vector<PSDChannel> m_image_data{}; // ARGB or RGB order.
	};

	class PSDRawImage : public PSDImage
	{
	public:
		psdw::PSDStatus load(const unsigned char* img,
			ChannelOrder channel_order, int width, int height) override;
		psdw::PSDStatus load(std::vector<PSDChannel> img, int channels,
			int width, int height) override;

		psdw::PSDStatus generate(int width, int height, psdw::PSDColour colour);

		void composite(const unsigned char* foreground, psdw::PSDRect rect,
			psdw::PSDChannelOrder foreground_channel_order);
	};

	class PSDCompressedImage : public PSDImage
	{
	public:
		psdw::PSDStatus load(const unsigned char* img,
			ChannelOrder channel_order, int width, int height) override;
		psdw::PSDStatus load(std::vector<PSDChannel> img, int channels,
			int width, int height) override;

	private:
		void finalise_pack(std::vector<uint8_t>& buf, uint16_t& bytes);
		void finalise_pack(const uint8_t val, int& reps, uint16_t& bytes);
	};
}

#endif