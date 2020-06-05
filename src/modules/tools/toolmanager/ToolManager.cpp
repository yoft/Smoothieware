/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "libs/Module.h"
#include "libs/Kernel.h"
#include "ToolManager.h"
#include "Tool.h"
#include "PublicDataRequest.h"
#include "ToolManagerPublicAccess.h"
#include "Config.h"
#include "Robot.h"
#include "ConfigValue.h"
#include "Conveyor.h"
#include "checksumm.h"
#include "PublicData.h"
#include "Gcode.h"
#include "utils.h"

#include "libs/SerialMessage.h"
#include "libs/StreamOutput.h"
#include "FileStream.h"

#include <math.h>

ToolManager::ToolManager()
{
    active_tool = -1;
    next_tool = -1;
    current_tool_name=0;
    default_x_stepper=THEROBOT->actuators[0];
}

uint16_t *ToolManager::get_active_tool_name() {
    current_tool_name=this->tools[active_tool]->get_name();
    return &current_tool_name;
}

const float *ToolManager::get_active_tool_offset() {
    return this->tools[active_tool]->get_offset();
}

void ToolManager::on_module_loaded()
{

    this->register_for_event(ON_CONSOLE_LINE_RECEIVED);
    this->register_for_event(ON_GCODE_RECEIVED);
    this->register_for_event(ON_GET_PUBLIC_DATA);
    this->register_for_event(ON_SET_PUBLIC_DATA);
}

void ToolManager::on_gcode_received(void *argument)
{
    Gcode *gcode = static_cast<Gcode*>(argument);

    if( gcode->has_letter('T') ) {
        int new_tool = gcode->get_value('T');
        if(new_tool >= (int)this->tools.size() || new_tool < 0) {
            // invalid tool
            char buf[32]; // should be big enough for any status
            int n = snprintf(buf, sizeof(buf), "T%d invalid tool ", new_tool);
            gcode->txt_after_ok.append(buf, n);

        } else {
            this->next_tool=new_tool;
        }
    }
    if (gcode->has_m && gcode->m==6) {
        if(this->next_tool >= (int)this->tools.size() || this->next_tool < 0) {
            // invalid tool
            char buf[32]; // should be big enough for any status
            int n = snprintf(buf, sizeof(buf), "T%d invalid tool ", this->next_tool);
            gcode->txt_after_ok.append(buf, n);

        }else if (this->next_tool != this->active_tool) {
            this->change_tool();
        }
    }
}

void ToolManager::on_console_line_received( void *argument )
{
    if(THEKERNEL->is_halted()) return; // if in halted state ignore any commands

    SerialMessage *msgp = static_cast<SerialMessage *>(argument);
    string possible_command = msgp->message;

    // ignore anything that is not lowercase or a letter
    if(possible_command.empty() || !islower(possible_command[0]) || !isalpha(possible_command[0])) {
        return;
    }

    string cmd = shift_parameter(possible_command);

    // Act depending on command
    if (cmd == "tools") {
        msgp->stream->printf("%d tools defined:\n", (int)this->tools.size());
        for(int i=0;i<(int)this->tools.size();i++) {
            msgp->stream->printf("%d: %d%s\n",i, this->tools[i]->get_name(),(i==this->active_tool)?"*":"");
        }
    }
}

void ToolManager::on_get_public_data(void* argument)
{
    PublicDataRequest* pdr = static_cast<PublicDataRequest*>(argument);

    if(!pdr->starts_with(tool_manager_checksum)) return;

    if(pdr->second_element_is(is_active_tool_checksum)) {

        // check that we control the given tool
        bool managed = false;
        for(auto t : tools) {
            uint16_t n = t->get_name();
            if(pdr->third_element_is(n)) {
                managed = true;
                break;
            }
        }

        // we are not managing this tool so do not answer
        if(!managed) return;

        pdr->set_data_ptr(this->get_active_tool_name());
        pdr->set_taken();

    }else if(pdr->second_element_is(get_active_tool_checksum)) {
        pdr->set_data_ptr(&this->active_tool);
        pdr->set_taken();
    }
}

void ToolManager::on_set_public_data(void* argument)
{
    PublicDataRequest* pdr = static_cast<PublicDataRequest*>(argument);

    if(!pdr->starts_with(tool_manager_checksum)) return;

    // ok this is targeted at us, so change tools
    //uint16_t tool_name= *static_cast<float*>(pdr->get_data_ptr());
    // TODO: fire a tool change gcode
    //pdr->set_taken();
}

// Add a tool to the tool list
void ToolManager::add_tool(Tool* tool_to_add)
{
    if(this->tools.size() == 0) {
        tool_to_add->select();
        //send new_tool_offsets to robot
        THEROBOT->setToolOffset(this->get_active_tool_offset());
    } else {
        tool_to_add->deselect();
    }
    this->tools.push_back( tool_to_add );
}

// Add a tool to the tool list
void ToolManager::change_tool()
{
    // We must wait for an empty queue before we can disable the current tool
    THEKERNEL->conveyor->wait_for_idle();
    this->tools[active_tool]->deselect();

    THEKERNEL->conveyor->wait_for_idle();
    this->active_tool = this->next_tool;

    //send new_tool_offsets to robot
    THEROBOT->setToolOffset(this->get_active_tool_offset());
    THEROBOT->actuators[0] = (this->tools[active_tool]->get_x_axis_stepper()!=NULL)?this->tools[active_tool]->get_x_axis_stepper():this->get_default_x_stepper();

    this->tools[active_tool]->select();
    THEKERNEL->conveyor->wait_for_idle();


}



