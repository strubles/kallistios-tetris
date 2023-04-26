//https://dreamcast.wiki/PowerVR_Introduction

// resolution is 640w x 480h


#include <kos.h>

#include <plx/matrix.h>
#include <plx/prim.h>

#include "vmu_img.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FIELD_HEIGHT 400 // 20 blocks x 20 pixels each
#define FIELD_WIDTH 200 // 10 blocks x 20 pixels each

typedef enum Color_Id {
	EMPTY = 0,
	RED = 1,
	ORANGE = 2,
	YELLOW = 3,
	GREEN = 4,
	LIGHT_BLUE = 5,
	DARK_BLUE = 6,
	PURPLE = 7
} color_id;

typedef struct Color {
	uint8 a;
	uint8 r;
	uint8 g;
	uint8 b;
} color;

int field_left = (SCREEN_WIDTH/2) - (FIELD_WIDTH/2);
int field_right = (SCREEN_WIDTH/2) + (FIELD_WIDTH/2);
int field_top = (SCREEN_HEIGHT/2) - (FIELD_HEIGHT/2);
int field_bottom = (SCREEN_HEIGHT/2) + (FIELD_HEIGHT/2);


color COLOR_RED = {255, 255, 0, 0};
color COLOR_ORANGE = {255, 255, 128, 0};
color COLOR_YELLOW = {255, 255, 255, 0};
color COLOR_GREEN = {255, 0, 255, 0};
color COLOR_LIGHT_BLUE = {255, 0, 255, 255};
color COLOR_DARK_BLUE = {255, 0, 0, 255};
color COLOR_PURPLE = {255, 255, 0, 255};
color COLOR_WHITE = {255, 255, 255, 255};
color COLOR_BLACK = {255, 0, 0, 0};

//setting up the field data structure
color_id field[20][10] = {EMPTY}; //20 rows, 10 columns

KOS_INIT_FLAGS(INIT_DEFAULT);

float position_x = SCREEN_WIDTH/2;
float position_y = SCREEN_HEIGHT/2;

color get_argb_from_enum(color_id id){
	switch(id){
		case RED:
			return COLOR_RED;
		case ORANGE:
			return COLOR_ORANGE;
		case YELLOW:
			return COLOR_YELLOW;
		case GREEN:
			return COLOR_GREEN;
		case LIGHT_BLUE:
			return COLOR_LIGHT_BLUE;
		case DARK_BLUE:
			return COLOR_DARK_BLUE;
		case PURPLE:
			return COLOR_PURPLE;
		default:
			printf("Get_argb_from_enum provided with invalid enum\n");
			return COLOR_WHITE;
	}
}

void init(){
	pvr_init_defaults();

	pvr_set_bg_color(1.0,0.5,0.2);
}

void draw_triangle(float x1, float y1,
				   float x2, float y2,
				   float x3, float y3,
				   color argb)
				   {
	// POINTS SUBMITTED MUST BE IN CLOCKWISE ORDER
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);
	pvr_prim(&hdr, sizeof(hdr));

	vert.flags = PVR_CMD_VERTEX;
	vert.x = x1;
	vert.y = y1;
	vert.z = 5.0f;
	vert.u = 0;
	vert.v = 0;
	vert.argb = PVR_PACK_COLOR(argb.a/255, argb.r/255, argb.g/255, argb.b/255);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	vert.x = x2;
	vert.y = y2;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL; //tells PVR that this will be the final point
	vert.x = x3;
	vert.y = y3;
	pvr_prim(&vert, sizeof(vert));
}

void draw_square(float left, float right, float top, float bottom, color argb) {
	pvr_poly_hdr_t hdr;
	pvr_poly_cxt_t cxt;
	pvr_vertex_t vert;

	// x1 = left
	// x2 = right
	// y1 = top
	// y2 = bottom

	if(top>bottom) {
		//printf("Warning: draw_square received a 'top' paramater that's greater than 'bottom', swapping\n");
		float swap_y;
		swap_y = top;
		top = bottom;
		bottom = swap_y;
	}

	if(left>right) {
		//printf("Warning: draw_square received a 'left' paramater that's greater than 'right', swapping\n");
		float swap_x;
		swap_x = left;
		left = right;
		right = swap_x;
	}

	pvr_poly_cxt_col(&cxt, PVR_LIST_TR_POLY);
	pvr_poly_compile(&hdr, &cxt);

	pvr_prim(&hdr, sizeof(hdr));
	vert.flags = PVR_CMD_VERTEX;
	// bottom left
	vert.x = left;
	vert.y = bottom;
	vert.z = 5.0f;
	vert.u = 0;
	vert.v = 0;
	vert.argb = PVR_PACK_COLOR(argb.a/255, argb.r/255, argb.g/255, argb.b/255);
	vert.oargb = 0;
	pvr_prim(&vert, sizeof(vert));

	// top left
	vert.y = top;
	pvr_prim(&vert, sizeof(vert));

	// bottom right
	vert.x = right;
	vert.y = bottom;
	pvr_prim(&vert, sizeof(vert));

	vert.flags = PVR_CMD_VERTEX_EOL;
	vert.y = top;
	pvr_prim(&vert, sizeof(vert));
}
void draw_square_centered_on(float center_x, float center_y, float width, float height, color argb) {
	float left = center_x - (width/2);
	float right = center_x + (width/2);
	float top = center_y - (height/2);
	float bottom = center_y + (height/2);

	//printf("Left: %f Right: %f Top: %f Bottom: %f\n",left,right,top,bottom);
	//printf("A: %d R: %d G: %d B: %d\n", argb.a, argb.r, argb.g, argb.b);

	draw_square(left, right, top, bottom, argb);
}

void draw_vert_line(float x, float top, float bottom, color argb) {
	draw_square(x, x+1, top, bottom, argb);
}

void draw_horiz_line(float left, float right, float y, color argb) {
	draw_square(left, right, y, y+1, argb);
}

int check_buttons(){
	maple_device_t *cont;
    cont_state_t *state;

    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

	if(cont) {
		state=(cont_state_t *)maple_dev_status(cont);

		if(!state){
			return 0;
		}
		if(state->buttons & CONT_DPAD_UP){
			position_y -= 1.0f;
		}
		if(state->buttons & CONT_DPAD_DOWN){
			position_y += 1.0f;
		}
		if(state->buttons & CONT_DPAD_LEFT){
			position_x -= 1.0f;
		}
		if(state->buttons & CONT_DPAD_RIGHT){
			position_x += 1.0f;
		}
	}
	return 0;
}

void draw_field(){
	// One block is 20 pixels x 20 pixels
	// it is 20 blocks * 20 pixels tall = 400 pixels
	// and 10 blocks * 20 pixels wide = 200 pixels

	//draw edges
	draw_horiz_line(field_left, field_right, field_top, COLOR_WHITE);
	draw_horiz_line(field_left, field_right, field_bottom, COLOR_WHITE);
	draw_vert_line(field_left, field_top, field_bottom, COLOR_WHITE);
	draw_vert_line(field_right, field_top, field_bottom, COLOR_WHITE);

	//draw each row
	for(int i=20; i<FIELD_HEIGHT; i=i+20){
		draw_horiz_line(field_left, field_right, field_top+i, COLOR_BLACK);
	}

	//draw each column
	for(int j=20; j<FIELD_WIDTH; j=j+20){
		draw_vert_line(field_left+j, field_top, field_bottom, COLOR_BLACK);
	}

	float block_x;
	float block_y;
	//now draw the blocks
	for(int row=0; row<20; row=row+1){
		for(int col=0;col<10; col=col+1){
			if (field[row][col]){
				block_x = field_left + (20*col) + 10;
				block_y = field_top + (20*row) + 10;
				draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_enum(field[row][col]));
			}
		}
	}
}

void draw_frame(){
	check_buttons();

	pvr_wait_ready(); // <-- Prevents those ugly flashes!

	pvr_scene_begin();

	pvr_list_begin(PVR_LIST_OP_POLY);
	//opaque drawing here
	pvr_list_finish();

	pvr_list_begin(PVR_LIST_TR_POLY);
	//translucent drawing here
	
	draw_horiz_line(100, SCREEN_WIDTH-100, 100, COLOR_RED); // red - top one
	draw_vert_line(SCREEN_WIDTH-100, 100, SCREEN_HEIGHT-100, COLOR_GREEN); // green - right one
	draw_horiz_line(100, SCREEN_WIDTH-100, SCREEN_HEIGHT-100, COLOR_LIGHT_BLUE); // blue - bottom
	draw_vert_line(100, SCREEN_HEIGHT-100, 100, COLOR_WHITE); // white - left

	// (0,0) is located at top left

	//draw_triangle(0, 0, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 255, 255, 255, 255);

	//draw_square(100, 200, 100, 200, 255, 100, 100, 100);

	//draw_square_centered_on(150, 150, 30, 30, argb);
	draw_square_centered_on(position_x, position_y, 30, 30, COLOR_BLACK);
	
	draw_field();

	pvr_list_finish();

	pvr_scene_finish();
}

int main(){

	int exitProgram = 0;

	init();
	field[5][5]=LIGHT_BLUE;
	field[6][6]=DARK_BLUE;
	field[0][0]=RED; //top-left
	field[19][9]=PURPLE; //bottom-right

	field[0][5]=GREEN;
	field[5][0]=YELLOW;

	printf("Hello world!\n");
	printf("How are you today? :)\n");

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