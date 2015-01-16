#pragma once

/////////////////////////////////////////////////////
//Definitions for the block sizes

#define BLOCK_XMAX 100
#define BLOCK_YMAX 100

#define BLOCKSIZE_X 24
#define BLOCKSIZE_Y 20

#define BLOCKDELTA_X 13
#define BLOCKDELTA_Y 20

#define BLOCK_AREA_X 12
#define BLOCK_AREA_Y 60

#define BLOCK_AREA_EDGE_X 12
#define BLOCK_AREA_EDGE_Y 12 

///////////////////////////////////////////////////
//Definitions for the frames

#define FRAME_INNER_OFFSET_X 2
#define FRAME_INNER_OFFSET_Y 3


///////////////////////////////////////////////////
//Definitions for the NewGame button

#define BUTTONWIDTH     24
#define BUTTONHEIGHT    24
#define BUTTONEDGE_TOP  15

///////////////////////////////////////////////////
//Definitions for the two timers

#define DIGITWIDTH      13
#define DIGITHEIGHT     23
#define DIGITCOUNT      3

#define DIGITEDGE_LEFT  15
#define DIGITEDGE_RIGHT 15
#define DIGITEDGE_TOP   15

////////////////////////////////////////////////////
//Definitions for the game modes

#define MODE_EASY_MAPX 11
#define MODE_EASY_MAPY 10
#define MODE_EASY_MINE 10

#define MODE_NORMAL_MAPX 21
#define MODE_NORMAL_MAPY 15
#define MODE_NORMAL_MINE 50

#define MODE_HARD_MAPX 41
#define MODE_HARD_MAPY 15
#define MODE_HARD_MINE 99

#define MAPX_MIN       MODE_EASY_MAPX
#define MAPX_MAX       MODE_HARD_MAPX
#define MAPY_MIN       MODE_EASY_MAPY
#define MAPY_MAX       MODE_HARD_MAPY
#define MINE_MIN       MODE_EASY_MINE
#define MINE_MAX(x,y)  ((x-1)*(y-1))
///////////////////////////////////////////////////
//Definitions for the misc items
#define ADVANCED_TIME_MAX 2147483646
#define TRANSPARENT_COLOR RGB(255,0,255)
