#include "gpupixel/gpupixel.h"
#include "utils/util.h"
#include "libyuv/video_common.h"
#include "libyuv/convert_argb.h"
#include "libyuv/rotate_argb.h"
namespace gpupixel {

std::string GPUPixel::GetResourcePath(const std::string& name) {
  return Util::GetResourcePath(name);
}

void GPUPixel::SetResourceRoot(const std::string& root) {
  Util::SetResourceRoot(root);
}

int GPUPixel::I420ToARGB(const uint8_t* yuv, uint8_t* argb, int width, int height, int rotation) {
	int stride = width * 4;
	if ((rotation/90)%2)
		stride = height * 4;
	return libyuv::ConvertToARGB(yuv, width * height * 3 /2, argb, stride, 0, 0, width, height, width, height, 
		(libyuv::RotationMode)rotation, libyuv::FourCC::FOURCC_I420);
}

int GPUPixel::ARGBRotation(const uint8_t* src, uint8_t* dst, int width, int height, int rotation) {
	int stride = width * 4;
	if ((rotation/90)%2)
		stride = height * 4;
	return libyuv::ARGBRotate(src, width * 4, dst, stride, width, height, (libyuv::RotationMode)rotation);
}

}  // namespace gpupixel
