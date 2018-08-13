//-----------------------------------------------------------------------------
/*

Patch and screen handler. Stores current state of the device for use in the midi keyboard case.

*/
//-----------------------------------------------------------------------------

#include "pmsynth.h"

//-----------------------------------------------------------------------------

// Number of implemented patches

#define NUM_PATCHES 2

// Defines for patch numbers of instruments accessable by the keyboard

#define WAVEGUIDE_1D 0
#define KARPLUS_STRONG 1
#define WOODWIND 2

// Defines for exciter numbers

#define MALLET_HIT 0
#define MOUTH_BLOW 1
#define GUITAR_PICK 2

// Works by hijacking the input midi channel and inserting the channel that is assigned to the
// instrument in this file.

void goto_next_patch(){
	current_patch_no += 1;
	if (current_patch_no > NUM_PATCHES) {
		current_patch_no = 0;
	}
	
}

static const int32_t FLUTE_IMAGE[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFF8, 0x7FFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0x79FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFBC7FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFC81FFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFEEE7F, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFF399, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFA7, 0xE7FFFFFF, 0xFFFFFFFF, 0xFFFFFFFD, 
	0x5F9FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xE77E7FFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFF9DF9FF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFCBFE7, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFF1BF, 0x9FFFFFFF, 0xFFFFFFFF, 0xFFFFFFCF, 
	0xFE7FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x2BF9FFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFCFFE7FF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFF37F9F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFCFFE, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFF2F, 
	0xF9FFFFFF, 0xFFFFFFFF, 0xFFFFFFFC, 0xFFE7FFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xF1FE1FFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFCFF67F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFF3FF9, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFCFF, 
	0x87FFFFFF, 0xFFFFFFFF, 0xFFFFFFF3, 0xFD9FFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xCFFC7FFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFF3FC1FF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFCFF67, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFF3FF, 
	0x9FFFFFFF, 0xFFFFFFFF, 0xFFFFFFCF, 0xF87FFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0x3FD9FFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFCFFE7FF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFF3FE1F, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFCFF6, 
	0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFF3F, 0xF9FFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFC, 0xFF87FFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xF3FD9FFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFCFFE7F, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF3FF9, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFCFF, 0xE7FFFFFF, 
	0xFFFFFFFF, 0xFFFFFFF3, 0xFF9FFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xCFFE7FFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFF3FF9FF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFCFE1F, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFF3C1, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFCC, 0x1FFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0x3FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFCFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
};

static const int32_t GUITAR_PICK_IMAGE[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFF7F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFF7FFFFF, 0xFFFFFFFF, 0xFFFFFFFB, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7FFF7FFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xDFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFF7F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFB7FFFFB, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFEFFFFF, 
	0xBFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFBFFFFFF, 
	0xFFFFFFFF, 0xFFF7DFFF, 0xFFBFFFFF, 0xFFFFFFFF, 
	0xFFFFDFFF, 0xFFFBFFFF, 0xFFFFFFFF, 0xFFFFFF5F, 
	0xF1FFBFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xF0E3FBFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xF0FFDFBF, 0xFFFFFFFF, 
	0xFFFFFF7F, 0xF3EABEFB, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFDAF3EF7, 0xBFFFFFFF, 0xFFFFFFFE, 0xFFDE3CB7, 
	0xBBFFFFFF, 0xFFFFFFFF, 0xFFFBFDEE, 0xFD7FFFFF, 
	0xFFFFFFFF, 0xFBFFBFEF, 0x5AE7FFFF, 0xFFFFFFFF, 
	0xFFFFF7FF, 0x7AFEFFFF, 0xFFFFFFFF, 0xFFFFFF7F, 
	0xFBAAAFFF, 0xFFFFFFFF, 0xFFFEFFEF, 0xFFDD5EFF, 
	0xFFFFFFFF, 0xFFFFFFFE, 0xFFFED557, 0xFFFFFFFF, 
	0xFFFFFDFF, 0xDFFFEE57, 0x7FFFFFFF, 0xFFFFFFFF, 
	0xFBFFFF72, 0xA7FFFFFF, 0xFFFFFFFB, 0xFFBFFFFB, 
	0x2A7FFFFF, 0xFFFFFFFF, 0xFFF7FFFF, 0xB057FFFF, 
	0xFFFFFFFF, 0xFFFEFFFF, 0xFB2A7FFF, 0xFFFFFFFF, 
	0xFFFFEFFF, 0xFFB027FF, 0xFFFFFFFF, 0xFFFFFDFF, 
	0xFFFB1400, 0x55, 0x7FFFFFDF, 0xFFFF7027, 
	0xFFFFFFFF, 0xFFFFFFFD, 0xFFFFF700, 0x7FFFFFFF, 
	0xFFFFFFFF, 0xDFFFFF60, 0x7FFFFFF, 0xFFFFFFFF, 
	0xFBFFFFEE, 0x7FFFFF, 0xFFFFFFFF, 0xFFBFFFFD, 
	0xC0000000, 0xAAFF, 0xFFFDFFFF, 0xBC00FFFF, 
	0xFFFFFFFF, 0xFFFFEFFF, 0xF7800FFF, 0xFFFFFFFF, 
	0xFFFFFF7F, 0xFCF000FF, 0xFFFFFFFF, 0xFFFBFFF8, 
	0x3E3E001F, 0xFFFFFFFF, 0xFFFFFFFF, 0xFC1FC001, 
	0xFFFFFFFF, 0xFFFFFF7F, 0xFFFFF800, 0x0, 
	0x557FFD, 0xFFFFFC00, 0x3FFFFFF, 0xFFFFFFFF, 
	0xF83FFF00, 0x3FFFFF, 0xFFFFFFFF, 0xFFFC0000, 
	0x3FFFF, 0xFFFFFFFF, 0xFFFFFFFC, 0x3FFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xF00007FF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFC0007F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFE0007, 0xFFFFFFFF, 0xFFFFFF6A, 0x80000000, 
	0x0, 0x557FFF, 0xFFFFFFC0, 0xFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0x81FFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFF3FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
};

static const int32_t HARP_IMAGE[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFC03F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFBBD, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFF02E, 
	0xAFFFFFFF, 0xFFFFFFFF, 0xFFFFFCF4, 0xB2FFFFFF, 
	0xFFFFFFFF, 0xFFFFFFBF, 0xA0CFFFFF, 0xFFFFFFFF, 
	0xFFFFFFF7, 0xF97DFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 
	0xE04FBFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xDDFCBBFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFBBFD7BF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFB7FCFB, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFF77FE7, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFECFFE, 
	0xF7FFFFFF, 0xFFFFFFFF, 0xFFFFDDFF, 0xEB7FFFFF, 
	0xFFFFFFFF, 0xFFFFFDBF, 0xFE77FFFF, 0xFFFFFFFF, 
	0xFFFFFFDB, 0xFFE77FFF, 0xFFFFFFFF, 0xFFFFFFFB, 
	0xBC0667FF, 0xFFFFFFFF, 0xFFFFFFFF, 0xB7CA897F, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFB7AAA77, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFF772AA6, 0x7FFFFFFF, 0xFFFFFFF8, 
	0x3FF4F2AA, 0x97FFFFFF, 0xFFFFFFFF, 0x7DFF6EAA, 
	0xA77FFFFF, 0xFFFFFFFF, 0xED6FF6EA, 0xAAF7FFFF, 
	0xFFFFFFFF, 0xFE7B7EEC, 0xAAA77FFF, 0xFFFFFFFF, 
	0xFFDC27ED, 0xCAAAF7FF, 0xFFFFFFFF, 0xFFFD4DBE, 
	0xDAAAAB7F, 0xFFFFFFFF, 0xFFFFCC9D, 0xDDAAAA77, 
	0xFFFFFFFF, 0xFFFFFC66, 0x6DB2AAA7, 0x7FFFFFFF, 
	0xFFFFFFD4, 0x37332AAA, 0xF7FFFFFF, 0xFFFFFFFE, 
	0x319F6AAA, 0xA77FFFFF, 0xFFFFFFFF, 0xEA8CAEAA, 
	0xAAF7FFFF, 0xFFFFFFFF, 0xFF5971CA, 0xAAAB7FFF, 
	0xFFFFFFFF, 0xFFF549F2, 0xAAAA77FF, 0xFFFFFFFF, 
	0xFFFFAE60, 0xAAAAA77F, 0xFFFFFFFF, 0xFFFFFA52, 
	0xAAAAAAF7, 0xFFFFFFFF, 0xFFFFFFD7, 0xAAAAAA7, 
	0x7FFFFFFF, 0xFFFFFFFD, 0x38AAAAAA, 0xF7FFFFFF, 
	0xFFFFFFFF, 0xEACAAAAA, 0xAB7FFFFF, 0xFFFFFFFF, 
	0xFE9D2AAA, 0xAA77FFFF, 0xFFFFFFFF, 0xFFF46AAA, 
	0xAAA77FFF, 0xFFFFFFFF, 0xFFFF4F0A, 0xAAAAF7FF, 
	0xFFFFFFFF, 0xFFFFFB36, 0xAAAAA77F, 0xFFFFFFFF, 
	0xFFFFFFA7, 0xA2AAAAF7, 0xFFFFFFFF, 0xFFFFFFFD, 
	0xAD2AAAAB, 0x7FFFFFFF, 0xFFFFFFFF, 0xD1CAAAAA, 
	0x77FFFFFF, 0xFFFFFFFF, 0xFEDE8AAA, 0xAF7FFFFF, 
	0xFFFFFFFF, 0xFFEAF6AA, 0xAA77FFFF, 0xFFFFFFFF, 
	0xFFFF6B22, 0xAAA77FFF, 0xFFFFFFFF, 0xFFFFF57A, 
	0x2AAAF7FF, 0xFFFFFFFF, 0xFFFFFFB3, 0xD8AAA77F, 
	0xFFFFFFFF, 0xFFFFFFFA, 0xBC8AAAF7, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xDCEAAAAB, 0x7FFFFFFF, 0xFFFFFFFF, 
	0xFD5F22AA, 0x77FFFFFF, 0xFFFFFFFF, 0xFFEEF2AA, 
	0xA77FFFFF, 0xFFFFFFFF, 0xFFFEC7A8, 0xAAF7FFFF, 
	0xFFFFFFFF, 0xFFFFF77D, 0x8AA77FFF, 0xFFFFFFFF, 
	0xFFFFFF7B, 0xEAAAF7FF, 0xFFFFFFFF, 0xFFFFFFFA, 
	0xBEA2AB7F, 0xFFFFFFFF, 0xFFFFFFFF, 0xBDF6AA77, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFDCFA8AF, 0x7FFFFFFF, 
	0xFFFFFFFF, 0xFFDAFBAA, 0x77FFFFFF, 0xFFFFFFFF, 
	0xFFFEF7DA, 0x277FFFFF, 0xFFFFFFFF, 0xFFFFEF7E, 
	0xE2F7FFFF, 0xFFFFFFFF, 0xFFFFFF7B, 0xEEAB7FFF, 
	0xFFFFFFFF, 0xFFFFFFF7, 0x9F6877FF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0x9DFAAF7F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFBEF9A77, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFDEFDA7, 
	0x7FFFFFFF, 0xFFFFFFFF, 0xFFFDF7EE, 0xF7FFFFFF, 
	0xFFFFFFFF, 0xFFFFEF7E, 0x6B7FFFFF, 0xFFFFFFFF, 
	0xFFFFFEFB, 0xF677FFFF, 0xFFFFFFFF, 0xFFFFFFF7, 
	0xDFAF7FFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7DF877FF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFBEFC77F, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFBEFEF7, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFBF7C0, 0xBFFFFFFF, 0xFFFFFFFF, 0xFFFFBF31, 
	0xFCFFFFFF, 0xFFFFFFFF, 0xFFFFFAF8, 0xC737FFFF, 
	0xFFFFFFFF, 0xFFFFFFAC, 0x370C7FFF, 0xFFFFFFFF, 
	0xFFFFFFF2, 0xBD2E97FF, 0xFFFFFFFF, 0xFFFFFFFE, 
	0x9D4C077F, 0xFFFFFFFF, 0xFFFFFFFF, 0xF6A3FFF7, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFE92DFFF, 0x7FFFFFFF, 
	0xFFFFFFFF, 0xFFE3A77F, 0xF7FFFFFF, 0xFFFFFFFF, 
	0xFFFFFADF, 0xFF7FFFFF, 0xFFFFFFFF, 0xFFFFFFA7, 
	0xFFEFFFFF, 0xFFFFFFFF, 0xFFFFFFFA, 0xD3F9FFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xC4C07FFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFE3FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
};

static const int32_t MALLET_IMAGE[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFF3FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFF847FF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF01BF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFF05B, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFE7F02, 0xBFFFFFFF, 0xFFFFFFFF, 
	0xFFFF08E0, 0x2DFFFFFF, 0xFFFFFFFF, 0xFFFFE076, 
	0x9DFFFFF, 0xFFFFFFFF, 0xFFFFFE0B, 0x707BFFFF, 
	0xFFFFFFFF, 0xFFFFFFE0, 0x7735BFFF, 0xFFFFFFFF, 
	0xFFFFFFFC, 0xBB5FBFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xD07B8C7F, 0xFFFFFFFF, 0xFFFFFFFF, 0xFE5F7F1F, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFE6F7F5, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFEFF7F, 0x5FFFFFFF, 0xFFFFFFFF, 
	0xFFFFF18F, 0xF5FFFFFF, 0xFFFFFFFF, 0xFFFFFFE5, 
	0xFF5FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x5FF5FFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFAFFAFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFAFFAFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFD7FAF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFD7FA, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFEBF, 0xAFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFEB, 0xFAFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0x5FD7FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xF5FD7FFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFAFD7FF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFAFD7F, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFD7D7, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFD7D, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFEB, 
	0xEBFFFFFF, 0xFFFFFFFF, 0xFFFFFFFE, 0xBEBFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xF5EBFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFF5EBFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFAEBFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFAEBF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFD75, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFD7, 0x5FFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFE, 0xB5FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xEB5FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF55FFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFF55FFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFAAFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFAAF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFD2, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFD, 0x2FFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xEAFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFEAFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFF57FFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF57FF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFA7F, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFA7, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFD, 0x7FFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xD7FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFEBFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFEBFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFE5FFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFE5FF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFEAF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFEA, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0x57FFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xF57FFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFF4BFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFF4BFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFF55FF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFF75F, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFAA, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFA, 
	0xAFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xAD7FFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFAD7FFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFAEBFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFAEBFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFD75F, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFD75, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFD7, 0xAFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFD, 0x7AFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xD7D7FFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFD7D7FFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFEBEBFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFEBEBF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFEBF5, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFEBF, 
	0x5FFFFFFF, 0xFFFFFFFF, 0xFFFFFFEB, 0xFBFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFE, 0xBFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xF7FFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
};

static const int32_t TUBE_IMAGE[] = {
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFC, 0x3FFFFFF, 
	0xFFFFFFFF, 0xFFFFFFC0, 0x3FC03FFF, 0xFFFFFFFF, 
	0xFFFFFFC2, 0xFEFFFC3F, 0xFFFFFFFF, 0xFFFFFFF1, 
	0x5ABFFFFC, 0xFFFFFFFF, 0xFFFFFFFC, 0xAB7F7FFF, 
	0xF3FFFFFF, 0xFFFFFFFF, 0xCAAD5FFF, 0xFFBFFFFF, 
	0xFFFFFFFF, 0xF856BFFF, 0xFFF1FFFF, 0xFFFFFFFF, 
	0xFFB15D5B, 0xFFFC9FFF, 0xFFFFFFFF, 0xFFFBC2BF, 
	0xFFFC29FF, 0xFFFFFFFF, 0xFFFFBFC0, 0x2FC0155F, 
	0xFFFFFFFF, 0xFFFFFBFF, 0xFC03DB55, 0xFFFFFFFF, 
	0xFFFFFFBF, 0xFFFFD76A, 0x9FFFFFFF, 0xFFFFFFFB, 
	0xFFFFEFD5, 0x55FFFFFF, 0xFFFFFFFF, 0xBFFFFFAB, 
	0x6A9FFFFF, 0xFFFFFFFF, 0xFBFFFFBF, 0xED55FFFF, 
	0xFFFFFFFF, 0xFFBFFFFF, 0x55AA9FFF, 0xFFFFFFFF, 
	0xFFFBFFFF, 0xEFEB55FF, 0xFFFFFFFF, 0xFFFFBFFF, 
	0xFFEBAA9F, 0xFFFFFFFF, 0xFFFFFBFF, 0xFFB5D555, 
	0xFFFFFFFF, 0xFFFFFFBF, 0xFFFFF6DA, 0x9FFFFFFF, 
	0xFFFFFFFB, 0xFFFFEDDA, 0xA9FFFFFF, 0xFFFFFFFF, 
	0xBFFFFF76, 0xB55FFFFF, 0xFFFFFFFF, 0xFBFFFFBD, 
	0xDAA9FFFF, 0xFFFFFFFF, 0xFFBFFFFE, 0xF6D55FFF, 
	0xFFFFFFFF, 0xFFFBFFFF, 0xFADB55FF, 0xFFFFFFFF, 
	0xFFFFBFFF, 0xFDF6AA9F, 0xFFFFFFFF, 0xFFFFFBFF, 
	0xFFF5DAA9, 0xFFFFFFFF, 0xFFFFFFBF, 0xFFFFF6B5, 
	0x5FFFFFFF, 0xFFFFFFFB, 0xFFFFB5DA, 0xA9FFFFFF, 
	0xFFFFFFFF, 0xBFFFFFF6, 0xD55FFFFF, 0xFFFFFFFF, 
	0xFBFFFFED, 0xDAA9FFFF, 0xFFFFFFFF, 0xFFBFFFFF, 
	0x76B55FFF, 0xFFFFFFFF, 0xFFFBFFFF, 0xBDDAA9FF, 
	0xFFFFFFFF, 0xFFFFBFFF, 0xFEF6D55F, 0xFFFFFFFF, 
	0xFFFFFBFF, 0xFFFADAA9, 0xFFFFFFFF, 0xFFFFFFBF, 
	0xFFFDF6B5, 0x5FFFFFFF, 0xFFFFFFFB, 0xFFFFF5DA, 
	0xA9FFFFFF, 0xFFFFFFFF, 0xBFFFFFF6, 0xD55FFFFF, 
	0xFFFFFFFF, 0xFBFFFFB5, 0xDAA9FFFF, 0xFFFFFFFF, 
	0xFFBFFFFF, 0xF6B55FFF, 0xFFFFFFFF, 0xFFFBFFFF, 
	0xEDDAA9FF, 0xFFFFFFFF, 0xFFFFBFFF, 0xFF76D55F, 
	0xFFFFFFFF, 0xFFFFFBFF, 0xFFBDDAA9, 0xFFFFFFFF, 
	0xFFFFFFBF, 0xFFFEF6B5, 0x5FFFFFFF, 0xFFFFFFFB, 
	0xFFFFFADA, 0xA9FFFFFF, 0xFFFFFFFF, 0xBFFFFDF6, 
	0xD55FFFFF, 0xFFFFFFFF, 0xFBFFFFF5, 0xDAA9FFFF, 
	0xFFFFFFFF, 0xFFBFFFFF, 0xF6B55FFF, 0xFFFFFFFF, 
	0xFFFBFFFF, 0xB5DAA9FF, 0xFFFFFFFF, 0xFFFFBFFF, 
	0xFFF6D55F, 0xFFFFFFFF, 0xFFFFFBFF, 0xFFEDDB55, 
	0xFFFFFFFF, 0xFFFFFFBF, 0xFFFF76AA, 0x9FFFFFFF, 
	0xFFFFFFFB, 0xFFFFBDDA, 0xA9FFFFFF, 0xFFFFFFFF, 
	0xBFFFFEF6, 0xB55FFFFF, 0xFFFFFFFF, 0xFBFFFFFA, 
	0xDAA9FFFF, 0xFFFFFFFF, 0xFFBFFFFD, 0xF6D55FFF, 
	0xFFFFFFFF, 0xFFFBFFFF, 0xF5DAA9FF, 0xFFFFFFFF, 
	0xFFFFBFFF, 0xFFF6B55F, 0xFFFFFFFF, 0xFFFFFBFF, 
	0xFFB5DAA9, 0xFFFFFFFF, 0xFFFFFFBF, 0xFFFFF6D5, 
	0x5FFFFFFF, 0xFFFFFFFB, 0xFFFFEDDA, 0xA9FFFFFF, 
	0xFFFFFFFF, 0xBFFFFF76, 0xB55FFFFF, 0xFFFFFFFF, 
	0xFBFFFFBD, 0xDAA9FFFF, 0xFFFFFFFF, 0xFFBFFFFE, 
	0xF6D55FFF, 0xFFFFFFFF, 0xFFFBFFFF, 0xFADAA9FF, 
	0xFFFFFFFF, 0xFFFFBFFF, 0xFDF6B55F, 0xFFFFFFFF, 
	0xFFFFFBFF, 0xFFF5DAA9, 0xFFFFFFFF, 0xFFFFFF9F, 
	0xFFFFF76A, 0x9FFFFFFF, 0xFFFFFFF8, 0xFFFFB5D5, 
	0x51FFFFFF, 0xFFFFFFFF, 0xB3FFFFF5, 0xD49FFFFF, 
	0xFFFFFFFF, 0xFBC3FFED, 0xE815FFFF, 0xFFFFFFFF, 
	0xFFBFC03F, 0xC0169FFF, 0xFFFFFFFF, 0xFFFBFFFC, 
	0x3DB55FF, 0xFFFFFFFF, 0xFFFFDFFF, 0xFFD76ABF, 
	0xFFFFFFFF, 0xFFFFFCFF, 0xFFEFD553, 0xFFFFFFFF, 
	0xFFFFFFF3, 0xFFFFD5D4, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xC3FFB7E8, 0x3FFFFFFF, 0xFFFFFFFF, 0xFFC03FC0, 
	0x3FFFFFFF, 0xFFFFFFFF, 0xFFFFFC03, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
	0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 
};

void update_screen(){
	
}

void update_patch_name(){
	switch(current_patch_no) {
		case WAVEGUIDE_1D:
		term_print(&pmsynth_display.term, "       1D Waveguide\n",2);
			break;
		case KARPLUS_STRONG:
		term_print(&pmsynth_display.term, "       Karplus Strong\n",2);
			break;
		case WOODWIND:
		term_print(&pmsynth_display.term, "       Woodwind\n",2);
			break;
	}
}