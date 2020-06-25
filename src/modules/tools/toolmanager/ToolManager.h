/*
      this file is part of smoothie (http://smoothieware.org/). the motion control part is heavily based on grbl (https://github.com/simen/grbl).
      smoothie is free software: you can redistribute it and/or modify it under the terms of the gnu general public license as published by the free software foundation, either version 3 of the license, or (at your option) any later version.
      smoothie is distributed in the hope that it will be useful, but without any warranty; without even the implied warranty of merchantability or fitness for a particular purpose. see the gnu general public license for more details.
      you should have received a copy of the gnu general public license along with smoothie. if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TOOLMANAGER_H
#define TOOLMANAGER_H

using namespace std;
#include <vector>
#include <stdint.h>
#include "StepperMotor.h"

class Tool;

class ToolManager : public Module
{
public:
    ToolManager();

    void on_module_loaded();
    uint16_t *get_active_tool_name();
    const float *get_active_tool_offset();
    void on_gcode_received(void *);
    void on_console_line_received(void *argument);
    void on_get_public_data(void *argument);
    void on_set_public_data(void *argument);
    void add_tool(Tool *tool_to_add);
    Tool * get_tool(unsigned int num);
    void set_tool_offset(unsigned int tool, float offset[3]);
    const float *get_tool_offset(unsigned int tool);
    StepperMotor *get_default_x_stepper() const { return default_x_stepper; }
    unsigned int get_tool_count() const { return this->tools.size(); }

private:
    void change_tool();
    vector<Tool *> tools;
    StepperMotor *default_x_stepper;

    unsigned int next_tool;
    unsigned int active_tool;
    uint16_t current_tool_name;
};



#endif
