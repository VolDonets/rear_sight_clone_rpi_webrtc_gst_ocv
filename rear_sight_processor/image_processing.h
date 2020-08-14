//
// Created by biba_bo on 2020-08-14.
//

#ifndef REAR_SIGHT_CLONE_PROJECT_IMAGE_PROCESSING_H
#define REAR_SIGHT_CLONE_PROJECT_IMAGE_PROCESSING_H

/*This is a video settings for the camera, it's impossible to change dynamically
 * STR_... version is used in configuring pipeline.*/
#define WIDTH                   640
#define HEIGHT                  480
#define STR_WIDTH               "640"
#define STR_HEIGHT              "480"
#define STR_FRAMERATE           "15/1"

/*This defines are used for cropping an image from the current frame
  in future code modifications it can be changed to variables
  ! be careful with frame's WIDTH nad HEIGHT
 * CROPPED_WIDTH - width of cropping
 * CROPPED_HEIGHT - height of cropping
 * CROPPED_X, CROPPED_Y - a coordinates of cropping*/
#define CROPPED_WIDTH_2           200
#define CROPPED_HEIGHT_2          160
#define CROPPED_X_BEGIN           220
#define CROPPED_Y_BEGIN           120

#define ZOOM_STEP                 20
#define MOVE_STEP                 20

/*This defines are used for resizing the current frame into a small image, where:
  in future code modifications it can be changed to variables
  ! be careful with frame's WIDTH nad HEIGHT
 * RESIZE_WIDTH - a width for a small image
 * RESIZE_HEIGHT - a height for a small image*/
#define RESIZE_WIDTH           200
#define RESIZE_HEIGHT          160
#define RESIZE_X               440
#define RESIZE_Y               0


/* This is managing constants for drawing helpful elements on a video frame*/
#define DRAW_CIRCLE_X           320
#define DRAW_CIRCLE_Y           240
#define DRAW_CIRCLE_RADIUS      10
// #define DRAW_LINE_LENGTH     150
#define DRAW_LINE_1B_X          320
#define DRAW_LINE_1B_Y          90
#define DRAW_LINE_1E_X          320
#define DRAW_LINE_1E_Y          230
//
#define DRAW_LINE_2B_X          330
#define DRAW_LINE_2B_Y          240
#define DRAW_LINE_2E_X          480
#define DRAW_LINE_2E_Y          240
//
#define DRAW_LINE_3B_X          320
#define DRAW_LINE_3B_Y          250
#define DRAW_LINE_3E_X          320
#define DRAW_LINE_3E_Y          400
//
#define DRAW_LINE_4B_X          310
#define DRAW_LINE_4B_Y          240
#define DRAW_LINE_4E_X          160
#define DRAW_LINE_4E_Y          240

class FrameParameters{
public:
    int CROPPED_WIDTH = CROPPED_WIDTH_2;
    int CROPPED_HEIGHT = CROPPED_HEIGHT_2;
    int CROPPED_X = CROPPED_X_BEGIN;
    int CROPPED_Y = CROPPED_Y_BEGIN;
};

#endif //REAR_SIGHT_CLONE_PROJECT_IMAGE_PROCESSING_H