#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

	PACKED_STRUCT_BEGIN BitmapFileHeader {
		char c1 = 'B'; 
		char c2 = 'M'; 
		uint32_t size; 
		uint32_t reserved = 0; 
		uint32_t indentation = 54; 
	}
	PACKED_STRUCT_END

	PACKED_STRUCT_BEGIN BitmapInfoHeader {
		uint32_t size = 40; 
		uint32_t width; 
		uint32_t height; 
		uint16_t planes = 1; 
		uint16_t bites = 24; 
		uint32_t compression = 0; 
		uint32_t size_image; 
		int32_t x_pxls_pm = 11811; 
		int32_t y_pxls_pm = 11811; 
		int32_t used_colors = 0; 
		int32_t colors = 0x1000000; 
	}
	PACKED_STRUCT_END

	static int GetBMPStride(int w) {
		return 4 * ((w * 3 + 3) / 4);
	}

	bool SaveBMP(const Path& file, const Image& image) {
		ofstream out(file, ios::binary);
		
		img_lib::BitmapFileHeader file_header;

		out.write(reinterpret_cast<const char*>(&file_header.c1), 1);
		out.write(reinterpret_cast<const char*>(&file_header.c2), 1);

		file_header.size = 54 + GetBMPStride(image.GetWidth()) * image.GetHeight();
		out.write(reinterpret_cast<const char*>(&file_header.size), sizeof(file_header.size));
		out.write(reinterpret_cast<const char*>(&file_header.reserved), sizeof(file_header.reserved));	
		out.write(reinterpret_cast<const char*>(&file_header.indentation), sizeof(file_header.indentation));
		
		img_lib::BitmapInfoHeader info_header;
		out.write(reinterpret_cast<const char*>(&info_header.size), sizeof(info_header.size));
		
		info_header.width = image.GetWidth();
		info_header.height = image.GetHeight();
		out.write(reinterpret_cast<const char*>(&info_header.width), sizeof(info_header.width));
		out.write(reinterpret_cast<const char*>(&info_header.height), sizeof(info_header.height));
		
		out.write(reinterpret_cast<const char*>(&info_header.planes), sizeof(info_header.planes));
		out.write(reinterpret_cast<const char*>(&info_header.bites), sizeof(info_header.bites));
		
		out.write(reinterpret_cast<const char*>(&info_header.compression), sizeof(info_header.compression));
		
		info_header.size_image = GetBMPStride(image.GetWidth()) * image.GetHeight();
		out.write(reinterpret_cast<const char*>(&info_header.size_image), sizeof(info_header.size_image));
		
		out.write(reinterpret_cast<const char*>(&info_header.y_pxls_pm), sizeof(info_header.y_pxls_pm));
		out.write(reinterpret_cast<const char*>(&info_header.x_pxls_pm), sizeof(info_header.x_pxls_pm));
		
		out.write(reinterpret_cast<const char*>(&info_header.used_colors), sizeof(info_header.used_colors));
		out.write(reinterpret_cast<const char*>(&info_header.colors), sizeof(info_header.colors));
		
		for (int y = info_header.height-1; y >= 0; --y) {
			const Color* line = image.GetLine(y);
			std::vector<char> buff(GetBMPStride(info_header.width));
			for (int x = 0; x < info_header.width; ++x) {
				buff[x * 3 + 0] = static_cast<char>(line[x].b);
				buff[x * 3 + 1] = static_cast<char>(line[x].g);
				buff[x * 3 + 2] = static_cast<char>(line[x].r);
			}
			out.write(buff.data(), buff.size());
		}
		
		return out.good();
	}

	Image LoadBMP(const Path& file) {
		ifstream in(file, ios::binary);
		
		img_lib::BitmapFileHeader file_header;
		img_lib::BitmapInfoHeader info_header;
		
		in.read(reinterpret_cast<char*>(&file_header.c1), 1);
		in.read(reinterpret_cast<char*>(&file_header.c2), 1);
		in.read(reinterpret_cast<char*>(&file_header.size), sizeof(file_header.size));
		in.read(reinterpret_cast<char*>(&file_header.reserved), sizeof(file_header.reserved));	
		in.read(reinterpret_cast<char*>(&file_header.indentation), sizeof(file_header.indentation));
		
		in.read(reinterpret_cast<char*>(&info_header.size), sizeof(info_header.size));
		in.read(reinterpret_cast<char*>(&info_header.width), sizeof(info_header.width));
		in.read(reinterpret_cast<char*>(&info_header.height), sizeof(info_header.height));
		in.read(reinterpret_cast<char*>(&info_header.planes), sizeof(info_header.planes));
		in.read(reinterpret_cast<char*>(&info_header.bites), sizeof(info_header.bites));
		in.read(reinterpret_cast<char*>(&info_header.compression), sizeof(info_header.compression));
		in.read(reinterpret_cast<char*>(&info_header.size_image), sizeof(info_header.size_image));
		in.read(reinterpret_cast<char*>(&info_header.y_pxls_pm), sizeof(info_header.y_pxls_pm));
		in.read(reinterpret_cast<char*>(&info_header.x_pxls_pm), sizeof(info_header.x_pxls_pm));
		in.read(reinterpret_cast<char*>(&info_header.used_colors), sizeof(info_header.used_colors));
		in.read(reinterpret_cast<char*>(&info_header.colors), sizeof(info_header.colors));
		
		Image result(info_header.width, info_header.height, Color::Black());
		
		for (int y = info_header.height - 1; y >= 0; --y) {
			Color* line = result.GetLine(y);
			std::vector<char> buff(GetBMPStride(info_header.width));
			
			in.read(buff.data(), GetBMPStride(info_header.width));

			for (int x = 0; x < info_header.width; ++x) {
				line[x].b = static_cast<byte>(buff[x * 3 + 0]);
				line[x].g = static_cast<byte>(buff[x * 3 + 1]);
				line[x].r = static_cast<byte>(buff[x * 3 + 2]);
			}
		}
		
		return result;		
	}

}  // namespace img_lib