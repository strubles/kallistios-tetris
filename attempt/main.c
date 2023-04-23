#include <kos.h>

#include <plx/matrix.h>
#include <plx/prim.h>

KOS_INIT_FLAGS(INIT_DEFAULT);

void init(){
	pvr_init_defaults();

	pvr_set_bg_color(1.0,0.5,0.2);
}

void draw_horizontal_line(float x1, float x2, float y, uint8 a, uint8 r, uint8 g, uint8 b){
	// Draws a 1-pixel wide horziontal line from x1 to x2 at height y
	// with color determined by rgba

	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;

	// make sure x2 is larger than x1
	float swap_x;
	if (x1 > x2) {
		swap_x = x1;
		x1 = x2;
		x2 = swap_x;
	}
	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);

	pvr_prim(&hdr, sizeof(hdr));

	vert.flags = PVR_CMD_VERTEX;
	vert.x=x1;
	vert.y=y+1;
	vert.z=5.0f;
	vert.u=0;
	vert.v=0;
	vert.argb=PVR_PACK_COLOR(a/255, r/255, g/255, b/255);
	vert.oargb=0;
	pvr_prim(&vert, sizeof(vert));

	vert.y=y;
	pvr_prim(&vert, sizeof(vert));

	vert.x=x2;
	vert.y=y+1;
	pvr_prim(&vert, sizeof(vert));

	vert.flags=PVR_CMD_VERTEX_EOL;
	vert.y = y;
	pvr_prim(&vert, sizeof(vert));
}

void draw_frame(){

	pvr_scene_begin();

	pvr_list_begin(PVR_LIST_OP_POLY);
	//opaque drawing here
	pvr_list_finish();

	pvr_list_begin(PVR_LIST_TR_POLY);
	//translucent drawing here
	draw_horizontal_line(200.0,300.0,200.0,255,255,0,255);

	pvr_list_finish();

	pvr_scene_finish();
}


int main(){

	int exitProgram = 0;

	init();

	printf("Hello world!\n");

	while(!exitProgram){
		maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		cont_state_t *controllerState = (cont_state_t*) maple_dev_status(controller);

		//check controller for START button press
		
		if (controllerState->buttons & CONT_START){
			exitProgram = 1;
		}
		
		draw_frame();
	}

	pvr_shutdown();

}