
#include "config.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

uint8_t homing(void){
/*
* Homing cycle:
*/
	lcd_screen(SCREEN_HOMING);
	homing_cycle();
	lcd_screen(SCREEN_HOMING_DONE);

	// should implement a security bounds check while homing
	return 0;
}

state_t choose_movement(void){
/*
* Select movement
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	state_t state = STATE_CREATE_MOVEMENT;

	lcd_screen(SCREEN_CHOOSE_MOVEMENT);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();

		// lcd options
		if(encoder.update){
			encoder.update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				state = STATE_CHOOSE_MANUAL_CONTROL;
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				state = STATE_CREATE_MOVEMENT;
			}
		}
		
		// Check encoder button
		if(btn.query) btn_check();		
		
		// Check action to be taken
		if((btn.action) && (btn.state == BTN_RELEASED) && (!btn.delay1)){
			btn.action = FALSE;
			break;
		}
	}	

	return state;
}

uint8_t choose_manual_control(void){
/*
* Select either position or speed control
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	uint8_t type = CONTROL_POS;
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_MANUAL_CONTROL);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder.update){
			encoder.update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				type = CONTROL_SPEED;
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				type = CONTROL_POS;
			}
		}
		
		// Check encoder button
		if(btn.query) btn_check();
		
		// Check action to be taken
		if((btn.action) && (btn.state == BTN_RELEASED) && (!btn.delay1)){
			btn.action = FALSE;
			break;
		}
		if((btn.action) && (btn.delay3)){
			btn.action = FALSE;
			type = QUIT_MENU;
			break;
		}
	}

	return type;
}

uint8_t choose_manual_movement(void){
/*
* Select either linear or exponential movement
*/
	uint8_t toggle = FALSE;
	uint16_t x;
	uint8_t type = SPEED_LINEAR;
	
	// LCD screen
	lcd_screen(SCREEN_CHOOSE_MANUAL_MOVEMENT);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		if(encoder.update){
			encoder.update = FALSE;
			toggle ^= 1;
			if(toggle){
				lcd_set_cursor(0,0);
				lcd_write_str(" ");
				lcd_set_cursor(1,0);
				lcd_write_str(">");
				type = SPEED_EXPONENTIAL;
			} else {
				lcd_set_cursor(0,0);
				lcd_write_str(">");
				lcd_set_cursor(1,0);
				lcd_write_str(" ");
				type = SPEED_LINEAR;
			}
		}
		
		// Check encoder button
		if(btn.query) btn_check();
		
		// Check action to be taken
		if((btn.action) && (btn.state == BTN_RELEASED) && (!btn.delay1)){
			btn.action = FALSE;
			break;
		}
		if((btn.action) && (btn.delay3)){
			btn.action = FALSE;
			type = QUIT_MENU;
			break;
		}
	}

	return type;
}

/*
const uint16_t exponential_speed[] PROGMEM = {
	1645,1470,1308,1160,1025,902,790,689,598,516,443,378,
	320,270,226,187,154,126,102,82,66,53,42,34,27,23,19,
	17,16,15,14,14,14
	};
*/
const uint16_t exponential_speed[] PROGMEM = {
	3000,3000,1593,896,573,398,293,224,177,143,118,100,85,
	73,64,56,50,44,40,36,33,30,27,25,23,21,20,18,17,16,15,14
};

const uint16_t linear_speed[] PROGMEM = {
	448,448,224,149,112,90,75,64,56,50,45,41,37,34,32,30,28,26,
	25,24,22,21,20,19,19,18,17,17,16,15,15,14
};

uint8_t manual_speed(uint8_t type){

	int8_t n = 0;
	uint16_t x;
	uint16_t speed = 0;
	uint8_t aux = FALSE;

	// LCD screen:
	if(type == SPEED_LINEAR)
		lcd_screen(SCREEN_LINEAR_SPEED);	
	else if(type == SPEED_EXPONENTIAL)
		lcd_screen(SCREEN_EXPONENTIAL_SPEED);
	lcd_update_speed(slider.speed);

	while(TRUE){

		// timing for loop execution
		clear_millis();
		x = 0;
		while(!x) x = millis();
		
		// lcd options
		// !!! there's still a problem with the speed the LCD shows: when in
		// marginal zone, the function lcd_update speed doesn't know that the
		// speed was corrected!. THus, it shows a wrong speed!
		if(encoder.update){
			encoder.update = FALSE;
			
			if(slider.out_of_bounds)
				n = 0;
			if(encoder.dir) n -= 5;
			else n += 5;
			
			if(n > 100) n = 100;
			else if(n < -100) n = -100;

			if((!slider.out_of_bounds) ||
			   ((slider.spin == CW) && (slider.position < MAX_COUNT)) ||
			   ((slider.spin == CCW) && (slider.position > 0))){
				motor_move_at_speed(n);
			} else {
				motor_move_at_speed(0);
			}
			lcd_update_speed(slider.speed);
			////////////////////////////////////////////////
		}
		
		// this little snippet should be re-arranged. Not elegant.
		if(slider.out_of_bounds){
			if(!aux){
				speed = 0;
				lcd_update_speed(speed);
				aux = TRUE;
			}
		} else {
			aux = FALSE;
		}

		
		// Check encoder button
		if(btn.query) btn_check();
		
		// Check action to be taken
		if((btn.action) && (btn.delay3)){
			btn.action = FALSE;
			speed_set(0);
			break;
		}
	}

	return 0;
}

/*
* Acceleration level
* 	Contains the time (in ms) of duration of every speed step
*	The higher the number, the lower the acceleration profile
*/
static const uint8_t accel_level[] = {
	25, 50, 75, 100, 125, 150
};

static const uint16_t SZ = sizeof(exponential_speed)/sizeof(uint16_t);

uint8_t manual_position(uint8_t type){

	// speed vector vars
	uint16_t n = 0;
	uint16_t n_prev = 0;
	// counters to coordinate timing and FSM execution
	uint16_t x = 0;
	// positioning vars
	static int32_t setpoint = 0;
	int32_t error = 0;
	// misc 
	uint16_t speed = 0;
	uint8_t state = 0;		// FSM
	uint8_t acc = 5;		// Temporary. Must be chosen by user

	// LCD screen:
	if(type == SPEED_LINEAR)
		lcd_screen(SCREEN_LINEAR_POSITION);	
	else if(type == SPEED_EXPONENTIAL)
		lcd_screen(SCREEN_EXPONENTIAL_POSITION);
	lcd_update_position(slider.position);

	clear_millis();

	while(TRUE){

		// timing for loop execution
		x = millis();
		
		switch (state) {

			case 0:
				// Encoder detection: if encoder movement is detected:
				// - look for rotation direction
				// - move position setpoint accordingly
				if(encoder.update){
					encoder.update = FALSE;
					
					// Determines encoder position change
					if(encoder.dir == CW){
						setpoint += (MAX_COUNT / 100);
						if(setpoint >= MAX_COUNT)
							setpoint = MAX_COUNT;
					} else if(encoder.dir == CCW){
						setpoint -= (MAX_COUNT / 100);
						if(setpoint <= 0)
							setpoint = 0;
					}
					// Motor direction determined according to error sign
					error = setpoint - ((int32_t)slider.position);
					if (error > 0) {
						slider.spin = CW;
						drv_spin_direction(slider.spin);
					} else if (error < 0) {
						slider.spin = CCW;
						drv_spin_direction(slider.spin);
					}
					state = 1;	// go to choose the value of n
				}
				break;

			case 1:
				// Speed value determined according to:
				// - previous speed value: can only increase one speed step at a time
				// - closeness to the end point: the closer, the slower
				error = setpoint - ((int32_t)slider.position);
				n = (uint16_t)(abs(error) / (2*accel_level[acc]));
				if(n > SZ)
					n = SZ;
				// Acceleration occurs by increasing one speed step at a time, at time
				// intervals determined by the selected acceleration level			
				if (n > n_prev) {
					n = n_prev + 1;
					if (x >= accel_level[acc]){
						x = 0;
						clear_millis();
						n_prev = n;				
					}
				} else {
					n_prev = n;
				}

				lcd_update_position(slider.position);
				DEBUG("1\n\rn: ");
				char str[3];
				itoa(n,str,10);
				DEBUG(str);
				DEBUG("\n\r");

				if (n == 0)
					state = 2;	// go to trimming position 
								
				if (type == SPEED_LINEAR) 
					speed = pgm_read_word(linear_speed + abs(n));
				else if (type == SPEED_EXPONENTIAL) 
					speed = pgm_read_word(exponential_speed + abs(n));
				speed_set(speed);
				
				if (encoder.update) {
					state = 0;
					DEBUG("e");
				}
				break;

			case 2:
				error = setpoint - ((int32_t)slider.position);
				if (error) {
					if (encoder.update)
						state = 0;
				} else {
					speed_set(0);
					DEBUG("trimmed\n\r");
					x = 0;
					clear_millis();
					state = 0;
				}
				break;

			default: 
				state = 0;
				break;
		}

		// Check encoder button
		if(btn.query) btn_check();
		
		// Check action to be taken
		if((btn.action) && (btn.delay3)){
			btn.action = FALSE;
			speed_set(0);
			break;
		}
	}

	return 0;
}

void user_movement(void){

	// LCD screen:
	//lcd_screen(SCREEN_INITIAL_POS);	
	
}

void fail_message(void){

	lcd_screen(SCREEN_FAIL_MESSAGE);	
}