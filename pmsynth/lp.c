
				// y[n] = bx[n]+bx[n-1]+ay[n-1]   low pass
				noise_in = rand_float() * 0.1f;
				noise_out = osc->lp_coef_b * noise_in
						 + osc->lp_coef_b * noise_in_old 
						 + osc->lp_coef_a * noise_out;
				noise_in_old = noise_in;
				osc->delay_l[osc->x_pos_l] += noise_out;
				osc->delay_l[osc->x_pos_r] += noise_out;