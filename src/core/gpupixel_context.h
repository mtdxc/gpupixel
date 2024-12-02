/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#pragma once

#include <mutex>
#include "framebuffer_cache.h"
#include "gpupixel_macros.h"
#include "dispatch_queue.h"

#include "filter.h"
#include "gl_program.h"

NS_GPUPIXEL_BEGIN
#if defined(GPUPIXEL_ANDROID)
struct GpuContext;
#endif

class GPUPIXEL_API GPUPixelContext {
 public:
  static GPUPixelContext* getInstance();
  static void destroy();

  FramebufferCache* getFramebufferCache() const;
  //todo(zhaoyou)
  void setActiveShaderProgram(GLProgram* shaderProgram);
  void purge();

  void runSync(std::function<void(void)> func);
  // 此函数暂时没用
  void runAsync(std::function<void(void)> func);
  void useAsCurrent(void);
  void presentBufferForDisplay();
  bool isCurrentThread() const {return std::this_thread::get_id() == _tid;}
  void step();
#if defined(GPUPIXEL_IOS)
  EAGLContext* getGLContext() const { return _eglContext; }
#elif defined(GPUPIXEL_MAC)
  NSOpenGLContext* getGLContext() const { return imageProcessingContext; }
#elif defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
  GLFWwindow* getGLContext() const { return gl_context_; }
#elif defined(GPUPIXEL_ANDROID)
  GpuContext* getGLContext(){ return gl_context_; }
#endif
 

  // used for capturing a processed frame data
  bool isCapturingFrame;
  std::shared_ptr<Filter> captureUpToFilter;
  unsigned char* capturedFrameData;
  int captureWidth;
  int captureHeight;

 private:
  GPUPixelContext();
  ~GPUPixelContext();

  void init();

  void createContext();
  void releaseContext();
 private:
  static GPUPixelContext* _instance;
  static std::mutex _mutex;
  FramebufferCache* _framebufferCache;
  GLProgram* _curShaderProgram;
  std::shared_ptr<LocalDispatchQueue> task_queue_;
  std::thread::id _tid;
#if defined(GPUPIXEL_ANDROID)
  GpuContext* gl_context_ = nullptr;
#elif defined(GPUPIXEL_IOS)
  EAGLContext* _eglContext = nullptr;
#elif defined(GPUPIXEL_MAC)
  NSOpenGLContext* imageProcessingContext = nullptr;
  NSOpenGLPixelFormat* _pixelFormat = nullptr;
#elif defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
  GLFWwindow* gl_context_ = nullptr;
#endif

};

NS_GPUPIXEL_END
