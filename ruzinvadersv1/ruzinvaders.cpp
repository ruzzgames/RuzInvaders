// (C) Robert Ruzbacky 2004 - 2005
// RuzInvaders version 1.0
// Note that all source code can be used freely according to the GNU General Public Licence
// You can read the details of the licence at this link: http://www.gnu.org/copyleft/gpl.html


#include "ruzinvaders.h"


// Main Windows Function
// Read a good Windows Programming book for extra explanation
// You don't need to know Windows Programming in detail - just enough to understand basics
// If you highlight / click on a word, press F1 to also get an explanation
int WINAPI WinMain (HINSTANCE hinstance,
					HINSTANCE hprevinstance,
					LPSTR lpcmdline,
					int ncmdshow)
{

	WNDCLASSEX winclass;
	MSG msg;
	HDC hdc;
	PAINTSTRUCT ps;


	// create window structure - this is the properties of the main window (our full screen)
	winclass.cbSize = sizeof (WNDCLASSEX);
	winclass.style = CS_DBLCLKS | CS_OWNDC |
					 CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hinstance;
	winclass.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	winclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = WINDOW_CLASS_NAME;
	winclass.hIconSm = LoadIcon (NULL, IDI_APPLICATION);

	if (!RegisterClassEx (&winclass))
		return (0);

	// this creates the actual window - this is a full screen window with no elements
	if (!(hwnd = CreateWindowEx (
							 NULL,
							 WINDOW_CLASS_NAME,
							 "APR",
							 WS_POPUP | WS_VISIBLE,
							 0,0,  // initial x,y
							 GetSystemMetrics (SM_CXSCREEN), // get width of screen
							 GetSystemMetrics (SM_CYSCREEN), // get height of screen
							 NULL,
							 NULL,
							 hinstance,
							 NULL)))
	return (0);

	ShowCursor (FALSE);

	main_window_handle = hwnd;
	main_instance = hinstance;

	// this is where the game initialisation occurs
	if (Game_Init () == E_FAIL)
	{
		MessageBox (hwnd,"Game Failed to Initialise","RuzInvaders",MB_OK); // something went wrong
		Game_Shutdown ();		// clean up memory
		ShowCursor (TRUE);		// bring back mouse arrow
		msg.message = WM_QUIT;	// we want to quit program
		return (msg.wParam);	// exit to Windows
	}
	else game_state = GAME_STATE_INIT;	// game is OK to start
		 


	// main event loop
	// Windows will use this loop to process events and receive / send messages
	// Note: this is an infinite loop
	while (1)
	{
		if (PeekMessage (&msg, NULL, 0,0,PM_REMOVE))
		{
			// checks if user Quits program - then exits loop if this is the case
			if (msg.message == WM_QUIT)
				break;

			// tells Windows to process messages
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}


		// main game processing occurs here
		Game_Main ();

	} // end while


	// game is over - all memory is released and system is cleaned up
	Game_Shutdown ();

	ShowCursor (TRUE);	// bring back mouse arrow

	return (msg.wParam); // return to Windows

} // end WinMain


// windows message handler
// This handles any messages given to us by Windows
LRESULT CALLBACK WindowProc (HWND hwnd,
							 UINT msg,
							 WPARAM wparam,
							 LPARAM lparam)
{
	PAINTSTRUCT	ps;
	HDC			hdc;

	// handle Windows messages
	switch (msg)
	{
	case WM_CREATE:
		{
			// init code
			return (0);
		} break;

	case WM_PAINT:
		{
			hdc = BeginPaint(hwnd, &ps);
			EndPaint (hwnd, &ps);
			return (0);
		} break;

	case WM_DESTROY:
		{
			PostQuitMessage (0);
			return (0);
		} break;

	default: break;
	} // end switch

	// pass unprocessed messages to windows
	return (DefWindowProc (hwnd, msg, wparam, lparam));

} // end WindowProc function





// *** main game functions ***

// initialise game variables prior to starting game
// Note: This function doesn't have to be fast - not part of game loop
// Do all heavy loading and initialisation here
int Game_Init (void)
{

	// create direct3D object
	if( NULL == (D3D_Object = Direct3DCreate9(D3D_SDK_VERSION)))
	{
		MessageBox (hwnd,"Cannot Create D3D Object - Check if DirectX 9.0 or later is installed","RuzInvaders",MB_OK);
		return E_FAIL;
	}


	// Check what modes the video card supports - just do common modes

	// *** check for 16 bit 555 colour support ***
	// get the maximum number of video modes the card can display for that colour
	NumVideoModes = D3D_Object->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_X1R5G5B5);
	// check what the maximum mode the video card can handle - stored in DisplayMode
	if( FAILED( D3D_Object->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X1R5G5B5,
												 NumVideoModes - 1, &DisplayMode16bit555 )))
	{
		support_16bit555 = false;
	}

	// *** check for 16 bit 565 colour support ***
	// get the maximum number of video modes the card can display for that colour
	NumVideoModes = D3D_Object->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_R5G6B5);
	// check what the maximum mode the video card can handle - stored in DisplayMode
	if( FAILED( D3D_Object->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_R5G6B5,
												 NumVideoModes - 1, &DisplayMode16bit565 )))
   	{
		support_16bit565 = false;
	}

	// *** check for 24 bit colour support ***
	// get the maximum number of video modes the card can display for that colour
	NumVideoModes = D3D_Object->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_R8G8B8);
	// check what the maximum mode the video card can handle - stored in DisplayMode
	if( FAILED( D3D_Object->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_R8G8B8,
												 NumVideoModes - 1, &DisplayMode24bit )))
    {
		support_24bit = false;
	}

	// *** check for 32 bit colour support ***
	// get the maximum number of video modes the card can display for that colour
	NumVideoModes = D3D_Object->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_X8R8G8B8);
	// check what the maximum mode the video card can handle - stored in DisplayMode
	if( FAILED( D3D_Object->EnumAdapterModes( D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8,
												 NumVideoModes - 1, &DisplayMode32bit )))
    {
		support_32bit = false;
	}

	// if all modes are no good, exit program - note this is rare
	if (support_16bit555 && support_16bit565 && support_24bit && support_32bit == false)
	{
		MessageBox (hwnd,"Video card doesn't support 16, 24, or 32 bit colour","RuzInvaders",MB_OK);
		return E_FAIL;
	}


	// For our video mode, check if the video card can support hardware acceleration
	// D3DDEVTYPE_HAL - is hardware support (ie. PC has compatible DirectX 3D card)
	// D3DDEVTYPE_REF - DirectX uses software emulation
	if (video_bpp == 32 && support_32bit == true)
	{
		if( FAILED( D3D_Object->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
												 D3DFMT_X8R8G8B8,D3DFMT_X8R8G8B8, FALSE)))
												
			support_videohardware = false;
	}
	else if(video_bpp == 24 && support_24bit == true)
	{
		if( FAILED( D3D_Object->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
												 D3DFMT_R8G8B8,D3DFMT_R8G8B8, FALSE)))
												
			support_videohardware = false;
	}
	else if(video_bpp == 16 && support_16bit565 == true)
	{
		if( FAILED( D3D_Object->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
												 D3DFMT_R5G6B5,D3DFMT_R5G6B5, FALSE)))
												
			support_videohardware = false;
	}
	else if(video_bpp == 16 && support_16bit555 == true)
	{
		if( FAILED( D3D_Object->CheckDeviceType( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
												 D3DFMT_X1R5G5B5,D3DFMT_X1R5G5B5, FALSE)))
												
			support_videohardware = false;
	}


	// get current display mode of video card
	if( FAILED( D3D_Object->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &VideoDisplayMode )))
	{
		MessageBox (hwnd,"Cannot Get Current DisplayMode on Video Card","RuzInvaders",MB_OK);
		return E_FAIL;
	}

	// sets display to our default values
	VideoDisplayMode.Width = WINDOW_WIDTH;		// use our screen width
	VideoDisplayMode.Height = WINDOW_HEIGHT;	// use our screen height
	VideoDisplayMode.RefreshRate = 0;			// use default refresh rate

    // check if our card can support what we set as default in arcade.h
	// Also, can use DisplayModexxxxx to set the Maximum window width or height if you want
	if (video_bpp == 32 && support_32bit == true)
	{
		VideoDisplayMode.RefreshRate = DisplayMode32bit.RefreshRate;
		VideoDisplayMode.Format = D3DFMT_X8R8G8B8;
	}
	else if (video_bpp == 24 && support_24bit == true)
	{
		VideoDisplayMode.RefreshRate = DisplayMode24bit.RefreshRate;
		VideoDisplayMode.Format = D3DFMT_R8G8B8;
	}
	else if (video_bpp == 16 && support_16bit565 == true)
	{	
		VideoDisplayMode.RefreshRate = DisplayMode16bit565.RefreshRate;
		VideoDisplayMode.Format = D3DFMT_R5G6B5;
	}
	else if (video_bpp == 16 && support_16bit555 == true)
	{	
		VideoDisplayMode.RefreshRate = DisplayMode16bit555.RefreshRate;
		VideoDisplayMode.Format = D3DFMT_X1R5G5B5;
	}
	else 
	{
		MessageBox (hwnd,"Video colour mode not supported - check your video_bpp variable in arcade.h and change it","RuzInvaders",MB_OK);
		return E_FAIL;
	}


	// set the properties of video card
	ZeroMemory( &VideoProperties, sizeof(VideoProperties) );	// clear the memory before filling with values
	VideoProperties.BackBufferWidth = VideoDisplayMode.Width;	// screen width - backbuffer
	VideoProperties.BackBufferHeight = VideoDisplayMode.Height;	// screen height - backbuffer
	VideoProperties.Windowed   = FALSE;					// we have full screen
	VideoProperties.SwapEffect = D3DSWAPEFFECT_DISCARD;
	VideoProperties.BackBufferCount = 1;					// only using one backbuffer (can have more)
	VideoProperties.hDeviceWindow = hwnd;
	VideoProperties.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	VideoProperties.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	VideoProperties.BackBufferFormat = VideoDisplayMode.Format;

	// create device - this is the video card interface
	// this gets our video card up and running so we can draw to it later
	// check if hardware is supported (we have a 3D video card)
	if (support_videohardware == true)
	{

		if( FAILED( D3D_Object->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
                                  D3DCREATE_HARDWARE_VERTEXPROCESSING,
                                  &VideoProperties, &D3D_VideoDevice )))
		{
			MessageBox (hwnd,"Your Video Card does not support 3D Hardware Vertex Processing - switching to software (Card is still hardware accelerated)","RuzInvaders",MB_OK);
			if ( FAILED( D3D_Object->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                  &VideoProperties, &D3D_VideoDevice )))
			{
				MessageBox (hwnd,"Your Video Card cannot continue CreateDevice(HAL / SOFTWARE VERTEXPROCESSING)","RuzInvaders",MB_OK);
				return E_FAIL;
			}

		}
	}
	else
	{
		MessageBox (hwnd,"Your Video Card is not a 3D card - Attempting to use DirectX Software","RuzInvaders",MB_OK);
		if( FAILED( D3D_Object->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hwnd,
                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                  &VideoProperties, &D3D_VideoDevice )))
		{
			MessageBox (hwnd,"Cannot implement DirectX Software routines on your video card - check the minimum game specifications required","RuzInvaders",MB_OK);
			return E_FAIL;
		}
	}


	// create font device - used to write the name of the game "Ruz Invaders"
	if (FAILED(D3DXCreateFontIndirect (D3D_VideoDevice, &pLogFont_name, &ppFont_name)))
	{
		MessageBox (hwnd,"Cannot create DirectX Font(Name)","RuzInvaders",MB_OK);
		return E_FAIL;
	}

	// rectangle or box to draw our font in (these are in screen co-ordinates)
	// this is name of game
	prect_name = &lrect_name;
	lrect_name.left = 500;	// the x-coordinate of the upper-left corner of the rectangle. 
	lrect_name.top = 10;		// the y-coordinate of the upper-left corner of the rectangle.
	lrect_name.right = 600;	// the x-coordinate of the lower-right corner of the rectangle. 
	lrect_name.bottom = 100;	// the y-coordinate of the lower-right corner of the rectangle.

	// create font device - used to write the word "Score"
	if (FAILED(D3DXCreateFontIndirect (D3D_VideoDevice, &pLogFont_score, &ppFont_score)))
	{
		MessageBox (hwnd,"Cannot create DirectX Font(Score)","RuzInvaders",MB_OK);
		return E_FAIL;
	}

	// rectangle or box to draw our font in (these are in screen co-ordinates)
	// this is the word "Score"
	prect_score = &lrect_score;
	lrect_score.left = 5;	    // the x-coordinate of the upper-left corner of the rectangle. 
	lrect_score.top = 10;		// the y-coordinate of the upper-left corner of the rectangle.
	lrect_score.right = 80;	// the x-coordinate of the lower-right corner of the rectangle. 
	lrect_score.bottom = 100;	// the y-coordinate of the lower-right corner of the rectangle.


	// create font device - used to write the word "High Score"
	if (FAILED(D3DXCreateFontIndirect (D3D_VideoDevice, &pLogFont_highscore, &ppFont_highscore)))
	{
		MessageBox (hwnd,"Cannot create DirectX Font(High Score)","RuzInvaders",MB_OK);
		return E_FAIL;
	}

	// rectangle or box to draw our font in (these are in screen co-ordinates)
	// this is the word "HighScore"
	prect_highscore = &lrect_highscore;
	lrect_highscore.left = 250;		// the x-coordinate of the upper-left corner of the rectangle. 
	lrect_highscore.top = 10;		// the y-coordinate of the upper-left corner of the rectangle.
	lrect_highscore.right = 350;	// the x-coordinate of the lower-right corner of the rectangle. 
	lrect_highscore.bottom = 100;	// the y-coordinate of the lower-right corner of the rectangle.


	// create font device - used to write the player's current score
	// I've called this "movingscore" - only for font
	if (FAILED(D3DXCreateFontIndirect (D3D_VideoDevice, &pLogFont_movingscore, &ppFont_movingscore)))
	{
		MessageBox (hwnd,"Cannot create DirectX Font(MovingScore)","RuzInvaders",MB_OK);
		return E_FAIL;
	}

	// rectangle or box to draw our font in (these are in screen co-ordinates)
	// this is the player's current score
	prect_movingscore = &lrect_movingscore;
	lrect_movingscore.left = 5;	    // the x-coordinate of the upper-left corner of the rectangle. 
	lrect_movingscore.top = 30;		// the y-coordinate of the upper-left corner of the rectangle.
	lrect_movingscore.right = 80;	// the x-coordinate of the lower-right corner of the rectangle. 
	lrect_movingscore.bottom = 120;	// the y-coordinate of the lower-right corner of the rectangle.



	// create a new random number
	srand( (unsigned)time( NULL ) );

	// initialise our buffer of points
	// this is used as our starfield background
	for (int i = 0; i < NUMPOINTS; i++)
	{
		points[i].x = 0.0f;
		points[i].y = 0.0f;
		points[i].z = 0.0f;
		points[i].rhw = 1.0f;
		points[i].color = 0x00000000;	// black colour as our background
	}


	// *** ship bitmap ***
	// create texture by loading the ship bitmap from a file
	// in our case, we use 32 bit colour (though can have different versions if 16 or 24bit)
	// (ie. our file should be saved with 32 bit colour when using a paint program
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/ship.bmp", &ShipTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Ship","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &ShipSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Ship","RuzInvaders",MB_OK);
		return E_FAIL;
    }



	// *** alien bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/alien.bmp", &AlienTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Alien","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &AlienSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Alien","RuzInvaders",MB_OK);
		return E_FAIL;
    }


	// *** ship bullet bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/shipBullet.bmp", &ShipBulletTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Ship's bullet","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &ShipBulletSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Ship's bullet","RuzInvaders",MB_OK);
		return E_FAIL;
    }


	// *** alien bullet bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/alienbullet.bmp", &AlienBulletTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Alien's bullet","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &AlienBulletSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Alien's bullet","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** alienbig bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/alienbig.bmp", &AlienBigTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Alien Big","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &AlienBigSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Alien Big","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** alienboss bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/bossalien.bmp", &AlienBossTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Alien Boss","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &AlienBossSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Alien Boss","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** mothership1 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/mothership1.bmp", &Mship1Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Mship1","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Mship1Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Mship1","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** mothership2 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/mothership2.bmp", &Mship2Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Mship2","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Mship2Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Mship2","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** explosion1 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/explosion1.bmp", &Explosion1Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Explosion1","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Explosion1Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Explosion1","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** explosion2 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/explosion2.bmp", &Explosion2Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Explosion2","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Explosion2Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Explosion2","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** explosion3 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/explosion3.bmp", &Explosion3Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Explosion3","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Explosion3Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Explosion3","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** explosion4 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/explosion4.bmp", &Explosion4Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Explosion4","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Explosion4Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Explosion4","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** explosion5 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/explosion5.bmp", &Explosion5Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Explosion5","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Explosion5Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Explosion5","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** explosion6 bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/explosion6.bmp", &Explosion6Texture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Explosion6","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &Explosion6Sprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Explosion6","RuzInvaders",MB_OK);
		return E_FAIL;
    }


	// *** Start Screen bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/startscreen.bmp", &StartScreenTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Startup Screen","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &StartScreenSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Startup Screen","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// *** MenuScreen bitmap ***
	if( FAILED( D3DXCreateTextureFromFile( D3D_VideoDevice, "images/menuscreen.bmp", &MenuScreenTexture ) ) )
    {
		MessageBox (hwnd,"Cannot load bitmap file for Menu Screen","RuzInvaders",MB_OK);
		return E_FAIL;
    }

	// create a new sprite - this will hold our bitmap
	if( FAILED( D3DXCreateSprite( D3D_VideoDevice, &MenuScreenSprite )))
    {
		MessageBox (hwnd,"Cannot create sprite for Menu Screen","RuzInvaders",MB_OK);
		return E_FAIL;
    }



	// *** Initialise Input from Keyboard ***
	// create DirectInput Object
	if (FAILED(DirectInput8Create(main_instance, DIRECTINPUT_VERSION, 
        IID_IDirectInput8, (void**)&DInputObject, NULL)))
	{
		MessageBox (hwnd,"Cannot create DirectInput Object","RuzInvaders",MB_OK);
		return E_FAIL;
	}

	// Create keyboard device	
	if (FAILED(DInputObject->CreateDevice(GUID_SysKeyboard, &DInputDevice, NULL)))
	{
		MessageBox (hwnd,"Cannot create DirectInput Device - Keyboard","RuzInvaders",MB_OK);
		return E_FAIL;
	}
	
	// Tell DirectInput to use keyboard data
	if (FAILED(DInputDevice->SetDataFormat(&c_dfDIKeyboard))) 
	{		
		MessageBox (hwnd,"Cannot Set Data Format for DirectInput Device - Keyboard","RuzInvaders",MB_OK);
		return E_FAIL;
	}
	
	// Co-operate with Windows
	if (FAILED(DInputDevice->SetCooperativeLevel(hwnd, 
                   DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
	{
		MessageBox (hwnd,"Cannot set DirectInput Cooperative Level","RuzInvaders",MB_OK);
		return E_FAIL;

	}

	// Aquire keyboard for input
	if (DInputDevice) DInputDevice->Acquire(); 


	// *** initialise music and sounds ***
	// for DirectX Audio
    CoInitialize(NULL);

	// for DirectX Audio    
    CoCreateInstance(CLSID_DirectMusicLoader, NULL, 
                     CLSCTX_INPROC, IID_IDirectMusicLoader8,
                     (void**)&g_pLoader);

    CoCreateInstance(CLSID_DirectMusicPerformance, NULL,
                     CLSCTX_INPROC, IID_IDirectMusicPerformance8,
                     (void**)&g_pPerformance );

//    CoCreateInstance(CLSID_DirectMusicSegment, NULL,
//                     CLSCTX_INPROC, IID_IDirectMusicSegment8,
 //                    (void**)&g_pSegment );


	// for DirectX Audio    
    g_pPerformance->InitAudio( 
        NULL,                  // IDirectMusic interface not needed.
        NULL,                  // IDirectSound interface not needed.
        NULL,                  // Window handle.
        DMUS_APATH_SHARED_STEREOPLUSREVERB,  // Default audiopath type.
        64,                    // Number of performance channels.
        DMUS_AUDIOF_ALL,       // Features on synthesizer.
        NULL                   // Audio parameters; use defaults.
 );
    // Find the Windows media directory.
 
 //   GetWindowsDirectory( strPath2, MAX_PATH );
    strcat( strPath2, "media" );
 
   // Convert to Unicode.
 
    MultiByteToWideChar( CP_ACP, 0, strPath2, -1, 
                         wstrSearchPath, MAX_PATH );
 
    // Set the search directory.
 
    g_pLoader->SetSearchDirectory( 
        GUID_DirectMusicAllTypes,   // Types of files sought.
        wstrSearchPath,             // Where to look.
        FALSE                       // Don't clear object data.
    );

 
    if (FAILED(g_pLoader->LoadObjectFromFile(
        CLSID_DirectMusicSegment,   // Class identifier.
        IID_IDirectMusicSegment8,   // ID of desired interface.
        wstrFileName,               // Filename.
        (LPVOID*) &g_pSegment       // Pointer that receives interface.
    )))
    {
        MessageBox( hwnd, "Media not found, sample will now quit.", 
                          "DMusic", MB_OK );
        return 0;
    }


  g_pSegment->Download( g_pPerformance );

      g_pPerformance->PlaySegmentEx(
        g_pSegment,  // Segment to play.
        NULL,        // Used for songs; not implemented.
        NULL,        // For transitions. 
        0,           // Flags.
        0,           // Start time; 0 is immediate.
        NULL,        // Pointer that receives segment state.
        NULL,        // Object to stop.
        NULL         // Audiopath, if not default.
    );      


	return (0);
} // end Game_Init



// shutdown game and release all memory in order of creation
int Game_Shutdown (void)
{

	// release the Starfield Buffer
	if( StarfieldBuffer != NULL )        
        StarfieldBuffer->Release();

// ***** Bug - check if music has started before stopping and closing *****
// stop and release music device
    g_pPerformance->Stop(
        NULL,   // Stop all segments.
        NULL,   // Stop all segment states.
        0,      // Do it immediately.
        0       // Flags.
    );
 
    g_pPerformance->CloseDown();
 
    g_pLoader->Release(); 
    g_pPerformance->Release();
    g_pSegment->Release();

	// release for Sound / Music
	CoUninitialize();



	// release input device
    if (DInputObject) 
    { 
        if (DInputDevice) 
        { 
        // Always unacquire device before calling Release(). 
            DInputDevice->Unacquire(); 
            DInputDevice->Release();
            DInputDevice = NULL; 
        } 
        DInputObject->Release();
        DInputObject = NULL; 
    } 

	// release startup screen sprite
	if( StartScreenSprite != NULL)
        StartScreenSprite->Release();

	// release menu screen sprite
	if( MenuScreenSprite != NULL)
        MenuScreenSprite->Release();

	// release explosion6 sprite
	if( Explosion6Sprite != NULL)
        Explosion6Sprite->Release();

	// release explosion5 sprite
	if( Explosion5Sprite != NULL)
        Explosion5Sprite->Release();

	// release explosion4 sprite
	if( Explosion4Sprite != NULL)
        Explosion4Sprite->Release();

	// release explosion3 sprite
	if( Explosion3Sprite != NULL)
        Explosion3Sprite->Release();

	// release explosion2 sprite
	if( Explosion2Sprite != NULL)
        Explosion2Sprite->Release();

	// release explosion1 sprite
	if( Explosion1Sprite != NULL)
        Explosion1Sprite->Release();

	// release mothership2 sprite
	if( Mship2Sprite != NULL)
        Mship2Sprite->Release();

	// release mothership1 sprite
	if( Mship1Sprite != NULL)
        Mship1Sprite->Release();

		// release alienboss sprite
	if( AlienBossSprite != NULL)
        AlienBossSprite->Release();

	// release alienbig sprite
	if( AlienBigSprite != NULL)
        AlienBigSprite->Release();

	// release alien bullet sprite
	if( AlienBulletSprite != NULL)
        AlienBulletSprite->Release();

	// release ship bullet sprite
	if( ShipBulletSprite != NULL)
        ShipBulletSprite->Release();

	// release alien sprite
	if( AlienSprite != NULL)
        AlienSprite->Release();

	// release ship sprite
	if( ShipSprite != NULL)
        ShipSprite->Release();

	// release the font device
    if( ppFont_movingscore != NULL)
        ppFont_movingscore->Release();

	// release the font device
    if( ppFont_highscore != NULL)
        ppFont_highscore->Release();

	// release the font device
    if( ppFont_score != NULL)
        ppFont_score->Release();

	// release the font device
    if( ppFont_name != NULL)
        ppFont_name->Release();
	
	// release the video card
    if( D3D_VideoDevice != NULL)
        D3D_VideoDevice->Release();

    if( D3D_Object != NULL)
        D3D_Object->Release();


	return (0);
} // end Game_Shutdown



// main game function
int Game_Main (void)
{
	if (game_state == GAME_STATE_INIT)
	{
		// initialisation code is here - just what is needed to start the game running

		// starting position of ship
		ShipPosition.x = WINDOW_WIDTH / 2.0f;			// 1/2 way down screen
		ShipPosition.y = WINDOW_HEIGHT - ShipSize;		// bottom of screen

		// setup alien
		AlienPosition.x = 95.0f;
		AlienPosition.y = 130.0f;
		alien_movingright = true;

		// setup alien big
		AlienBigPosition.x = 65.0f;
		AlienBigPosition.y = 95.0f;
		alienbig_movingright = true;
		AlienBulletPosition.x = AlienBulletPosition.x + 8.0f;
		AlienBulletPosition.y = AlienPosition.y + 30.0f;

		// setup mothership
		Mship1Position.x = WINDOW_WIDTH - 40.0f;
		Mship1Position.y = 50.0f;
		mship1_movingright = false;

		// setup alienboss
		AlienBossPosition.x = WINDOW_WIDTH / 2.0f;			// 1/2 way down screen
		AlienBossPosition.y = WINDOW_HEIGHT / 2.0f;			// 1/2 way down screen
		alienboss_movingright = true;


		// misc game items
		ship_fires = false;				// test if player's ship fired a bullet
		explosion_count = 0;			// setup explosion count
		ship_count = NUM_SHIPS;			// max number of ships before "Game Over"

		// setup scores
		highscore = 0;
		score = 0;
		_ltoa(score, score_ascii,10);	// convert the score to a string
		strcat (score_ascii,"\0");


		// ***** Display start up screen - company logo*****
		StartScreenPosition.x = 60;
		StartScreenPosition.y = 0;

		// This clears the screen in memory (the back buffer) to a black colour
		if (FAILED(D3D_VideoDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 )))
		{
			MessageBox (hwnd,"Failed to clear video card - Startup Screen","RuzInvaders",MB_OK);
			return E_FAIL;

		}

		// render scene in the back buffer
		if (FAILED(D3D_VideoDevice->BeginScene()))
		{
			MessageBox (hwnd,"Failed to Render (Begin) Scene to backbuffer - Startup Screen","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// display startup screen
		StartScreenSprite->Draw(StartScreenTexture,NULL,NULL,NULL,0,&StartScreenPosition,D3DCOLOR_ARGB(0xff,255,255,255));
		
		if (FAILED(D3D_VideoDevice->EndScene()))
		{
			MessageBox (hwnd,"Failed to End Scene (Render backbuffer) - Startup Screen","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// show scene to video screen
		// this automatically copies the contents of the back buffer to video RAM, 
		// hence displaying it on the screen.
		if (FAILED(D3D_VideoDevice->Present( NULL, NULL, NULL, NULL )))
		{
			MessageBox (hwnd,"Failed to Render scene to video card - Startup Screen","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// pause the startup screen
		Start_Clock();
		Wait_Clock(5000);

		game_state = GAME_STATE_START;
	}
	else if (game_state == GAME_STATE_START)
	{
		// *** put starting game code here ***


		// ***** Display start screen - Main Menu *****
		// Image size is 500 x 375 at a resolution of 250 pixels / inch
		MenuScreenPosition.x = 60;
		MenuScreenPosition.y = 0;
		// This clears the screen in memory (the back buffer) to a black colour
		if (FAILED(D3D_VideoDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 )))
		{
			MessageBox (hwnd,"Failed to clear video card - Menu Screen","RuzInvaders",MB_OK);
			return E_FAIL;

		}

		// render scene in the back buffer
		if (FAILED(D3D_VideoDevice->BeginScene()))
		{
			MessageBox (hwnd,"Failed to Render (Begin) Scene to backbuffer - Menu Screen","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// display startup screen
		MenuScreenSprite->Draw(MenuScreenTexture,NULL,NULL,NULL,0,&MenuScreenPosition,D3DCOLOR_ARGB(0xff,255,255,255));
		
		if (FAILED(D3D_VideoDevice->EndScene()))
		{
			MessageBox (hwnd,"Failed to End Scene (Render backbuffer) - Menu Screen","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// show scene to video screen
		// this automatically copies the contents of the back buffer to video RAM, 
		// hence displaying it on the screen.
		if (FAILED(D3D_VideoDevice->Present( NULL, NULL, NULL, NULL )))
		{
			MessageBox (hwnd,"Failed to Render scene to video card - Menu Screen","RuzInvaders",MB_OK);
			return E_FAIL;
		}



		// Capture keystrokes of user
		if( FAILED(DInputDevice->GetDeviceState(sizeof(keybuffer),(LPVOID)&keybuffer)))  
		{ 
         // If it failed, the device has probably been lost. 
         // Check for (hr == DIERR_INPUTLOST) 
         // and attempt to reacquire it here.

			if (hr == DIERR_INPUTLOST)
			{
				if( FAILED(DInputDevice->Acquire() ))
				{
					MessageBox( hwnd, "Could not re-aquire keyboard","RuzInvaders", MB_OK );
					return E_FAIL; 
				}
			}
			else 
			{	
				MessageBox( hwnd, "Keyboard not found","RuzInvaders", MB_OK );
				return E_FAIL;
			}
	   } 
 
		// capture quit button (ESC key)
		if (KEYDOWN(keybuffer, DIK_ESCAPE)) 
		{
			PostMessage (main_window_handle, WM_DESTROY, 0,0);
			game_state = GAME_STATE_SHUTDOWN;
		}


		// capture play button (P key)
		if (KEYDOWN(keybuffer, DIK_P)) 
		{
			game_state = GAME_STATE_RUN;
		}
		
	}
	else if (game_state == GAME_STATE_RUN)
	{

		// This is the main running phase of our game

		// start the timing clock
		Start_Clock();


		// ***** Keyboard Section - Capture keystrokes of user *****
		if( FAILED(DInputDevice->GetDeviceState(sizeof(keybuffer),(LPVOID)&keybuffer)))  
		{ 
         // If it failed, the device has probably been lost. 
         // Check for (hr == DIERR_INPUTLOST) 
         // and attempt to reacquire it here.

			if (hr == DIERR_INPUTLOST)
			{
				if( FAILED(DInputDevice->Acquire() ))
				{
					MessageBox( hwnd, "Could not re-aquire keyboard","RuzInvaders", MB_OK );
					return E_FAIL; 
				}
			}
			else 
			{	
				MessageBox( hwnd, "Keyboard not found","RuzInvaders", MB_OK );
				return E_FAIL;
			}
	   } 
 
		// capture quit button (ESC key)
		if (KEYDOWN(keybuffer, DIK_ESCAPE)) 
		{
			PostMessage (main_window_handle, WM_DESTROY, 0,0);
			game_state = GAME_STATE_SHUTDOWN;
		}

		// check right arrow key
		if (KEYDOWN(keybuffer, DIK_RIGHT)) 
		{
			if (ShipPosition.x < WINDOW_WIDTH - SHIPNUMPIXELS)	
				ShipPosition.x = ShipPosition.x + SHIPSPEED;
			if (ShipPosition.x >= WINDOW_WIDTH - SHIPNUMPIXELS)
				ShipPosition.x = WINDOW_WIDTH - SHIPNUMPIXELS;
		}

		// check left arrow key
		if (KEYDOWN(keybuffer, DIK_LEFT)) 
		{
			if (ShipPosition.x < WINDOW_WIDTH - SHIPNUMPIXELS)
				ShipPosition.x =  ShipPosition.x - SHIPSPEED;	
			if (ShipPosition.x <= 0)
				ShipPosition.x = 0;
			if (ShipPosition.x >= WINDOW_WIDTH - SHIPNUMPIXELS)
				ShipPosition.x = WINDOW_WIDTH - SHIPNUMPIXELS - SHIPSPEED;
		}

		// check if user hits the fire button - space bar
		if (KEYDOWN(keybuffer, DIK_SPACE)) 
		{
			ship_fires = true;
			ShipBulletPosition.x = ShipPosition.x + 25;
			ShipBulletPosition.y = ShipPosition.y - 20;
		}

		// ***** Game Physics / Game AI Section *****
		// (Well very simplistic physics! AI = Artificial Intelligence)

		// get alien to move back and forth horizontally across screen
		// move alien to right
		if (AlienPosition.x < WINDOW_WIDTH - ALIENNUMPIXELS && alien_movingright == true)	
			AlienPosition.x = AlienPosition.x + ALIENSPEED;
		// test if alien reaches full right of screen
		if (AlienPosition.x >= WINDOW_WIDTH - ALIENNUMPIXELS - ALIENSPEED)
			alien_movingright = false;
		// move alien to left
		if (AlienPosition.x < WINDOW_WIDTH - ALIENNUMPIXELS && alien_movingright == false)	
			AlienPosition.x = AlienPosition.x - ALIENSPEED;
		// test if alien reaches full left of screen
		if (AlienPosition.x <=0)
			alien_movingright = true;

		// get alienbig to move back and forth horizontally across screen
		// move alienbig to right
		if (AlienBigPosition.x < WINDOW_WIDTH - ALIENNUMPIXELS && alienbig_movingright == true)	
			AlienBigPosition.x = AlienBigPosition.x + ALIENSPEED;
		// test if alien reaches full right of screen
		if (AlienBigPosition.x >= WINDOW_WIDTH - ALIENNUMPIXELS - ALIENSPEED)
			alienbig_movingright = false;
		// move alien to left
		if (AlienBigPosition.x < WINDOW_WIDTH - ALIENNUMPIXELS && alienbig_movingright == false)	
			AlienBigPosition.x = AlienBigPosition.x - ALIENSPEED;
		// test if alien reaches full left of screen
		if (AlienBigPosition.x <=0)
			alienbig_movingright = true;


		// get mothership to move from right to left of screen only
		if (Mship1Position.x < WINDOW_WIDTH - ALIENNUMPIXELS && mship1_movingright == false)	
			Mship1Position.x = Mship1Position.x - MSHIPSPEED;
		// test if alien reaches full left of screen
		if (Mship1Position.x <= - 1000)
		{
			//	mship1_movingright = true;
			Mship1Position.x = WINDOW_WIDTH - 40.0f;
		}

		// get alienboss to move back and forth horizontally across screen
		// move alienboss to right
		if (AlienBossPosition.x < WINDOW_WIDTH - ALIENNUMPIXELS && alienboss_movingright == true)	
			AlienBossPosition.x = AlienBossPosition.x + ALIENSPEED;
		// test if alien reaches full right of screen
		if (AlienBossPosition.x >= WINDOW_WIDTH - ALIENNUMPIXELS - ALIENSPEED)
			alienboss_movingright = false;
		// move alien to left
		if (AlienBossPosition.x < WINDOW_WIDTH - ALIENNUMPIXELS && alienboss_movingright == false)	
			AlienBossPosition.x = AlienBossPosition.x - ALIENSPEED;
		// test if alien reaches full left of screen
		if (AlienBossPosition.x <=0)
			alienboss_movingright = true;


		// *** collision detection ***
		// test if bullet hit one alien only (any alien)
		alien_hitonce = false;

		// check if bullet has hit alien
		shipbullet_hitalien = false;
//		if (ShipBulletPosition.x == AlienPosition.x && ShipBulletPosition.y == AlienPosition.y)
//			shipbullet_hitalien = true;		// direct hit
		if (ShipBulletPosition.x >= AlienPosition.x - 15 && ShipBulletPosition.x <= AlienPosition.x + 15)
		{
			if (ShipBulletPosition.y >= AlienPosition.y - 15 && ShipBulletPosition.y <= AlienPosition.y + 15)
			{
				if (alien_hitonce == false)
				{
					alien_hitonce = true;
					shipbullet_hitalien = true;
					score +=ALIENHITSCORE;
					_ltoa(score, score_ascii,10);	// convert the score to a string
					strcat (score_ascii,"\0");
				}
			}
		}

		// check if bullet has hit alienbig
		shipbullet_hitalienbig = false;
		if (ShipBulletPosition.x >= AlienBigPosition.x - 15 && ShipBulletPosition.x <= AlienBigPosition.x + 15)
		{
			if (ShipBulletPosition.y >= AlienBigPosition.y - 15 && ShipBulletPosition.y <= AlienBigPosition.y + 15)
			{
				if (alien_hitonce == false)
				{
					alien_hitonce = true;
					shipbullet_hitalienbig = true;
					score +=ALIENBIGHITSCORE;
					_ltoa(score, score_ascii,10);	// convert the score to a string
					strcat (score_ascii,"\0");
				}
			}
		}

		// check if alien is alive - this triggers the alien to fire a bullet
		if (shipbullet_hitalien)
			alien_alive = false;
		else
			alien_alive = true;

		// check if alien bullet has hit player ship
		alienbullet_hitship = false;
		if (AlienBulletPosition.x >= ShipPosition.x + 5 && AlienBulletPosition.x <= ShipPosition.x + 45)
		{
			if (AlienBulletPosition.y >= ShipPosition.y - 5 && AlienBulletPosition.y <= ShipPosition.y + 50)
			{
				alienbullet_hitship = true;
			}
		}

		// check if bullet has hit mothership
		shipbullet_hitmship1 = false;
		if (ShipBulletPosition.x >= Mship1Position.x - 15 && ShipBulletPosition.x <= Mship1Position.x + 15)
		{
			if (ShipBulletPosition.y >= Mship1Position.y - 15 && ShipBulletPosition.y <= Mship1Position.y + 15)
			{
				shipbullet_hitmship1 = true;
				score +=ALIENMOTHERSHIPHITSCORE;
				_ltoa(score, score_ascii,10);	// convert the score to a string
				strcat (score_ascii,"\0");
			}
		}

		// check if bullet has hit alienboss
		shipbullet_hitalienboss = false;
//		if (ShipBulletPosition.x == AlienBossPosition.x && ShipBulletBossPosition.y == AlienBossPosition.y)
//			shipbullet_hitalienboss = true;		// direct hit
		if (ShipBulletPosition.x >= AlienBossPosition.x - 15 && ShipBulletPosition.x <= AlienBossPosition.x + 15)
		{
			if (ShipBulletPosition.y >= AlienBossPosition.y - 15 && ShipBulletPosition.y <= AlienBossPosition.y + 15)
			{
				alien_hitonce = true;
				shipbullet_hitalienboss = true;
				score +=ALIENBOSSHITSCORE;
				_ltoa(score, score_ascii,10);	// convert the score to a string
				strcat (score_ascii,"\0");
			}
		}



		// ***** Rendering to BackBuffer Section *****
		// setup starfield (lots of white points)
		for (int i = 0; i < NUMPOINTS; i++)
		{
			points[i].x = float (rand()%WINDOW_WIDTH);
			points[i].y = float (rand()%WINDOW_HEIGHT);
			points[i].z = 0.0f;
			points[i].rhw = 1.0f;
			points[i].color = 0xffffffff;
		}

		// create a buffer in memory to hold our points
		if( FAILED( D3D_VideoDevice->CreateVertexBuffer( NUMPOINTS*sizeof(CUSTOMVERTEX),
                                                  0, D3DFVF_CUSTOMVERTEX,
                                                  D3DPOOL_DEFAULT, &StarfieldBuffer, NULL)))
		{
			MessageBox (hwnd,"Cannot create Vertex Buffer (Starfield)","RuzInvaders",MB_OK);
			return E_FAIL;
		}
	
		// lock the buffer
		if( FAILED(StarfieldBuffer->Lock(0,sizeof(points),(void**)&StarfieldMemory,0)))
		{
			MessageBox (hwnd,"Cannot lock Vertex Buffer (Starfield)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// copy vertices to buffer
		memcpy( StarfieldMemory, points, sizeof(points));

		// unlock the buffer
		if( FAILED(StarfieldBuffer->Unlock()))
		{
			MessageBox (hwnd,"Cannot unlock Vertex Buffer (Starfield)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

 		// Clear the back buffer to a black color this time
		// This clears the screen in memory (the back buffer)
		// RGB = Red, Green, Blue
		if (FAILED(D3D_VideoDevice->Clear( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,0), 1.0f, 0 )))
		{
			MessageBox (hwnd,"Failed to clear video card","RuzInvaders",MB_OK);
			return E_FAIL;

		}

		// render scene in the back buffer
		if (FAILED(D3D_VideoDevice->BeginScene()))
		{
			MessageBox (hwnd,"Failed to Render (Begin) Scene to backbuffer","RuzInvaders",MB_OK);
			return E_FAIL;
		}
		

		// draw the text
		if ((ppFont_name->DrawText( GameName,-1, prect_name, DT_CENTER, D3DCOLOR_XRGB(255,255,0))) == 0)
		{
			MessageBox (hwnd,"Failed to Draw Font (Name)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		if ((ppFont_score->DrawText( ScoreName,-1, prect_score, DT_CENTER, D3DCOLOR_XRGB(255,255,0))) == 0)
		{
			MessageBox (hwnd,"Failed to Draw Font (Score)","RuzInvaders",MB_OK);
			return E_FAIL;
		}
	
		if ((ppFont_highscore->DrawText( HighScoreName,-1, prect_highscore, DT_CENTER, D3DCOLOR_XRGB(255,255,0))) == 0)
		{
			MessageBox (hwnd,"Failed to Draw Font (High Score)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		if ((ppFont_movingscore->DrawText( MovingScoreName,-1, prect_movingscore, DT_CENTER, D3DCOLOR_XRGB(255,255,0))) == 0)
		{
			MessageBox (hwnd,"Failed to Draw Font (Moving Score)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// *** Draw the starfield background
		// needed to show points
		if (FAILED(D3D_VideoDevice->SetStreamSource( 0, StarfieldBuffer, 0, sizeof(CUSTOMVERTEX))))
		{
			MessageBox (hwnd,"Failed to set stream source (Starfield)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// needed to show points
		if (FAILED(D3D_VideoDevice->SetFVF(D3DFVF_CUSTOMVERTEX)))
		{
			MessageBox (hwnd,"Failed to set FVF Vertex Shader","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// draw the object (polygon)
		if (FAILED(D3D_VideoDevice->DrawPrimitive( D3DPT_POINTLIST, 0, NUMPOINTS)))
		{
			MessageBox (hwnd,"Failed to Draw the points (Starfield)","RuzInvaders",MB_OK);
			return E_FAIL;
		}


		// *** draw the player's ship - as a bitmap this time
		// note on colour setting: this keeps the bitmap the same
		if (alienbullet_hitship)
	{
			Wait_Clock(250);		// show explosions - wait a little
			explosion_count++;

			// wait for 6 explosions / then make the ship blink
			if (explosion_count <= 11)
			{
				alienbullet_hitship = true;
				// draw explosions in order
				if (explosion_count == 1)
					Explosion1Sprite->DrawTransform(Explosion1Texture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				if (explosion_count == 2)
				Explosion2Sprite->DrawTransform(Explosion2Texture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				if (explosion_count == 3)
				Explosion3Sprite->DrawTransform(Explosion3Texture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				if (explosion_count == 4)
				Explosion4Sprite->DrawTransform(Explosion4Texture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				if (explosion_count == 5)
				Explosion5Sprite->DrawTransform(Explosion5Texture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				if (explosion_count == 6)
				Explosion6Sprite->DrawTransform(Explosion6Texture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				if (explosion_count == 7)
				{
					// starting position of ship
					ShipPosition.x = WINDOW_WIDTH / 2.0;			// 1/2 way down screen
					ShipPosition.y = WINDOW_HEIGHT - ShipSize;		// bottom of screen
					D3DXMatrixTranslation (&ShipMove,ShipPosition.x,ShipPosition.y,0);
					ShipSprite->DrawTransform(ShipTexture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				}
				if (explosion_count == 8)
				{
					ShipPosition.x = OFF_SCREEN;	// move ship off the screen
					ShipPosition.y = OFF_SCREEN;
					D3DXMatrixTranslation (&ShipMove,ShipPosition.x,ShipPosition.y,0);
					ShipSprite->DrawTransform(ShipTexture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				}
				if (explosion_count == 9)
				{
					// starting position of ship
					ShipPosition.x = OFF_SCREEN;	// move ship off the screen
					ShipPosition.y = OFF_SCREEN;
					D3DXMatrixTranslation (&ShipMove,ShipPosition.x,ShipPosition.y,0);
					ShipSprite->DrawTransform(ShipTexture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				}
				if (explosion_count == 10)
				{
					ShipPosition.x = OFF_SCREEN;	// move ship off the screen
					ShipPosition.y = OFF_SCREEN;
					D3DXMatrixTranslation (&ShipMove,ShipPosition.x,ShipPosition.y,0);
					ShipSprite->DrawTransform(ShipTexture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				}
				if (explosion_count == 11)
				{
					// starting position of ship
					ShipPosition.x = WINDOW_WIDTH / 2.0;			// 1/2 way down screen
					ShipPosition.y = WINDOW_HEIGHT - ShipSize;		// bottom of screen
					D3DXMatrixTranslation (&ShipMove,ShipPosition.x,ShipPosition.y,0);
					ShipSprite->DrawTransform(ShipTexture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
				}

			}
			else
			{
				alienbullet_hitship = false;
				ship_count--;						// take away ship as it crashed
				AlienBulletPosition.x = OFF_SCREEN;	// move bullet off the screen
				AlienBulletPosition.y = OFF_SCREEN;
			}

		}
		else
		{
			// make sure this is reset for explosions if player ship gets hit
			explosion_count = 0;

			// draw ship
			ShipSprite->DrawTransform(ShipTexture,NULL,&ShipMove,D3DCOLOR_ARGB(0xff,255,255,255));
			// move the player's ship
			D3DXMatrixTranslation (&ShipMove,ShipPosition.x,ShipPosition.y,0);
		}

		// *** draw alien ***
		if (shipbullet_hitalien)
		{
			// move alien off the screen (easier than just erasing the alien :-)
			AlienPosition.x = OFF_SCREEN;
			AlienPosition.y = OFF_SCREEN;
		}
		else
		{
			D3DXMatrixTranslation (&AlienMove,AlienPosition.x,AlienPosition.y,0);
			AlienSprite->DrawTransform(AlienTexture,NULL,&AlienMove,D3DCOLOR_ARGB(0xff,255,255,255));
		}

//		AlienSprite->Draw(AlienTexture,NULL,NULL,NULL,0,&AlienPosition,D3DCOLOR_ARGB(0xff,255,255,255));

		// *** draw alienbig ***
		if (shipbullet_hitalienbig)
		{
			// move alien off the screen (easier than just erasing the alien :-)
			AlienBigPosition.x = OFF_SCREEN;
			AlienBigPosition.y = OFF_SCREEN;
		}
		else
		{
			D3DXMatrixTranslation (&AlienBigMove,AlienBigPosition.x,AlienBigPosition.y,0);
			AlienBigSprite->DrawTransform(AlienBigTexture,NULL,&AlienBigMove,D3DCOLOR_ARGB(0xff,255,255,255));
		}

		// *** draw ship bullet ***
		if (ship_fires)
		{
			D3DXMatrixTranslation (&ShipBulletMove,ShipBulletPosition.x,ShipBulletPosition.y,0);
			ShipBulletSprite->DrawTransform(ShipBulletTexture,NULL,&ShipBulletMove,D3DCOLOR_ARGB(0xff,255,255,255));
			ShipBulletPosition.y = ShipBulletPosition.y - SHIPBULLETSPEED;

			if (ShipBulletPosition.y < 0)
				ship_fires = false;
		}

		// *** draw alien bullet if alien is alive ***
		if (alien_alive)
		{
			D3DXMatrixTranslation (&AlienBulletMove,AlienBulletPosition.x,AlienBulletPosition.y,0);
			AlienBulletSprite->DrawTransform(AlienBulletTexture,NULL,&AlienBulletMove,D3DCOLOR_ARGB(0xff,255,255,255));
			AlienBulletPosition.y = AlienBulletPosition.y + ALIENBULLETSPEED;
		}
		if (AlienBulletPosition.y > WINDOW_HEIGHT)
		{
			if (alien_alive)
			{
				AlienBulletPosition.x = AlienPosition.x + 8;
				AlienBulletPosition.y = AlienPosition.y + 30;
			}
		}

		// *** draw mothership ***
		if (shipbullet_hitmship1)
		{
			// move mothership off screen
			Mship1Position.x = - 40.0f;
			Mship1Position.y = 50.0f;
			mship1_movingright = false;

			shipbullet_hitmship1 = false;
		}
		else
		{
			D3DXMatrixTranslation (&Mship1Move,Mship1Position.x,Mship1Position.y,0);
			Mship1Sprite->DrawTransform(Mship1Texture,NULL,&Mship1Move,D3DCOLOR_ARGB(0xff,255,255,255));
		}


		// *** draw alienboss ***
		if (shipbullet_hitalienboss)
		{
			// move alien off the screen (easier than just erasing the alien :-)
			AlienBossPosition.x = OFF_SCREEN;
			AlienBossPosition.y = OFF_SCREEN;
		}
		else
		{
			D3DXMatrixTranslation (&AlienBossMove,AlienBossPosition.x,AlienBossPosition.y,0);
			AlienSprite->DrawTransform(AlienBossTexture,NULL,&AlienBossMove,D3DCOLOR_ARGB(0xff,255,255,255));
		}


		// end the scene
		if (FAILED(D3D_VideoDevice->EndScene()))
		{
			MessageBox (hwnd,"Failed to End Scene (Render backbuffer)","RuzInvaders",MB_OK);
			return E_FAIL;
		}

		// ***** Show Rendering to Screen *****
		// show scene to video screen
		// this automatically copies the contents of the back buffer to video RAM, 
		// hence displaying it on the screen.
		if (FAILED(D3D_VideoDevice->Present( NULL, NULL, NULL, NULL )))
		{
			MessageBox (hwnd,"Failed to Render scene to video card","RuzInvaders",MB_OK);
			return E_FAIL;
		}




		// display the message on screen
		// This uses the Windows GDI system instead of DirectX
		// strcpy (message_text,"Test GDI Text!");
		// strcat (message_text,"\0");
		// HDC xdc; // the working dc
		// xdc = GetDC( NULL );
		// SetTextColor(xdc,RGB(255,0,0));
		// TextOut(xdc,0,0,message_text,strlen(message_text));
		// ReleaseDC( NULL, xdc );


		// Wait for a little while (we want to run the game at 30 frames per second)
		Wait_Clock(30);
		
	}
	else if (game_state == GAME_STATE_SHUTDOWN)
	{
		// put any game specific cleanup code here


		game_state = GAME_STATE_EXIT;
	}

	return (0);
} // end Game_Main
		


// Timing functions - used to make sure that game plays consistently
// on all computers (fast or slow)
// We try to run the game at a constant 30 frames per second

DWORD Start_Clock(void)
{
// this function starts the clock, that is, saves the current
// count, use in conjunction with Wait_Clock()

return(start_clock_count = Get_Clock());

} // end Start_Clock

///////////////////////////////////////////////////////////////

DWORD Get_Clock(void)
{
// this function returns the current tick count

// return time
return(GetTickCount());

} // end Get_Clock

///////////////////////////////////////////////////////////////

DWORD Wait_Clock(DWORD count)
{
// this function is used to wait for a specific number of clicks
// since the call to Start_Clock

while((Get_Clock() - start_clock_count) < count);
return(Get_Clock());

} // end Wait_Clock


					
















