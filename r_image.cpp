//	r_image.cpp
//

#include "local.h"
#pragma hdrstop

#include "resource.h"
#include <malloc.h>

#define	MAX_IMAGES	256
#define	TEXNUM_IMAGES	1024

typedef struct image_s {
	int		target;
	float	u, v;		//	maximum u v coords
} image_t;

image_t	g_images[ MAX_IMAGES ];
int		numImages	= 0;

rimage_t cRender::LoadImage( const char *szFilename ) {
	HANDLE	hImage;
	BITMAP	bm;

	byte *	buffer;
	byte *	copyBuf;
	int		w, h;
	int		i, j;

	if ( numImages == MAX_IMAGES ) {
		return -1;
	}

	//	try loading from resources first
	if ( (hImage = ::LoadImage( g_Application->get_hInstance( ), szFilename, IMAGE_BITMAP, 0, 0,
		LR_CREATEDIBSECTION ) ) == NULL ) {
		int	err = GetLastError( );
		//	try loading from filesystem
		hImage = ::LoadImage( NULL, szFilename, IMAGE_BITMAP, 0, 0,
			LR_CREATEDIBSECTION | LR_DEFAULTSIZE | LR_LOADFROMFILE );
	}

	if ( hImage == NULL ) {
		return -1;
	}

	GetObject( hImage, sizeof(bm), &bm );

	for ( w=2 ; w<bm.bmWidth; w<<=1 )
		;
	for ( h=2 ; h<bm.bmHeight ; h<<=1 )
		;

	buffer = (byte *)_malloca( w * h * 4 );
	copyBuf = (byte *)_malloca( bm.bmWidthBytes * bm.bmHeight );
	
	GetBitmapBits( (HBITMAP )hImage, bm.bmWidthBytes * bm.bmHeight, copyBuf );

	memset( buffer, 0, w * h * 4 );

	for ( j=0 ; j<bm.bmHeight ; j++ ) {
		for ( i=0 ; i<bm.bmWidth ; i++ ) {
			buffer[ 4*(j*w+i)+0 ] = copyBuf[ 3*(j*bm.bmWidth+i)+2 ];
			buffer[ 4*(j*w+i)+1 ] = copyBuf[ 3*(j*bm.bmWidth+i)+1 ];
			buffer[ 4*(j*w+i)+2 ] = copyBuf[ 3*(j*bm.bmWidth+i)+0 ];
			buffer[ 4*(j*w+i)+3 ] = ( buffer[ 4*(j*w+i)+0 ] || buffer[ 4*(j*w+i)+1 ] || buffer[ 4*(j*w+i)+2 ] ) ? 255 : 0;
		}
	}

	g_images[ numImages ].target = numImages + TEXNUM_IMAGES;
	g_images[ numImages ].u = (float )bm.bmWidth / (float )w;
	g_images[ numImages ].v = (float )bm.bmHeight / (float )h;

	glBindTexture( GL_TEXTURE_2D, g_images[ numImages ].target );
	glTexImage2D( GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	DeleteObject( hImage );

	return numImages++;
}

void cRender::DrawImage( rimage_t img, vec2 org, vec2 sz, vec4 color ) {
	if ( img < 0 || img >= MAX_IMAGES ) {
		return;
	}

	glMatrixMode( GL_TEXTURE );
	glLoadIdentity( );

	glScalef( g_images[ img ].u, g_images[ img ].v, 1.0f );

	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, g_images[ img ].target );

	glColor4f( color.r, color.g, color.b, color.a );

	glBegin( GL_TRIANGLE_STRIP );
		glTexCoord2f( 0.0f, 0.0f );
		glVertex2f( org.x, org.y );

		glTexCoord2f( 1.0f, 0.0f );
		glVertex2f( org.x + sz.x, org.y );

		glTexCoord2f( 0.0f, 1.0f );
		glVertex2f( org.x, org.y + sz.y );

		glTexCoord2f( 1.0f, 1.0f );
		glVertex2f( org.x + sz.x, org.y + sz.y );
	glEnd( );

	glDisable( GL_TEXTURE_2D );
}