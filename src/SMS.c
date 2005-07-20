/*
#     ___  _ _      ___
#    |    | | |    |
# ___|    |   | ___|    PS2DEV Open Source Project.
#----------------------------------------------------------
# Based on ffmpeg project (no copyright notes in the original source code)
# (c) 2005 Eugene Plotnikov <e-plotnikov@operamail.com>
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/
#include "SMS.h"
#include "SMS_DSP.h"
#include "DMA.h"
#include "GUI.h"

#include <malloc.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <libpad.h>

#ifndef _WIN32
# include <kernel.h>
# include <sifrpc.h>
# include <loadfile.h>
# include <sbv_patches.h>
# include <fileXio_rpc.h>
# include <fcntl.h>
# include <libhdd.h>
# include "Timer.h"
#endif  /* _WIN32 */

# define ALIGN( x, a ) (   (  (x) + (a) - 1  ) & ~(  (a) - 1  )   )

int g_Trace;

const uint32_t g_SMS_InvTbl[ 256 ] = {
         0U, 4294967295U, 2147483648U, 1431655766U, 1073741824U,  858993460U,  715827883U,  613566757U,
 536870912U,  477218589U,  429496730U,  390451573U,  357913942U,  330382100U,  306783379U,  286331154U,
 268435456U,  252645136U,  238609295U,  226050911U,  214748365U,  204522253U,  195225787U,  186737709U,
 178956971U,  171798692U,  165191050U,  159072863U,  153391690U,  148102321U,  143165577U,  138547333U,
 134217728U,  130150525U,  126322568U,  122713352U,  119304648U,  116080198U,  113025456U,  110127367U,
 107374183U,  104755300U,  102261127U,   99882961U,   97612894U,   95443718U,   93368855U,   91382283U,
  89478486U,   87652394U,   85899346U,   84215046U,   82595525U,   81037119U,   79536432U,   78090315U,
  76695845U,   75350304U,   74051161U,   72796056U,   71582789U,   70409300U,   69273667U,   68174085U,
  67108864U,   66076420U,   65075263U,   64103990U,   63161284U,   62245903U,   61356676U,   60492498U,
  59652324U,   58835169U,   58040099U,   57266231U,   56512728U,   55778797U,   55063684U,   54366675U,
  53687092U,   53024288U,   52377650U,   51746594U,   51130564U,   50529028U,   49941481U,   49367441U,
  48806447U,   48258060U,   47721859U,   47197443U,   46684428U,   46182445U,   45691142U,   45210183U,
  44739243U,   44278014U,   43826197U,   43383509U,   42949673U,   42524429U,   42107523U,   41698712U,
  41297763U,   40904451U,   40518560U,   40139882U,   39768216U,   39403370U,   39045158U,   38693400U,
  38347923U,   38008561U,   37675152U,   37347542U,   37025581U,   36709123U,   36398028U,   36092163U,
  35791395U,   35495598U,   35204650U,   34918434U,   34636834U,   34359739U,   34087043U,   33818641U,
  33554432U,   33294321U,   33038210U,   32786010U,   32537632U,   32292988U,   32051995U,   31814573U,
  31580642U,   31350127U,   31122952U,   30899046U,   30678338U,   30460761U,   30246249U,   30034737U,
  29826162U,   29620465U,   29417585U,   29217465U,   29020050U,   28825284U,   28633116U,   28443493U,
  28256364U,   28071682U,   27889399U,   27709467U,   27531842U,   27356480U,   27183338U,   27012373U,
  26843546U,   26676816U,   26512144U,   26349493U,   26188825U,   26030105U,   25873297U,   25718368U,
  25565282U,   25414008U,   25264514U,   25116768U,   24970741U,   24826401U,   24683721U,   24542671U,
  24403224U,   24265352U,   24129030U,   23994231U,   23860930U,   23729102U,   23598722U,   23469767U,
  23342214U,   23216040U,   23091223U,   22967740U,   22845571U,   22724695U,   22605092U,   22486740U,
  22369622U,   22253717U,   22139007U,   22025474U,   21913099U,   21801865U,   21691755U,   21582751U,
  21474837U,   21367997U,   21262215U,   21157475U,   21053762U,   20951060U,   20849356U,   20748635U,
  20648882U,   20550083U,   20452226U,   20355296U,   20259280U,   20164166U,   20069941U,   19976593U,
  19884108U,   19792477U,   19701685U,   19611723U,   19522579U,   19434242U,   19346700U,   19259944U,
  19173962U,   19088744U,   19004281U,   18920561U,   18837576U,   18755316U,   18673771U,   18592933U,
  18512791U,   18433337U,   18354562U,   18276457U,   18199014U,   18122225U,   18046082U,   17970575U,
  17895698U,   17821442U,   17747799U,   17674763U,   17602325U,   17530479U,   17459217U,   17388532U,
  17318417U,   17248865U,   17179870U,   17111424U,   17043522U,   16976156U,   16909321U,   16843010
};

const uint8_t g_SMS_Log2Tbl[ 256 ] = {
 0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

uint32_t SMS_Align ( unsigned int aSize, unsigned int anAlign ) {

 return ALIGN( aSize, anAlign );

}  /* end SMS_Align */

uint32_t SMS_Linesize ( unsigned int aWidth, unsigned int* apLinesize ) {

 const int lYWidth = aWidth + 32;

 *apLinesize = ALIGN( lYWidth, 32 ) >> 4;

 return 2;

}  /* end SMS_Linesize */

void* SMS_Realloc ( void* apData, unsigned int* apSize, unsigned int aMinSize ) {

 if ( aMinSize < *apSize ) return apData;

 *apSize = 17 * aMinSize / 16 + 32;

 return realloc ( apData, *apSize );

}  /* SMS_Realloc */

typedef struct LoadParams {

 const char* m_pName;
 void*       m_pBuffer;
 int         m_BufSize;
 int         m_nArgs;
 void*       m_pArgs;

} LoadParams;

static char s_HDDArgs[] = {
 '-', 'o', '\x00', '4',      '\x00',
 '-', 'n', '\x00', '2', '0', '\x00'
};

static char s_PFSArgs[] = {
 '-', 'm', '\x00', '4',      '\x00',
 '-', 'o', '\x00', '1', '0', '\x00',
 '-', 'n', '\x00', '4', '0', '\x00'
};

static LoadParams s_LoadParams[] = {

 { "AUDSRV",   &g_DataBuffer[ SMS_AUDSRV_OFFSET   ], SMS_AUDSRV_SIZE,   0,                    NULL      },
 { "IOMANX",   &g_DataBuffer[ SMS_IOMANX_OFFSET   ], SMS_IOMANX_SIZE,   0,                    NULL      },
 { "FILEXIO",  &g_DataBuffer[ SMS_FILEXIO_OFFSET  ], SMS_FILEXIO_SIZE,  0,                    NULL      },
 { "PS2DEV9",  &g_DataBuffer[ SMS_PS2DEV9_OFFSET  ], SMS_PS2DEV9_SIZE,  0,                    NULL      },
 { "PS2ATAD",  &g_DataBuffer[ SMS_PS2ATAD_OFFSET  ], SMS_PS2ATAD_SIZE,  0,                    NULL      },
 { "PS2HDD",   &g_DataBuffer[ SMS_PS2HDD_OFFSET   ], SMS_PS2HDD_SIZE,   sizeof ( s_HDDArgs ), s_HDDArgs },
 { "PS2FS",    &g_DataBuffer[ SMS_PS2FS_OFFSET    ], SMS_PS2FS_SIZE,    sizeof ( s_PFSArgs ), s_PFSArgs },
 { "POWEROFF", &g_DataBuffer[ SMS_POWEROFF_OFFSET ], SMS_POWEROFF_SIZE, 0,                    NULL      }

};

static void LoadModule ( GUIContext* apGUICtx, int anIndex ) {

 int  lRes;
 char lBuff[ 128 ]; sprintf ( lBuff, "Loading %s...", s_LoadParams[ anIndex ].m_pName );

 apGUICtx -> Status ( lBuff );

 SifExecModuleBuffer (
  s_LoadParams[ anIndex ].m_pBuffer, s_LoadParams[ anIndex ].m_BufSize,
  s_LoadParams[ anIndex ].m_nArgs,   s_LoadParams[ anIndex ].m_pArgs, &lRes
 );

}  /* end LoadModule */

void SMS_Initialize ( void* apParam ) {
#ifndef _WIN32
 int         i;
 GUIContext* lpGUICtx = ( GUIContext* )apParam;

 DMA_Init ( D_CTRL_RELE_ON, D_CTRL_MFD_OFF, D_CTRL_STS_UNSPEC, D_CTRL_STD_OFF, D_CTRL_RCYC_8 );
 DMA_ChannelInit ( DMA_CHANNEL_GIF      );
 DMA_ChannelInit ( DMA_CHANNEL_TO_IPU   );
 DMA_ChannelInit ( DMA_CHANNEL_FROM_IPU );
 DMA_ChannelInit ( DMA_CHANNEL_TO_SPR   );

 sbv_patch_enable_lmb           ();
 sbv_patch_disable_prefix_check ();

 ChangeThreadPriority (  GetThreadId (), 64  );

 SifLoadModule ( "rom0:LIBSD", 0, NULL );

 for ( i = 0; i < sizeof ( s_LoadParams ) / sizeof ( s_LoadParams[ 0 ] ); ++i ) LoadModule ( lpGUICtx, i );

 lpGUICtx -> Status ( "Initializing SMS..." );

 hddPreparePoweroff ();

 memcpy ( SMS_DSP_SPR_CONST, &g_DataBuffer[ SMS_IDCT_CONST_OFFSET ], SMS_IDCT_CONST_SIZE );
#endif  /* _WIN32 */
 SMS_DSP_Init ();

}  /* end SMS_Initialize */