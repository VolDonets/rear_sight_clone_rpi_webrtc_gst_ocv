//
// Created by biba_bo on 2020-08-17.
//

#include <iostream>
#include "rear_sight_processor.h"

RearSightProcessor::RearSightProcessor(std::shared_ptr<FrameParameters> frame_param) {
    this->frame_param = frame_param;
    m_CROPPED_WIDTH = frame_param->CROPPED_WIDTH;
    m_CROPPED_HEIGHT = frame_param->CROPPED_HEIGHT;
    m_CROPPED_X = frame_param->CROPPED_X;
    m_CROPPED_Y = frame_param->CROPPED_Y;
    m_CURRENT_ZOOM_SIZE = ( ((double)m_CROPPED_HEIGHT / (double)HEIGHT) + ((double)m_CROPPED_WIDTH / (double)WIDTH) ) / 2;
}

int RearSightProcessor::on_zoom_plus_processor() {
    if (m_CURRENT_ZOOM_SIZE == MIN_ZOOM_COEFFICIENT)
        return ZOOM_OP_UNSUCCESS;
    if (m_CURRENT_ZOOM_SIZE - ZOOMING_STEP_COEFFICIENT < MIN_ZOOM_COEFFICIENT)
        m_CURRENT_ZOOM_SIZE = MIN_ZOOM_COEFFICIENT;
    else
        m_CURRENT_ZOOM_SIZE -= ZOOMING_STEP_COEFFICIENT;
    int n_CROPPED_WIDTH = m_CURRENT_ZOOM_SIZE * WIDTH;
    int n_CROPPED_HEIGHT = m_CURRENT_ZOOM_SIZE * HEIGHT;
    m_CROPPED_X += (m_CROPPED_WIDTH -  n_CROPPED_WIDTH) / 2;
    m_CROPPED_Y += (m_CROPPED_HEIGHT - n_CROPPED_HEIGHT) / 2;
    m_CROPPED_WIDTH = n_CROPPED_WIDTH;
    m_CROPPED_HEIGHT = n_CROPPED_HEIGHT;
    return OPERATION_SUCCESSFUL;
}

int RearSightProcessor::on_zoom_minus_processor() {
    if (m_CURRENT_ZOOM_SIZE == MAX_ZOOM_COEFFICIENT)
        return ZOOM_OP_UNSUCCESS;
    if (m_CURRENT_ZOOM_SIZE + ZOOMING_STEP_COEFFICIENT > MAX_ZOOM_COEFFICIENT) {
        m_CURRENT_ZOOM_SIZE = MAX_ZOOM_COEFFICIENT;
        m_CROPPED_X = 0;
        m_CROPPED_Y = 0;
        m_CROPPED_WIDTH = WIDTH;
        m_CROPPED_HEIGHT = HEIGHT;
    } else {
        m_CURRENT_ZOOM_SIZE += ZOOMING_STEP_COEFFICIENT;
        int n_CROPPED_WIDTH = m_CURRENT_ZOOM_SIZE * WIDTH;
        int n_CROPPED_HEIGHT = m_CURRENT_ZOOM_SIZE * HEIGHT;
        int diff_WIDTH = n_CROPPED_WIDTH - m_CROPPED_WIDTH;
        int diff_HEIGHT = n_CROPPED_HEIGHT - m_CROPPED_HEIGHT;

        if (m_CROPPED_X == 0) {
            m_CROPPED_WIDTH = n_CROPPED_WIDTH;
        } else if (m_CROPPED_X + m_CROPPED_WIDTH == WIDTH) {
            m_CROPPED_X -= diff_WIDTH;
            m_CROPPED_WIDTH = n_CROPPED_WIDTH;
        } else {
            if (m_CROPPED_X - (diff_WIDTH / 2) < 0) {
                m_CROPPED_X = 0;
                m_CROPPED_WIDTH = n_CROPPED_WIDTH;
            } else if (m_CROPPED_X + n_CROPPED_WIDTH > WIDTH) {
                m_CROPPED_X = WIDTH - n_CROPPED_WIDTH;
                m_CROPPED_WIDTH = n_CROPPED_WIDTH;
            } else {
                m_CROPPED_X -= (diff_WIDTH / 2);
                m_CROPPED_WIDTH = n_CROPPED_WIDTH;
            }
        }

        if (m_CROPPED_Y == 0) {
            m_CROPPED_HEIGHT = n_CROPPED_HEIGHT;
        } else if (m_CROPPED_Y + m_CROPPED_HEIGHT == HEIGHT) {
            m_CROPPED_Y -= diff_HEIGHT;
            m_CROPPED_HEIGHT = n_CROPPED_HEIGHT;
        } else {
            if (m_CROPPED_Y - (diff_HEIGHT / 2) < 0) {
                m_CROPPED_Y = 0;
                m_CROPPED_HEIGHT = n_CROPPED_HEIGHT;
            } else if (m_CROPPED_Y + n_CROPPED_HEIGHT > HEIGHT) {
                m_CROPPED_Y = HEIGHT - n_CROPPED_HEIGHT;
                m_CROPPED_HEIGHT = n_CROPPED_HEIGHT;
            } else {
                m_CROPPED_Y -= (diff_HEIGHT / 2);
                m_CROPPED_HEIGHT = n_CROPPED_HEIGHT;
            }
        }

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
