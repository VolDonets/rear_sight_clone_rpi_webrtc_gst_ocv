//
// Created by biba_bo on 2020-08-14.
//

#ifndef REAR_SIGHT_CLONE_PROJECT_REAR_SIGHT_PROCESSOR_H
#define REAR_SIGHT_CLONE_PROJECT_REAR_SIGHT_PROCESSOR_H

#define OPERATION_SUCCESSFUL        0
#define MOVE_OP_UNSUCCESS           -1
#define ZOOM_OP_UNSUCCESS           -2

#define MIN_ZOOM_COEFFICIENT        0.1
#define MAX_ZOOM_COEFFICIENT        1.0
#define ZOOMING_STEP_COEFFICIENT    0.02

#include <memory>
#include "image_processing.h"

class RearSightProcessor {
private:
    std::shared_ptr<FrameParameters> frame_param;
    int m_CROPPED_WIDTH;
    int m_CROPPED_HEIGHT;
    int m_CROPPED_X;
    int m_CROPPED_Y;
    double m_CURRENT_ZOOM_SIZE;

public:
    RearSightProcessor(std::shared_ptr<FrameParameters> frame_param);

    int on_zoom_plus_processor();
    int on_zoom_minus_processor();

    int on_move_left_processor();
    int on_move_right_processor();
    int on_move_up_processor();
    int on_move_down_processor();

    int set_new_frame_param();
};


#endif //REAR_SIGHT_CLONE_PROJECT_REAR_SIGHT_PROCESSOR_H
