//
// Created by biba_bo on 2020-08-27.
//

#include "face_detection_processor.h"

FaceDetectionProcessor::FaceDetectionProcessor() {
    mutexProc.lock();
    processRecognition();
    cascadeClassifier.load("../haarcascade_frontalface_alt_tree.xml");
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
            facesCoords = new std::vector<cv::Rect>();
            cascadeClassifier.detectMultiScale(currentFrame, *facesCoords, 1.1, 2, 0 | cv::CASCADE_SCALE_IMAGE, cv::Size(30, 30));
            std::cout << "aaa\n";
            for (cv::Rect rect: *facesCoords) {
                std::cout << "x: " << rect.x << ", y: " << rect.y << "\n";
            }
            if (!queueFrames.empty())
                mutexProc.unlock();
            if (queueFrames.size() == -100)
                return ;
        }
    });
}

std::vector<cv::Rect> *FaceDetectionProcessor::getLastDetectedFaces() {
    return facesCoords;
}
