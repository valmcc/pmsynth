//-----------------------------------------------------------------------------
/*

2d waveguide grid

*/
//-----------------------------------------------------------------------------

#include "pmsynth.h"
#include "utils.h"

#define DEBUG
#include "logging.h"

//-----------------------------------------------------------------------------

#define STRIKE_POS_X 2
#define	STRIKE_POS_Y 4
#define GRID_SIZE 7
#define DECAY 0.5f // decay between junctions (keep below 0.5)
#define BOUNDARY_REFLN -0.99f

//-----------------------------------------------------------------------------

void wg_2d_gen(struct wg_2d *osc, float *out, size_t n) {
	float mallet_out = 0;
	for (size_t i = 0; i < n; i++) {
		if (i % 2 == 0){

			// TICK

			// mallet hit
			if (osc->estate == 1){
				mallet_out = mallet_gen_2d(osc);		
			} 

			// calculate junction velocity
			for (size_t x = 0; x <= (GRID_SIZE - 1); x++){
				for (size_t y = 0; y <= (GRID_SIZE - 1); y++){
					osc->mesh[x][y].vJ = DECAY * (osc->mesh[x][y].vE +
										 osc->mesh[x][y+1].vW +
										 osc->mesh[x][y].vN +
										 osc->mesh[x+1][y].vS);
				}
			}
			// add the exciter sample
			osc->mesh[STRIKE_POS_X][STRIKE_POS_Y].vJ += mallet_out;
			// calculate out-travelling waves from junction
			for (size_t x = 1; x <= GRID_SIZE; x++){
				for (size_t y = 1; y <= GRID_SIZE; y++){
					osc->mesh[x-1][y].vE1 = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x-1][y].vW;

					osc->mesh[x][y-1].vN1 = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x][y-1].vS;

					osc->mesh[x-1][y-1].vW1 = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x-1][y-1].vE;

					osc->mesh[x-1][y-1].vS1 = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x-1][y-1].vN;
				}
			}

		} else{

			// TOCK
			// mallet hit
			if (osc->estate == 1){
				mallet_out = mallet_gen_2d(osc);		
			} 

			// calculate junction velocity
			for (size_t x = 0; x <= GRID_SIZE - 1; x++){
				for (size_t y = 0; y <= GRID_SIZE - 1; y++){
					osc->mesh[x][y].vJ = DECAY * 
										(osc->mesh[x][y].vE1 +
										 osc->mesh[x][y+1].vW1 +
										 osc->mesh[x][y].vN1 +
										 osc->mesh[x+1][y].vS1);
				}
			}
			
			// add the exciter sample
			osc->mesh[STRIKE_POS_X][STRIKE_POS_Y].vJ += mallet_out;

			// calculate out-travelling waves from junction
			for (size_t x = 1; x <= GRID_SIZE; x++){
				for (size_t y = 1; y <= GRID_SIZE; y++){
					osc->mesh[x-1][y].vE = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x-1][y].vW1;

					osc->mesh[x][y-1].vN = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x][y-1].vS1;

					osc->mesh[x-1][y-1].vW = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x-1][y-1].vE1;

					osc->mesh[x-1][y-1].vS = osc->mesh[x-1][y-1].vJ - 
											osc->mesh[x-1][y-1].vN1;
				}
			}
			// boundary calculations
			for (size_t x = 1; x <= GRID_SIZE; x++){
				osc->mesh[x-1][0].vE = BOUNDARY_REFLN * 
									osc->mesh[x-1][0].vW1;

				osc->mesh[x-1][GRID_SIZE].vW = BOUNDARY_REFLN * 
									osc->mesh[x-1][GRID_SIZE].vE1;	
			}
			for (size_t y = 1; y <= GRID_SIZE; y++){
				osc->mesh[0][y-1].vN = BOUNDARY_REFLN * 
									osc->mesh[0][y-1].vS1;

				osc->mesh[GRID_SIZE][y-1].vS = BOUNDARY_REFLN * 
									osc->mesh[GRID_SIZE][y-1].vN1;	
			}
		
		}
		out[i] = osc->mesh[2][2].vJ;
	}
}

//-----------------------------------------------------------------------------

void wg_2d_pluck(struct wg_2d *osc) {
	// On each time interval, insert the next sample point of the exciter
	// sample into a point of the delay line.

	// for now it sets a flag for wg_gen to begin adding this to the delay
	// line
	osc->estate = 1;
	osc->epos = 0;
}

//-----------------------------------------------------------------------------

void wg_2d_ctrl_attenuate(struct wg_2d *osc, float attenuate) {
	//osc->k = 0.5f * attenuate;
}

void wg_2d_ctrl_frequency(struct wg_2d *osc, float freq) {
	//osc->freq = freq;
	//osc->xstep = (uint32_t) (osc->freq * KS_FSCALE);
}

void wg_2d_init(struct wg_2d *osc) {
	// do nothing
}

//-----------------------------------------------------------------------------
