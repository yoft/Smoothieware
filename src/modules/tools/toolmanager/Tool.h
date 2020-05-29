/*
      this file is part of smoothie (http://smoothieware.org/). the motion control part is heavily based on grbl (https://github.com/simen/grbl).
      smoothie is free software: you can redistribute it and/or modify it under the terms of the gnu general public license as published by the free software foundation, either version 3 of the license, or (at your option) any later version.
      smoothie is distributed in the hope that it will be useful, but without any warranty; without even the implied warranty of merchantability or fitness for a particular purpose. see the gnu general public license for more details.
      you should have received a copy of the gnu general public license along with smoothie. if not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Module.h"
#include "Pin.h"
//#include "libs/StepperMotor.h"

#include <stdint.h>

class StepperMotor;

class Tool : public Module
{
public:
    Tool(){};
    virtual ~Tool() {};

    virtual void select()= 0;
    virtual void deselect()= 0;
    virtual bool is_selected()= 0;
    virtual const float *get_offset() const { return offset; }
    virtual uint16_t get_name() const { return identifier; }
    virtual StepperMotor *get_x_axis_stepper() const { return x_stepper; }
    virtual void set_x_axis_stepper(StepperMotor *new_x_stepper) { x_stepper=new_x_stepper; }

protected:
    float offset[3];
    uint16_t identifier;
    StepperMotor *x_stepper=NULL;
};

