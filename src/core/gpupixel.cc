#include "gpupixel/gpupixel.h"
#include <cstdint>
#include <vector>
#include "utils/util.h"
#include "libyuv.h"
namespace gpupixel {

void GPUPixel::SetResourcePath(const std::string& path) {
  Util::SetResourcePath(fs::path(path));
}

int GPUPixel::I420ToARGB(uint8_t* argb, int width, int height, 
	const uint8_t* yuv_y, int stride_y,
	const uint8_t* yuv_u, int stride_u,
	const uint8_t* yuv_v, int stride_v) {
	if (!yuv_u)
		yuv_u = yuv_y + width * height;
	if (!yuv_v)
		yuv_v = yuv_y + width * height * 5 / 4;
	if (!stride_y)
		stride_y = width;
	if (!stride_u)
		stride_u = width / 2;
	if (!stride_v)
		stride_v = width / 2;
	return libyuv::I420ToARGB(yuv_y, stride_y,	
		yuv_u, stride_u,
		yuv_v, stride_v,
		argb, width * 4,
		width, height);
}

int GPUPixel::I420ToARGB(const uint8_t* yuv, uint8_t* argb, int width, int height, bool swapuv) {
	const uint8_t* yuv_u = yuv + width * height;
	const uint8_t* yuv_v = yuv + width * height * 5 / 4;
	if (swapuv) {
		std::swap(yuv_u, yuv_v);
	}
	return libyuv::I420ToARGB(yuv, width, 
		yuv_u, width / 2, 
		yuv_v, width / 2, 
		argb, width * 4, 
		width, height);
}

int GPUPixel::ARGBRotation(const uint8_t* src, uint8_t* dst, int width, int height, int rotation) {
	int stride = width * 4;
	if ((rotation/90)%2)
		stride = height * 4;
	return libyuv::ARGBRotate(src, width * 4, dst, stride, width, height, (libyuv::RotationMode)rotation);
}

}  // namespace gpupixel
