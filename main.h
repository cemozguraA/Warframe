#include <windows.h>
#include <vector>
#include <fstream>
#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")
#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib") 
#pragma comment(lib, "winmm.lib")
#include "MinHook/include/MinHook.h" //detour
using namespace std;

#pragma warning (disable: 4244) //

//==========================================================================================================================

HMODULE dllHandle;

//Stride
UINT Stride;

//vertexshader
IDirect3DVertexShader9* vShader;
UINT vSize;

//pixelshader
IDirect3DPixelShader9* pShader;
UINT pSize;

//DPvertexshader
//IDirect3DVertexShader9* DPvShader;
//UINT DPvSize;

//DPpixelshader
//IDirect3DPixelShader9* DPpShader;
//UINT DPpSize;

// model rec
UINT mStartRegister;
UINT mVector4fCount;

//gettexture
LPDIRECT3DBASETEXTURE9 tex = NULL;
D3DSURFACE_DESC desc;
int dWidth;
int dHeight;
//int dPitch;

//esp model distance
float bestRealDistance;

//used for logging/cycling through values
bool logger = false;
int countnum = -1;
char szString[64];

bool FirstInit = false; //init once

//D3DLOCKED_RECT d3dlr; //text crc

//DWORD VirtualQReturnAddress = NULL;

//vdesc.Size
D3DVERTEXBUFFER_DESC vdesc;
D3DVERTEXBUFFER_DESC DPvdesc;

//sprite
bool MenuSpriteCreated, MenuSpriteCreated2 = NULL;
LPDIRECT3DTEXTURE9 MenuIMAGE, MenuIMAGE2;
LPD3DXSPRITE MenuSPRITE, MenuSPRITE2;
D3DXVECTOR3 MenuImagePos, MenuImagePos2;

DWORD dwStartTime = 0; //time as the timer started
DWORD dwTime = 0; //windowsuptime
//bool bResetStartTime = true; //refresh starttime

//models
#define MODELS (mStartRegister == 52 && mVector4fCount >= 82)

//shiny glow around caches and everything
#define CACHE_GLOW (Stride == 24 && vSize == 352 && pSize == 540 && mStartRegister == 12 && mVector4fCount == 1)
#define STUFFTHATSHOULDNOTGLOW (NumVertices == 258 || NumVertices == 98 || NumVertices == 428 || NumVertices == 462 || NumVertices == 507 || NumVertices == 1432 || NumVertices == 1376 || NumVertices == 355 || NumVertices == 1619)
//NumVertices == 746 small container
//NumVertices == 994 && 999 big container
//NumVertices == 258 & 462 & 507 & 98 & 428 ammo
//NumVertices == 1432 credits
//NumVertices == 355 resource
//NumVertices == 1619 mod

//yellow round thing, blue round thing, mod thing1&2&3, credits
#define YELLOWPART ((Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 768 && /*pSize == 1252 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1))

#define BLUEPART ((Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 768 && /*pSize == 1224 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 355 && primCount == 297 && vSize == 768 && /*pSize == 1492 && */mStartRegister == 18 && mVector4fCount == 1))

#define MOD1 ((Stride == 24 && NumVertices == 1619 && vSize == 768 && /*pSize == 1460 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 596 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 596 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 768 && /*pSize == 1724 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1619 && primCount == 2384 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1))

#define CREDITS ((Stride == 24 && NumVertices == 1377 && vSize == 804 && /*pSize == 2612 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1432 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 193 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 193 && vSize == 804 && /*pSize == 3144 && */mStartRegister == 15 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 776 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 776 && vSize == 804 && /*pSize == 3144 && */mStartRegister == 15 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 1376 && primCount == 582 && vSize == 352 && /*pSize == 540 && */mStartRegister == 12 && mVector4fCount == 1)||\
(Stride == 24 && NumVertices == 1376 && primCount == 582 && vSize == 804 && /*pSize == 3144 && */mStartRegister == 15 && mVector4fCount == 1))

#define GRENADE ((Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 768 && /*pSize == 1636 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 223 && primCount == 128 && vSize == 768 && /*pSize == 1636 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 223 && primCount == 249 && vSize == 768 && /*pSize == 1636 && */mStartRegister == 18 && mVector4fCount == 1)|| \
(Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 840 && /*pSize == 2076 && */mStartRegister == 18 && mVector4fCount == 1))
//(Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 608 && /*pSize == 732 && */mStartRegister == 20 && mVector4fCount == 1)|| \
//(Stride == 24 && NumVertices == 223 && primCount == 222 && vSize == 680 && /*pSize == 1280 && */mStartRegister == 15 && mVector4fCount == 1)|| \
//(Stride == 24 && NumVertices == 223 && primCount == 332 && vSize == 188 && /*pSize == 60 && */mStartRegister == 13 && mVector4fCount == 1))

//rare and reinforced grineer, may have changed with new patch
#define Rare_Grineer_Container (Stride == 24 && NumVertices == 1063 && primCount == 1184 && vSize == 840 && /*pSize == 1740 && */mStartRegister == 18 && mVector4fCount == 1)
#define Reinforced_Grineer_Container_MetalPlatesA (Stride == 24 && NumVertices == 104 && primCount == 520 && vSize == 768 && /*pSize == 1264 && */mStartRegister == 18 && mVector4fCount == 1)
#define Reinforced_Grineer_Container_MetalPlatesB (Stride == 24 && NumVertices == 440 && primCount == 520 && vSize == 768 && /*pSize == 1264 && */mStartRegister == 18 && mVector4fCount == 1)
#define Reinforced_Grineer_Container_Glow (Stride == 24 && NumVertices == 247 && primCount == 384 && vSize == 492 && /*pSize == 904 && */mStartRegister == 16 && mVector4fCount == 1)
#define Reinforced_Orokin_Container_Frame (Stride == 24 && NumVertices == 6045 && primCount == 3216 && vSize == 804 && /*pSize == 3016 && */mStartRegister == 12 && mVector4fCount == 1)
#define RareandNormal_Orokin_Container (Stride == 24 && NumVertices == 1505 && primCount == 1880 && vSize == 900 && /*pSize == 3884 && */mStartRegister == 12 && mVector4fCount == 1) //no, dfr

#define RARECONTAINER (Rare_Grineer_Container||Reinforced_Grineer_Container_MetalPlatesA||Reinforced_Grineer_Container_MetalPlatesB||Reinforced_Grineer_Container_Glow||Reinforced_Orokin_Container_Frame)

#define MISC (YELLOWPART||BLUEPART||MOD1||CREDITS||RARECONTAINER||RareandNormal_Orokin_Container)

//corpus helmet
//Stride == 24 && NumVertices == 572 && primCount == 228 && vSize == 864 && pSize == 2444 && mStartRegister == 24 && mVector4fCount == 1 && vdesc.Size == 50432

//Rare Grineer Container _/
//Rare Corpus Container
//Rare Orokin Container _/
//Reinforced Grineer Container _/
//Reinforced Corpus Container
//Reinforced Orokin Container
//Syndicate Medallions(18)

#define PLAYERS (vdesc.Size == 113216 || \
	vdesc.Size == 93312 || \
	vdesc.Size == 203456 || \
	vdesc.Size == 147328 || \
	vdesc.Size == 160064 || \
	vdesc.Size == 245504 || \
	vdesc.Size == 166272 || \
	vdesc.Size == 264640 || \
	vdesc.Size == 174080 || \
	vdesc.Size == 263040 || \
	vdesc.Size == 358912 || \
	vdesc.Size == 349920 || \
	vdesc.Size == 204512 || \
	vdesc.Size == 103904 || \
	vdesc.Size == 126752 || \
	vdesc.Size == 405984 || \
	vdesc.Size == 223872 || \
	vdesc.Size == 281088 || \
	vdesc.Size == 284160 || \
	vdesc.Size == 151168 || \
	vdesc.Size == 115200 || \
	vdesc.Size == 229696 || \
	vdesc.Size == 113696 || \
	vdesc.Size == 178464 || \
	vdesc.Size == 115968 || \
	vdesc.Size == 244736 || \
	vdesc.Size == 119808 || \
	vdesc.Size == 130208 || \
	vdesc.Size == 197952 || \
	vdesc.Size == 269632)

/*
//all player models in main menu
vdesc.Size == 113216 && Stride == 32 && NumVertices == 3538 && primCount == 5907 && vSize == 1392 && pSize == 4524 && mStartRegister == 52 && mVector4fCount == 192
vdesc.Size == 113216 && Stride == 32 && NumVertices == 3538 && primCount == 5907 && vSize == 1376 && pSize == 2328 && mStartRegister == 52 && mVector4fCount == 192
vdesc.Size == 113216 && Stride == 32 && NumVertices == 3538 && primCount == 5907 && vSize == 1392 && pSize == 4524 && mStartRegister == 52 && mVector4fCount == 192
vdesc.Size == 113216 && Stride == 32 && NumVertices == 3538 && primCount == 5907 && vSize == 1392 && pSize == 4524 && mStartRegister == 52 && mVector4fCount == 192

vdesc.Size == 93312 && Stride == 32 && NumVertices == 2916 && primCount == 4696 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 93312 && Stride == 32 && NumVertices == 2916 && primCount == 4696 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 93312 && Stride == 32 && NumVertices == 2916 && primCount == 4696 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 93312 && Stride == 32 && NumVertices == 2916 && primCount == 4696 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 203456 && Stride == 32 && NumVertices == 6358 && primCount == 9898 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 203456 && Stride == 32 && NumVertices == 6358 && primCount == 9898 && vSize == 1376 && pSize == 2628 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 203456 && Stride == 32 && NumVertices == 6358 && primCount == 9898 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 203456 && Stride == 32 && NumVertices == 6358 && primCount == 9898 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 147328 && Stride == 32 && NumVertices == 4604 && primCount == 7266 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 147328 && Stride == 32 && NumVertices == 4604 && primCount == 7266 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 147328 && Stride == 32 && NumVertices == 4604 && primCount == 7266 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 147328 && Stride == 32 && NumVertices == 4604 && primCount == 7266 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 160064 && Stride == 32 && NumVertices == 5002 && primCount == 8414 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 147
vdesc.Size == 160064 && Stride == 32 && NumVertices == 5002 && primCount == 8414 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 147
vdesc.Size == 245504 && Stride == 32 && NumVertices == 7672 && primCount == 13442 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 245504 && Stride == 32 && NumVertices == 7672 && primCount == 13442 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 166272 && Stride == 32 && NumVertices == 5196 && primCount == 8212 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 166272 && Stride == 32 && NumVertices == 5196 && primCount == 8212 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 166272 && Stride == 32 && NumVertices == 5196 && primCount == 8212 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 166272 && Stride == 32 && NumVertices == 5196 && primCount == 8212 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 264640 && Stride == 32 && NumVertices == 8270 && primCount == 8182 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 54
vdesc.Size == 264640 && Stride == 32 && NumVertices == 8270 && primCount == 8182 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 54
vdesc.Size == 264640 && Stride == 32 && NumVertices == 8270 && primCount == 5494 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 153
vdesc.Size == 264640 && Stride == 32 && NumVertices == 8270 && primCount == 5494 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 153

vdesc.Size == 174080 && Stride == 32 && NumVertices == 5440 && primCount == 8608 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 201
vdesc.Size == 174080 && Stride == 32 && NumVertices == 5440 && primCount == 8608 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 201
vdesc.Size == 174080 && Stride == 32 && NumVertices == 5440 && primCount == 8608 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 201
vdesc.Size == 174080 && Stride == 32 && NumVertices == 5440 && primCount == 8608 && vSize == 1376 && pSize == 2628 && mStartRegister == 52 && mVector4fCount == 201

vdesc.Size == 263040 && Stride == 32 && NumVertices == 8220 && primCount == 13434 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 263040 && Stride == 32 && NumVertices == 8220 && primCount == 13434 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 263040 && Stride == 32 && NumVertices == 8220 && primCount == 13434 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 263040 && Stride == 32 && NumVertices == 8220 && primCount == 13434 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 358912 && Stride == 32 && NumVertices == 11216 && primCount == 17579 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 204
vdesc.Size == 358912 && Stride == 32 && NumVertices == 11216 && primCount == 17579 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 204
vdesc.Size == 358912 && Stride == 32 && NumVertices == 11216 && primCount == 17579 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 204
vdesc.Size == 358912 && Stride == 32 && NumVertices == 11216 && primCount == 17579 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 204

vdesc.Size == 349920 && Stride == 32 && NumVertices == 10935 && primCount == 15665 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 349920 && Stride == 32 && NumVertices == 10935 && primCount == 15665 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 349920 && Stride == 32 && NumVertices == 10935 && primCount == 15665 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 349920 && Stride == 32 && NumVertices == 10935 && primCount == 15665 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 204512 && Stride == 32 && NumVertices == 6391 && primCount == 10362 && vSize == 1392 && pSize == 4268 && mStartRegister == 52 && mVector4fCount == 174
vdesc.Size == 204512 && Stride == 32 && NumVertices == 6391 && primCount == 10362 && vSize == 1376 && pSize == 1840 && mStartRegister == 52 && mVector4fCount == 174
vdesc.Size == 204512 && Stride == 32 && NumVertices == 6391 && primCount == 10362 && vSize == 1392 && pSize == 4268 && mStartRegister == 52 && mVector4fCount == 174
vdesc.Size == 204512 && Stride == 32 && NumVertices == 6391 && primCount == 10362 && vSize == 1392 && pSize == 4268 && mStartRegister == 52 && mVector4fCount == 174

vdesc.Size == 103904 && Stride == 32 && NumVertices == 3247 && primCount == 5186 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 195
vdesc.Size == 103904 && Stride == 32 && NumVertices == 3247 && primCount == 5186 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 195
vdesc.Size == 103904 && Stride == 32 && NumVertices == 3247 && primCount == 5186 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 195
vdesc.Size == 103904 && Stride == 32 && NumVertices == 3247 && primCount == 5186 && vSize == 1376 && pSize == 2628 && mStartRegister == 52 && mVector4fCount == 195

vdesc.Size == 126752 && Stride == 32 && NumVertices == 3961 && primCount == 6474 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 126752 && Stride == 32 && NumVertices == 3961 && primCount == 6474 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 126752 && Stride == 32 && NumVertices == 3961 && primCount == 6474 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 126752 && Stride == 32 && NumVertices == 3961 && primCount == 6474 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 405984 && Stride == 32 && NumVertices == 12687 && primCount == 18354 && vSize == 1392 && pSize == 4016 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 405984 && Stride == 32 && NumVertices == 12687 && primCount == 18354 && vSize == 1376 && pSize == 1812 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 405984 && Stride == 32 && NumVertices == 12687 && primCount == 18354 && vSize == 1392 && pSize == 4016 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 405984 && Stride == 32 && NumVertices == 12687 && primCount == 18354 && vSize == 1392 && pSize == 4016 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 223872 && Stride == 32 && NumVertices == 6996 && primCount == 11646 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 223872 && Stride == 32 && NumVertices == 6996 && primCount == 11646 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 223872 && Stride == 32 && NumVertices == 6996 && primCount == 11646 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 223872 && Stride == 32 && NumVertices == 6996 && primCount == 11646 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 281088 && Stride == 32 && NumVertices == 8784 && primCount == 14132 && vSize == 1356 && pSize == 2096 && mStartRegister == 52 && mVector4fCount == 183
vdesc.Size == 281088 && Stride == 32 && NumVertices == 8784 && primCount == 14132 && vSize == 1356 && pSize == 2096 && mStartRegister == 52 && mVector4fCount == 183
vdesc.Size == 281088 && Stride == 32 && NumVertices == 8784 && primCount == 14132 && vSize == 1356 && pSize == 2096 && mStartRegister == 52 && mVector4fCount == 183
vdesc.Size == 281088 && Stride == 32 && NumVertices == 8784 && primCount == 14132 && vSize == 1100 && pSize == 1056 && mStartRegister == 52 && mVector4fCount == 183

vdesc.Size == 284160 && Stride == 32 && NumVertices == 8880 && primCount == 14089 && vSize == 1392 && pSize == 4724 && mStartRegister == 52 && mVector4fCount == 186
vdesc.Size == 284160 && Stride == 32 && NumVertices == 8880 && primCount == 14089 && vSize == 1392 && pSize == 4724 && mStartRegister == 52 && mVector4fCount == 186
vdesc.Size == 284160 && Stride == 32 && NumVertices == 8880 && primCount == 14089 && vSize == 1392 && pSize == 4724 && mStartRegister == 52 && mVector4fCount == 186
vdesc.Size == 284160 && Stride == 32 && NumVertices == 8880 && primCount == 14089 && vSize == 1392 && pSize == 4724 && mStartRegister == 52 && mVector4fCount == 186

vdesc.Size == 151168 && Stride == 32 && NumVertices == 4724 && primCount == 7046 && vSize == 1100 && pSize == 1056 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 151168 && Stride == 32 && NumVertices == 4724 && primCount == 7046 && vSize == 1332 && pSize == 2096 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 151168 && Stride == 32 && NumVertices == 4724 && primCount == 7046 && vSize == 1332 && pSize == 2096 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 151168 && Stride == 32 && NumVertices == 4724 && primCount == 7046 && vSize == 1332 && pSize == 2096 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 115200 && Stride == 32 && NumVertices == 3600 && primCount == 6017 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 115200 && Stride == 32 && NumVertices == 3600 && primCount == 6017 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 115200 && Stride == 32 && NumVertices == 3600 && primCount == 6017 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 115200 && Stride == 32 && NumVertices == 3600 && primCount == 6017 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 229696 && Stride == 32 && NumVertices == 7178 && primCount == 11782 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 186
vdesc.Size == 229696 && Stride == 32 && NumVertices == 7178 && primCount == 11782 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 186
vdesc.Size == 229696 && Stride == 32 && NumVertices == 7178 && primCount == 11782 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 186
vdesc.Size == 229696 && Stride == 32 && NumVertices == 7178 && primCount == 11782 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 186

vdesc.Size == 113696 && Stride == 32 && NumVertices == 3553 && primCount == 5886 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 195
vdesc.Size == 113696 && Stride == 32 && NumVertices == 3553 && primCount == 5886 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 195
vdesc.Size == 113696 && Stride == 32 && NumVertices == 3553 && primCount == 5886 && vSize == 1100 && pSize == 1020 && mStartRegister == 52 && mVector4fCount == 195
vdesc.Size == 113696 && Stride == 32 && NumVertices == 3553 && primCount == 5886 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 195

vdesc.Size == 178464 && Stride == 32 && NumVertices == 5577 && primCount == 8811 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 183
vdesc.Size == 178464 && Stride == 32 && NumVertices == 5577 && primCount == 8811 && vSize == 1376 && pSize == 2612 && mStartRegister == 52 && mVector4fCount == 183
vdesc.Size == 178464 && Stride == 32 && NumVertices == 5577 && primCount == 8811 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 183
vdesc.Size == 178464 && Stride == 32 && NumVertices == 5577 && primCount == 8811 && vSize == 1392 && pSize == 4708 && mStartRegister == 52 && mVector4fCount == 183

vdesc.Size == 115968 && Stride == 32 && NumVertices == 3624 && primCount == 5562 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 115968 && Stride == 32 && NumVertices == 3624 && primCount == 5562 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 115968 && Stride == 32 && NumVertices == 3624 && primCount == 5562 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 115968 && Stride == 32 && NumVertices == 3624 && primCount == 5562 && vSize == 1356 && pSize == 2060 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 244736 && Stride == 32 && NumVertices == 7648 && primCount == 11306 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 244736 && Stride == 32 && NumVertices == 7648 && primCount == 11306 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 244736 && Stride == 32 && NumVertices == 7648 && primCount == 11306 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 244736 && Stride == 32 && NumVertices == 7648 && primCount == 11306 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 119808 && Stride == 32 && NumVertices == 3744 && primCount == 6006 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 119808 && Stride == 32 && NumVertices == 3744 && primCount == 6006 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 119808 && Stride == 32 && NumVertices == 3744 && primCount == 6006 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 119808 && Stride == 32 && NumVertices == 3744 && primCount == 6006 && vSize == 1356 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 130208 && Stride == 32 && NumVertices == 4069 && primCount == 6542 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 130208 && Stride == 32 && NumVertices == 4069 && primCount == 6542 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 130208 && Stride == 32 && NumVertices == 4069 && primCount == 6542 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189
vdesc.Size == 130208 && Stride == 32 && NumVertices == 4069 && primCount == 6542 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 189

vdesc.Size == 197952 && Stride == 32 && NumVertices == 6186 && primCount == 8148 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 197952 && Stride == 32 && NumVertices == 6186 && primCount == 8148 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 197952 && Stride == 32 && NumVertices == 6186 && primCount == 8148 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 197952 && Stride == 32 && NumVertices == 6186 && primCount == 8148 && vSize == 1392 && pSize == 4700 && mStartRegister == 52 && mVector4fCount == 177

vdesc.Size == 269632 && Stride == 32 && NumVertices == 8426 && primCount == 13302 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 269632 && Stride == 32 && NumVertices == 8426 && primCount == 13302 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 269632 && Stride == 32 && NumVertices == 8426 && primCount == 13302 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
vdesc.Size == 269632 && Stride == 32 && NumVertices == 8426 && primCount == 13302 && vSize == 1332 && pSize == 2076 && mStartRegister == 52 && mVector4fCount == 177
*/

//==========================================================================================================================

D3DVIEWPORT9 Viewport; //use this viewport
float ScreenCenterX;
float ScreenCenterY;

// getdir & log
char dlldir[320];
char* GetDirectoryFile(char *filename)
{
	static char path[320];
	strcpy_s(path, dlldir);
	strcat_s(path, filename);
	return path;
}

void Log(const char *fmt, ...)
{
	if (!fmt)	return;

	char		text[4096];
	va_list		ap;
	va_start(ap, fmt);
	vsprintf_s(text, fmt, ap);
	va_end(ap);

	ofstream logfile(GetDirectoryFile("log.txt"), ios::app);
	if (logfile.is_open() && text)	logfile << text << endl;
	logfile.close();
}
/*
DWORD qCRC;
void *pData;
DWORD QuickChecksum(DWORD *pData, int size)
{
if (!pData) { return 0x0; }

DWORD sum;
DWORD tmp;
sum = *pData;

for (int i = 1; i < (size / 4); i++)
{
tmp = pData[i];
tmp = (DWORD)(sum >> 29) + tmp;
tmp = (DWORD)(sum >> 17) + tmp;
sum = (DWORD)(sum << 3) ^ tmp;
}

return sum;
}
*/
//==========================================================================================================================

// colors
#define Green				D3DCOLOR_ARGB(255, 000, 255, 000)
#define Red					D3DCOLOR_ARGB(255, 255, 000, 000)
#define Blue				D3DCOLOR_ARGB(255, 000, 000, 255)
#define Orange				D3DCOLOR_ARGB(255, 255, 165, 000)
#define Yellow				D3DCOLOR_ARGB(255, 255, 255, 000)
#define Pink				D3DCOLOR_ARGB(255, 255, 192, 203)
#define Cyan				D3DCOLOR_ARGB(255, 000, 255, 255)
#define Purple				D3DCOLOR_ARGB(255, 160, 032, 240)
#define Black				D3DCOLOR_ARGB(255, 000, 000, 000) 
#define White				D3DCOLOR_ARGB(255, 255, 255, 255)
#define Grey				D3DCOLOR_ARGB(255, 112, 112, 112)
#define SteelBlue			D3DCOLOR_ARGB(255, 033, 104, 140)
#define LightSteelBlue		D3DCOLOR_ARGB(255, 201, 255, 255)
#define LightBlue			D3DCOLOR_ARGB(255, 026, 140, 306)
#define Salmon				D3DCOLOR_ARGB(255, 196, 112, 112)
#define Brown				D3DCOLOR_ARGB(255, 168, 099, 020)
#define Teal				D3DCOLOR_ARGB(255, 038, 140, 140)
#define Lime				D3DCOLOR_ARGB(255, 050, 205, 050)
#define ElectricLime		D3DCOLOR_ARGB(255, 204, 255, 000)
#define Gold				D3DCOLOR_ARGB(255, 255, 215, 000)
#define OrangeRed			D3DCOLOR_ARGB(255, 255, 69, 0)
#define GreenYellow			D3DCOLOR_ARGB(255, 173, 255, 047)
#define AquaMarine			D3DCOLOR_ARGB(255, 127, 255, 212)
#define SkyBlue				D3DCOLOR_ARGB(255, 000, 191, 255)
#define SlateBlue			D3DCOLOR_ARGB(255, 132, 112, 255)
#define Crimson				D3DCOLOR_ARGB(255, 220, 020, 060)
#define DarkOliveGreen		D3DCOLOR_ARGB(255, 188, 238, 104)
#define PaleGreen			D3DCOLOR_ARGB(255, 154, 255, 154)
#define DarkGoldenRod		D3DCOLOR_ARGB(255, 255, 185, 015)
#define FireBrick			D3DCOLOR_ARGB(255, 255, 048, 048)
#define DarkBlue			D3DCOLOR_ARGB(255, 000, 000, 204)
#define DarkerBlue			D3DCOLOR_ARGB(255, 000, 000, 153)
#define DarkYellow			D3DCOLOR_ARGB(255, 255, 204, 000)
#define LightYellow			D3DCOLOR_ARGB(255, 255, 255, 153)
#define DarkOutline			D3DCOLOR_ARGB(255, 37,   48,  52)
#define TBlack				D3DCOLOR_ARGB(180, 000, 000, 000) 

//==========================================================================================================================

// menu

int MenuSelection = 0;
int Current = true;

int PosX = 30;
int PosY = 27;

int Show = false; //off by default

POINT cPos;

#define ItemColorOn Green
#define ItemColorOff Red
#define ItemCurrent White
#define GroupColor Yellow
#define KategorieFarbe Yellow
#define ItemText White

LPD3DXFONT pFont; //font
bool m_bCreated = false;

int CheckTabs(int x, int y, int w, int h)
{
	if (Show)
	{
		GetCursorPos(&cPos);
		ScreenToClient(GetForegroundWindow(), &cPos);
		if (cPos.x > x && cPos.x < x + w && cPos.y > y && cPos.y < y + h)
		{
			if (GetAsyncKeyState(VK_LBUTTON) & 1)
			{
				return 1;
			}
			return 2;
		}
	}
	return 0;
}

void FillRGB(LPDIRECT3DDEVICE9 pDevice, int x, int y, int w, int h, D3DCOLOR color)
{
	D3DRECT rec = { x, y, x + w, y + h };
	pDevice->Clear(1, &rec, D3DCLEAR_TARGET, color, 0, 0);
}

HRESULT DrawRectangle(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT w, FLOAT h, DWORD Color)
{
	HRESULT hRet;

	const DWORD D3D_FVF = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}
	V[4] =
	{
		{ x, (y + h), 0.0f, 0.0f, Color },
		{ x, y, 0.0f, 0.0f, Color },
		{ (x + w), (y + h), 0.0f, 0.0f, Color },
		{ (x + w), y, 0.0f, 0.0f, Color }
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3D_FVF);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	}

	return hRet;
}

VOID DrawBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, INT px, DWORD BorderColor)
{
	DrawRectangle(Device, x, (y + h - px), w, px, BorderColor);
	DrawRectangle(Device, x, y, px, h, BorderColor);
	DrawRectangle(Device, x, y, w, px, BorderColor);
	DrawRectangle(Device, (x + w - px), y, px, h, BorderColor);
}

VOID DrawBoxWithBorder(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor, DWORD BorderColor)
{
	DrawRectangle(Device, x, y, w, h, BoxColor);
	DrawBorder(Device, x, y, w, h, 1, BorderColor);
}

VOID DrawBox(LPDIRECT3DDEVICE9 Device, INT x, INT y, INT w, INT h, DWORD BoxColor)
{
	DrawBorder(Device, x, y, w, h, 1, BoxColor);
}

void DrawBox2(LPDIRECT3DDEVICE9 pD3D9, int x, int y, int w, int h, D3DCOLOR Color)
{
	struct Vertex
	{
		float x, y, z, ht;
		DWORD Color;
	}
	V[4] = { { x, y + h, 0.0f, 0.0f, Color },{ x, y, 0.0f, 0.0f, Color },{ x + w, y + h, 0.0f, 0.0f, Color },{ x + w, y, 0.0f, 0.0f, Color } };
	pD3D9->SetTexture(0, NULL);
	pD3D9->SetPixelShader(0);
	pD3D9->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pD3D9->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pD3D9->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	pD3D9->SetRenderState(D3DRS_ZENABLE, FALSE);
	pD3D9->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pD3D9->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, V, sizeof(Vertex));
	return;
}

void DrawBox3(IDirect3DDevice9* m_pD3Ddev, int x, int y, int w, int h, D3DCOLOR Color)
{
	D3DRECT rec;
	rec.x1 = x;
	rec.x2 = x + w;
	rec.y1 = y;
	rec.y2 = y + h;
	m_pD3Ddev->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	m_pD3Ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	m_pD3Ddev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pD3Ddev->SetRenderState(D3DRS_ALPHABLENDENABLE, D3DPT_TRIANGLESTRIP);
	m_pD3Ddev->Clear(1, &rec, D3DCLEAR_TARGET, Color, 1, 1);
}

void WriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_LEFT, color);
}

void lWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_RIGHT, color);
}

void cWriteText(int x, int y, DWORD color, char *text)
{
	RECT rect;
	SetRect(&rect, x, y, x, y);
	pFont->DrawText(0, text, -1, &rect, DT_NOCLIP | DT_CENTER, color);
}

HRESULT DrawString(LPD3DXFONT pFont, INT X, INT Y, DWORD dColor, CONST PCHAR cString, ...)
{
	HRESULT hRet;

	CHAR buf[512] = { NULL };
	va_list ArgumentList;
	va_start(ArgumentList, cString);
	_vsnprintf_s(buf, sizeof(buf), sizeof(buf) - strlen(buf), cString, ArgumentList);
	va_end(ArgumentList);

	RECT rc[2];
	SetRect(&rc[0], X, Y, X, 0);
	SetRect(&rc[1], X, Y, X + 50, 50);

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		pFont->DrawTextA(NULL, buf, -1, &rc[0], DT_NOCLIP, 0xFF000000);
		hRet = pFont->DrawTextA(NULL, buf, -1, &rc[1], DT_NOCLIP, dColor);
	}

	return hRet;
}

void Category(LPDIRECT3DDEVICE9 pDevice, char *text)
{
	if (Show)
	{
		int Check = CheckTabs(PosX, PosY + (Current * 15), 190, 10);
		DWORD ColorText;

		ColorText = KategorieFarbe;

		if (Check == 2)
			ColorText = ItemCurrent;

		if (MenuSelection == Current)
			ColorText = ItemCurrent;

		WriteText(PosX - 5, PosY + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 175, PosY + (Current * 15) - 1, ColorText, "[-]");
		Current++;
	}
}

//-----------------------------------------------------------------------------
// Name: Save()
// Desc: Saves Menu Item states for later Restoration
//-----------------------------------------------------------------------------

void Save(char* szSection, char* szKey, int iValue, LPCSTR file)
{
	char szValue[255];
	sprintf_s(szValue, "%d", iValue);
	WritePrivateProfileString(szSection, szKey, szValue, file);
}

//-----------------------------------------------------------------------------
// Name: Load()
// Desc: Loads Menu Item States From Previously Saved File
//-----------------------------------------------------------------------------

int Load(char* szSection, char* szKey, int iDefaultValue, LPCSTR file)
{
	int iResult = GetPrivateProfileInt(szSection, szKey, iDefaultValue, file);
	return iResult;
}

void AddItem(LPDIRECT3DDEVICE9 pDevice, char *text, int &var, char **opt, int MaxValue)
{
	if (Show)
	{
		int Check = CheckTabs(PosX, PosY + (Current * 15), 190, 10);
		DWORD ColorText;

		if (var)
		{
			DrawBox(pDevice, PosX, PosY + (Current * 15), 10, 10, Green);
			ColorText = ItemColorOn;
		}
		if (var == 0)
		{
			DrawBox(pDevice, PosX, PosY + (Current * 15), 10, 10, Red);
			ColorText = ItemColorOff;
		}

		if (Check == 1)
		{
			var++;
			if (var > MaxValue)
				var = 0;
		}

		if (Check == 2)
			ColorText = ItemCurrent;

		if (MenuSelection == Current)
		{
			if (GetAsyncKeyState(VK_RIGHT) & 1)
			{
				var++;
				if (var > MaxValue)
					var = 0;
			}
			else if (GetAsyncKeyState(VK_LEFT) & 1)
			{
				var--;
				if (var < 0)
					var = MaxValue;
			}
		}

		if (MenuSelection == Current)
			ColorText = ItemCurrent;

		WriteText(PosX + 13, PosY + (Current * 15) - 1, ColorText, text);
		lWriteText(PosX + 148, PosY + (Current * 15) - 1, ColorText, opt[var]);
		Current++;
	}
}

//==========================================================================================================================

// crosshair
HRESULT DoubleLine(LPDIRECT3DDEVICE9 Device, FLOAT x, FLOAT y, FLOAT x2, FLOAT y2, DWORD Color)
{
	HRESULT hRet;

	const DWORD D3D_FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;

	struct Vertex
	{
		float x, y, z, ht;
		DWORD vcolor;
	}

	V[2] =
	{
		{ (float)x, (float)y, 0.0f, 1.0f, Color },
		{ (float)x2, (float)y2, 0.0f, 1.0f, Color },
	};

	hRet = D3D_OK;

	if (SUCCEEDED(hRet))
	{
		Device->SetPixelShader(0); //fix black color
		Device->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		Device->SetFVF(D3D_FVF);
		Device->SetTexture(0, NULL);
		hRet = Device->DrawPrimitiveUP(D3DPT_LINELIST, 2, V, sizeof(Vertex));
	}

	return hRet;
}

VOID DrawCrosshair(LPDIRECT3DDEVICE9 Device)
{
	INT center_x = (Viewport.Width / 2);
	INT center_y = (Viewport.Height / 2);
	DrawBox(Device, center_x - 5, center_y - 1, 11, 3, Black);
	DrawBox(Device, center_x - 1, center_y - 5, 3, 11, Black);
	DoubleLine(Device, center_x - 4, center_y, center_x + 5, center_y, White);
	DoubleLine(Device, center_x, center_y - 4, center_x, center_y + 5, White);
}

//==========================================================================================================================

IDirect3DPixelShader9 *shadRed;
IDirect3DPixelShader9 *shadGreen;
IDirect3DPixelShader9 *shadBlue;
IDirect3DPixelShader9 *shadDepthBlue;
IDirect3DPixelShader9 *shadDepthGreen;
IDirect3DPixelShader9 *shadYellow;
IDirect3DPixelShader9 *shadViolet;
IDirect3DPixelShader9 *shadSiren;
IDirect3DPixelShader9 *shadOrange;
IDirect3DPixelShader9 *shadLimon;
IDirect3DPixelShader9 *shadPink;
IDirect3DPixelShader9 *shadWhite;

//generate shader
HRESULT GenerateShader(IDirect3DDevice9 *pDevice, IDirect3DPixelShader9 **pShader, float r, float g, float b, float a, bool setzBuf)
{
	char szShader[256];
	ID3DXBuffer *pShaderBuf = NULL;
	D3DCAPS9 caps;
	pDevice->GetDeviceCaps(&caps);
	int PXSHVER1 = (D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion));
	int PXSHVER2 = (D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion));
	if (setzBuf)
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", PXSHVER1, PXSHVER2, r, g, b, a);
	else
		sprintf_s(szShader, "ps.%d.%d\ndef c0, %f, %f, %f, %f\nmov oC0,c0", PXSHVER1, PXSHVER2, r, g, b, a);
	D3DXAssembleShader(szShader, sizeof(szShader), NULL, NULL, 0, &pShaderBuf, NULL);
	if (FAILED(pDevice->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
	return S_OK;
}

/*
IDirect3DPixelShader9 *sSubA = NULL;
IDirect3DPixelShader9 *sSubB = NULL;
char ShaderSubA[] = "ps.1.0 def c0, 0.0f, 0.0f, 1.0f, 0.5f tex t0 sub r0, c0, t0";
char ShaderSubB[] = "ps.1.0 def c0, 0.0f, 0.0f, 0.5f, 0.5f tex t0 sub r0, c0, t0";
ID3DXBuffer *ShaderBufferSubA = NULL;
ID3DXBuffer *ShaderBufferSubB = NULL;

HRESULT GenerateShader(IDirect3DDevice9 *pD3Ddev, IDirect3DPixelShader9 **pShader, float r, float g, float b, bool setzBuf)
{
char szShader[256];
ID3DXBuffer *pShaderBuf = NULL;
if (setzBuf)
sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov oC0,c0\nmov oDepth, c0.x", r, g, b, 1.0f);
else
sprintf_s(szShader, "ps_3_0\ndef c0, %f, %f, %f, %f\nmov oC0,c0", r, g, b, 1.0f);
D3DXAssembleShader(szShader, (strlen(szShader)), NULL, NULL, 0, &pShaderBuf, NULL);
if (FAILED(pD3Ddev->CreatePixelShader((const DWORD*)pShaderBuf->GetBufferPointer(), pShader)))return E_FAIL;
return S_OK;
}
*/

//==========================================================================================================================
/*
//generate texture
LPDIRECT3DTEXTURE9 texRed;
LPDIRECT3DTEXTURE9 texGreen;
LPDIRECT3DTEXTURE9 texBlue;
LPDIRECT3DTEXTURE9 texYellow;
LPDIRECT3DTEXTURE9 texTur;

const BYTE bRed[60] =
{
0x42, 0x4D, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0xFF, 0x00, 0x00, 0x00
};

const BYTE bGreen[60] =
{
0x42, 0x4D, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xFF, 0x00, 0x00, 0x00, 0x00
};

const BYTE bBlue[60] =
{
0x42, 0x4D, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0x00, 0x00, 0x00, 0x00, 0x00
};

const BYTE bYellow[60] =
{
0x42, 0x4D, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00,
0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00
};

const BYTE bTur[60] = //turquoise
{
0x42, 0x4D, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01,
0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x12, 0x0B, 0x00, 0x00, 0x12, 0x0B, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00
};
*/

//==========================================================================================================================

//draw shader (may not work in all games)
IDirect3DPixelShader9 *ellipse = NULL;

DWORD deffault_color8[] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };
struct VERTEX
{
	float x, y, z, rhw;
	DWORD color;
	float tu, tv;
};
DWORD FVF = D3DFVF_XYZRHW | D3DFVF_TEX1 | D3DFVF_DIFFUSE;

int DX9CreateEllipseShader(LPDIRECT3DDEVICE9 Device)
{
	char vers[100];
	char *strshader = "\
					  float4 radius: register(c0);\
					  sampler mytexture;\
					  struct VS_OUTPUT\
					  {\
					  float4 Pos : SV_POSITION;\
					  float4 Color : COLOR;\
					  float2 TexCoord : TEXCOORD;\
					  };\
					  float4 PS(VS_OUTPUT input) : SV_TARGET\
					  {\
					  if( ( (input.TexCoord[0]-0.5)*(input.TexCoord[0]-0.5) + (input.TexCoord[1]-0.5)*(input.TexCoord[1]-0.5) <= 0.5*0.5) &&\
					  ( (input.TexCoord[0]-0.5)*(input.TexCoord[0]-0.5) + (input.TexCoord[1]-0.5)*(input.TexCoord[1]-0.5) >= radius[0]*radius[0]) )\
					  return input.Color;\
					  else return float4(0,0,0,0);\
					  };";

	D3DCAPS9 caps;
	Device->GetDeviceCaps(&caps);
	UINT V1 = D3DSHADER_VERSION_MAJOR(caps.PixelShaderVersion);
	UINT V2 = D3DSHADER_VERSION_MINOR(caps.PixelShaderVersion);
	sprintf(vers, "ps_%d_%d", V1, V2);
	LPD3DXBUFFER pshader;
	D3DXCompileShader(strshader, strlen(strshader), 0, 0, "PS", vers, 0, &pshader, 0, 0);
	if (pshader == NULL)
	{
		//MessageBoxA(0, "pshader == NULL", 0, 0);
		return 1;
	}
	Device->CreatePixelShader((DWORD*)pshader->GetBufferPointer(), (IDirect3DPixelShader9**)&ellipse);
	if (!ellipse)
	{
		//MessageBoxA(0, "ellipseshader == NULL", 0, 0);
		return 2;
	}

	memset(strshader, 0, strlen(strshader));
	pshader->Release();
	return 0;
}

//IDirect3DVertexBuffer9 *vb = 0;
//IDirect3DIndexBuffer9 *ib = 0;
int DX9DrawEllipse(LPDIRECT3DDEVICE9 Device, float x, float y, float w, float h, float linew, DWORD *color)
{
	if (!Device)return 1;
	static IDirect3DVertexBuffer9 *vb = 0;
	static IDirect3DIndexBuffer9 *ib = 0;
	static IDirect3DSurface9 *surface = 0;
	static IDirect3DTexture9 *pstexture = 0;

	//Device->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_WRITEONLY, FVF, D3DPOOL_MANAGED, &vb, NULL);
	Device->CreateVertexBuffer(sizeof(VERTEX) * 4, D3DUSAGE_WRITEONLY, FVF, D3DPOOL_DEFAULT, &vb, NULL);
	if (!vb) { MessageBoxA(0, "DrawEllipse error vb", 0, 0); return 2; }

	//Device->CreateIndexBuffer((3 * 2) * 2, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &ib, NULL);
	Device->CreateIndexBuffer((3 * 2) * 2, D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &ib, NULL);
	if (!ib) { MessageBoxA(0, "DrawEllipse error ib", 0, 0); return 3; }

	if (!color)color = deffault_color8;
	float tu = 0, tv = 0;
	float tw = 1.0, th = 1.0;
	VERTEX v[4] = { { x, y, 0, 1, color[0], tu, tv },{ x + w, y, 0, 1, color[1], tu + tw, tv },{ x + w, y + h, 0, 1, color[2], tu + tw, tv + th },{ x, y + h, 0, 1, color[3], tu, tv + th } };
	WORD i[2 * 3] = { 0, 1, 2, 2, 3, 0 };
	void *p;
	vb->Lock(0, sizeof(v), &p, 0);
	memcpy(p, v, sizeof(v));
	vb->Unlock();

	ib->Lock(0, sizeof(i), &p, 0);
	memcpy(p, i, sizeof(i));
	ib->Unlock();

	float radius[4] = { 0, w, h, 0 };

	radius[0] = (linew) / w;
	if (radius[0]>0.5)radius[0] = 0.5;
	radius[0] = 0.5 - radius[0];

	Device->SetPixelShaderConstantF(0, radius, 1);
	Device->SetFVF(FVF);
	Device->SetTexture(0, 0);
	Device->SetPixelShader((IDirect3DPixelShader9*)ellipse);
	Device->SetVertexShader(0);
	Device->SetStreamSource(0, vb, 0, sizeof(VERTEX));
	Device->SetIndices(ib);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 4, 0, 2);
	if (vb != NULL) { vb->Release(); }
	if (ib != NULL) { ib->Release(); }

	return 0;
};

