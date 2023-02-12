#include <kos.h>
#include "vmu_img.h"
#include <stdbool.h>

//initialize KOS
//KOS_INIT_FLAGS(INIT_DEFUALT);

//initialize romdisk
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);


bool exitProgram = false;

//pvr_ptr_t smile;

void init(){

	//init VMU image
	maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
	vmu_draw_lcd(vmu, vmu_carl);

	//init pvr
	pvr_init_defaults();

	pvr_set_bg_color(1.0, 0.5, 0.2);

	//smile = pvr_mem_malloc(25 * 25 * 2);
	//png_to_texture("/rd/smile.png", smile, PNG_NO_ALPHA);
}

void update(){

	// wait for the pvr to accept a frame
	pvr_wait_ready();

	pvr_scene_begin(); //start of one frame

	pvr_list_begin(PVR_LIST_OP_POLY); //opaque polygon mode

	pvr_list_finish();

	pvr_scene_finish(); //end of one frame
}

void cleanup(){
	maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
	vmu_draw_lcd(vmu, vmu_clear);
}

int main(void){

	init();

	
	while (!exitProgram){
		maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		cont_state_t *controllerState = (cont_state_t*) maple_dev_status(controller);

		//check controller for START button press
		
		if (controllerState->buttons & CONT_START){
			exitProgram = true;
		}
		

		//MAPLE_FOREACH_BEGIN(MAPLE_FUNC_CONTROLLER,)
		update();
		
	}

	cleanup();

	return 0;

}