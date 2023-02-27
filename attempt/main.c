
#include <kos.h>

#include <plx/matrix.h>
#include <plx/prim.h>

#include "vmu_img.h"

#include <stdbool.h>

//initialize KOS
KOS_INIT_FLAGS(INIT_DEFAULT);

//initialize romdisk
extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);


bool exitProgram = false;

pvr_poly_hdr_t nontexturedHeader;

vector_t verts[4] = {
	{ -1, 1, 0, 1},
	{ -1, -1, 0, 1},
	{ 1, 1, 0, 1},
	{ 1, -1, 0, 1}
};

//pvr_ptr_t smile;

void init(){

	//init VMU image
	maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
	vmu_draw_lcd(vmu, vmu_carl);

	//init pvr (PowerVR graphics chip)
	pvr_init_defaults();
	plx_mat3d_init(); //init the Parralax library

	// compile a polygon header (information about the polygon, ie
	// whether it should be textured) before giving it your vertices
	pvr_poly_cxt_t nontexturedContext; //like a human readable version of the header

	// setup the context, giving it a pointer to put the data and the poly list type
	pvr_poly_cxt_col(&nontexturedContext, PVR_LIST_OP_POLY);

	nontexturedContext.gen.culling = PVR_CULLING_CW;

	pvr_poly_compile(&nontexturedHeader, &nontexturedContext);
	// ^ turns it from the human readable context to the real header for the graphics chip

	pvr_set_bg_color(1.0, 0.5, 0.2);

	//setup the camera
	plx_mat3d_mode(PLX_MAT_PROJECTION); //projection matrix
	plx_mat3d_identity(); //resetting it back to 0 (no transformation)

	plx_mat3d_perspective(60.0f, 640.0f / 480.0f, 0.1f, 100.0f); // camera "lens"
	// field of view, aspect ratio, near plane, far plane (depth of field)

	//actually position camera
	plx_mat3d_mode(PLX_MAT_MODELVIEW);
	plx_mat3d_identity();

	point_t cameraPosition = { 0.0f, 0.0f, 5.0f, 1.0f };
	// with this setup:
	// 0,0 is center of screen
	// pos Y is up, pos X is right, pos Z is out of screen

	//positioned at 0,0 , 5 units away
	//last value indicates that this is describing a point and not a direction

	
	point_t cameraTarget = { 0.0f, 0.0f, 0.0f, 1.0f }; //which point the camera should look at

	vector_t cameraUp = { 0.0f, 1.0f, 0.0f, 0.0f }; //positive Y is "up"

	plx_mat3d_lookat(&cameraPosition, &cameraTarget, &cameraUp);

	//smile = pvr_mem_malloc(25 * 25 * 2);
	//png_to_texture("/rd/smile.png", smile, PNG_NO_ALPHA);
}

void update(){
	plx_mat_identity();
	plx_mat3d_apply_all(); //apply camera settings

	vector_t transformedVerts[4];
	plx_mat_transform(verts, transformedVerts, 4, 4*sizeof(float));
	// input: verts, output: transformedVerts, # of verts, size of each vert


	// wait for the pvr to accept a frame
	pvr_wait_ready();

	pvr_scene_begin(); //start of one frame

	pvr_list_begin(PVR_LIST_OP_POLY); //opaque polygon mode

	pvr_prim(&nontexturedHeader, sizeof(pvr_poly_hdr_t)); // send header to pvr chip to tell it how we're gonna draw it...
	//now let's tell it what to draw

	
	// triangle
	/*
	plx_vert_fnp(PVR_CMD_VERTEX, -1, 1, 0, 1, 1, 1, 1); //top left, white
	// x, y, z, alpha, r, g, b
	plx_vert_fnp(PVR_CMD_VERTEX, -1, -1, 0, 1, 1, 0, 0); //bottom left, red

	plx_vert_fnp(PVR_CMD_VERTEX, 1, 1, 0, 1, 0, 1, 1); //top right, green
	// second triangle
	plx_vert_fnp(PVR_CMD_VERTEX_EOL, 1, -1, 0, 1, 0, 0, 1); //bottom right, blue
	// ^ EOL tells it that it's the last vertex in the list
	*/

	plx_vert_fnp(PVR_CMD_VERTEX, transformedVerts[0].x, transformedVerts[0].y, transformedVerts[0].z, 1, 1, 1, 1); //top left, white
	// x, y, z, alpha, r, g, b
	plx_vert_fnp(PVR_CMD_VERTEX, transformedVerts[1].x, transformedVerts[1].y, transformedVerts[1].z, 1, 1, 0, 0); //bottom left, red

	plx_vert_fnp(PVR_CMD_VERTEX, transformedVerts[2].x, transformedVerts[2].y, transformedVerts[2].z, 1, 0, 1, 1); //top right, green
	// second triangle
	plx_vert_fnp(PVR_CMD_VERTEX_EOL, transformedVerts[3].x, transformedVerts[3].y, transformedVerts[3].z, 1, 0, 0, 1); //bottom right, blue





	pvr_list_finish();

	pvr_scene_finish(); //end of one frame
}

void cleanup(){
	maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
	vmu_draw_lcd(vmu, vmu_clear);

	pvr_shutdown();
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