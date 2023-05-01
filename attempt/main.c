//https://dreamcast.wiki/PowerVR_Introduction

// resolution is 640w x 480h

// FOR FONTS: REFER TO:
// C:\Data\kos-examples\parallax\font\font.c


// RUN ON DREAMCAST:
// (after running 'make' and attaching bus 2-2 to the WSL instance)
// sudo /opt/toolchains/dc/bin/dc-tool-ser -x attempt.elf -t /dev/ttyUSB0 -c /mnt/c/Data/Projects/attempt/romdisk

#include <kos.h>

//#include <plx/matrix.h>
//#include <plx/prim.h>

#include <stdlib.h>

#include "vmu_img.h"
#include "display.c"

// font stuff
#include <plx/font.h>
#include <plx/sprite.h>
#include <plx/list.h>
#include <plx/dr.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define FIELD_HEIGHT 400 // 20 blocks x 20 pixels each
#define FIELD_WIDTH 200 // 10 blocks x 20 pixels each

plx_font_t * fnt;
plx_fcxt_t * fnt_cxt;
point_t w;


// When a tetromino is rotated, Tetris does a series of tests to find a valid (open) position to rotate the tetromino into.
// The tests are done in order and the first test that succeeds determines where the tetromino is placed.
// The test sets for Z, S, L, J, and T are all the same, but I has its own (O doesn't have one cause it doesn't rotate).
// If they all fail, the rotation is cancelled.
// The first test is a simple in place 90 degree rotation.
// Tests 2-5 involve nudging the tetromino left, right, up, and down by a block or two to find a free spot.
// This 4-dimensional matrix represents the (x, y) offsets of each test.
// IMPORTANT: Because the tests are done IN ORDER, each pair of (x,y) represents the position RELATIVE TO THE LAST TEST,
// NOT relative to the first test or the original position!
// The last inner array (undo) represents the offset to get from test 5 back to the first test position, which is needed to cancel the rotation
// when all tests fail.
// First level: the tetromino type
// Second level: the rotation type
// Third level: the test number
// Fourth level: x, y offset
int rotation_tests_cw[2][4][5][2] =
{
	{ // This test matrix applies to Red/Z, Green/S, Orange/L, Dark Blue/J, Purple/T tetros. (Everything but Light Blue/I )
		// Test 2, 3, 4, 5, and undo.
		{ {-1,0}, {0,-1}, {1,3}, {-1,0}, {1,-2} }, // 0 to R
		{ {1,0}, {0,1}, {-1,-3}, {1,0}, {-1,2} }, // R to 2
		{ {1,0}, {0,-1}, {-1,3}, {1,0}, {-1,-2} }, // 2 to L
		{ {-1,0}, {0,1}, {1,-3}, {-1,0}, {1,2} } // L to 0
	},
	{ // This test matrix applies only to Light Blue/I tetrominos
		{ {-2,0}, {3,0}, {-3,1}, {3,-3}, {-1,2} }, // 0 to R
		{ {-1,0}, {3,0}, {-3,-2}, {3,3}, {-2,-1} }, // R to 2
		{ {2,0}, {-3,0}, {3,-1}, {-3,3}, {1,-2} }, // 2 to L
		{ {1,0}, {-3,0}, {3,2}, {-3,-3}, {2,1} } // L to 0
	},
};

int rotation_tests_ccw[2][4][5][2] =
{
	{ //J, L, S, T, Z
		{ {1, 0}, {0, -1}, {-1,3}, {1,0}, {-1,-2} }, //0 to L
		{ {1, 0}, {0, 1}, {-1,-3}, {1,0}, {-1,2} }, // R to 0
		{ {-1, 0}, {0, -1}, {1, 3}, {-1, 0}, {1,-2} }, //2 to R
		{ {-1, 0}, {0, 1}, {1,-3}, {-1,0}, {1,2} } //L to 2
	},
	{ //Light Blue
		{ {-1, 0}, {3, 0}, {-3,-2}, {3,-3}, {-2,-1} }, //0 to L
		{ {2, 0}, {-3, 0}, {3,-1}, {-3,3}, {1,-2} }, // R to 0
		{ {1, 0}, {-3, 0}, {3, 2}, {-3,-3}, {2,1} }, //2 to R
		{ {-2, 0}, {3, 0}, {-3, 1}, {3, -3}, {-1,2} } //L to 2
	}
};

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
	color_id type;
	int dimensions;
	rotation orientation;
	int set;
	int tetro_index;
} tetrodata;

int field_left = (SCREEN_WIDTH/2) - (FIELD_WIDTH/2);
int field_right = (SCREEN_WIDTH/2) + (FIELD_WIDTH/2);
int field_top = (SCREEN_HEIGHT/2) - (FIELD_HEIGHT/2);
int field_bottom = (SCREEN_HEIGHT/2) + (FIELD_HEIGHT/2);

// Arrays representing what each tetromino looks like.
// These are intended to be read-only and are copied over to the dummy arrays
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

// Arrays for holding the real current tetromino.
// When a new tetromino is spawned, the relevant tetro array (TETRO_Z, TETRO_L, etc)
// is copied into it.
// Then, these "dummy" arrays can be rotated, without effecting how the next
// tetromino of the same type will spawn in.
// There are 3 of them because while most tetrominos go in the 3x3 one, the I needs 4x4 and
// the O needs 2x2.
color_id tetro_dummy_4x4[4][4] = {0};
color_id tetro_dummy_3x3[3][3] = {0};
color_id tetro_dummy_2x2[2][2] = {0};


int has_drawn_new_tetro = 0;

int line_clears = 0;
long int score = 0;
int level = 1;
int falltime = 93;

color COLOR_RED = {255, 255, 0, 0};
color COLOR_ORANGE = {255, 255, 174, 94};
color COLOR_YELLOW = {255, 255, 255, 0};
color COLOR_GREEN = {255, 0, 255, 0};
color COLOR_LIGHT_BLUE = {255, 0, 255, 255};
color COLOR_DARK_BLUE = {255, 0, 0, 255};
color COLOR_PURPLE = {255, 255, 0, 255};
color COLOR_WHITE = {255, 255, 255, 255};
color COLOR_BLACK = {255, 0, 0, 0};

//setting up the field data structure.
color_id field[24][12] = {
//    0       1  2  3  4  5  6  7  8  9  10      11
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 0
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 1
	{ 1, /**/ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /**/ 1}, // 2
	/**********************************************/ //Top boundary of visible area
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 3
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 4
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 5
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 6
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 7
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 8
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 9
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 10
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 11
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 12
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 13
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 14
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 15
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 16
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 17
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 18
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 19
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 20
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 21
	{ 1, /**/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, /**/ 1}, // 22
	/**********************************************/ //Bottom boundary of visible area
	{ 1, /**/ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, /**/ 1}, // 23
//        ^--- Left boundary of vis. area     ^--- Right boundary of visible area
};
// The border of 1's is interpreted as tetromino blocks (red ones to be exact) but are not
// rendered to the screen and are ignored when checking for filled lines. The purpose of
// this is so I can prevent tetrominos from going off the screen in the same exact way that I
// prevent tetrominos from overlapping/intersecting with other tetrominos.

// an array of 23 arrays, each with 12 items in it
color_id temp_field[24][12] = {EMPTY};
// This variable an empty version of the field matrix. It is where the current active tetromino is put.
// It is rendered to the screen directly on top of the main field matrix. When a tetromino is set,
// it is copied over to the main field matrix in the same location, and the temp_field matrix is
// cleared out for the next tetromino.

extern uint8 romdisk[];
KOS_INIT_FLAGS(INIT_DEFAULT);
KOS_INIT_ROMDISK(romdisk);

float position_x = SCREEN_WIDTH/2;
float position_y = SCREEN_HEIGHT/2;

tetrodata active_tetro;
// This holds data on the current active tetromino. It is continuously overwritten with the next
// tetromino as the old tetromino get committed to the field matrix and doesn't need to be kept
// track of anymore.

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

	maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
	vmu_draw_lcd(vmu, vmu_carl);

	//plx_font_t * fnt = plx_font_load("/rd/axaxax.txf");
	plx_font_t * fnt = plx_font_load("/pc/typewriter.txf");

	fnt_cxt = plx_fcxt_create(fnt, PVR_LIST_TR_POLY);

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

void init_new_tetro(color_id id){
	// This function takes a color_id (which also dictates the type of tetromino) and
	// populates the active_tetro variable with data representing a newly spawned
	// tetromino of that type.
	// It also fills the relevant "dummy" tetromino array with a fresh copy of that tetromino's
	// data array (TETRO_I, TETRO_J, etc)

	active_tetro.orientation=DEFAULT;
	active_tetro.type=id;
	active_tetro.set=0;

	if(id==LIGHT_BLUE){
		// reset the I tetromino holder to its initial state
		for(int i=0; i<4; i++){
			for(int j=0; j<4; j++){
				tetro_dummy_4x4[i][j] = TETRO_I[i][j];
			}
		}
		//now configure the correct initial settings in the active tetro struct
		active_tetro.left_x=4;
		active_tetro.top_y=2;
		active_tetro.dimensions=4;
		active_tetro.tetro_index=1;
	}
	else if(id==YELLOW){
		for(int i=0; i<2; i++){
			for(int j=0; j<2; j++){
				tetro_dummy_2x2[i][j] = TETRO_O[i][j];
			}
		}

		active_tetro.left_x = 5;
		active_tetro.top_y=3;
		active_tetro.dimensions=2;
		active_tetro.tetro_index=-1;
	}
	else {
		active_tetro.dimensions=3;
		active_tetro.left_x=4;
		active_tetro.top_y=3;

		if(id==RED){ //Z			
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_Z[i][j];
				}
			}
			active_tetro.tetro_index=0;
		}
		else if(id==ORANGE){ //L
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_L[i][j];
				}
			}
			active_tetro.tetro_index=0;
		}
		else if(id==GREEN){ //S
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_S[i][j];
				}
			}
			active_tetro.tetro_index=0;
		}

		else if(id==DARK_BLUE){ //J
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_J[i][j];
				}
			}
			active_tetro.tetro_index=0;
		}

		else if(id==PURPLE){ //T
			//printf("Loading T tetro...\n");
			for(int i=0; i<3; i++){
				for(int j=0; j<3; j++){
					tetro_dummy_3x3[i][j] = TETRO_T[i][j];
					//printf("%d ",tetro_dummy_3x3[i][j]);
				}
				//printf("\n");
			}
			active_tetro.tetro_index=0;
		}

		else{
			printf("ERROR: invalid tetro id provided to init_tetro(): %d\n",id);
			exit(1);
		}
	}
}

void replot_active_tetro(){
	// Adds active_tetro to the temp_field matrix, in the position and
	// orientation specified by the data members in active_tetro.
	// You have to do this before you check the validity of the fields.

	memset(temp_field, EMPTY, sizeof(temp_field));

	if(active_tetro.dimensions==2){
		// use tetro_dummy_2x2
		for(int row=0; row<2; row++){
			for(int cell=0; cell<2; cell++){
				if(active_tetro.top_y+row<24 && active_tetro.left_x+cell<12){ //don't go off the grid
					temp_field[active_tetro.top_y+row][active_tetro.left_x+cell] = tetro_dummy_2x2[row][cell];
				}
			}
		}
	}
	else if (active_tetro.dimensions==4){
		// use tetro_Dummy_4x4
		for(int row=0; row<4; row++){
			for(int cell=0; cell<4; cell++){
				if(active_tetro.top_y+row<24 && active_tetro.left_x+cell<12){ //don't go off the grid
					temp_field[active_tetro.top_y+row][active_tetro.left_x+cell] = tetro_dummy_4x4[row][cell];
				}
			}
		}
	}
	else if (active_tetro.dimensions==3){
		// use tetro_dummy_3x3
		for(int row=0; row<3; row++){
			for(int cell=0; cell<3; cell++){
				if(active_tetro.top_y+row<24 && active_tetro.left_x+cell<12){ //don't go off the grid
					temp_field[active_tetro.top_y+row][active_tetro.left_x+cell] = tetro_dummy_3x3[row][cell];
				}
			}
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
	// THIS DOES NOT DO CHECKS to validate position! Check it first with check_valid_state()

	for(int row=0; row<24; row=row+1){
		for(int cell=0; cell<12; cell=cell+1){
			if(temp_field[row][cell]>0){
				field[row][cell]=temp_field[row][cell];
			}
		}
	}

}

int check_valid_state(){
	// Checks for overlapping tiles/blocks between the temp_field (the active tetromino) and
	// the field (all other tetrominos and the edges of the screen).
	// Returns 0 (false) if an overlap is found (state is invalid).
	// Returns 1 (true) if an overlap is NOT found (state is valid).
	// It does not undo it, it just determines if it's valid.

	for(int row=0; row<24; row++){
		for(int cell=0; cell<12; cell++){
			if(field[row][cell]!=0 && temp_field[row][cell]!=0){
				return 0;
			}
		}
	}
	return 1;
}

void tetro_left(){
	active_tetro.left_x -= 1;
	replot_active_tetro();
	if(!check_valid_state()){
		active_tetro.left_x += 1; //undo it
		replot_active_tetro();
	}
}

void tetro_right(){
	active_tetro.left_x += 1;
	replot_active_tetro();
	if(!check_valid_state()){
		active_tetro.left_x -= 1; //undo it
		replot_active_tetro();
	}
}

void tetro_fall(int award_score){
	active_tetro.top_y += 1;
	replot_active_tetro();
	if(!check_valid_state()){
		active_tetro.top_y -= 1; //undo it
		replot_active_tetro();
		commit_tetro();
		active_tetro.set=1;
	}
	if(award_score){
		score+=1;
	}
}

void hard_drop(){
	// Falls straight down until the tetromino gets set
	int blocks_fallen=0;
	while(!active_tetro.set){
		tetro_fall(0);
		blocks_fallen++;
	}
	score = score + (blocks_fallen * 2);
}

// https://stackoverflow.com/questions/27288694/transpose-of-a-matrix-2d-array

void print_tetro_array(){
	int n = active_tetro.dimensions;

	color_id (*arr_ptr)[n][n];

	if(n==3){
		arr_ptr = &(tetro_dummy_3x3);
	}
	else if(n==4){
		arr_ptr = &(tetro_dummy_4x4);
	}
	else if(n==2){
		arr_ptr = &(tetro_dummy_2x2);
	}

	for(int i=0; i<n; i++){
		for(int j=0; j<n; j++){
			printf("%d",(*arr_ptr)[i][j]);
		}
	printf("\n");
	}

}

void swap(color_id* arg1, color_id* arg2)
// Swap the two values in memory (in place)
// Used in tetromino rotation
{
    color_id buffer = *arg1;
    *arg1 = *arg2;
    *arg2 = buffer;
}

void transpose_active_tetro(){
	// Flips the array of the active tetromino along the diagonal (in place)
	// This is one of the steps in tetromino rotation

	int n = active_tetro.dimensions;
	for( int i = 0; i < n; i++)
	{
		for ( int j = i+1; j < n; j++ ) // only the upper is iterated
		{
			if(active_tetro.dimensions==3){
				swap(&(tetro_dummy_3x3[i][j]), &(tetro_dummy_3x3[j][i]));}
			else if(active_tetro.dimensions==4){
				swap(&(tetro_dummy_4x4[i][j]), &(tetro_dummy_4x4[j][i]));}
		}
	}
}


void reverse_active_tetro_row(int row){
	// Reverse the given row in the active tetromino array (in place)
	// This is one of the steps in tetromino rotation

	int n = active_tetro.dimensions;

	color_id (*arr_ptr)[n]; //pointer to this row in memory

	if(n==3){
		arr_ptr = &(tetro_dummy_3x3[row]); //point to the row in the 3x3 array
	}
	else if(n==4){
		arr_ptr = &(tetro_dummy_4x4[row]); //point to the row in the 4x4 array
	}
	else{
		printf("ERROR: reverse_active_tetro_row called with an array size other than 3 or 4: %d\n",n);
		exit(1);
	}

	// Reverse the row in the relevant array
	for(int i = 0; i<n/2; i++)
	{
		int temp = (*arr_ptr)[i];
		(*arr_ptr)[i] = (*arr_ptr)[n-i-1];
		(*arr_ptr)[n-i-1] = temp;        
	}
}

void rotate_tetro_counterclockwise();
// ^ tell compiler that this function will exist so it doesnt get mad when I call it before i define it.

void update_orientation_cw(){
	active_tetro.orientation++;
	if(active_tetro.orientation>3){
		active_tetro.orientation=0;
	}
}

void update_orientation_ccw(){
	active_tetro.orientation--;
	if(active_tetro.orientation<0){
		active_tetro.orientation=3;
	}
}

void move_tetro_like_this(int x, int y){
	active_tetro.left_x+=x;
	active_tetro.top_y+=y;
}

// Tetromino rotation test cases taken from here:
// https://www.reddit.com/r/Tetris/comments/bdu02w/i_made_some_srs_charts/

void rotate_tetro_clockwise(){

	if(active_tetro.dimensions==2){
		return; // O tetrominos don't rotate :)
	}

	//test 1 - Plain rotation, no offset.

	// 1. Transpose
	transpose_active_tetro();

	// 2. Reverse each row
	for(int i=0; i<active_tetro.dimensions; i++){
		reverse_active_tetro_row(i);
	}

	replot_active_tetro();

	if(check_valid_state()){
		update_orientation_cw();
		return;
	}

	// If the basic tetro rotation failed, we will start to iterate through the tests,
	// each test involves translating the tetromino in different ways.

	int rotation_type_index = active_tetro.orientation;

	// iterate through each test
	for (int test_index=0; test_index<=3; test_index++){
		//printf("Running rotation test: %d\n",test_index);
		move_tetro_like_this(rotation_tests_cw[active_tetro.tetro_index][rotation_type_index][test_index][0],
							 rotation_tests_cw[active_tetro.tetro_index][rotation_type_index][test_index][1]);
		replot_active_tetro();

		if(check_valid_state()){
			//printf("This rotation test worked!\n");
			return;
		}
	}

	//Never found a valid one, undo
	// (The 4th index in the rotation test inner array is movement instructions to get back to the original location)
	//printf("Never found a valid rotation, undoing it\n");
	move_tetro_like_this(rotation_tests_cw[active_tetro.tetro_index][rotation_type_index][4][0],
						 rotation_tests_cw[active_tetro.tetro_index][rotation_type_index][4][1]);
	rotate_tetro_counterclockwise();
	replot_active_tetro();

}

void rotate_tetro_counterclockwise(){
	//printf("Rotate CCW called\n");
	//printf("Array before CCW:\n");
	//print_tetro_array();

	if(active_tetro.dimensions==2){
		return;
	}

	// Test 1 - plain rotation, no offset

	// 1. Reverse each row
	for(int i=0; i<active_tetro.dimensions; i++){
		reverse_active_tetro_row(i);
	}
	// 2. Transpose
	transpose_active_tetro();

	replot_active_tetro();

	if(check_valid_state()){
		update_orientation_ccw();
		return;
	}
	
	// Iterate through each test
	for (int test_index=0; test_index<=3; test_index++){
		move_tetro_like_this(rotation_tests_ccw[active_tetro.tetro_index][active_tetro.orientation][test_index][0],
							 rotation_tests_ccw[active_tetro.tetro_index][active_tetro.orientation][test_index][1]);
		replot_active_tetro();

		if(check_valid_state()){
			return;
		}
	}

	//Never found a valid one, undo
	move_tetro_like_this(rotation_tests_cw[active_tetro.tetro_index][active_tetro.orientation][4][0],
						 rotation_tests_cw[active_tetro.tetro_index][active_tetro.orientation][4][1]);
	rotate_tetro_clockwise();
	replot_active_tetro();

	//printf("Array after CCW:\n");
	//print_tetro_array();
}


int move_timebuffer = 10;
int released_y_button=1;
int released_x_button=1;

int move_tetromino(){

	// https://cadcdev.sourceforge.net/docs/kos-2.0.0/group__controller__buttons.html

	maple_device_t *cont;
    cont_state_t *state;

    cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);

	if(cont) {
		state=(cont_state_t *)maple_dev_status(cont);

		if(!state){
			return 0;
		}
		if(move_timebuffer==0){
			if(state->buttons & CONT_DPAD_UP){
				hard_drop();
				move_timebuffer=10;
			}
			if(state->buttons & CONT_DPAD_DOWN){
				// softdrop
				tetro_fall(1);
				move_timebuffer=10;
			}
			if(state->buttons & CONT_DPAD_LEFT){
				tetro_left();
				move_timebuffer=10;
			}
			if(state->buttons & CONT_DPAD_RIGHT){
				tetro_right();
				move_timebuffer=10;
			}
			
			if( (state->buttons & CONT_Y) && released_y_button){
				rotate_tetro_clockwise();
				released_y_button=0;
			}
			if( (state->buttons & CONT_X) && released_x_button){
				rotate_tetro_counterclockwise();
				released_x_button=0;
			}
			if( !(state->buttons & CONT_X) && !released_x_button){
				released_x_button=1;
			}
			if( !(state->buttons & CONT_Y) && !released_y_button){
				released_y_button=1;
			}
			
		}
		if(move_timebuffer>0){
			move_timebuffer-=1;
		}

	}
	return 0;
}

void generate_new_tetro(){
	color_id random_id = (rand() % 7)+1;

	init_new_tetro(random_id);
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
		for(int col=1;col<11; col=col+1){
			if (field[row][col]){
				block_x = field_left + (20*(col-1)) + 10;
				block_y = field_top + (20*(row-3)) + 10;
				draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_enum(field[row][col]));
			}
			if (temp_field[row][col]){
				block_x = field_left + (20*(col-1)) + 10;
				block_y = field_top + (20*(row-3)) + 10;
				draw_square_centered_on(block_x, block_y, 20, 20, get_argb_from_enum(temp_field[row][col]));
			}
		}
	}
}

void clear_line(int rownum){
	// Copy every line above the cleared line down 1 row
	for(int row=rownum; row>=4; row--){
		for(int cell=1; cell<=10; cell++){
			field[row][cell] = field[row-1][cell];
		}
	}

	field[3][0]=1;
	for(int cell=1; cell<=10; cell++){
		field[3][cell]=0;
	}
	field[3][11]=1;
}

void check_lines(){
	//printf("Checking lines...\n");
	int found_empty_tile=0;
	
	int new_line_clears=0;

	for(int row=3; row<=22; row++){
		int cell=1;
		while(cell<=10 && !found_empty_tile){
			//printf("%d ", field[row][cell]);
			if(!field[row][cell]){
				found_empty_tile=1;
			}
			cell++;
			//printf("Row: %d Cell: %d\n",row, cell);
		}
		//printf("\n");
		if(!found_empty_tile){
			//printf("Found full line: %d\n",row);
			clear_line(row);
			line_clears++;
			new_line_clears++;
			printf("Total line clears: %d\n",line_clears);
		}
		found_empty_tile=0;
	}
	if(new_line_clears==1){
		score = score + (level * 100);
	}
	else if(new_line_clears==2){
		score = score + (level * 300);
	}
	else if(new_line_clears==3){
		score = score + (level * 500);
	}
	else if(new_line_clears==4){
		printf("Tetris!");
		score = score + (level * 800);
	}

	if(level < 15){
		int new_level = ((line_clears-(line_clears%10))/10)+1;
		if(new_level>level){
			level=new_level;
			//falltime=85-(new_level*5);
			falltime= 93 - (new_level*6);
		}
	}
	// FALL TIME CALCULATION:
	// Level 1 - falls every 87 frames
	// Level 15 - falls every 2 frames
	// Equation:  y = -6x + 93
	// falltime = -6*level + 93
}



void draw_text(float x, float y, char * text){
	//w.x = 30.0f;
	//w.y = 50.0f;
	w.x = x;
	w.y = y;
	w.z = 5.0f;

	plx_fcxt_begin(fnt_cxt);
	plx_fcxt_setpos_pnt(fnt_cxt, &w);
	//plx_fcxt_draw(fnt_cxt, "This is a test!");
	plx_fcxt_draw(fnt_cxt, text);
	//plx_spr_inp(256, 256, 320, 240, 20, 0xffffffff);    // texture test
	plx_fcxt_end(fnt_cxt);
}

char score_string[10];
char lines_string[10];
char level_string[10];

void draw_hud(){
	draw_text(50,300,"Score");
	// draw score
	sprintf(score_string, "%ld", score);
	draw_text(50,340,score_string);

	draw_text(500,200,"Level");
	sprintf(level_string, "%d", level);
	draw_text(500,240,level_string);

	draw_text(500,300,"Lines");
	sprintf(lines_string, "%d", line_clears);
	draw_text(500,340,lines_string);
}

//void move_active_tetro_downwards

int fall_timer = 93;
int first_run=1;

void draw_frame(){

	//check_buttons();
	move_tetromino();

	fall_timer=fall_timer-1;

	if(active_tetro.set==1 || first_run==1){
		check_lines();
		generate_new_tetro();
		fall_timer=falltime;
		first_run=0;
		//printf("Generating new tetro...");
	}

	if(fall_timer<=0){
		fall_timer=falltime;
		tetro_fall(0);
	}


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
	
	draw_field();

	//draw_text(100,100,"What's going on?");
	//draw_text(200,200,"Hello there!");
	draw_hud();

	pvr_list_finish();

	pvr_scene_finish();
}

int main(){

	int exitProgram = 0;

	init();

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

	maple_device_t *vmu = maple_enum_type(0, MAPLE_FUNC_LCD);
	vmu_draw_lcd(vmu, vmu_clear);
	pvr_shutdown();

}