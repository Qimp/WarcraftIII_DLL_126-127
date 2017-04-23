#include "Main.h"
#include "blpaletter.h"


vector<RawImageStruct> ListOfRawImages;


double pDistance( int x1, int y1, int x2, int y2 )
{
	return sqrt( ( x2 - x1 )*( x2 - x1 ) + ( y2 - y1 )*( y2 - y1 ) );
}

// ������� RawImage (RGBA) � ��������� ������
int __stdcall CreateRawImage( int width, int height, RGBAPix defaultcolor )
{
	int resultid = ListOfRawImages.size( );
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
	cout << "CreateRawImage:" << endl;
#endif
	width = width + ( width % 2 );
	height = height + ( height % 2 );

	RawImageStruct tmpRawImage = RawImageStruct( );
	Buffer tmpRawImageBuffer = Buffer( );
	tmpRawImageBuffer.Resize( width * height * 4 );

	for ( int i = 0; i < width * height; i++ )
	{
		*( RGBAPix* )&tmpRawImageBuffer[ i * 4 ] = defaultcolor;
	}

	tmpRawImage.img = tmpRawImageBuffer;
	tmpRawImage.width = width;
	tmpRawImage.height = height;
	tmpRawImage.filename = string( );
	tmpRawImage.RawImage = resultid;
	ListOfRawImages.push_back( tmpRawImage );

	return resultid;
}

// ��������� RawImage �� filename (tga,blp)
int __stdcall LoadRawImage( const char * filename )
{
	int resultid = ListOfRawImages.size( );
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
	cout << "LoadRawImage:" << endl;
#endif

	int filenamelen = strlen( filename );


	int PatchFileData = 0;
	size_t PatchFileSize = 0;
	GameGetFile_ptr( filename, &PatchFileData, &PatchFileSize, TRUE );
	if ( !PatchFileData || !PatchFileSize )
	{
		GameGetFile_ptr( ( filename + string( ".tga" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
		if ( !PatchFileData || !PatchFileSize )
		{
			GameGetFile_ptr( ( filename + string( ".blp" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
			if ( !PatchFileData || !PatchFileSize )
			{
				if ( filenamelen >= 4 )
				{
					char * tmpfilename = new char[ filenamelen ];
					memset( tmpfilename, 0, filenamelen );
					memcpy( tmpfilename, filename, filenamelen - 4 );
					GameGetFile_ptr( ( tmpfilename + string( ".blp" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
					if ( !PatchFileData || !PatchFileSize )
					{
						GameGetFile_ptr( ( tmpfilename + string( ".tga" ) ).c_str( ), &PatchFileData, &PatchFileSize, TRUE );
					}

					delete[ ] tmpfilename;
				}

			}
		}
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
	cout << "LoadRawImage2:" << endl;
#endif
	if ( PatchFileData &&  PatchFileSize > 5 )
	{
		BOOL IsBlp = memcmp( ( LPCVOID )PatchFileData, "BLP1", 4 ) == 0;
		int w = 0, h = 0, bpp = 0, mipmaps = 0, alphaflag = 8, compress = 1, alphaenconding = 5;
		unsigned long rawImageSize = 0;

		Buffer OutBuffer = Buffer( );
		Buffer InBuffer( ( char * )PatchFileData, PatchFileSize );

		if ( !IsBlp )
			rawImageSize = ( unsigned long )TGA2Raw( InBuffer, OutBuffer, w, h, bpp, filename );
		else
			rawImageSize = Blp2Raw( InBuffer, OutBuffer, w, h, bpp, mipmaps, alphaflag, compress, alphaenconding, filename );

		if ( rawImageSize > 0 )
		{
			RawImageStruct tmpRawImage = RawImageStruct( );
			tmpRawImage.img = OutBuffer;
			tmpRawImage.width = w;
			tmpRawImage.height = h;
			tmpRawImage.filename = filename;
			tmpRawImage.RawImage = resultid;
			ListOfRawImages.push_back( tmpRawImage );
		}
	}
	else return 0;

	return resultid;
}


enum BlendModes : int
{
	BlendNormal,
	BlendAdd,
	BlendSubtract,
	BlendMultiple

};

// ������ RawImage2 �� RawImage
int __stdcall RawImage_DrawImg( int RawImage, int RawImage2, int drawx, int drawy, int blendmode )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}


	if ( RawImage2 >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RawImageStruct & tmpRawImage2 = ListOfRawImages[ RawImage2 ];


	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	RGBAPix* RawImageData2 = ( RGBAPix* )tmpRawImage2.img.buf;

	for ( int x = drawx, x2 = 0; x < tmpRawImage.width && x2 < tmpRawImage2.width; x++, x2++ )
	{
		for ( int y = drawy, y2 = 0; y < tmpRawImage.height && y2 < tmpRawImage2.height; y++, y2++ )
		{
			if ( blendmode == BlendModes::BlendNormal )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] = RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else if ( blendmode == BlendModes::BlendAdd )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] + RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else if ( blendmode == BlendModes::BlendSubtract )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] - RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else if ( blendmode == BlendModes::BlendMultiple )
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] * RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
			else
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] =
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] / RawImageData2[ ArrayXYtoId( tmpRawImage2.width, x2, y2 ) ];
		}
	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}

// ��������� ��������� ������� ��������� ������
int __stdcall RawImage_DrawPixel( int RawImage, int x, int y, RGBAPix color )//RGBAPix = unsigned int
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	if ( x >= 0 && y >= 0 && x < tmpRawImage.width && y < tmpRawImage.height )
	{
		RawImageData[ ArrayXYtoId( tmpRawImage.width, x, y ) ] = color;
	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}

// ������ ������������� � ��������� ������ � ��������
int __stdcall RawImage_DrawRect( int RawImage, int drawx, int drawy, int widthsize, int heightsize, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	for ( int xsize = 0; xsize < widthsize; xsize++ )
	{
		for ( int ysize = 0; ysize < heightsize; ysize++ )
		{
			RawImage_DrawPixel( RawImage, drawx + xsize, drawy + ysize, color );
		}
	}

	return TRUE;
}

#pragma region DrawLineAlgorithm

/*
*
* @date 25.03.2013
* @author Armin Joachimsmeyer
* https://github.com/ArminJo/STMF3-Discovery-Demos/blob/master/lib/graphics/src/thickLine.cpp
*
*/

#define LINE_OVERLAP_NONE 0 	// No line overlap, like in standard Bresenham
#define LINE_OVERLAP_MAJOR 0x01 // Overlap - first go major then minor direction. Pixel is drawn as extension after actual line
#define LINE_OVERLAP_MINOR 0x02 // Overlap - first go minor then major direction. Pixel is drawn as extension before next line
#define LINE_OVERLAP_BOTH 0x03  // Overlap - both

#define LINE_THICKNESS_MIDDLE 0                 // Start point is on the line at center of the thick line
#define LINE_THICKNESS_DRAW_CLOCKWISE 1         // Start point is on the counter clockwise border line
#define LINE_THICKNESS_DRAW_COUNTERCLOCKWISE 2  // Start point is on the clockwise border line

void drawLineOverlap( int RawImage, int aXStart, int aYStart, int aXEnd, int aYEnd, uint8_t aOverlap,
	RGBAPix aColor ) {
	int16_t tDeltaX, tDeltaY, tDeltaXTimes2, tDeltaYTimes2, tError, tStepX, tStepY;
	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	int maxwidth = tmpRawImage.width;
	int maxheight = tmpRawImage.height;
	/*
	* Clip to display size
	*/
	if ( aXStart >= maxwidth ) {
		aXStart = maxwidth - 1;
	}
	if ( aXStart < 0 ) {
		aXStart = 0;
	}
	if ( aXEnd >= maxwidth ) {
		aXEnd = maxwidth - 1;
	}
	if ( aXEnd < 0 ) {
		aXEnd = 0;
	}
	if ( aYStart >= maxheight ) {
		aYStart = maxheight - 1;
	}
	if ( aYStart < 0 ) {
		aYStart = 0;
	}
	if ( aYEnd >= maxheight ) {
		aYEnd = maxheight - 1;
	}
	if ( aYEnd < 0 ) {
		aYEnd = 0;
	}

	if ( ( aXStart == aXEnd ) || ( aYStart == aYEnd ) ) {
		//horizontal or vertical line -> fillRect() is faster

		if ( aXEnd >= aXStart && aYEnd >= aYStart )
		{
			RawImage_DrawRect( RawImage, aXStart, aYStart, aXEnd - aXStart + 1, aYEnd - aYStart + 1, aColor );
		}
		else if ( aXEnd >= aXStart )
		{
			RawImage_DrawRect( RawImage, aXStart, aYEnd, aXEnd - aXStart + 1, aYStart - aYEnd + 1, aColor );
		}
		else if ( aYEnd >= aYStart )
		{
			RawImage_DrawRect( RawImage, aXEnd, aYStart, aXStart - aXEnd + 1, aYEnd - aYStart + 1, aColor );
		}
	}
	else {
		//calculate direction
		tDeltaX = aXEnd - aXStart;
		tDeltaY = aYEnd - aYStart;
		if ( tDeltaX < 0 ) {
			tDeltaX = -tDeltaX;
			tStepX = -1;
		}
		else {
			tStepX = +1;
		}
		if ( tDeltaY < 0 ) {
			tDeltaY = -tDeltaY;
			tStepY = -1;
		}
		else {
			tStepY = +1;
		}
		tDeltaXTimes2 = tDeltaX << 1;
		tDeltaYTimes2 = tDeltaY << 1;
		//draw start pixel
		RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
		if ( tDeltaX > tDeltaY ) {
			// start value represents a half step in Y direction
			tError = tDeltaYTimes2 - tDeltaX;
			while ( aXStart != aXEnd ) {
				// step in main direction
				aXStart += tStepX;
				if ( tError >= 0 ) {
					if ( aOverlap & LINE_OVERLAP_MAJOR ) {
						// draw pixel in main direction before changing
						RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
					}
					// change Y
					aYStart += tStepY;
					if ( aOverlap & LINE_OVERLAP_MINOR ) {
						// draw pixel in minor direction before changing
						RawImage_DrawPixel( RawImage, aXStart - tStepX, aYStart, aColor );
					}
					tError -= tDeltaXTimes2;
				}
				tError += tDeltaYTimes2;
				RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
			}
		}
		else {
			tError = tDeltaXTimes2 - tDeltaY;
			while ( aYStart != aYEnd ) {
				aYStart += tStepY;
				if ( tError >= 0 ) {
					if ( aOverlap & LINE_OVERLAP_MAJOR ) {
						// draw pixel in main direction before changing
						RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
					}
					aXStart += tStepX;
					if ( aOverlap & LINE_OVERLAP_MINOR ) {
						// draw pixel in minor direction before changing
						RawImage_DrawPixel( RawImage, aXStart, aYStart - tStepY, aColor );
					}
					tError -= tDeltaYTimes2;
				}
				tError += tDeltaXTimes2;
				RawImage_DrawPixel( RawImage, aXStart, aYStart, aColor );
			}
		}
	}
}

/**
* Bresenham with thickness
* no pixel missed and every pixel only drawn once!
*/
void drawThickLine( int RawImage, int aXStart, int aYStart, int aXEnd, int aYEnd, int aThickness,
	uint8_t aThicknessMode, RGBAPix aColor ) {
	int16_t i, tDeltaX, tDeltaY, tDeltaXTimes2, tDeltaYTimes2, tError, tStepX, tStepY;
	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	int maxwidth = tmpRawImage.width;
	int maxheight = tmpRawImage.height;

	if ( aThickness <= 1 ) {
		drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, LINE_OVERLAP_NONE, aColor );
	}
	/*
	* Clip to display size
	*/
	if ( aXStart >= maxwidth ) {
		aXStart = maxwidth - 1;
	}
	if ( aXStart < 0 ) {
		aXStart = 0;
	}
	if ( aXEnd >= maxwidth ) {
		aXEnd = maxwidth - 1;
	}
	if ( aXEnd < 0 ) {
		aXEnd = 0;
	}
	if ( aYStart >= maxheight ) {
		aYStart = maxheight - 1;
	}
	if ( aYStart < 0 ) {
		aYStart = 0;
	}
	if ( aYEnd >= maxheight ) {
		aYEnd = maxheight - 1;
	}
	if ( aYEnd < 0 ) {
		aYEnd = 0;
	}

	/**
	* For coordinate system with 0.0 top left
	* Swap X and Y delta and calculate clockwise (new delta X inverted)
	* or counterclockwise (new delta Y inverted) rectangular direction.
	* The right rectangular direction for LINE_OVERLAP_MAJOR toggles with each octant
	*/
	tDeltaY = aXEnd - aXStart;
	tDeltaX = aYEnd - aYStart;
	// mirror 4 quadrants to one and adjust deltas and stepping direction
	BOOL tSwap = TRUE; // count effective mirroring
	if ( tDeltaX < 0 ) {
		tDeltaX = -tDeltaX;
		tStepX = -1;
		tSwap = !tSwap;
	}
	else {
		tStepX = +1;
	}
	if ( tDeltaY < 0 ) {
		tDeltaY = -tDeltaY;
		tStepY = -1;
		tSwap = !tSwap;
	}
	else {
		tStepY = +1;
	}
	tDeltaXTimes2 = tDeltaX << 1;
	tDeltaYTimes2 = tDeltaY << 1;
	BOOL tOverlap;
	// adjust for right direction of thickness from line origin
	int tDrawStartAdjustCount = aThickness / 2;
	if ( aThicknessMode == LINE_THICKNESS_DRAW_COUNTERCLOCKWISE ) {
		tDrawStartAdjustCount = aThickness - 1;
	}
	else if ( aThicknessMode == LINE_THICKNESS_DRAW_CLOCKWISE ) {
		tDrawStartAdjustCount = 0;
	}

	// which octant are we now
	if ( tDeltaX >= tDeltaY ) {
		if ( tSwap ) {
			tDrawStartAdjustCount = ( aThickness - 1 ) - tDrawStartAdjustCount;
			tStepY = -tStepY;
		}
		else {
			tStepX = -tStepX;
		}
		/*
		* Vector for draw direction of lines is rectangular and counterclockwise to original line
		* Therefore no pixel will be missed if LINE_OVERLAP_MAJOR is used
		* on changing in minor rectangular direction
		*/
		// adjust draw start point
		tError = tDeltaYTimes2 - tDeltaX;
		for ( i = tDrawStartAdjustCount; i > 0; i-- ) {
			// change X (main direction here)
			aXStart -= tStepX;
			aXEnd -= tStepX;
			if ( tError >= 0 ) {
				// change Y
				aYStart -= tStepY;
				aYEnd -= tStepY;
				tError -= tDeltaXTimes2;
			}
			tError += tDeltaYTimes2;
		}
		//draw start line
		drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, LINE_OVERLAP_NONE, aColor );
		// draw aThickness lines
		tError = tDeltaYTimes2 - tDeltaX;
		for ( i = aThickness; i > 1; i-- ) {
			// change X (main direction here)
			aXStart += tStepX;
			aXEnd += tStepX;
			tOverlap = LINE_OVERLAP_NONE;
			if ( tError >= 0 ) {
				// change Y
				aYStart += tStepY;
				aYEnd += tStepY;
				tError -= tDeltaXTimes2;
				/*
				* change in minor direction reverse to line (main) direction
				* because of choosing the right (counter)clockwise draw vector
				* use LINE_OVERLAP_MAJOR to fill all pixel
				*
				* EXAMPLE:
				* 1,2 = Pixel of first lines
				* 3 = Pixel of third line in normal line mode
				* - = Pixel which will additionally be drawn in LINE_OVERLAP_MAJOR mode
				*           33
				*       3333-22
				*   3333-222211
				* 33-22221111
				*  221111                     /\
				*  11                          Main direction of draw vector
				*  -> Line main direction
				*  <- Minor direction of counterclockwise draw vector
				*/
				tOverlap = LINE_OVERLAP_MAJOR;
			}
			tError += tDeltaYTimes2;
			drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, tOverlap, aColor );
		}
	}
	else {
		// the other octant
		if ( tSwap ) {
			tStepX = -tStepX;
		}
		else {
			tDrawStartAdjustCount = ( aThickness - 1 ) - tDrawStartAdjustCount;
			tStepY = -tStepY;
		}
		// adjust draw start point
		tError = tDeltaXTimes2 - tDeltaY;
		for ( i = tDrawStartAdjustCount; i > 0; i-- ) {
			aYStart -= tStepY;
			aYEnd -= tStepY;
			if ( tError >= 0 ) {
				aXStart -= tStepX;
				aXEnd -= tStepX;
				tError -= tDeltaYTimes2;
			}
			tError += tDeltaXTimes2;
		}
		//draw start line
		drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, LINE_OVERLAP_NONE, aColor );
		tError = tDeltaXTimes2 - tDeltaY;
		for ( i = aThickness; i > 1; i-- ) {
			aYStart += tStepY;
			aYEnd += tStepY;
			tOverlap = LINE_OVERLAP_NONE;
			if ( tError >= 0 ) {
				aXStart += tStepX;
				aXEnd += tStepX;
				tError -= tDeltaYTimes2;
				tOverlap = LINE_OVERLAP_MAJOR;
			}
			tError += tDeltaXTimes2;
			drawLineOverlap( RawImage, aXStart, aYStart, aXEnd, aYEnd, tOverlap, aColor );
		}
	}
}

#pragma endregion

// ������ ����� � ��������� ������ � ��������
int __stdcall RawImage_DrawLine( int RawImage, int x1, int y1, int x2, int y2, int size, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	drawThickLine( RawImage, x1, y1, x2, y2, size, 0, color );

	return TRUE;
}

// ������ ���� � ��������� �������� � ��������
int __stdcall RawImage_DrawCircle( int RawImage, int x, int y, int radius, int size, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}
	size /= 2;
	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			double dist = pDistance( x, y, x2, y2 );
			if ( pDistance( x, y, x2, y2 ) >= radius - size && pDistance( x, y, x2, y2 ) <= radius + size )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = color;
			}

		}

	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}




// ��������� ���� ��������� ������
int __stdcall RawImage_FillCircle( int RawImage, int x, int y, int radius, RGBAPix color )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			if ( pDistance( x, y, x2, y2 ) <= radius )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = color;
			}
		}
	}

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}


// ��������� ������ ���� � ��������� ��������
int __stdcall RawImage_EraseCircle( int RawImage, int x, int y, int radius, BOOL inverse )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	RGBAPix tmpPix = RGBAPix( );

	if ( !inverse )
	{
		return RawImage_FillCircle( RawImage, x, y, radius, tmpPix );
	}

	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			if ( pDistance( x, y, x2, y2 ) > radius )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = tmpPix;
			}
		}
	}
	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}


	return TRUE;
}

// ������ ������� � ������ color - �����������, power �� 0 �� 255
int __stdcall RawImage_EraseColor( int RawImage, RGBAPix color, int power )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
	RGBAPix tmpPix = RGBAPix( );
	unsigned char// A = color.A,
		R = color.R,
		G = color.G,
		B = color.B;

	for ( int x2 = 0; x2 < tmpRawImage.width; x2++ )
	{
		for ( int y2 = 0; y2 < tmpRawImage.width; y2++ )
		{
			unsigned char// A2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].A,
				R2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].R,
				G2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].G,
				B2 = RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ].B;

			if ( //( A >= A2 - power && A <= A2 + power ) &&
				( R >= R2 - power && R <= R2 + power ) &&
				( G >= G2 - power && G <= G2 + power ) &&
				( B >= B2 - power && B <= B2 + power ) )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x2, y2 ) ] = tmpPix;
			}
		}
	}
	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}

	return TRUE;
}

const char * _fontname = "Arial";
int _fontsize = 20;
unsigned int _flags = 0;
// 0x1 = BOLD


// ������������� ��������� ������ ��� RawImage_DrawText
int __stdcall RawImage_LoadFontFromResource( const char * filepath )
{
	int PatchFileData = 0;
	size_t PatchFileSize = 0;
	GameGetFile_ptr( filepath, &PatchFileData, &PatchFileSize, TRUE );
	DWORD Font = NULL;//Globals, this is the Font in the RAM
	AddFontMemResourceEx( ( void* )PatchFileData, PatchFileSize, NULL, &Font );
	return TRUE;
}


// ������������� ��������� ������ ��� RawImage_DrawText
int __stdcall RawImage_SetFontSettings( const char * fontname, int fontsize, unsigned int flags )
{
	_fontname = fontname;
	_fontsize = fontsize;
	_flags = flags;
	return TRUE;
}

// ����� ����� � ��������� ����������� � ���������� ������ � ����������� ������ RawImage_SetFontSettings
int __stdcall RawImage_DrawText( int RawImage, const char * text, int x, int y, RGBAPix color )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	RGBAPix* RawImageData = ( RGBAPix* )tmpRawImage.img.buf;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	HDC hDC = CreateCompatibleDC( NULL );
	char* pSrcData = 0;
	BITMAPINFO bmi = { sizeof( BITMAPINFOHEADER ), tmpRawImage.width, tmpRawImage.height, 1, 24, BI_RGB, 0, 0, 0, 0, 0 };
	HBITMAP hTempBmp = CreateDIBSection( hDC, &bmi, DIB_RGB_COLORS, ( void** )&pSrcData, NULL, 0 );
	if ( !hTempBmp )
	{
		DeleteDC( hDC );
		return FALSE;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	RECT rect = RECT( );
	rect.left = x;
	rect.top = y;
	rect.bottom = tmpRawImage.height;
	rect.right = tmpRawImage.width;

	HBITMAP hBmpOld = ( HBITMAP )SelectObject( hDC, hTempBmp );
	HFONT NewFont = CreateFontA( _fontsize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
	HFONT TempFont = NULL;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	UINT textcolor = color.ToUINT( );
	UINT oldcolor = color.ToUINT( );


	RGBAPix tmpPix = RGBAPix( );


	SetBkColor( hDC, 0x00000000 );
	SetBkMode( hDC, TRANSPARENT );

	SelectObject( hDC, NewFont );
	SetTextColor( hDC, color.ToUINT( ) );

#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	int len = strlen( text );
	BOOL boldenabled = FALSE;
	BOOL italicenabled = FALSE;
	BOOL underlineenabled = FALSE;
	BOOL strikeoutenabled = FALSE;

	BOOL newline = FALSE;
	for ( int i = 0; i < len; )
	{
		if ( len - i > 1 )
		{
			if ( text[ i ] == '|' && ( text[ i + 1 ] == 'n' || text[ i + 1 ] == 'N' ) )
			{
				i += 2;
				newline = TRUE;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'b' || text[ i + 1 ] == 'B' ) )
			{
				i += 2;
				boldenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'u' || text[ i + 1 ] == 'U' ) )
			{
				i += 2;
				underlineenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 's' || text[ i + 1 ] == 'S' ) )
			{
				i += 2;
				strikeoutenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'i' || text[ i + 1 ] == 'I' ) )
			{
				i += 2;
				italicenabled = TRUE;
				TempFont = CreateFontA( _fontsize, 0, 0, 0, boldenabled ? FW_BOLD : 0, italicenabled, underlineenabled, strikeoutenabled, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'r' || text[ i + 1 ] == 'R' ) )
			{
				i += 2;
				textcolor = oldcolor;
				SetTextColor( hDC, color.ToUINT( ) );
				TempFont = CreateFontA( _fontsize, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, 0, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, _fontname );
				SelectObject( hDC, TempFont );
				DeleteObject( NewFont );
				NewFont = TempFont;
				TempFont = NULL;
				boldenabled = FALSE;
				italicenabled = FALSE;
				underlineenabled = FALSE;
				strikeoutenabled = FALSE;
				continue;
			}
			else if ( text[ i ] == '|' && ( text[ i + 1 ] == 'c' || text[ i + 1 ] == 'C' ) )
			{
				oldcolor = textcolor;
				i += 2;
				if ( len - i > 7 )
				{
					char colorstr[ 11 ];
					colorstr[ 0 ] = '0';// text[ i + 2 ];
					colorstr[ 1 ] = 'x';//text[ i + 3 ];
										//A
					colorstr[ 2 ] = text[ i ];
					colorstr[ 3 ] = text[ i + 1 ];
					//R
					colorstr[ 4 ] = text[ i + 6 ];
					colorstr[ 5 ] = text[ i + 7 ];
					//G
					colorstr[ 6 ] = text[ i + 4 ];
					colorstr[ 7 ] = text[ i + 5 ];
					//B
					colorstr[ 8 ] = text[ i + 2 ];
					colorstr[ 9 ] = text[ i + 3 ];
					colorstr[ 10 ] = '\0';

					// ������ �� ����������� ������ ��� ��� ��� ������� ��� FF ��� 0 ������������
					textcolor = strtoul( colorstr, NULL, 0 );
					if ( ( textcolor & 0xFF000000 ) == 0xFF000000 )
						textcolor -= 0xFF000000;

					SetTextColor( hDC, textcolor );
					i += 8;
				}
				continue;
			}
		}

		ostringstream strfordraw;

		for ( ; i < len; i++ )
		{
			if ( text[ i ] != '|' || len - i < 2 )
				strfordraw << text[ i ];
			else if ( text[ i ] == '|' )
			{
				break;
			}
		}

		if ( strfordraw.str( ).length( ) > 0 )
		{
			//MessageBoxA( 0, strfordraw.str( ).c_str( ), "Draw:", 0 );

			RECT newsize = { 0,0,0,0 };
			DrawTextA( hDC, strfordraw.str( ).c_str( ), -1, &newsize, DT_CALCRECT );
			if ( newline )
			{
				newline = FALSE;
				rect.left = x;
				rect.top += newsize.top + newsize.bottom;
			}
			DrawTextA( hDC, strfordraw.str( ).c_str( ), -1, &rect, DT_LEFT | DT_SINGLELINE );
			rect.left += newsize.right - newsize.left;
			strfordraw.str( "" );
			strfordraw.clear( );
		}
	}

	DeleteObject( NewFont );

	SelectObject( hDC, hBmpOld );
	GdiFlush( );
	ReleaseDC( NULL, hDC );

	RGBPix* tmpBitmapPixList = ( RGBPix* )pSrcData;


	for ( int x0 = 0; x0 < tmpRawImage.width; x0++ )
	{
		for ( int y0 = 0; y0 < tmpRawImage.height; y0++ )
		{
			if ( tmpBitmapPixList[ ArrayXYtoId( tmpRawImage.width, x0, y0 ) ].ToUINT( ) != 0 )
			{
				RawImageData[ ArrayXYtoId( tmpRawImage.width, x0, y0 ) ] = tmpBitmapPixList[ ArrayXYtoId( tmpRawImage.width, x0, y0 ) ].ToRGBAPix( );
			}
		}
	}

	DeleteDC( hDC );
	DeleteObject( hBmpOld );
	DeleteObject( hTempBmp );

	if ( tmpRawImage.used_for_overlay )
	{
		tmpRawImage.needResetTexture = TRUE;
	}
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif

	return TRUE;
}


// ��������� RawImage � blp � ������ ��������� ��� ������������� � ����
int __stdcall SaveRawImageToGameFile( int RawImage, const char * filename, BOOL IsTga, BOOL enabled )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}


	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	tmpRawImage.filename = filename;
	Buffer tmpRawImageBuffer = tmpRawImage.img;
	Buffer ResultBuffer = Buffer( );
	if ( tmpRawImage.ingamebuffer.buf )
		tmpRawImage.ingamebuffer.Clear( );

	if ( enabled )
	{
		int mipmaps = 0;
		if ( IsTga )
			RAW2Tga( tmpRawImageBuffer, ResultBuffer, tmpRawImage.width, tmpRawImage.height, 4, filename );
		else
			CreatePalettedBLP( tmpRawImageBuffer, ResultBuffer, 256, filename, tmpRawImage.width, tmpRawImage.height, 4, 8, mipmaps );
		tmpRawImage.ingamebuffer = ResultBuffer;
	}

	tmpRawImage.ingame = enabled;
	return TRUE;
}


// ��������� RawImage �� ���� � TGA �� ���������� ����
int __stdcall DumpRawImageToFile( int RawImage, const char * filename )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct tmpRawImage = ListOfRawImages[ RawImage ];
	Buffer outbuffer;
	Buffer inbuffer = tmpRawImage.img;
	RAW2Tga( inbuffer, outbuffer, tmpRawImage.width, tmpRawImage.height, 4, filename );
	FILE * f;
	fopen_s( &f, filename, "wb" );
	if ( f )
	{
		fwrite( outbuffer.buf, outbuffer.length, 1, f );
		fclose( f );
	}


	return TRUE;
}


// �������� RawImage �� ������ RawImages �� ����� �����.
int __stdcall GetRawImageByFile( const char * filename )
{
	int id = 0;
	for ( RawImageStruct & s : ListOfRawImages )
	{
		if ( ToLower( s.filename ) == ToLower( filename ) )
			return id;
		id++;
	}

	return 0;
}

// �������� ������ RawImage
int __stdcall RawImage_GetWidth( int RawImage )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return 64;
	}

	return ListOfRawImages[ RawImage ].width;
}

// �������� ������ RawImage
int __stdcall RawImage_GetHeight( int RawImage )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return 64;
	}

	return ListOfRawImages[ RawImage ].height;
}

// �������� ������ RawImage
int __stdcall RawImage_Resize( int RawImage, int newwidth, int newheight )
{
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	newwidth = newwidth + ( newwidth % 2 );
	newheight = newheight + ( newheight % 2 );

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	Buffer tmpOldBuffer = tmpRawImage.img;
	Buffer tmpNewBuffer = Buffer( );
	ScaleImage( ( unsigned char * )tmpOldBuffer.buf, tmpRawImage.width, tmpRawImage.height, newwidth, newheight, 4, tmpNewBuffer );
	tmpOldBuffer.Clear( );
	tmpRawImage.img = tmpNewBuffer;
	tmpRawImage.height = newheight;
	tmpRawImage.width = newwidth;
	if ( tmpRawImage.used_for_overlay )
		tmpRawImage.needResetTexture = TRUE;

	return TRUE;
}

// ������ RawImage �� �������� ����������� (�� 0.0 �� 1.0) � ����. 
int __stdcall RawImage_DrawOverlay( int RawImage, BOOL enabled, float xpos, float ypos, float xsize, float ysize )
{
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];
	tmpRawImage.used_for_overlay = enabled;
	tmpRawImage.overlay_x = xpos;
	tmpRawImage.overlay_y = ypos;
	tmpRawImage.size_x = xsize;
	tmpRawImage.size_y = ysize;
#ifdef DOTA_HELPER_LOG
	AddNewLineToDotaHelperLog( __func__, __LINE__ );
#endif
	return TRUE;
}

RawImageCallbackData * GlobalRawImageCallbackData = NULL;

int __stdcall RawImage_AddCallback( int RawImage, const char * MouseActionCallback, RawImageCallbackData * callbackdata, unsigned int events )
{
	GlobalRawImageCallbackData = callbackdata;


	if ( RawImage >= ( int )ListOfRawImages.size( ) )
	{
		return FALSE;
	}

	RawImageStruct & tmpRawImage = ListOfRawImages[ RawImage ];

	if ( !MouseActionCallback || MouseActionCallback[ 0 ] == '\0' )
	{
		tmpRawImage.MouseCallback = FALSE;
		tmpRawImage.MouseActionCallback = RCString( );
	}
	else
	{
		tmpRawImage.MouseActionCallback = RCString( );
		str2jstr( &tmpRawImage.MouseActionCallback, MouseActionCallback );
		tmpRawImage.MouseCallback = TRUE;
	}

	tmpRawImage.events = events;
	tmpRawImage.IsMouseDown = FALSE;
	tmpRawImage.IsMouseEntered = FALSE;

	return TRUE;
}


BOOL RawImageGlobalCallbackFunc( RawImageEventType callbacktype, float mousex, float mousey )
{
	if ( !GlobalRawImageCallbackData )
		return FALSE;



	GlobalRawImageCallbackData->IsAltPressed = IsKeyPressed( VK_MENU );
	GlobalRawImageCallbackData->IsCtrlPressed = IsKeyPressed( VK_CONTROL );
	GlobalRawImageCallbackData->EventType = callbacktype;
	float ScreenX = *GetWindowXoffset;
	float ScreenY = *GetWindowYoffset;

	float zoomx = ScreenX / DesktopScreen_Width;
	float zoomy = ScreenY / DesktopScreen_Height;

	float mouseposx = mousex / ScreenX;
	float mouseposy = mousey / ScreenY;
	GlobalRawImageCallbackData->mousex = mouseposx;
	GlobalRawImageCallbackData->mousey = mouseposy;

	for ( unsigned int i = ListOfRawImages.size( ) - 1; i > 0; i-- )
	{
		BOOL retval = rawimage_skipmouseevent;

		RawImageStruct & img = ListOfRawImages[ i ];
		RGBAPix* RawImageData = ( RGBAPix* )img.img.buf;

		if ( img.used_for_overlay &&
			img.MouseCallback &&
			( img.events & ( unsigned int )callbacktype ) > 0 )
		{
			BOOL MouseEnteredInRawImage = FALSE;
			float posx = ScreenX * img.overlay_x;
			float posy = ScreenY * img.overlay_y;
			float sizex = img.width * zoomx;
			float sizey = img.height * zoomy;
			int img_x, img_y;
			//posy -= sizey;
			GlobalRawImageCallbackData->RawImage = img.RawImage;

			if ( mousex > posx && mousex < posx + sizex && mousey > posy && mousey < posy + sizey )
			{
				img_x = img.width - ( int )( posx + sizex - mousex );
				img_y = img.height - ( int )( posy + sizey - mousey );


				if ( img_x > img.width )
					img_x = img.width;

				if ( img_y > img.height )
					img_y = img.height;

				if ( img_x < 0 )
					img_x = 0;

				if ( img_y < 0 )
					img_y = 0;

				RGBAPix eventpix = RawImageData[ ArrayXYtoId( img.width, img_x, img_y ) ];

				if ( eventpix.A <= 20 )
					retval = FALSE;

				MouseEnteredInRawImage = TRUE;
			}
			else
			{
				img_x = 0;
				img_y = 0;
			}

			GlobalRawImageCallbackData->offsetx = img_x;
			GlobalRawImageCallbackData->offsety = img_y;

			switch ( callbacktype )
			{
			case RawImageEventType::MouseUp:
				if ( img.IsMouseDown )
				{
					img.IsMouseDown = FALSE;

					if ( MouseEnteredInRawImage )
						GlobalRawImageCallbackData->EventType = RawImageEventType::MouseClick;
					if ( retval )
						ExecuteFunc( &img.MouseActionCallback );
					return FALSE;
				}
				break;
			case RawImageEventType::MouseDown:
				if ( !img.IsMouseDown && MouseEnteredInRawImage )
				{
					img.IsMouseDown = TRUE;
					if ( retval )
						ExecuteFunc( &img.MouseActionCallback );
					return retval;
				}
				break;
			case RawImageEventType::MouseClick:
				break;
			case RawImageEventType::MouseEnter:
				break;
			case RawImageEventType::MouseLeave:
				break;
			case RawImageEventType::MouseMove:
				if ( img.IsMouseEntered )
				{
					if ( !MouseEnteredInRawImage )
					{
						img.IsMouseEntered = FALSE;
						GlobalRawImageCallbackData->EventType = RawImageEventType::MouseLeave;
						ExecuteFunc( &img.MouseActionCallback );
					}
				}
				else
				{
					if ( MouseEnteredInRawImage )
					{
						if ( retval )
						{
							img.IsMouseEntered = TRUE;
							GlobalRawImageCallbackData->EventType = RawImageEventType::MouseEnter;
							ExecuteFunc( &img.MouseActionCallback );
						}
					}
				}
				break;
			case RawImageEventType::ALL:
				if ( img.IsMouseDown )
				{
					img.IsMouseDown = FALSE;
					GlobalRawImageCallbackData->EventType = RawImageEventType::MouseUp;
					ExecuteFunc( &img.MouseActionCallback );
				}
				if ( img.IsMouseEntered )
				{
					img.IsMouseEntered = FALSE;
					GlobalRawImageCallbackData->EventType = RawImageEventType::MouseLeave;
					ExecuteFunc( &img.MouseActionCallback );
				}
				break;
			default:
				break;
			}
		}
	}

	return FALSE;
}


//
//void ApplyIconFrameFilter2( string filename, int * OutDataPointer, size_t * OutSize )
//{
//	int RawImage = CreateRawImage( 128, 128, RGBAPix( ) );
//	//RawImage_Resize( RawImage, 128, 128 );
//	int RawImage2 = LoadRawImage( filename.c_str( ) );
//	RawImage_DrawImg( RawImage, RawImage2, 32, 32 );
//	RGBAPix tmppix = RGBAPix( );
//	RawImage_EraseCircle( RawImage, 64, 64, 29, TRUE );
//	RawImage_DrawCircle( RawImage, 64, 64, 35, 6, tmppix.RGBAPixWar3( 0, 255, 0, 255 ) );
//
//
//	RawImage_DrawText( RawImage, "|C00FF0000RED|r |CFF00FF00GREEN|r |CFF0000FFBLUE|r", 10, 10, tmppix.RGBAPixWar3( 255, 0, 0, 0 ) );
//
//	SaveRawImageToGameFile( RawImage, ( filename + "_frame.blp" ).c_str( ), FALSE, TRUE );
//	DumpRawImageToFile( RawImage, "temp.tga" );
//
//	RawImage_DrawOverlay( RawImage, TRUE, 0.1f, 0.1f, 0, 0 );
//	ApplyIconFrameFilter3( filename, OutDataPointer, OutSize );
//}
//
//

void ApplyIconFrameFilter( string filename, int * OutDataPointer, size_t * OutSize )
{
	int RawImage = CreateRawImage( 128, 128, RGBAPix( ) );
	int RawImage2 = LoadRawImage( filename.c_str( ) );
	RawImage_DrawImg( RawImage, RawImage2, 32, 32, 0 );
	RGBAPix tmppix = RGBAPix( );
	SaveRawImageToGameFile( RawImage, ( filename + "_frame.blp" ).c_str( ), FALSE, TRUE );
}




void ClearAllRawImages( )
{
	for ( RawImageStruct & s : ListOfRawImages )
	{
		s.used_for_overlay = FALSE;
		if ( s.img.buf )
			s.img.Clear( );
		if ( s.ingame )
		{
			if ( s.ingamebuffer.buf )
				s.ingamebuffer.Clear( );
			s.ingame = FALSE;
		}
	}
	ListOfRawImages.clear( );
	RGBAPix tmppix = RGBAPix( );
	CreateRawImage( 64, 64, tmppix.RGBAPixWar3( 0, 255, 0, 255 ) );
}


float __stdcall GetScreenWidth( int )
{
	return DesktopScreen_Width;
}
float __stdcall GetScreenHeight( int )
{
	return DesktopScreen_Height;
}

float __stdcall GetWindowWidth( int )
{
	if ( *InGame )
		return  *GetWindowXoffset;
	return DesktopScreen_Width;
}
float __stdcall GetWindowHeight( int )
{
	if ( *InGame )
		return  *GetWindowYoffset;
	return DesktopScreen_Height;
}