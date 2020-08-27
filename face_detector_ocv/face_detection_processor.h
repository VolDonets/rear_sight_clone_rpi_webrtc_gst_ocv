//
// Created by biba_bo on 2020-08-27.
//

#ifndef REAR_SIGHT_CLONE_PROJECT_FACE_DETECTION_PROCESSOR_H
#define REAR_SIGHT_CLONE_PROJECT_FACE_DETECTION_PROCESSOR_H

#define MAX_MATS_LIST_SIZE                  3;

#include <list>
#include <opencv2/core/mat.hpp>

class FaceDetectionProcessor {
private:
    std::list<cv::Mat> queueFrames;
public:
    FaceDetectionProcessor();
    ~FaceDetectionProcessor();

    void add_frame(cv::Mat frame);
    void processRecognition();
    void /*not void I'll patch it*/ getLastDetectedFaces();
};


#endif //REAR_SIGHT_CLONE_PROJECT_FACE_DETECTION_PROCESSOR_H
