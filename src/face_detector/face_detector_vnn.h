#pragma once
#include "gpupixel/face_detector/face_detector.h"
namespace gpupixel {
class FaceDetectorVNN : public FaceDetector {
 public:
  FaceDetectorVNN();
  ~FaceDetectorVNN();

  std::vector<float> Detect(const uint8_t* data,
                             int width,
                             int height,
                             int stride,
                             GPUPIXEL_MODE_FMT fmt,
                             GPUPIXEL_FRAME_TYPE type) override;

 private:
  uint32_t vnn_handle_;
  int use_278pts = 0;
};

}