/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#pragma once

#include "framebuffer.h"
#include "gpupixel_macros.h"

#include <map>

NS_GPUPIXEL_BEGIN
GPUPIXEL_API enum RotationMode {
  NoRotation = 0,
  RotateLeft,
  RotateRight,
  FlipVertical,
  FlipHorizontal,
  RotateRightFlipVertical,
  RotateRightFlipHorizontal,
  Rotate180
};

class GPUPIXEL_API Target {
 public:
  Target(int inputNumber = 1);
  virtual ~Target();
  virtual void setInputFramebuffer(std::shared_ptr<Framebuffer> framebuffer,
                                   RotationMode rotationMode = NoRotation,
                                   int texIdx = 0);
  // 判断入参是否都准备好没?
  virtual bool isPrepared() const;
  // 清空入参，为下一帧处理做好准备
  virtual void unPrepear();
  // 处理函数
  virtual void update(int64_t frameTime){};
  // 返回下一入参索引
  virtual int getNextAvailableTextureIndex() const;
  // virtual void setInputSizeWithIdx(int width, int height, int textureIdx) {};
 protected:
  struct InputFrameBufferInfo {
    std::shared_ptr<Framebuffer> frameBuffer;
    RotationMode rotationMode;
    int texIndex; ///< 参数索引
    bool ignoreForPrepare; ///< 参数可否为空
  };
  // 入参列表
  std::map<int, InputFrameBufferInfo> _inputFramebuffers;
  // 入参个数: 只有所有参数都准备好后isPrepared, 才可处理 update
  int _inputNum;
};

NS_GPUPIXEL_END
