
/*
   DirectFB Tutorials

   (c) Copyright 2000-2002  convergence integrated media GmbH.
   (c) Copyright 2002       convergence GmbH.
   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de> and
              Sven Neumann <neo@directfb.org>.
              
   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * image.c
 *
 * Image loading and displaying
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <directfb.h>

int ParseArgs( int argc, char *argv[] );
/*
 * (Globals)
 */
static IDirectFB *dfb = NULL;
static IDirectFBSurface *primary = NULL;
static int screen_width  = 0;
static int screen_height = 0;
#define DFBCHECK(x...)                                         \
  {                                                            \
    DFBResult err = x;                                         \
                                                               \
    if (err != DFB_OK)                                         \
      {                                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
      }                                                        \
  }

#define MAX_FILE_NAME_SIZE 100
/*
 * The image is to be loaded into a surface that we can blit from.
 */
static IDirectFBSurface *logo = NULL;
static char m_filename[MAX_FILE_NAME_SIZE];

int ParseArgs( int argc, char *argv[] )
{
	/* Parse the command line. */
	if (argc < 2 || !argv[1] || !argv[1][0]) {
		printf("Usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	strlcpy(m_filename, argv[1], MAX_FILE_NAME_SIZE);
	printf("filename %s\n", m_filename);

	return 0;
}


int main (int argc, char **argv)
{
  int i;

  /*
   * (Locals)
   */
  DFBSurfaceDescription dsc;
  DFBImageDescription   m_dsc;
  DFBSurfaceBlittingFlags m_flags = 0;

  /*
   * Loading an image is done with an Image Provider.
   */
  IDirectFBImageProvider *provider;

  /*
   * (Initialize)
   */
  ParseArgs(argc,argv);

  DFBCHECK (DirectFBInit (&argc, &argv));
  DFBCHECK (DirectFBCreate (&dfb));
  DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));
  dsc.flags = DSDESC_CAPS;
  dsc.caps  = DSCAPS_PRIMARY | DSCAPS_FLIPPING;
  DFBCHECK (dfb->CreateSurface( dfb, &dsc, &primary ));
  DFBCHECK (primary->GetSize (primary, &screen_width, &screen_height));

  /*
   * First we need to create an Image Provider by passing a filename.
   * DirectFB will find (or not) an Image Provider for the file type.
   */
  DFBCHECK (dfb->CreateImageProvider (dfb, m_filename, &provider));
  DFBCHECK (provider->GetImageDescription (provider, &m_dsc));

  /*
   * Get a surface description from the provider. It will contain the width,
   * height, bits per pixel and the flag for an alphachannel if the image has
   * one. If the image has no alphachannel the bits per pixel is set to the
   * bits per pixel of the primary layer to use simple blitting without pixel
   * format conversion.
   */
  DFBCHECK (provider->GetSurfaceDescription (provider, &dsc));

  /*
   * Create a surface based on the description of the provider.
   */
  DFBCHECK (dfb->CreateSurface( dfb, &dsc, &logo ));
  
  /*
   * Let the provider render to our surface. Image providers are supposed to
   * support every destination pixel format and size. If the size differs the
   * image will be scaled (bilinear). The last parameter allows to specify
   * an optional destination rectangle. We use NULL here so that our image
   * covers the whole logo surface.
   */
  DFBCHECK (provider->RenderTo (provider, logo, NULL));

  /*
   * Release the provider, we don't need it anymore.
   */
  provider->Release (provider);

  if (m_dsc.caps & DICAPS_ALPHACHANNEL)
	m_flags |= DSBLIT_BLEND_ALPHACHANNEL;
  if (m_dsc.caps & DICAPS_COLORKEY) {
	m_flags |= DSBLIT_SRC_COLORKEY;
	DFBCHECK (logo->SetSrcColorKey(logo, m_dsc.colorkey_r, m_dsc.colorkey_g, m_dsc.colorkey_b));
  }
  DFBCHECK (logo->SetBlittingFlags(logo, m_flags));
  DFBCHECK (primary->SetBlittingFlags(primary, m_flags));
  /*
   * We want to let the logo slide in on the left and slide out on the right.
   */
  for (i = -dsc.width; i < screen_width; i++)
    {
      /*
       * Clear the screen.
       */
      DFBCHECK (primary->Clear(primary, 0,0,0,0));
      //DFBCHECK (primary->FillRectangle (primary, 0, 0, screen_width, screen_height));

      /*
       * Blit the logo vertically centered with "i" as the X coordinate.
       * NULL means that we want to blit the whole surface.
       */
      DFBCHECK (primary->Blit (primary, logo, NULL, i, (screen_height - dsc.height) / 2));

      /*
       * Flip the front and back buffer, but wait for the vertical retrace to
       * avoid tearing.
       */
      DFBCHECK (primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC));
    }

  /*
   * Release the image.
   */
  logo->Release (logo);

  /*
   * (Release)
   */
  primary->Release (primary);
  dfb->Release (dfb);
  
  return 23;
}

