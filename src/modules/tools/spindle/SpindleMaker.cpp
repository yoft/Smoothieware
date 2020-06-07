/*
      This file is part of Smoothie (http://smoothieware.org/). The motion control part is heavily based on Grbl (https://github.com/simen/grbl).
      Smoothie is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
      Smoothie is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
      You should have received a copy of the GNU General Public License along with Smoothie. If not, see <http://www.gnu.org/licenses/>.
*/

#include "SpindleMaker.h"
#include "libs/Module.h"
#include "libs/Kernel.h"
#include "SpindleControl.h"
#include "PWMSpindleControl.h"
#include "AnalogSpindleControl.h"
#include "HuanyangSpindleControl.h"
#include "Config.h"
#include "checksumm.h"
#include "ConfigValue.h"
#include "StreamOutputPool.h"
#include "nuts_bolts.h"
#include "ToolManager.h"

#define spindle_checksum                   CHECKSUM("spindle")
#define enable_checksum                    CHECKSUM("enable")
#define spindle_type_checksum              CHECKSUM("type")
#define spindle_vfd_type_checksum          CHECKSUM("vfd_type")
#define spindle_ignore_on_halt_checksum    CHECKSUM("ignore_on_halt")
#define spindle_motor_for_x_axis_checksum  CHECKSUM("motor_for_x_axis")

void SpindleMaker::load_spindle(){

    // If the spindle module is disabled load no Spindle 
    if( !THEKERNEL->config->value( spindle_checksum, enable_checksum  )->by_default(false)->as_bool() ) {
        THEKERNEL->streams->printf("NOTE: Spindle Module is disabled\n");
        return;    
    }
    
    spindle = NULL;

    // get the two config options that make us able to determine which spindle module we need to load
    std::string spindle_type = THEKERNEL->config->value( spindle_checksum, spindle_type_checksum )->by_default("pwm")->as_string();
    std::string vfd_type = THEKERNEL->config->value( spindle_checksum, spindle_vfd_type_checksum )->by_default("none")->as_string();

    // check config which spindle type we need
    if( spindle_type.compare("pwm") == 0 ) {
        spindle = new PWMSpindleControl();
    } else if ( spindle_type.compare("analog") == 0 ) {
        spindle = new AnalogSpindleControl();
    } else if ( spindle_type.compare("modbus") == 0 ) {
        if(vfd_type.compare("huanyang") == 0) { 
            spindle = new HuanyangSpindleControl();
        } else {
            delete spindle;
            THEKERNEL->streams->printf("ERROR: No valid spindle VFD type defined\n");
        }
    } else {
        delete spindle;
        THEKERNEL->streams->printf("ERROR: No valid spindle type defined\n");
    }

    // Add the spindle if we successfully initialized one
    if( spindle != NULL) {

        int axis_num=THEKERNEL->config->value(spindle_checksum, spindle_motor_for_x_axis_checksum)->by_default(ALPHA_STEPPER)->as_int();
        if (axis_num>ALPHA_STEPPER && axis_num<=GAMMA_STEPPER) {
            THEKERNEL->streams->printf("Error: Spindle cannot use axis %d to replace X axis stepper! Must be >%d. Using default X stepper motor.\n", axis_num, GAMMA_STEPPER);
        }else{
            spindle->set_x_axis_stepper(axis_num);
        }

        spindle->register_for_event(ON_GCODE_RECEIVED);
        if (!THEKERNEL->config->value(spindle_checksum, spindle_ignore_on_halt_checksum)->by_default(false)->as_bool()) {
            spindle->register_for_event(ON_HALT);
        }

        THEKERNEL->add_module( spindle );
        THEKERNEL->tool_manager->add_tool( spindle );
    }

}

