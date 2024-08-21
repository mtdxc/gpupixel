/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

package com.pixpark.gpupixel;

import android.graphics.Bitmap;
import java.nio.ByteBuffer;

public class GPUPixelSourceRawInput extends GPUPixelSource {
    public GPUPixelSourceRawInput() {
        if (mNativeClassID != 0) return;
        GPUPixel.getInstance().runOnDraw(new Runnable() {
            @Override
            public void run() {
                mNativeClassID = GPUPixel.nativeSourceRawInputNew();
            }
        });
    }

    public void SetRotation(int rotation)
    {
        GPUPixel.nativeSourceRawInputSetRotation(mNativeClassID, rotation);
    }

    public void uploadBytes(final int[] pixels, int width, int height, int stride) {
        GPUPixel.getInstance().runOnDraw(new Runnable() {
            @Override
            public void run() {
                GPUPixel.nativeSourceRawInputUploadBytes(mNativeClassID, pixels, width, height, stride);
            }
        });
        proceed(true, true);
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
                            GPUPixel.nativeSourceRawInputDestroy(mNativeClassID);
                            mNativeClassID = 0;
                        }
                    }
                });
            } else {
                GPUPixel.nativeSourceRawInputDestroy(mNativeClassID);
                mNativeClassID = 0;
            }
        }
    }
}
