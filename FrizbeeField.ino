
/*FASTLED LED library : https://github.com/FastLED/FastLED  */
#include "FastLED.h"
/* OneButton Button library :  https://github.com/mathertel/OneButton  */
#include <OneButton.h>


/*
lEFT							RIGHT
|-----|-----------------------|-----|
|	  |						  |		|
|	  |						  |		|
|-----|-----------------------|-----|
	  ^			 ^^			  ^
arrows are the injection points		:		  
"left goal, left border, right border, right goal "
*/
//#define DEBUG

#ifdef DEBUG
#define LONGLED 60 // Half number of LEDs on the long side of the field 
#define SHORTLED 30 // number of LEDs in the width of the field ( goal line length )

#define LED_TYPE    WS2812B // use WS2811_400 for the real field
#define COLOR_ORDER GRB

#define GOALBOXLEN 5  // how long the "goalbox" is ( number of LEDs )
#define BRIGHTNESS 32
#define FIELD_MODE_BUTTON   2 // change teh whole field "look"

#else
#define LONGLED 74 // Half number of LEDs on the long side of the field 
#define SHORTLED 56 // number of LEDs in the width of the field ( goal line length )

#define LED_TYPE     WS2811_400
#define COLOR_ORDER RGB

#define GOALBOXLEN 27  // how long the "goalbox" is ( number of LEDs )
#define BRIGHTNESS 255
#define FIELD_MODE_BUTTON   5 // change teh whole field "look"

#endif

/*where the LEDs are connected*/
#define DATA_PIN_1   A3 // left "border"
#define DATA_PIN_2   A1 // left Goal line
#define DATA_PIN_3   A2 // Right "border"
#define DATA_PIN_4   A4 // right goal line

/*buttons*/
#define GOAL_LEFT_BUTTON  3 // play a goal animation on left side
#define GOAL_RIGHT_BUTTON   4 // play a goal animation on right side

#define NUMLEDBORDER (LONGLED+SHORTLED+LONGLED) // number of LEDs on the really long LED strips ( the ~75m ones )
#define NUMLEDSGOAL (SHORTLED) // number of LEDs on the short strips ( like the 20m goal line )
#define GOALPOS  (LONGLED-GOALBOXLEN) // LED number on teh long strip where the Goal Line is

#define FPS 100 // framerate

#define MAX_IDLE_STATES 6
#define MAX_IDLE_STATE_IDX MAX_IDLE_STATES-1
#define LEFT_GOAL_STATE MAX_IDLE_STATE_IDX+1
#define RIGHT_GOAL_STATE LEFT_GOAL_STATE+1
#define MAX_STATE RIGHT_GOAL_STATE

/*some names so code has easier reading*/
#define LEFTSIDE true 
#define RIGHTSIDE false 

/*what animation is currently on the field*/
uint8_t fieldState = 4;
uint8_t gHue = 0; // global hue that the field cycles through
uint8_t hueL, hueC, hueR;
// LED arrays
CRGB borderLeft[NUMLEDBORDER]; // the long ( 75 m ) piece that goes around the  left side
CRGB goalLeft[NUMLEDSGOAL]; // shorter ( 20 m ) Goal line on left side
CRGB borderRight[NUMLEDBORDER];
CRGB goalRight[NUMLEDSGOAL];

OneButton leftGoal(GOAL_LEFT_BUTTON, true);
OneButton rightGoal(GOAL_RIGHT_BUTTON, true);
OneButton modeButton(FIELD_MODE_BUTTON, true);

void setup()
{
	/*set LED controllers*/
	FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(borderLeft, NUMLEDBORDER).setCorrection(TypicalLEDStrip); // long left
	FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(goalLeft, NUMLEDSGOAL).setCorrection(TypicalLEDStrip); //goal line left

	FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(borderRight, NUMLEDBORDER).setCorrection(TypicalLEDStrip); // long right
	FastLED.addLeds<LED_TYPE, DATA_PIN_4, COLOR_ORDER>(goalRight, NUMLEDSGOAL).setCorrection(TypicalLEDStrip); // goal line right

// set master brightness control
	FastLED.setBrightness(BRIGHTNESS);
	Serial.begin(115200);
	/*set what happens when a button is pressed*/
	leftGoal.attachClick(leftGoalClick);
	rightGoal.attachClick(rightGoalClick);
	modeButton.attachClick(modeClick);
	// set the button pins
	pinMode(GOAL_LEFT_BUTTON, INPUT_PULLUP);
	pinMode(GOAL_RIGHT_BUTTON, INPUT_PULLUP);
	pinMode(FIELD_MODE_BUTTON, INPUT_PULLUP);

}

void loop()
{
	/* add main program code here */
	EVERY_N_MILLISECONDS(25) { gHue++; } // slowly cycle the "base color" through the rainbow
	EVERY_N_MINUTES(20) { fieldState = random8(MAX_IDLE_STATE_IDX); } // change pattern every few minutes

	switch (fieldState){
		case 0 : // rainbow Cycle, whiteish centre out to full colour at goals
			halfGradient(LEFTSIDE, CHSV(gHue, 0, 255), CHSV(gHue + 50, 200, 255)); // left
			halfGradient(RIGHTSIDE, CHSV(gHue, 0, 255), CHSV(gHue + 50, 200, 255)); // right
			break;
		case 1: // "bouncy ball things" going up and down each side
			heterodyne(LEFTSIDE);
			heterodyne(RIGHTSIDE);
			break;
		case 2: // whole field sparkle
				// random colored speckles that blink in and fade smoothly
			fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
			fadeToBlackBy(goalLeft, NUMLEDSGOAL, 10);
			borderLeft[random16(NUMLEDBORDER)] += CHSV(gHue + random8(64), beatsin8(1), 255);
			goalLeft[random16(NUMLEDSGOAL)] += CHSV(gHue + random8(64), beatsin8(1), 255);

			fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
			fadeToBlackBy(goalRight, NUMLEDSGOAL, 10);
 		    borderRight[random16(NUMLEDBORDER)] += CHSV(gHue + random8(64), beatsin8(1), 255);
			goalRight[random16(NUMLEDSGOAL)] += CHSV(gHue + random8(64), beatsin8(1), 255);

			break;
		case 3 :  // white field, coloured goal boxes
			fill_solid(borderLeft, NUMLEDBORDER, CHSV(gHue, 0, 255));
			fillGoalbox(LEFTSIDE, CHSV(gHue,255,255));

			fill_solid(borderRight, LONGLED, CHSV(gHue, 0, 255));
			fillGoalbox(RIGHTSIDE, CHSV(gHue+128, 255, 255));
			break;
		case 4:  // sluggish rainbow
			sluggishRainbow();
			break;

		case 5:  // one wave over the whole field
			hueC = gHue;
			hueL = gHue - 32;
			hueR =  gHue + 32;
			halfGradient(LEFTSIDE, CHSV(hueC, 200, beatsin8(1)), CHSV(hueL, 50, beatsin8(2,32))); // left
			halfGradient(RIGHTSIDE, CHSV(hueC, 200, beatsin8(1)), CHSV(hueR, 50, beatsin8(2,32))); // right
			break;

		case LEFT_GOAL_STATE: // LEFT GOAL scored
			sparkleGoalbox(LEFTSIDE);
			EVERY_N_SECONDS(10) { fieldState = 0; }//  sparkle for 10 seconds
			break;
		case RIGHT_GOAL_STATE: // RIGHT GOAL scored
			sparkleGoalbox(RIGHTSIDE);
			EVERY_N_SECONDS(10) { fieldState = 0; } //  sparkle for 10 seconds
			break;
		default:
			fieldState = 0;
			break;
		}
	// some serial control to change modes
	if (Serial.available()>0)
	{
		char c = Serial.read();
		switch (c)
		{
		case '0' : // cycle overall mode
			fieldState++;
			if (fieldState >= LEFT_GOAL_STATE)
			{
				fieldState = 0;
			}
			Serial.print("State: ");
			Serial.println(fieldState);
			break;
		case '1': // LEFT goal animation
			fieldState = LEFT_GOAL_STATE;
			break;
		case '2': // Right Goal animation
			fieldState = RIGHT_GOAL_STATE;
			break;
		default:
			break;
		}
	}

	FastLED.show();
	FastLED.delay(1000 / FPS);
	/*check buttons*/
	leftGoal.tick();
	rightGoal.tick();
	modeButton.tick();
}

/*buttons:*/
/*left goal animation*/
void leftGoalClick() {
	fieldState = LEFT_GOAL_STATE;
	Serial.println(F("left goal"));

}
/*right goal animation*/
void rightGoalClick() {
	fieldState = RIGHT_GOAL_STATE;
	Serial.println(F("Right goal"));
}
/*increment mode*/
void modeClick() {
	fieldState++;
	if (fieldState >= LEFT_GOAL_STATE) {
		fieldState = 0;
	}
	Serial.print(F("mode is :"));
	Serial.println(fieldState);
}

/*a rainbow fade that does discrete movements between hues*/
void sluggishRainbow() {
	static boolean pause = false;

	// fade to a hue
	// pause
	
	if (!pause) {
		// fade to a hue
		static uint8_t oldHue;
		// fade old out
		CHSV newCol,newColEdge;
		for (int i = 0; i < 255; i++) {
			newCol = blend(CHSV(oldHue,200,255), CHSV(gHue,200,255), i);
			newColEdge = newCol;
			newCol.sat = 10;

			halfGradient(LEFTSIDE, newCol, newColEdge); // left
			newColEdge.h += 128;
			halfGradient(RIGHTSIDE, newCol, newColEdge); // right
			FastLED.show();
		}
		halfGradient(LEFTSIDE, CHSV(gHue, 10, 255), CHSV(gHue, 200, 255)); // left
		halfGradient(RIGHTSIDE, CHSV(gHue, 10, 255), CHSV(gHue+128, 200, 255)); // // right
	//	Serial.println(F("Faded"));
		// fade new in
		//for (int i = 0; i < 64; i++) {
		//	halfGradient(LEFTSIDE, CHSV(gHue, 10, i * 4), CHSV(gHue, 200, i * 4)); // left
		//	halfGradient(RIGHTSIDE, CHSV(gHue, 10, i * 4), CHSV(gHue + 128, 200, i * 4)); // right
		//	FastLED.show();
		//}
		oldHue = gHue;
			pause = true;
	}
	else {
		EVERY_N_MINUTES(1) { pause = false; }
		//EVERY_N_SECONDS(20) { pause = false; }
	}
}


/* shadows of a bouncy ball*/
void pongBounce() {
	
	static uint8_t  ball[2]; // holds the ball X and Y position
	// x == 0 -> LONGLED*2
	// y == 0 -> SHORTLED

	static uint8_t workingArray[LONGLED * 2];
	int ballSpeed = 10;
	double thetaFrac;
	int deltaMid;
	double theta;

	double attenuation[2];
	// [0] is the normalised distance from the line ( 0.00 == near, 1.00 == far )
	// [1] is the inverse of that (1.00 == near, 0.00 == far )


	// move the ball
	if (millis() % ballSpeed < 2) {
		moveBall(ball);
		// "project" ball position on edges
		// (0) calculate ball position
			// (1) draw a sine wave on our "side"
			// (2) shift the wave so the crest is where the ball is
			// (3) attenuate the wave according to teh distance of betwene the ball and the side we are drawng

		// first we do the long sides
		thetaFrac = 128 / (double)(LONGLED * 2); // fraction we add every pixel ( from 0 - 180 degrees  - 0-127 in 8 bit units)
		 theta = 0;
		//(1) draw a sine wave on our "side"
		for (int i = 0; i < (LONGLED * 2); i++)
		{
			workingArray[i] = (sin8((uint8_t)theta) -128)*2;
			theta += thetaFrac;
			// print the array to look
			//Serial.print(workingArray[i]);
			//Serial.print(", ");
		}
	//	Serial.println();
	//	delay(500);
		// (2) shift the wave so the crest is where the ball is
		// X is long direction
		// Y is short direction
		deltaMid = ball[0] - LONGLED;
		// if deltaMid is >0 shift Right >>>
		if (deltaMid >= 0)
		{
			// shift "deltaMid" pixels
			//shift right deltaMid times
			for (int shiftcount = 0; shiftcount < abs(deltaMid); shiftcount++)
			{
				for (int i = (LONGLED * 2) - 1; i > 0; i--) {
					workingArray[i] = workingArray[i - 1];
				}
				workingArray[0] = 0;
			}
		}

		else { 		// if deltaMid is < 0 , shift Left <<<
			//shift left deltaMid times
			for (int shiftcount = 0; shiftcount < abs(deltaMid); shiftcount++)
			{
				for (int i = 0; i < (LONGLED * 2) - 1; i++) {
					workingArray[i] = workingArray[i + 1];
				}
				workingArray[(LONGLED * 2) - 1] = 0;
			}
		}
		// (3) calculate attenuation based on the other direction
		attenuation[0] = ball[1] / (double)SHORTLED; // 0 - 1 scale of distance of ball to Y-zero
		attenuation[0] = constrain(attenuation[0], 0, 1.00);
		attenuation[1] = 1.0 - attenuation[0]; // flip the effect 9 so that 1 means close, 0 means far )
		// apply to the long sides
		// (0,0) long  side first ( attenuation[1] ) 
		// (0,Y_MAX) thereafter ( attenuation[0] ) 

		// inject right [0 -> LONGLED] : workingArray[LONGLED -> (LONGLED * 2)]
		uint8_t attenuatedVal;
		for (int i = 0; i < LONGLED; i++)
		{
			attenuatedVal = workingArray[LONGLED + i] * attenuation[1];
			borderRight[i] = CHSV(gHue, 255, attenuatedVal);
			// far right
			borderRight[(LONGLED + SHORTLED + LONGLED - 1) - i] = CHSV(gHue, 255, attenuatedVal);
		}
		// inject left : left[0 -> LONGLED] : flipped ( workingArray[0 : LONGLED] )
		for (int i = 0; i < LONGLED; i++) 
		{
			 attenuatedVal = workingArray[LONGLED - i] * attenuation[1];
			 // close left
			borderLeft[i] = CHSV(gHue, 255, attenuatedVal);
			//far long left
			borderLeft[(LONGLED + SHORTLED + LONGLED - 1) - i] = CHSV(gHue, 255, attenuatedVal);
		}


		// ******************* endfield lines *********************** 
		thetaFrac = 128 / (double)(SHORTLED); // fraction we add every pixel ( from 0 - 180 degrees  - 0-127 in 8 bit units)
		theta = 0;
		for (int i = 0; i < sizeof(workingArray); i++)
		{
			workingArray[i] = 0;
		}
		//(1) draw a sine wave on our "side"
		for (int i = 0; i < SHORTLED; i++)
		{
			workingArray[i] = (sin8((uint8_t)theta) - 128) * 2;
			theta += thetaFrac;
		}
		deltaMid = ball[1] - (SHORTLED/2);
		// if deltaMid is >0 shift Right >>>
		if (deltaMid >= 0)
		{
			// shift "deltaMid" pixels
			//shift right deltaMid times
			for (int shiftcount = 0; shiftcount < abs(deltaMid); shiftcount++)
			{
				for (int i = SHORTLED - 1; i > 0; i--) {
					workingArray[i] = workingArray[i - 1];
				}
				workingArray[0] = 0;
			}
		}

		else { 		// if deltaMid is < 0 , shift Left <<<
					//shift left deltaMid times
			for (int shiftcount = 0; shiftcount < abs(deltaMid); shiftcount++)
			{
				for (int i = 0; i < SHORTLED - 1; i++) {
					workingArray[i] = workingArray[i + 1];
				}
				workingArray[SHORTLED - 1] = 0;
			}
		}
		// (3) calculate attenuation based on the other direction
		attenuation[0] = ball[0] / (double)(LONGLED*2); // 0 - 1 scale of distance of ball to X-zero
		attenuation[0] = constrain(attenuation[0], 0, 1.00);
		attenuation[1] = 1.0 - attenuation[0]; // flip the effect ( so that 1 means close, 0 means far )
		// left  endfield	
		for (int i = 0; i < SHORTLED; i++)
		{
			attenuatedVal = workingArray[i] * attenuation[1];
			borderLeft[LONGLED + i] = CHSV(gHue, 255, attenuatedVal);
		}
		// right  endfield	
		for (int i = 0; i < SHORTLED; i++)
		{
			attenuatedVal = workingArray[i] * attenuation[0];
			borderLeft[i] = CHSV(gHue, 255, attenuatedVal);
		}

	}
}

void moveBall(uint8_t ball[]) {
	static int ballDirectionX = 1;
	static int ballDirectionY = 1;

	if (ball[0] > LONGLED || ball[0] < 0) {
		ballDirectionX = -ballDirectionX;
	}
	if (ball[1] > SHORTLED|| ball[1] < 0) {
		ballDirectionY = -ballDirectionY;
	}

	ball[0] += ballDirectionX;
	ball[1] += ballDirectionY;
}


/*Fill half the field with a gradient centre toward the endfield*/
void halfGradient(boolean side, CHSV startCol, CHSV endCol ) {
	if (side) // LEFT
	{
		fill_gradient(borderLeft, LONGLED, startCol, endCol);
		fill_gradient(borderLeft+LONGLED+SHORTLED, LONGLED, endCol, startCol);
		fill_solid(borderLeft + LONGLED, SHORTLED, endCol);
		fill_solid (goalLeft, SHORTLED, borderLeft[GOALPOS]); // borderLeft[GOALPOS]
	}
	else // RIGHT
	{
		fill_gradient(borderRight, LONGLED, startCol, endCol);
		fill_gradient(borderRight + LONGLED + SHORTLED, LONGLED, endCol, startCol);
		fill_solid(borderRight + LONGLED, SHORTLED, endCol);
		fill_solid(goalRight, SHORTLED, CRGB(borderRight[GOALPOS]));
	}

}


/*Fill half the field with a gradient*/
void fillGoalbox(boolean side, CHSV goalCol) {
	if (side) // LEFT
	{
		fill_solid(goalLeft, SHORTLED, goalCol); // |
		fill_solid(borderLeft+ GOALPOS,GOALBOXLEN , goalCol); //
		fill_solid(borderLeft + LONGLED, SHORTLED, goalCol);
		fill_solid(borderLeft + LONGLED+ SHORTLED, GOALBOXLEN, goalCol);
	}
	else // RIGHT
	{
		fill_solid(goalRight, SHORTLED, goalCol); // |
		fill_solid(borderRight+ GOALPOS,GOALBOXLEN , goalCol); //
		fill_solid(borderRight + LONGLED, SHORTLED, goalCol);
		fill_solid(borderRight + LONGLED+ SHORTLED, GOALBOXLEN, goalCol);
	}
}


/*glitter in teh goalbox only*/
void addGlitterGoalbox(boolean side) {
	uint8_t chance = 80;
	if (side) // LEFT
	{
		if (random8() < chance) {
			goalLeft[random16(SHORTLED)] += CRGB::White;
		}
		if (random8() < chance) {
			borderLeft[random16(GOALPOS, (LONGLED+ SHORTLED+ GOALBOXLEN) )] += CRGB::White;
		}
	}
	else // RIGHT
	{
		if (random8() < chance) {
			goalRight[random16(SHORTLED)] += CRGB::White;
		}
		if (random8() < chance) {
			borderRight[random16(GOALPOS, (LONGLED + SHORTLED + GOALBOXLEN))] += CRGB::White;
		}
	}
}


/*add sparkles to the goalbox, fades everything else*/
void sparkleGoalbox(boolean side) {
	if (side)
	{
		addGlitterGoalbox(LEFTSIDE);
		fadeToBlackBy(goalLeft, SHORTLED, 10);

		/*fade only the goal box*/
//		fadeToBlackBy(borderLeft+ GOALPOS, (GOALBOXLEN+ SHORTLED + GOALBOXLEN), 10);
		/*fade everything in this half*/
		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
		/*fade whole field*/
//		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
//		fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
//		fadeToBlackBy(goalRight, SHORTLED, 10);

	}
	else
	{
		addGlitterGoalbox(RIGHTSIDE);
		fadeToBlackBy(goalRight, SHORTLED, 10);

		//fade only the goal box
				//		fadeToBlackBy(borderRight+ GOALPOS, (GOALBOXLEN+ SHORTLED + GOALBOXLEN), 10);
		//fade everything in this half
		fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
		//fade whole field
				//		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
				//		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
				//		fadeToBlackBy(goalLeft, SHORTLED, 10);
				
	}
	
}

void heterodyne(boolean side) {
	// eight colored dots, weaving in and out of sync with each other
	byte dothue = 0;
	if (side)
	{
		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
		fadeToBlackBy(goalLeft, SHORTLED, 10);


		for (int i = 0; i < 5; i++) {
			borderLeft[beatsin16(i + 4, 0, LONGLED)] |= CHSV(dothue+gHue, 200, 255); // sideline
			fill_solid(goalLeft, SHORTLED, borderLeft[GOALPOS]); // goalline
		    fill_solid(borderLeft+LONGLED, SHORTLED, borderLeft[LONGLED-1]); // endfield line
			
			dothue += 32;
			for (uint16_t i = 0; i < LONGLED; i++)
			{
				borderLeft[(NUMLEDBORDER-1)-i] = borderLeft[i];
			}

			
		}
	}
	else
	{
		fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
		fadeToBlackBy(goalRight, SHORTLED, 10);


		for (int i = 0; i < 8; i++) {
			borderRight[beatsin16(i + 4, 0, LONGLED)] |= CHSV(dothue, 200, 255); // sideline
			fill_solid(goalRight, SHORTLED, borderRight[GOALPOS]); // goalline
			fill_solid(borderRight + LONGLED, SHORTLED, borderRight[LONGLED - 1]); // endfield line
			 
			dothue += 32;
			for (uint16_t i = 0; i < LONGLED; i++)
			{
				borderRight[(NUMLEDBORDER - 1) - i] = borderRight[i];
			}
		}
	}
	
}