/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#include <cassert>
#include "vnn_kit.h"
#include "vnn_face.h"
#include "utils/filesystem.h"
#include "utils/logging.h"
#include "utils/util.h"
#include "face_detector_vnn.h"

namespace gpupixel {
std::shared_ptr<FaceDetector> FaceDetector::Create() {
  return std::shared_ptr<FaceDetector>(new FaceDetectorVNN());
}

FaceDetectorVNN::FaceDetectorVNN() {
  //  init 
  VNN_SetLogLevel(VNN_LOG_LEVEL_ALL);
#if defined(GPUPIXEL_IOS) || defined(GPUPIXEL_ANDROID)
  auto model_path = Util::GetResourcePath() / "face_mobile[1.0.0].vnnmodel";
#elif defined(GPUPIXEL_WIN) || defined(GPUPIXEL_MAC) || defined(GPUPIXEL_LINUX)
  auto model_path = Util::GetResourcePath() / "face_pc[1.0.0].vnnmodel";
#endif
  if (fs::exists(model_path)) {
    const void *argv[] = {model_path.string().c_str()};
    const int argc = sizeof(argv)/sizeof(argv[0]);
    VNN_Result ret = VNN_Create_Face(&vnn_handle_, argc, argv);
  }
}

FaceDetectorVNN::~FaceDetectorVNN() {
  if (vnn_handle_ > 0)
    VNN_Destroy_Face(&vnn_handle_);
}


std::vector<float> FaceDetectorVNN::Detect(const uint8_t* data,
                                        int width,
                                        int height,
                                        int stride,
                                        GPUPIXEL_MODE_FMT fmt,
                                        GPUPIXEL_FRAME_TYPE type) {
  std::vector<float> landmarks;
  if (vnn_handle_ == 0) {
    return landmarks;
  }
  
  VNN_Set_Face_Attr(vnn_handle_, "_use_278pts", &use_278pts);

  VNN_Image input;
  input.width = width;
  input.height = height;
  input.channels = 4;
  switch (type) {
    case GPUPIXEL_FRAME_TYPE_RGBA:
      input.pix_fmt = VNN_PIX_FMT_BGRA8888; 
      break;
    case GPUPIXEL_FRAME_TYPE_YUVI420:
      input.pix_fmt = VNN_PIX_FMT_YUVI420;
      break;
    default:
      break;
  }

  input.data = (VNNVoidPtr)data;
  switch (fmt)
  {
  case GPUPIXEL_MODE_FMT_VIDEO:
    input.mode_fmt = VNN_MODE_FMT_VIDEO;
    break;
  case GPUPIXEL_MODE_FMT_PICTURE:
    input.mode_fmt = VNN_MODE_FMT_PICTURE;
    break;
  default:
    break;
  }

  input.ori_fmt = VNN_ORIENT_FMT_DEFAULT;

  VNN_FaceFrameDataArr output;
  VNN_Result ret = VNN_Apply_Face_CPU(vnn_handle_, &input, &output);
 
  if (output.facesNum > 0) {
    for (int i = 0; i < output.facesArr[0].faceLandmarksNum; i++) {
      landmarks.push_back(output.facesArr[0].faceLandmarks[i].x);
      landmarks.push_back(output.facesArr[0].faceLandmarks[i].y);
    }

    // 106
    auto point_x = (output.facesArr[0].faceLandmarks[102].x + output.facesArr[0].faceLandmarks[98].x)/2;
    auto point_y = (output.facesArr[0].faceLandmarks[102].y + output.facesArr[0].faceLandmarks[98].y)/2;
    landmarks.push_back(point_x);
    landmarks.push_back(point_y);
    
    // 107
    point_x = (output.facesArr[0].faceLandmarks[35].x + output.facesArr[0].faceLandmarks[65].x)/2;
    point_y = (output.facesArr[0].faceLandmarks[35].y + output.facesArr[0].faceLandmarks[65].y)/2;
    landmarks.push_back(point_x);
    landmarks.push_back(point_y);
    
    
    // 108
    point_x = (output.facesArr[0].faceLandmarks[70].x + output.facesArr[0].faceLandmarks[40].x)/2;
    point_y = (output.facesArr[0].faceLandmarks[70].y + output.facesArr[0].faceLandmarks[40].y)/2;
    landmarks.push_back(point_x);
    landmarks.push_back(point_y);
    
    // 109
    point_x = (output.facesArr[0].faceLandmarks[5].x + output.facesArr[0].faceLandmarks[80].x)/2;
    point_y = (output.facesArr[0].faceLandmarks[5].y + output.facesArr[0].faceLandmarks[80].y)/2;
    landmarks.push_back(point_x);
    landmarks.push_back(point_y);

    // 110
    point_x = (output.facesArr[0].faceLandmarks[81].x + output.facesArr[0].faceLandmarks[27].x)/2;
    point_y = (output.facesArr[0].faceLandmarks[81].y + output.facesArr[0].faceLandmarks[27].y)/2;
    landmarks.push_back(point_x);
    landmarks.push_back(point_y);
  }
  
  return landmarks;
}

}  // namespace gpupixel
