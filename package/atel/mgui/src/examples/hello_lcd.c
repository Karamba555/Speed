#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 

#include <directfb.h>
#include "mgui_config.h"

static IDirectFB *dfb = NULL;
static IDirectFBSurface *primary = NULL;
static int screen_width  = 0;
static int screen_height = 0;

#define	KMSG_OPEN(f)	f=open("/dev/kmsg", O_RDWR)
#define	KMSG_CLOSE(f)	if (f > -1) close(f)

#define	KMSG_WRITE(f, ...)	\
	do	{ \
		char buffer	[128];	\
		if (f > -1) \
		{ \
			sprintf(buffer,  __VA_ARGS__); \
			write(f, buffer, strlen(buffer)+1); \
		} \
	} while(0);

#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
		KMSG_WRITE(kmsgfile, "DFBCHECK error\n");			   \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }
  
static IDirectFBFont *font = NULL;
static char *text = "DirectFB rulez!";
int main (int argc, char **argv)
{
  int i, width;
  int	kmsgfile;
 
  DFBFontDescription font_dsc;
  DFBSurfaceDescription dsc;
  
  KMSG_OPEN(kmsgfile);
  
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  
  DFBCHECK (DirectFBInit (&argc, &argv));
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);

  DFBCHECK (DirectFBCreate (&dfb));
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);

  DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));


  dsc.flags = DSDESC_CAPS;
  dsc.caps  = DSCAPS_PRIMARY | DSCAPS_FLIPPING;

  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));
  font_dsc.flags = DFDESC_HEIGHT;
  font_dsc.height = 48;
  KMSG_WRITE(kmsgfile, "TOMER line %d FONTDIR=%s, combined=%s\n", __LINE__, FONTDIR, FONTDIR"decker.ttf");
  DFBCHECK (dfb->CreateFont (dfb, FONTDIR"/decker.ttf", &font_dsc, &font));
  //DFBCHECK (dfb->CreateFont (dfb, "/usr/share/mgui/fonts/decker.ttf", &font_dsc, &font));
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  DFBCHECK (primary->SetFont (primary, font));
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  DFBCHECK (font->GetStringWidth (font, text, -1, &width));
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  for (i = screen_width; i > -width; i--)
  {
	//KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
      DFBCHECK (primary->SetColor (primary, 0x0, 0x0, 0x0, 0xFF));
  //KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
      DFBCHECK (primary->FillRectangle (primary, 0, 0, screen_width, screen_height));
  //KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
      DFBCHECK (primary->SetColor (primary, 0x80, 0x0, 0x20, 0xFF));
  //KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
      DFBCHECK (primary->DrawString (primary, text, -1, i, screen_height / 2, DSTF_LEFT));
  //KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
      DFBCHECK (primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC));
  //KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  }
  sleep(20);
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  font->Release (font);
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  primary->Release (primary);
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  dfb->Release (dfb);
  KMSG_WRITE(kmsgfile, "line %d\n", __LINE__);
  
  KMSG_CLOSE(kmsgfile);
  return 23;
}

