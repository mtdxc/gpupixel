/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

package com.pixpark.gpupixel;

public class GPUPixelTargetRawDataOutput implements GPUPixelTarget {
    public interface RawOutputCallback {
        public void onBytes(byte[] bytes, int width, int height, long ts);
    }

    protected long mNativeClassID = 0;
    public long getNativeClassID() { return mNativeClassID; }

    public GPUPixelTargetRawDataOutput() {
        if (mNativeClassID != 0) return;
        GPUPixel.getInstance().runOnDraw(new Runnable() {
            @Override
            public void run() {
                mNativeClassID = nativeNew();
                if (i420_cb_!=null) {
                    nativeSetI420Callback(mNativeClassID, i420_cb_);
                }
                if (pixels_cb_!=null) {
                    nativeSetPixelsCallback(mNativeClassID, pixels_cb_);
                }
            }
        });
    }

    public void destroy() {
        destroy(true);
    }

    public void destroy(boolean onGLThread) {
        if (mNativeClassID != 0) {
            if (onGLThread) {
                GPUPixel.getInstance().runOnDraw(new Runnable() {
                    @Override
                    public void run() {
                        if (mNativeClassID != 0) {
                            nativeDestroy(mNativeClassID);
                            mNativeClassID = 0;
                        }
                    }
                });
            } else {
                nativeDestroy(mNativeClassID);
                mNativeClassID = 0;
            }
        }
    }

    public static native long nativeNew();
    public static native void nativeDestroy(final long clsId);
    public static native void nativeSetI420Callback(final long clsId, RawOutputCallback cb);
    public static native void nativeSetPixelsCallback(final long clsId, RawOutputCallback cb);

    RawOutputCallback i420_cb_, pixels_cb_;
    public void setI420Callbck(RawOutputCallback cb) {
        i420_cb_ = cb;
        if (mNativeClassID != 0){
            nativeSetI420Callback(mNativeClassID, cb);
        }
    }
    public void setPixelsCallback(RawOutputCallback cb) {
        pixels_cb_ = cb;
        if (mNativeClassID!=0) {
            nativeSetPixelsCallback(mNativeClassID, cb);
        }
    }
}
