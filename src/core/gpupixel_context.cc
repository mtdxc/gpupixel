/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#include "gpupixel_context.h"
#include <future>
#include "util.h"

#if defined(GPUPIXEL_IOS) || defined(GPUPIXEL_MAC)

typedef void (^TaskBlock)(void);

@interface iOSHelper : NSObject {
  bool isActive;
}

- (bool)isAppActive;
- (void)run:(TaskBlock)task;
@end

@implementation iOSHelper
- (id)init {
  if (self = [super init]) {
#if defined(GPUPIXEL_IOS)
    // register notification
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(willResignActive)
               name:UIApplicationWillResignActiveNotification
             object:nil];
    [[NSNotificationCenter defaultCenter]
        addObserver:self
           selector:@selector(didBecomeActive)
               name:UIApplicationDidBecomeActiveNotification
             object:nil];
#endif
    isActive = true;
  }
  return self;
}

- (void)willResignActive {
  @synchronized(self) {
    isActive = false;
  }
}

- (void)didBecomeActive {
  @synchronized(self) {
    isActive = true;
  }
}

- (bool)isAppActive {
  @synchronized(self) {
    return isActive;
  }
}

- (void)run:(TaskBlock)task {
  [self performSelectorOnMainThread:@selector(doRunTask:)
                         withObject:task
                      waitUntilDone:NO];
}
- (void)doRunTask:(TaskBlock)task {
  task();
}
@end

#endif

NS_GPUPIXEL_BEGIN

#if defined(GPUPIXEL_IOS) || defined(GPUPIXEL_MAC)
iOSHelper* iosHelper;
#elif defined(GPUPIXEL_ANDROID)
const std::string kRtcLogTag = "Context";
struct GpuContext {
  EGLDisplay egldisplay = EGL_NO_DISPLAY;
  EGLSurface eglsurface = EGL_NO_SURFACE;
  EGLContext eglcontext = EGL_NO_CONTEXT;
  bool inited_ = false;
  int surface_width_ = 0;
  int surface_height_ = 0;

  ~GpuContext() {destory();}
  bool create() {
    inited_ = true;
    surface_width_ = 1;
    surface_height_ = 1;
    egldisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == egldisplay) {
      Util::Log("ERROR", "eglGetDisplay Error!");
      return false;
    }

    GLint majorVersion;
    GLint minorVersion;
    if (!eglInitialize(egldisplay, &majorVersion, &minorVersion)) {
      Util::Log("ERROR", "eglInitialize Error!");
      return false;
    }
    Util::Log("INFO", "GL Version minor:%d major:%d", minorVersion, majorVersion);

    // 如果创建WindowSurface使用EGL_WINDOW_BIT，PBufferSurface使用EGL_PBUFFER_BIT
    EGLint config_attribs[] = {EGL_BLUE_SIZE,
                                8,
                                EGL_GREEN_SIZE,
                                8,
                                EGL_RED_SIZE,
                                8,
                                EGL_RENDERABLE_TYPE,
                                EGL_OPENGL_ES2_BIT,
                                EGL_SURFACE_TYPE,
                                EGL_PBUFFER_BIT,
                                EGL_NONE};

    int num_configs = 0;
    EGLConfig eglConfig;
    if (!eglChooseConfig(egldisplay, config_attribs, &eglConfig, 1,
                          &num_configs)) {
      Util::Log("ERROR", "eglChooseConfig Error!");
      return false;
    }

    EGLint context_attrib[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
    eglcontext = eglCreateContext(egldisplay, eglConfig, EGL_NO_CONTEXT, context_attrib);
    if (EGL_NO_CONTEXT == eglcontext) {
      Util::Log("ERROR", "eglCreateContext Error!");
      return false;
    }

    int attribListPbuffer[] = {EGL_WIDTH, surface_width_, EGL_HEIGHT,
                                surface_height_, EGL_NONE};

    eglsurface = eglCreatePbufferSurface(
            egldisplay, eglConfig, attribListPbuffer);
    if (EGL_NO_SURFACE == eglsurface) {
      Util::Log("ERROR", "eglCreatePbufferSurface Error!");
      return false;
    }

    if (!eglQuerySurface(egldisplay, eglsurface,EGL_WIDTH, &surface_width_)
      || !eglQuerySurface(egldisplay, eglsurface,EGL_HEIGHT, &surface_height_)) {
      Util::Log("ERROR", "eglQuerySurface Error!");
      return false;
    }
    Util::Log("INFO", "Create Surface width:%d height:%d", surface_width_, surface_width_);
    return true;
  }

  bool makeCurrent() {
    return eglMakeCurrent(egldisplay, eglsurface,eglsurface, eglcontext);
  }

  void destory() {
    if (!inited_) {
      return;
    }
    inited_ = false;
    Util::Log("INFO", "destory");
    if (egldisplay != EGL_NO_DISPLAY) {
      if (eglcontext != EGL_NO_CONTEXT) {
        eglDestroyContext(egldisplay, eglcontext);
      }
      if (eglsurface != EGL_NO_SURFACE) {
        eglDestroySurface(egldisplay, eglsurface);
      }

      eglMakeCurrent(egldisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
      if (!eglTerminate(egldisplay)) {
        Util::Log("ERROR", "Free egldisplay Error!");
      }
    }
  }
};

#elif defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
const unsigned int VIEW_WIDTH = 1280;
const unsigned int VIEW_HEIGHT = 720;
#endif

GPUPixelContext* GPUPixelContext::_instance = 0;
std::mutex GPUPixelContext::_mutex;

GPUPixelContext::GPUPixelContext()
    : _curShaderProgram(0),
      isCapturingFrame(false),
      captureUpToFilter(0),
      capturedFrameData(0) {
  _framebufferCache = new FramebufferCache();
  task_queue_ = std::make_shared<LocalDispatchQueue>();
  init();
}

GPUPixelContext::~GPUPixelContext() {
  releaseContext();
  delete _framebufferCache;
}

GPUPixelContext* GPUPixelContext::getInstance() {
  if (!_instance) {
    std::unique_lock<std::mutex> lock(_mutex);
    if (!_instance) {
      _instance = new (std::nothrow) GPUPixelContext;
    }
  }
  return _instance;
};

void GPUPixelContext::destroy() {
  std::unique_lock<std::mutex> lock(_mutex);
  if (_instance) {
    delete _instance;
    _instance = 0;
  }
}

void GPUPixelContext::init() {
  _tid = std::this_thread::get_id();
  runSync([=] {
    Util::Log("INFO", "start init GPUPixelContext");
    this->createContext();
  });
}

FramebufferCache* GPUPixelContext::getFramebufferCache() const {
  return _framebufferCache;
}

void GPUPixelContext::setActiveShaderProgram(GLProgram* shaderProgram) {
  if (_curShaderProgram != shaderProgram) {
    _curShaderProgram = shaderProgram;
    shaderProgram->use();
  }
}

void GPUPixelContext::purge() {
  _framebufferCache->purge();
}
 
void GPUPixelContext::createContext() {
#if defined(GPUPIXEL_IOS) 
  _eglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  [EAGLContext setCurrentContext:_eglContext];
  iosHelper = [[iOSHelper alloc] init];
#elif defined(GPUPIXEL_MAC)
  NSOpenGLPixelFormatAttribute pixelFormatAttributes[] = {
      NSOpenGLPFADoubleBuffer,
      NSOpenGLPFAOpenGLProfile,
      NSOpenGLProfileVersionLegacy,
      NSOpenGLPFAAccelerated,
      0,
      NSOpenGLPFAColorSize,
      24,
      NSOpenGLPFAAlphaSize,
      8,
      NSOpenGLPFADepthSize,
      24,
      0};

  _pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:pixelFormatAttributes];
  imageProcessingContext = [[NSOpenGLContext alloc] initWithFormat:_pixelFormat shareContext:nil];

  GLint interval = 0;
  [imageProcessingContext makeCurrentContext];
  [imageProcessingContext setValues:&interval
                       forParameter:NSOpenGLContextParameterSwapInterval];

#elif defined(GPUPIXEL_ANDROID)
  Util::Log("INFO", "GPUPixelContext::createContext start");
  gl_context_ = new GpuContext();
  gl_context_->create();
#elif defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
  int ret = glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  // must use legacy opengl profile
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

  if (ret) {
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  } else {
    // todo log error
    return;
  }
  gl_context_ = glfwCreateWindow(VIEW_WIDTH, VIEW_HEIGHT, "gpupixel opengl context", NULL, NULL);
  if (!gl_context_) {
    // todo log error
    glfwTerminate();
    return;
  }
  glfwMakeContextCurrent(gl_context_);

  gladLoadGL();
#endif
}

void GPUPixelContext::useAsCurrent() {
    if(!getGLContext()) return ;
  #if defined(GPUPIXEL_IOS)
    if ([EAGLContext currentContext] != _eglContext) {
      [EAGLContext setCurrentContext:_eglContext];
    }
  #elif defined(GPUPIXEL_MAC)
    if ([NSOpenGLContext currentContext] != imageProcessingContext) {
      [imageProcessingContext makeCurrentContext];
    }
#elif defined(GPUPIXEL_ANDROID)
  if (!gl_context_->makeCurrent()) {
    Util::Log("ERROR", "Set Current Context Error!");
  }
#elif defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
   if (glfwGetCurrentContext() != gl_context_) {
    glfwMakeContextCurrent(gl_context_);
  }
#endif
}

void GPUPixelContext::presentBufferForDisplay() {
#if defined(GPUPIXEL_IOS)
  [_eglContext presentRenderbuffer:GL_RENDERBUFFER];
#elif defined(GPUPIXEL_MAC)

#endif
}

void GPUPixelContext::releaseContext() {
#if defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
  if (gl_context_) {
    glfwDestroyWindow(gl_context_);
  }
  glfwTerminate();
#elif defined(GPUPIXEL_ANDROID)
  if (gl_context_ != nullptr) {
    delete gl_context_;
    gl_context_ = nullptr;
  }
#endif
}

void GPUPixelContext::step() {
  task_queue_->processOne();
}

void GPUPixelContext::runSync(std::function<void(void)> func) {
#if defined(GPUPIXEL_WIN) || defined(GPUPIXEL_LINUX)
  if (isCurrentThread()) {
    useAsCurrent();
    func();
    return ;
  }
  std::promise<void> promise;
  auto future = promise.get_future();
  task_queue_->add([&]() {
      useAsCurrent();
      func();
      promise.set_value();
  });
  future.wait();
#else
  useAsCurrent();
  func();
#endif  
}

void GPUPixelContext::runAsync(std::function<void(void)> func) {
  task_queue_->add([=]() {
      useAsCurrent();
      func();
  });
}

NS_GPUPIXEL_END
