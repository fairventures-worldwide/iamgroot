package com.lae.iamgroot.utils;

import android.hardware.Camera;

public class CameraUtils {
    private static Object sAccesLock = new Object();
    private static float[] fov = null;

    public static float[] getCameraFOV(Camera sOpenedCamera) {
        if (fov == null) {
            synchronized (sAccesLock) {
                if (sOpenedCamera != null) {
                    fov = new float[2];
                    fov[0] = sOpenedCamera.getParameters()
                            .getHorizontalViewAngle();
                    fov[1] = sOpenedCamera.getParameters()
                            .getVerticalViewAngle();
                } else {
                    try {
                        Camera camera = Camera.open();
                        fov = new float[2];
                        fov[0] = camera.getParameters()
                                .getHorizontalViewAngle();
                        fov[1] = camera.getParameters()
                                .getVerticalViewAngle();
                        camera.release();
                    } catch (RuntimeException e) {
                        fov = new float[] { 60, 45 };
                    }/*  w w  w.  j a  v  a 2  s. c  o  m*/
                }
            }
        }
        return fov;
    }

}
