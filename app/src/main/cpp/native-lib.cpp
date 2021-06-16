#include <jni.h>
#include <string>
#include <opencv2/core.hpp>
#include "ObjectDetector.h"
#include <android/log.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <opencv2/imgproc.hpp>
#include <android/bitmap.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#define  LOG_TAG    "IAMGROOT-JNI"



namespace {

    constexpr char* RES_RAW_CONFIG_PATH_ENV_VAR = "RES_RAW_CONFIG_PATH";
    constexpr char* RES_CARD_FILE_NAME = "treeo_card.png";
    constexpr char* RES_SAMPLE_FILE_NAME = "tree.jpeg";
    jobject getAssetManagerFromJava(JNIEnv* env, jobject obj);
    cv::Mat readFileFromAsset(JNIEnv* env, jobject obj);
    cv::Mat readSampleImage(JNIEnv* env, jobject obj);
    std::string readFile(std::string filePath);
}

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))


extern "C"
JNIEXPORT jdouble JNICALL
Java_com_lae_iamgroot_MainActivity_measureTree(JNIEnv *env, jobject thiz, jlong mat) {

    cv::Mat cardImageFile = readFileFromAsset(env,thiz);

    ObjectDetector objectDetector =  ObjectDetector(cardImageFile);

    cv::Mat input = *(cv::Mat*)mat;

    double diameter;

    objectDetector.measureTree(input, diameter);

    double _diameter = objectDetector.getDiameterValue();

    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "Diameter Value from CPP = %d", _diameter);

    return _diameter;

}

namespace {

    cv::Mat readFileFromAsset(JNIEnv *env, jobject obj) {

        jobject jam = getAssetManagerFromJava(env, obj);
        cv::Mat h;
        if (jam) {
            AAssetManager *am = AAssetManager_fromJava(env, jam);
            if (am) {
                AAsset *assetFile = AAssetManager_open(am, RES_CARD_FILE_NAME, AASSET_MODE_BUFFER);

                const void *buf = AAsset_getBuffer(assetFile);


                LOGD("%s:\n%s", RES_CARD_FILE_NAME, static_cast<const char *>(buf));

                long sizeOfImg = AAsset_getLength(assetFile);
                char* buffer = (char*) malloc(sizeof(char)*sizeOfImg);
                AAsset_read(assetFile,buffer,sizeOfImg);

                std::vector<char> data(buffer,buffer+sizeOfImg);

                h = cv::imdecode(data,-1);


            }
        }
        return h;
    }

    cv::Mat readSampleImage(JNIEnv *env, jobject obj) {

        jobject jam = getAssetManagerFromJava(env, obj);
        cv::Mat h;
        if (jam) {
            AAssetManager *am = AAssetManager_fromJava(env, jam);
            if (am) {
                AAsset *assetFile = AAssetManager_open(am, RES_SAMPLE_FILE_NAME, AASSET_MODE_BUFFER);

                const void *buf = AAsset_getBuffer(assetFile);


                LOGD("%s:\n%s", RES_SAMPLE_FILE_NAME, static_cast<const char *>(buf));

                long sizeOfImg = AAsset_getLength(assetFile);
                char* buffer = (char*) malloc(sizeof(char)*sizeOfImg);
                AAsset_read(assetFile,buffer,sizeOfImg);

                std::vector<char> data(buffer,buffer+sizeOfImg);

                h = cv::imdecode(data,-1);


            }
        }
        return h;
    }

    jobject getAssetManagerFromJava(JNIEnv *env, jobject obj) {
        jclass clazz = env->GetObjectClass(
                obj); // or env->FindClass("com/example/myapp/MainActivity");
        jmethodID method =
                env->GetMethodID(clazz, "getAssetManager", "()Landroid/content/res/AssetManager;");
        jobject ret = env->CallObjectMethod(obj, method);

        return ret;
    }

    std::string readFile(std::string filePath) {
        std::ifstream ifs(filePath);
        std::stringstream ss;

        ss << ifs.rdbuf();

        return ss.str();
    }

}
extern "C"
JNIEXPORT jint JNICALL
Java_com_lae_iamgroot_MainActivity_checkCard(JNIEnv *env, jobject thiz) {
    // TODO: implement checkCard()

}