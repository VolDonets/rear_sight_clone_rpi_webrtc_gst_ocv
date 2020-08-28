//
// Created by biba_bo on 2020-08-27.
//

#include "face_detection_processor.h"

FaceDetectionProcessor::FaceDetectionProcessor() {
    mutexProc.lock();
}

FaceDetectionProcessor::~FaceDetectionProcessor() {

}

void FaceDetectionProcessor::add_frame(cv::Mat frame) {
    mutexRes.lock();
    if (queueFrames.size() > MAX_MATS_LIST_SIZE) {
        queueFrames.pop_front();
        queueFrames.push_back(frame);
    } else
        queueFrames.push_back(frame);
    mutexProc.unlock();
    mutexRes.unlock();
}

void FaceDetectionProcessor::processRecognition() {
    recognitionProcessThread = std::thread([this](){
        while (true) {
            mutexProc.lock();
            mutexRes.lock();
            cv::Mat currentFrame, greyFrame;
            currentFrame = queueFrames.front();
            queueFrames.pop_front();
            mutexRes.unlock();
            cv::cvtColor(currentFrame, greyFrame, cv::COLOR_BGR2GRAY);
            cv::resize(greyFrame, currentFrame, cv::Size(), 1/SCALING_IMG, 1/SCALING_IMG, cv::INTER_LINEAR);
            cv::equalizeHist(currentFrame, currentFrame);
            cascadeClassifier.detectMultiScale(smallIm)
        }
    });
}

std::vector<cv::Rect> *FaceDetectionProcessor::getLastDetectedFaces() {
    return nullptr;
}
