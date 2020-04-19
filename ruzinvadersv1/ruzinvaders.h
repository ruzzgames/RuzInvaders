// (C) Robert Ruzbacky 2004 - 2005
// RuzInvaders version 1.0
// Note that all source code can be used freely according to the GNU General Public Licence
// You can read the details of the licence at this link: http://www.gnu.org/copyleft/gpl.html

// Game loop modelled from Andre LaMothe's books: 
// Tricks of the Windows Game Programming Gurus
// Tricks of the 3D Game Programming Gurus

// Good idea to make sure all functions are checked for errors and handle them if they fail.
// I haven't used any naming conventions for variables as such
// Just using what makes sense to me so that the code can be understood

//-------------------------------------------------------------------------------------

// Before compiling ruzinvaders.cpp:
// ---------------------------------
// 1. Create a lib folder in your project folder. Then copy all the .lib files from the
//    DirectX 9 SDK into this folder
// 2. In your project lib folder, remove the files d3d8.lib, d3dx8.lib, d3dx8d.lib, d3dx8t.lib
//    They seem to clash when you compile (Direct X 8 clashing with Direct X 9)
// 3. Create a lib folder inside your actual project under your workspace
// 4. Right-click this folder, and click add files to folder
// 5. Find and add all the lib files to this folder
// 6. Need to remove d3dx.lib from lib folder ****
// 7. Need to add WINMM.LIB to the lib folder (do a search on C:\ to find it)
// 8. Delete ksuser.lib from the lib folder (conflicts with Directmusic)

//-------------------------------------------------------------------------------------
// Images
// All images and startup / menu screens done using Adobe Photoshop
// explosions: used Filter -> Noise then Filter -> Distort -> Glass
//-------------------------------------------------------------------------------------

#define WIN32_LEAN_AND_MEAN		// make fast as possible - no MFC
#define INITGUID

// needed for windows and other standard headers
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <string.h>
#include <math.h>
#include <io.h>
#include <fcntl.h>
#include <tchar.h>
#include <objbase.h>
#include <stdarg.h>
#include <time.h>
#include <strsafe.h>

#include <ddraw.h>
#include <d3d9.h>
#include <d3dx9core.h>
#include <dinput.h>
#include <dmusici.h>



// ***** game defines *****

// game states - tells you what status the game is at

#define GAME_STATE_INIT		0
#define GAME_STATE_START	1
#define GAME_STATE_RUN		2
#define GAME_STATE_SHUTDOWN	3
#define GAME_STATE_EXIT		4


// default screen size - this is how big our game screen is
// this can be changed (eg: to 800 x 600)
#define WINDOW_WIDTH	640
#define WINDOW_HEIGHT	480

#define OFF_SCREEN		5000

// the default video mode colours to display for our game - this is equal to 16, 24, or 32
// where 16 is 16bit colour, etc.
int video_bpp = 32;

// other defaults
#define WINDOW_CLASS_NAME "WIN2DCLASS"	// name of window

// simple way of getting keystrokes using a Windows API function
// #define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
// #define KEYUP(vk_code)   ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)


// number of points in the starfield
#define NUMPOINTS 100



// ***** Globals *****
// In games, just about all variables will be globals
// This makes the game a lot faster rather than doing a lot of function calls


// globals specific to Windows
HWND main_window_handle = NULL;			// main window handle (the full screen)
HINSTANCE main_instance = NULL;			// holds application instance
HWND hwnd;								// a window handle
HRESULT  hr;							// Windows error result



// game specific
int game_state;							// initial game state is initialise
DWORD start_clock_count;				// used for timing frames
long score, highscore;					// keeps track of the game score
										// Note: largest score is 2,147,483,647 (long integer)
char score_ascii[100];					// holds string version of score
char message_text[255];					// buffer to write messages to screen
int explosion_count;					// keep track of explosion sprites
D3DXVECTOR2	SavePosition;				// temporary variable used to save position of objects (ship, etc)
int ship_count;							// stores number of player ships




// variables for graphics
LPDIRECT3D9             D3D_Object = NULL;	 // Used to create the D3D Object
LPDIRECT3DDEVICE9       D3D_VideoDevice = NULL;   // Our rendering device (video card)
D3DDISPLAYMODE			VideoDisplayMode;	 // our screen display mode
D3DPRESENT_PARAMETERS	VideoProperties;	 // used to set properties of video card
UINT					NumVideoModes;		 // number of video modes card can support

// graphics colour formats - these are the DirectX flags for them
// used to query video card to check what it supports (assume first card supports all)
// 16bit 555 mode = D3DFMT_X1R5G5B5			16bit 565 mode = D3DFMT_R5G6B5
// 24bit mode = D3DFMT_R8G8B8				32bit mode = D3DFMT_X8R8G8B8
bool support_16bit555 = true;
bool support_16bit565 = true;
bool support_24bit = true;
bool support_32bit = true;
bool support_videohardware = true;	// video card can support hardware

// these are used to display video card details
D3DDISPLAYMODE		DisplayMode16bit555;
D3DDISPLAYMODE		DisplayMode16bit565;
D3DDISPLAYMODE		DisplayMode24bit;
D3DDISPLAYMODE		DisplayMode32bit;

// fonts - these are displayed on the game screen as the game is played
LOGFONT pLogFont_name, pLogFont_score, pLogFont_highscore, pLogFont_movingscore;
LPD3DXFONT ppFont_name, ppFont_score, ppFont_highscore, ppFont_movingscore;
LPRECT prect_name, prect_score, prect_highscore, prect_movingscore;
RECT lrect_name, lrect_score, lrect_highscore, lrect_movingscore;
LPCSTR GameName="Ruz Invaders";	
LPCSTR ScoreName="Score";	
LPCSTR HighScoreName="High Score";
LPCSTR MovingScoreName = score_ascii;


// D3D drawing Variables
// We are going to use 3D structures from Direct3D
// but use them in a 2D fashion
LPDIRECT3DVERTEXBUFFER9 StarfieldBuffer = NULL; // Buffer (memory) to hold vertices of starfield
LPDIRECT3DVERTEXBUFFER9 ShipBuffer = NULL; // Buffer (memory) to hold vertices of ship


// A structure for our custom vertex type or point
// example color - 0xffff0000 (red) - format is Alpha, Red, Green, Blue; ff = 255
struct CUSTOMVERTEX
{
    FLOAT x, y, z, rhw; // The vertex position (x,y,z) and rhw (rotation)
    DWORD color;        // The vertex color - 32 bit
	FLOAT tu, tv;		// The texture coordinates (same co-ords as rect)
};

// Our custom FVF, which describes our custom vertex structure
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE)

// the actual video buffer in memory
VOID* StarfieldMemory;	// memory for starfield
VOID* ShipMemory;		// memory for player ship

// array to hold our starfield
CUSTOMVERTEX points [NUMPOINTS];

// Variables for Texture - this is used for bitmaps loaded from files
// Probably should load everything into an array, but it is easier
// to see what is happening if I program it this way
// Also, usually 2D graphics are done in a tile rather than separately
// (ie. on one big bitmap file - then you can load the lot into an array)
LPDIRECT3DTEXTURE9		ShipTexture;			// Texture to load player ship bitmap
LPD3DXSPRITE			ShipSprite;				// Sprite of player ship
D3DXVECTOR2				ShipPosition;			// position of player ship
const float				ShipSize = 60.0f;
LPDIRECT3DTEXTURE9		ShipBulletTexture;		// player ship bullet
LPD3DXSPRITE			ShipBulletSprite;
D3DXVECTOR2				ShipBulletPosition;
LPDIRECT3DTEXTURE9		AlienTexture;			// first type of alien
LPD3DXSPRITE			AlienSprite;		
D3DXVECTOR2				AlienPosition;
LPDIRECT3DTEXTURE9		AlienBulletTexture;		// alien bullet
LPD3DXSPRITE			AlienBulletSprite;		
D3DXVECTOR2				AlienBulletPosition;
LPDIRECT3DTEXTURE9		AlienBigTexture;		// second type of alien
LPD3DXSPRITE			AlienBigSprite;		
D3DXVECTOR2				AlienBigPosition;
LPDIRECT3DTEXTURE9		AlienBossTexture;		// third type of alien
LPD3DXSPRITE			AlienBossSprite;		// Note: using DirectX logo as this :-)
D3DXVECTOR2				AlienBossPosition;		// (hopefully Bill won't mind LOL)
LPDIRECT3DTEXTURE9		Mship1Texture;			// mothership - lights on
LPD3DXSPRITE			Mship1Sprite;		
D3DXVECTOR2				Mship1Position;
LPDIRECT3DTEXTURE9		Mship2Texture;			// mothership - lights off
LPD3DXSPRITE			Mship2Sprite;		
D3DXVECTOR2				Mship2Position;
LPDIRECT3DTEXTURE9		Explosion1Texture;		// explosion part 1
LPD3DXSPRITE			Explosion1Sprite;		
D3DXVECTOR2				Explosion1Position;
LPDIRECT3DTEXTURE9		Explosion2Texture;		// explosion part 2
LPD3DXSPRITE			Explosion2Sprite;		
D3DXVECTOR2				Explosion2Position;
LPDIRECT3DTEXTURE9		Explosion3Texture;		// explosion part 3
LPD3DXSPRITE			Explosion3Sprite;		
D3DXVECTOR2				Explosion3Position;
LPDIRECT3DTEXTURE9		Explosion4Texture;		// explosion part 4
LPD3DXSPRITE			Explosion4Sprite;		
D3DXVECTOR2				Explosion4Position;
LPDIRECT3DTEXTURE9		Explosion5Texture;		// explosion part 5
LPD3DXSPRITE			Explosion5Sprite;		
D3DXVECTOR2				Explosion5Position;
LPDIRECT3DTEXTURE9		Explosion6Texture;		// explosion part 6
LPD3DXSPRITE			Explosion6Sprite;		
D3DXVECTOR2				Explosion6Position;

LPDIRECT3DTEXTURE9		StartScreenTexture;		// startup screen
LPD3DXSPRITE			StartScreenSprite;
D3DXVECTOR2				StartScreenPosition;
LPDIRECT3DTEXTURE9		MenuScreenTexture;		// menu screen
LPD3DXSPRITE			MenuScreenSprite;
D3DXVECTOR2				MenuScreenPosition;


// Variables DirectInput and for moving bitmaps
// this is where we use keyboard to move ship and other variables to move aliens, etc.
LPDIRECTINPUT8  DInputObject;
LPDIRECTINPUTDEVICE8  DInputDevice; 
#define KEYDOWN(name, key) (name[key] & 0x80) 
char keybuffer[256]; 
D3DXMATRIX ShipMove;
D3DXMATRIX AlienMove; 
D3DXMATRIX ShipBulletMove;
D3DXMATRIX AlienBulletMove;
D3DXMATRIX AlienBigMove; 
D3DXMATRIX Mship1Move;
D3DXMATRIX Mship2Move;
D3DXMATRIX AlienBossMove;




bool ship_fires;					// player fires bullet
bool alien_alive;					// alien is alive and can fire a bullet
bool shipbullet_hitalien;			// did the ship's bullet hit the alien?
bool alien_movingright;				// is the alien moving to the right?
bool alienbullet_hitship;			// did the alien's bullet hit the ship?
bool alienbig_movingright;			// is the alien moving to the right?
bool shipbullet_hitalienbig;		// did the ship's bullet hit the alien?
bool alien_hitonce;					// avoid hitting 2 aliens with same bullet
bool mship1_movingright;			// is the mothership1 moving to the right?
bool mship2_movingright;			// is the mothership2 moving to the right?
bool shipbullet_hitmship1;			// did the ship's bullet hit the mothership1?
bool shipbullet_hitmship2;			// did the ship's bullet hit the mothership1?
bool alienboss_movingright;			// is the Alien Boss moving to the right?
bool shipbullet_hitalienboss;		// did the ship's bullet hit the Alien Boss?



// this is the size of the bitmaps in pixels
// this was obtained by trial and error by checking if bitmap went off the screen
#define SHIPNUMPIXELS 60
#define ALIENNUMPIXELS 30

// these are the default speeds
// some may later go into variables (eg: to move alien at a varying speed for harder levels)
#define SHIPSPEED 5
#define ALIENSPEED 5
#define MSHIPSPEED 3
#define SHIPBULLETSPEED 5
#define ALIENBULLETSPEED 5

// misc game defines
#define ALIENHITSCORE 10		// scores for hitting aliens and alienship
#define ALIENBIGHITSCORE 100
#define ALIENMOTHERSHIPHITSCORE 500
#define ALIENBOSSHITSCORE 1000
#define MAX_SCORE 999999		// maximum you can score before you "clock" the game
#define	NUM_SHIPS 3				// number of ships before end of game


// *** for playing music / sound ***
IDirectMusicLoader8*      g_pLoader       = NULL;
IDirectMusicPerformance8* g_pPerformance  = NULL;
IDirectMusicSegment8*     g_pSegment      = NULL;
CHAR strPath2[MAX_PATH] = "";
WCHAR wstrSearchPath[MAX_PATH];
WCHAR wstrFileName[MAX_PATH] = L"midifile9.mid";



// ***** Function Prototypes *****

// windows message handler
LRESULT CALLBACK WindowProc (HWND hwnd,UINT msg,WPARAM wparam,
							 LPARAM lparam);
// windows main function
int WINAPI WinMain (HINSTANCE hinstance, HINSTANCE hprevinstance,
					LPSTR lpcmdline, int ncmdshow);

int Game_Main (void);				// main game loop
int Game_Init (void);				// initialise game variables
int Game_Shutdown (void);			// shutdown game and cleanup memory
DWORD Start_Clock(void);			// start the clock
DWORD Get_Clock(void);				// get system clock
DWORD Wait_Clock(DWORD count);		// wait - to adjust frame timing to 30 fps






