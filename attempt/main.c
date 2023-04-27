//https://dreamcast.wiki/PowerVR_Introduction

// resolution is 640w x 480h


#include <kos.h>

#include <plx/matrix.h>
#include <plx/prim.h>

#include <stdlib.h>

#include "vmu_img.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FIELD_HEIGHT 400 // 20 blocks x 20 pixels each
#define FIELD_WIDTH 200 // 10 blocks x 20 pixels each

typedef enum Color_Id {
	EMPTY = 0,
	RED = 1, // Z
	ORANGE = 2, // L
	YELLOW = 3, // O
	GREEN = 4, // S
	LIGHT_BLUE = 5, // I
	DARK_BLUE = 6, // J
	PURPLE = 7 // T
} color_id;

typedef struct Color {
	uint8 a;
	uint8 r;
	uint8 g;
	uint8 b;
} color;

typedef enum Rotation {
	DEFAULT,
	RIGHT,
	TWO,
	LEFT
} rotation;

typedef struct Tetrodata {
	int top_y;
	int left_x;
	int bottom_y;
	int right_x;
	color_id type;
	int dimensions;
	rotation orientation;
} tetrodata;

int field_left = (SCREEN_WIDTH/2) - (FIELD_WIDTH/2);
int field_right = (SCREEN_WIDTH/2) + (FIELD_WIDTH/2);
int field_top = (SCREEN_HEIGHT/2) - (FIELD_HEIGHT/2);
int field_bottom = (SCREEN_HEIGHT/2) + (FIELD_HEIGHT/2);

color_id TETRO_Z[3][3] = {
	{ 1, 1, 0 },
	{ 0, 1, 1 },
	{ 0, 0, 0 }
};
color_id TETRO_L[3][3] = {
	{ 0, 0, 2 },
	{ 2, 2, 2 },
	{ 0, 0, 0 }
};
color_id TETRO_O[2][2] = {
	{ 3, 3 },
	{ 3, 3 }
};
color_id TETRO_S[3][3] = {
	{ 0, 4, 4 },
	{ 4, 4, 0 },
	{ 0, 0, 0 }
};
color_id TETRO_I[4][4] = {
	{ 0, 0, 0, 0 },
	{ 5, 5, 5, 5 },
	{ 0, 0, 0, 0 },
	{ 0, 0, 0, 0 }
};
color_id TETRO_J[3][3] = {
	{ 6, 0, 0 },
	{ 6, 6, 6 },
	{ 0, 0, 0 }
};
color_id TETRO_T[3][3] = {
	{ 0, 7, 0 },
	{ 7, 7, 7 },
	{ 0, 0, 0 }
};

color_id tetro_dummy_4x4[4][4] = {0};
color_id tetro_dummy_3x3[3][3] = {0};
color_id tetro_dummy_2x2[2][2] = {0};

int fall_timer = 200;

int has_drawn_new_tetro = 0;
/*
int active_tetro_left;
int active_tetro_top;
int active_tetro_rotation;
int active_tetro_x_offset;
int active_tetro_y_offset;
color_id active_tetro_type;
*/

color COLOR_RED = {255, 255, 0, 0};
color COLOR_ORANGE = {255, 255, 174, 94};
color COLOR_YELLOW = {255, 255, 255, 0};
color COLOR_GREEN = {255, 0, 255, 0};
color COLOR_LIGHT_BLUE = {255, 0, 255, 255};
color COLOR_DARK_BLUE = {255, 0, 0, 255};
color COLOR_PURPLE = {255, 255, 0, 255};
color COLOR_WHITE = {255, 255, 255, 255};
color COLOR_BLACK = {255, 0, 0, 0};

//setting up the field data structure
color_id field[23][10] = {EMPTY}; //23 rows, 10 columns

// an array of 23 arrays, each with 10 items in it
color_id temp_field[23][10] = {EMPTY};

KOS_INIT_FLAGS(INIT_DEFAULT);

float position_x = SCREEN_WIDTH/2;
float position_y = SCREEN_HEIGHT/2;

tetrodata active_tetro;

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
			printf("Get_argb_from_enum provided with invalid enum: %d\n",id);
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

	// top right
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

void init_new_tetro(color_id id){

	active_tetro.orientation=DEFAULT;
	active_tetro.type=id;

	if(id==LIGHT_BLUE){
		// reset the I tetromino holder to its initial state
		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				tetro_dummy_4x4[i][j] = TETRO_I[i][j];
			}
		}
		//now configure the correct initial settings in the active tetro struct
		active_tetro.left_x=3;
		active_tetro.top_y=2 + 5;
		active_tetro.dimensions=4;
	}
	else if(id==YELLOW){
		for(int i=0; i<2; i++){
			for(int j=0; j<2; j++){
				tetro_dummy_2x2[i][j] = TETRO_O[i][j];
			}
		}

		active_tetro.left_x = 4;
		active_tetro.top_y=3 + 5;
		active_tetro.dimensions=2;
	}
	else {
		active_tetro.dimensions=3;
		active_tetro.left_x=3;
		active_tetro.top_y=3 + 5;

		if(id==RED){ //Z			
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_Z[i][j];
				}
			}
		}
		else if(id==ORANGE){ //L
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_L[i][j];
				}
			}
		}
		else if(id==GREEN){ //S
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_S[i][j];
				}
			}
		}

		else if(id==DARK_BLUE){ //J
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_J[i][j];
				}
			}
		}

		else if(id==PURPLE){ //T
			printf("Loading T tetro...\n");
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_T[i][j];
					//printf("%d ",tetro_dummy_3x3[i][j]);
				}
				//printf("\n");
			}
		}

		else{
			printf("ERROR: invalid tetro id provided to init_tetro(): %d\n",id);
			exit(1);
		}
	}
}

void replot_active_tetro(){
	// Adds the current active_tetro to the temp field (using data from the
	// associated tetro_dummy array)

	memset(temp_field, EMPTY, sizeof(temp_field));

	if(active_tetro.dimensions==2){
		for(int row=0; row<2; row++){
			for(int cell=0; cell<2; cell++){
				if(active_tetro.top_y+row<23 && active_tetro.left_x+cell<10){ //don't go off the grid
					temp_field[active_tetro.top_y+row][active_tetro.left_x+cell] = tetro_dummy_2x2[row][cell];
				}
			}
		}
		// use tetro_dummy_2x2
		//tetro_dummy_2x2
	}
	else if (active_tetro.dimensions==4){
		// use tetro_Dummy_4x4
		for(int row=0; row<4; row++){
			for(int cell=0; cell<4; cell++){
				if(active_tetro.top_y+row<23 && active_tetro.left_x+cell<10){ //don't go off the grid
					temp_field[active_tetro.top_y+row][active_tetro.left_x+cell] = tetro_dummy_4x4[row][cell];
				}
			}
		}
	}
	else if (active_tetro.dimensions==3){
		// use tetro_dummy_3x3
		//printf("Active Tetro type: %d\n",active_tetro.type);
		for(int row=0; row<3; row++){
			for(int cell=0; cell<3; cell++){
				if(active_tetro.top_y+row<23 && active_tetro.left_x+cell<10){ //don't go off the grid
					temp_field[active_tetro.top_y+row][active_tetro.left_x+cell] = tetro_dummy_3x3[row][cell];
				}
				//printf("%d ",tetro_dummy_3x3[row][cell]);
			}
			//printf("\n");
		}
	}
	else{
		printf("ERROR: replot_active_tetro() called with invalid dimension: %d\n",active_tetro.dimensions);
		exit(1);
	}
}

void commit_tetro(){
	// The active tetromino gets copied from the active tetromino array to the main field
	// data structure to "set" it.
	// THIS DOES NOT DO CHECKS to validate position!

	for(int row=0; row<23; row=row+1){
		for(int cell=0; cell<10; cell=cell+1){
			if(temp_field[row][cell]>0){
				field[row][cell]=temp_field[row][cell];
			}
		}
	}

}

int num=1;

int check_valid_state(){
	//Checks for overlapping tiles between active tetromino and field
	//Returns 0 (false) if an overlap is found (state is invalid)
	//Returns 1 (true) if an overlap is NOT found (state is valid)
	for(int row=0; row<23; row++){
		for(int cell=0; cell<10; cell++){
			if(field[row][cell]!=0 && temp_field[row][cell]!=0){
				return 0;
			}
		}
	}
	return 1;
}

void tetro_fall(){
	active_tetro.top_y+=1;
}

void generate_new_tetro(){
	//color_id random_id = (rand() % 7)+1;
	//printf("Random color: %d\n",random_id);

	//init_new_tetro(random_id);
	init_new_tetro(num);
	if(num>=7){
		num=1;
	}
	else{
		num++;
	}

	replot_active_tetro();
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
	for(int row=3; row<23; row=row+1){
		for(int col=0;col<10; col=col+1){
			if (field[row][col]){
				block_x = field_left + (20*col) + 10;
				block_y = field_top + (20*(row-3)) + 10;
				draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_enum(field[row][col]));
			}
			if (temp_field[row][col]){
				block_x = field_left + (20*col) + 10;
				block_y = field_top + (20*(row-3)) + 10;
				draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_enum(temp_field[row][col]));
			}
		}
	}
}

//void move_active_tetro_downwards

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

	fall_timer=fall_timer-1;
	//printf("%d\n",fall_timer);
	if(fall_timer<=0){ //&& has_drawn_new_tetro==0){
		fall_timer=200;
		//add_tetro_to_temp_field(1);
		printf("Generating new tetro...\n");
		generate_new_tetro();
		
		//has_drawn_new_tetro=1;
	}
	//else if(fall_timer<=0 && has_drawn_new_tetro==1){
	//	fall_timer=200;
	//}

	// (0,0) is located at top left

	//draw_triangle(0, 0, SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 255, 255, 255, 255);

	//draw_square(100, 200, 100, 200, 255, 100, 100, 100);

	//draw_square_centered_on(150, 150, 30, 30, argb);
	//draw_square_centered_on(position_x, position_y, 30, 30, COLOR_BLACK);
	
	draw_field();

	pvr_list_finish();

	pvr_scene_finish();
}

int main(){

	int exitProgram = 0;

	init();
	/*
	field[8][5]=LIGHT_BLUE;
	field[9][6]=DARK_BLUE;
	field[3][0]=RED; //top-left
	field[22][9]=PURPLE; //bottom-right
	field[19][9]=RED;
	field[20][9]=RED;
	field[21][9]=RED;

	field[3][5]=GREEN;
	field[8][0]=YELLOW;
	*/

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