#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <Foundation/Foundation.h>

#define GPUImageRotationSwapsWidthAndHeight(rotation) \
  (((rotation) == kGPUImageRotateLeft) ||             \
   ((rotation) == kGPUImageRotateRight) ||            \
   ((rotation) == kGPUImageRotateRightFlipVertical))

typedef enum {
  kGPUImageNoRotation,
  kGPUImageRotateLeft,
  kGPUImageRotateRight,
  kGPUImageFlipVertical,
  kGPUImageFlipHorizonal,
  kGPUImageRotateRightFlipVertical,
  kGPUImageRotateRightFlipHorizontal,
  kGPUImageRotate180
} GPUImageRotationMode;

// Delegate Protocal for Face Detection.
@protocol GPUImageVideoCameraDelegate <NSObject>

@optional
- (void)willOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer;
@end

/**
 A GPUImageOutput that provides frames from either camera
*/
@interface VideoCameraManager
    : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate,
                AVCaptureAudioDataOutputSampleBufferDelegate> {
  NSUInteger numberOfFramesCaptured;
  CGFloat totalFrameTimeDuringCapture;

  AVCaptureSession* _captureSession;
  AVCaptureDevice* _inputCamera;
  AVCaptureDevice* _microphone;
  AVCaptureDeviceInput* videoInput;
  AVCaptureVideoDataOutput* videoOutput;

  BOOL capturePaused;
  GPUImageRotationMode outputRotation;
  dispatch_semaphore_t frameRenderingSemaphore;

  BOOL captureAsYUV;
  GLuint luminanceTexture, chrominanceTexture;

  __unsafe_unretained id<GPUImageVideoCameraDelegate> _delegate;
}

/// The AVCaptureSession used to capture from the camera
@property(readonly, retain, nonatomic) AVCaptureSession* captureSession;

/// This enables the capture session preset to be changed on the fly
@property(readwrite, nonatomic, copy) NSString* captureSessionPreset;

/// This sets the frame rate of the camera (iOS 5 and above only)
/**
 Setting this to 0 or below will set the frame rate back to the default setting
 for a particular preset.
 */
@property(readwrite) NSInteger frameRate;

/// Easy way to tell if front-facing camera is present on device
@property(readonly, getter=isFrontFacingCameraPresent)
    BOOL frontFacingCameraPresent;

/// This enables the benchmarking mode, which logs out instantaneous and average
/// frame times to the console
@property(readwrite, nonatomic) BOOL runBenchmark;

/// Use this property to manage camera settings. Focus point, exposure point,
/// etc.
@property(readonly) AVCaptureDevice* inputCamera;

/// These properties determine whether or not the two camera orientations should
/// be mirrored. By default, both are NO.
@property(readwrite, nonatomic) BOOL horizontallyMirrorFrontFacingCamera,
    horizontallyMirrorRearFacingCamera;

@property(nonatomic, assign) id<GPUImageVideoCameraDelegate> delegate;

/// @name Initialization and teardown

+ (NSArray*)connectedCameraDevices;

/** Begin a capture session

 See AVCaptureSession for acceptable values

 @param sessionPreset Session preset to use
 @param cameraPosition Camera to capture from
 */
- (id)initWithDeviceUniqueID:(NSString*)deviceUniqueID;
- (id)initWithSessionPreset:(NSString*)sessionPreset
             deviceUniqueID:(NSString*)deviceUniqueID;
- (id)initWithSessionPreset:(NSString*)sessionPreset
               cameraDevice:(AVCaptureDevice*)cameraDevice;

/** Tear down the capture session
 */
- (void)removeInputsAndOutputs;

/// @name Manage the camera video stream

/** Start camera capturing
 */
- (void)startCameraCapture;

/** Stop camera capturing
 */
- (void)stopCameraCapture;

/** Pause camera capturing
 */
- (void)pauseCameraCapture;

/** Resume camera capturing
 */
- (void)resumeCameraCapture;

/** Process a video sample
 @param sampleBuffer Buffer to process
 */
- (void)processVideoSampleBuffer:(CMSampleBufferRef)sampleBuffer;

/** Process an audio sample
 @param sampleBuffer Buffer to process
 */
- (void)processAudioSampleBuffer:(CMSampleBufferRef)sampleBuffer;

/** Get the position (front, rear) of the source camera
 */
- (AVCaptureDevicePosition)cameraPosition;

/** Get the AVCaptureConnection of the source camera
 */
- (AVCaptureConnection*)videoCaptureConnection;

/** This flips between the front and rear cameras
 */
- (void)rotateCamera;

/// @name Benchmarking

/** When benchmarking is enabled, this will keep a running average of the time
 * from uploading, processing, and final recording or display
 */
- (CGFloat)averageFrameDurationDuringCapture;

- (void)printSupportedPixelFormats;

@end
