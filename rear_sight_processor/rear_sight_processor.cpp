//
// Created by biba_bo on 2020-08-14.
//

#include "rear_sight_processor.h"

RearSightProcessor::RearSightProcessor(std::shared_ptr<FrameParameters> frame_param) {
    this->frame_param = frame_param;
    m_CROPPED_WIDTH = frame_param->CROPPED_WIDTH;
    m_CROPPED_HEIGHT = frame_param->CROPPED_HEIGHT;
    m_CROPPED_X = frame_param->CROPPED_X;
    m_CROPPED_Y = frame_param->CROPPED_Y;
}


int RearSightProcessor::on_zoom_plus_processor() {
    int half_zoom_step = ZOOM_STEP / 2;
    if (m_CROPPED_HEIGHT - ZOOM_STEP <= 0 || m_CROPPED_WIDTH - ZOOM_STEP <= 0)
        return ZOOM_OP_UNSUCCESS;
    m_CROPPED_X += half_zoom_step;
    m_CROPPED_Y += half_zoom_step;
    m_CROPPED_WIDTH -= ZOOM_STEP;
    m_CROPPED_HEIGHT -= ZOOM_STEP;
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::on_zoom_minus_processor() {
    int half_zoom_step = ZOOM_STEP / 2;
    if (m_CROPPED_HEIGHT == HEIGHT || m_CROPPED_WIDTH == WIDTH)
        return ZOOM_OP_UNSUCCESS;
    if (m_CROPPED_X == 0) {
        if (m_CROPPED_Y == 0) {
            m_CROPPED_WIDTH += ZOOM_STEP;
            m_CROPPED_HEIGHT += ZOOM_STEP;
        } else if (m_CROPPED_Y + m_CROPPED_HEIGHT == HEIGHT) {
            m_CROPPED_WIDTH += ZOOM_STEP;
            m_CROPPED_Y -= ZOOM_STEP;
            m_CROPPED_HEIGHT += ZOOM_STEP;
        } else {
            m_CROPPED_WIDTH += ZOOM_STEP;
            m_CROPPED_Y -= half_zoom_step;
            m_CROPPED_HEIGHT += ZOOM_STEP;
        }
    } else if (m_CROPPED_X + m_CROPPED_WIDTH == WIDTH) {
        if (m_CROPPED_Y == 0) {
            m_CROPPED_X -= ZOOM_STEP;
            m_CROPPED_WIDTH += ZOOM_STEP;
            m_CROPPED_HEIGHT += ZOOM_STEP;
        } else if (m_CROPPED_Y + m_CROPPED_HEIGHT == HEIGHT) {
            m_CROPPED_X -= ZOOM_STEP;
            m_CROPPED_Y -= ZOOM_STEP;
            m_CROPPED_WIDTH += ZOOM_STEP;
            m_CROPPED_HEIGHT += ZOOM_STEP;
        } else {
            m_CROPPED_X -= ZOOM_STEP;
            m_CROPPED_Y -= half_zoom_step;
            m_CROPPED_WIDTH += ZOOM_STEP;
            m_CROPPED_HEIGHT += ZOOM_STEP;
        }
    } else if (m_CROPPED_Y == 0) {
        m_CROPPED_X -=half_zoom_step;
        m_CROPPED_HEIGHT += ZOOM_STEP;
        m_CROPPED_WIDTH += ZOOM_STEP;
    } else if (m_CROPPED_Y + m_CROPPED_HEIGHT == HEIGHT) {
        m_CROPPED_X -= half_zoom_step;
        m_CROPPED_Y -= ZOOM_STEP;
        m_CROPPED_HEIGHT += ZOOM_STEP;
        m_CROPPED_WIDTH += ZOOM_STEP;
    } else {
        m_CROPPED_X -= half_zoom_step;
        m_CROPPED_Y -= half_zoom_step;
        m_CROPPED_HEIGHT += ZOOM_STEP;
        m_CROPPED_WIDTH += ZOOM_STEP;
    }
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::on_move_left_processor() {
    if (m_CROPPED_X - MOVE_STEP < 0)
        return MOVE_OP_UNSUCCESS;
    m_CROPPED_X = m_CROPPED_X - MOVE_STEP;
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::on_move_right_processor() {
    if (m_CROPPED_X + m_CROPPED_WIDTH + MOVE_STEP > WIDTH)
        return MOVE_OP_UNSUCCESS;
    m_CROPPED_X = m_CROPPED_X + MOVE_STEP;
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::on_move_up_processor() {
    if (m_CROPPED_Y - MOVE_STEP < 0)
        return MOVE_OP_UNSUCCESS;
    m_CROPPED_Y = m_CROPPED_Y - MOVE_STEP;
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::on_move_down_processor() {
    if (m_CROPPED_Y + m_CROPPED_HEIGHT + MOVE_STEP > HEIGHT)
        return MOVE_OP_UNSUCCESS;
    m_CROPPED_Y = m_CROPPED_Y + MOVE_STEP;
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::set_new_frame_param() {
    frame_param->CROPPED_WIDTH = m_CROPPED_WIDTH;
    frame_param->CROPPED_HEIGHT = m_CROPPED_HEIGHT;
    frame_param->CROPPED_X = m_CROPPED_X;
    frame_param->CROPPED_Y = m_CROPPED_Y;
    return OPERATION_SUCCESSFUL;
}
