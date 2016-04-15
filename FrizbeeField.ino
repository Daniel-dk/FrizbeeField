#include "FastLED.h"

#define LONGLED 60
#define SHORTLED 30

#define NUMLEDBORDER (LONGLED+SHORTLED+LONGLED) // number of LEDs per side 
#define NUMLEDSGOAL (SHORTLED)
#define GOALBOXLEN 5
#define GOALPOS  (LONGLED-GOALBOXLEN)
#define FPS 100

#define LEFTSIDE true
#define RIGHTSIDE false 

#define DATA_PIN_1   A0
#define DATA_PIN_2   A1
#define DATA_PIN_3   A2
#define DATA_PIN_4   A3

#define LED_TYPE    WS2812B // WS2811_400
#define COLOR_ORDER GRB

#define MAX_IDLE_STATES 4
#define MAX_IDLE_STATE_IDX MAX_IDLE_STATES-1
#define LEFT_GOAL_STATE MAX_IDLE_STATE_IDX+1
#define RIGHT_GOAL_STATE LEFT_GOAL_STATE+1
#define MAX_STATE RIGHT_GOAL_STATE

uint8_t fieldState = 0;
uint8_t gHue = 0;

CRGB borderLeft[NUMLEDBORDER];
CRGB goalLeft[NUMLEDSGOAL];

//CRGB borderRight[NUMLEDBORDER];
//CRGB goalRight[NUMLEDSGOAL];





void setup()
{
	FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(borderLeft, NUMLEDBORDER).setCorrection(TypicalLEDStrip); // long left
	FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(goalLeft, NUMLEDSGOAL).setCorrection(TypicalLEDStrip); //goal line left

//	FastLED.addLeds<LED_TYPE, DATA_PIN_3, COLOR_ORDER>(borderRight, NUMLEDBORDER).setCorrection(TypicalLEDStrip); // long right
//	FastLED.addLeds<LED_TYPE, DATA_PIN_4, COLOR_ORDER>(goalRight, NUMLEDSGOAL).setCorrection(TypicalLEDStrip); // goal line right

// set master brightness control
	FastLED.setBrightness(32);

	Serial.begin(115200);
	halfGradient(true, CHSV(0, 200, 255), CHSV(120, 200, 255)); // left : red to green
}

void loop()
{
	/* add main program code here */
	EVERY_N_MILLISECONDS(20) { gHue++; } // slowly cycle the "base color" through the rainbow
	EVERY_N_MINUTES(10) { fieldState = random8(MAX_IDLE_STATE_IDX); } // change pattern every few minutes

	switch (fieldState){
		case 0 : // rainbow Cycle, whiteish centre out to full colour at goals
			halfGradient(LEFTSIDE, CHSV(gHue, 50, 255), CHSV(gHue + 50, 200, 255)); // left
			halfGradient(RIGHTSIDE, CHSV(gHue, 50, 255), CHSV(gHue + 50, 200, 255)); // right
			break;
		case 1: // "bouncy ball things"
			heterodyne(LEFTSIDE);
			heterodyne(RIGHTSIDE);
			break;
		case 2: // whole field sparkle
				// random colored speckles that blink in and fade smoothly
			fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
			fadeToBlackBy(goalLeft, NUMLEDSGOAL, 10);

			borderLeft[random16(NUMLEDBORDER)] += CHSV(gHue + random8(64), 200, 255);
			goalLeft[random16(NUMLEDSGOAL)] += CHSV(gHue + random8(64), 200, 255);

//			fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
//			fadeToBlackBy(goalRight, NUMLEDSGOAL, 10);
//  		borderRight[random16(NUMLEDBORDER)] += CHSV(gHue + random8(64), 200, 255);
//			goalRight[random16(NUMLEDSGOAL)] += CHSV(gHue + random8(64), 200, 255);

			break;
		case 3 :  // white field, coloured goal boxes
			fill_solid(borderLeft, NUMLEDBORDER, CHSV(gHue, 0, 255));
			fillGoalbox(LEFTSIDE, CHSV(gHue,255,255));

//			fill_solid(borderRight, LONGLED, CHSV(gHue, 0, 255));
//			fillGoalbox(RIGHTSIDE, CHSV(gHue+128, 255, 255));


			break;

		case LEFT_GOAL_STATE: // LEFT GOAL
			sparkleGoalbox(LEFTSIDE);
			EVERY_N_SECONDS(10) { fieldState = 0; }// only sparkle for 10 seconds
			break;
		case RIGHT_GOAL_STATE: // RIGHT GOAL
			sparkleGoalbox(RIGHTSIDE);
			EVERY_N_SECONDS(10) { fieldState = 0; } // only sparkle for 10 seconds
			break;
		default:
			fieldState = 0;
			break;
		}

	if (Serial.available()>0)
	{
		char c = Serial.read();
		switch (c)
		{
		case '1' :
			halfGradient(LEFTSIDE, CHSV(random8(),200, random8()), CHSV(random8(), random8(), random8())); // left
			break;
		case '2':
			fillGoalbox(LEFTSIDE, CHSV(random8(), 200, 255));
			break;
		case '3':

			for (int i = 0; i < 200; i++)
			{
				sparkleGoalbox(LEFTSIDE);
				FastLED.show();
				FastLED.delay(1000 / 500);
			}
			
			break;
		case 's':
			fieldState++;
			break;
		default:
			break;
		}
	}



	FastLED.show();
	FastLED.delay(1000 / FPS);
}

/*Fill half the field with a gradient*/
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
		/*fill_gradient(borderRight, LONGLED, startCol, endCol);
		fill_gradient(borderRight + LONGLED + SHORTLED, LONGLED, endCol, startCol);
		fill_solid(borderRight + LONGLED, SHORTLED, endCol);
		fill_solid(goalRight, SHORTLED, CRGB(borderRight[GOALPOS]));*/
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
		/*fill_solid(goalRight, SHORTLED, goalCol); // |
		fill_solid(borderRight+ GOALPOS,GOALBOXLEN , goalCol); //
		fill_solid(borderRight + LONGLED, SHORTLED, goalCol);
		fill_solid(borderRight + LONGLED+ SHORTLED, GOALBOXLEN, goalCol);*/
	}
}


/*Fill half the field with a gradient*/
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
		/*fill_solid(goalRight, SHORTLED, goalCol); // |
		fill_solid(borderRight+ GOALPOS,GOALBOXLEN , goalCol); //
		fill_solid(borderRight + LONGLED, SHORTLED, goalCol);
		fill_solid(borderRight + LONGLED+ SHORTLED, GOALBOXLEN, goalCol);*/
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
		/*addGlitterGoalbox(RIGHTSIDE);
		fadeToBlackBy(goalRight, SHORTLED, 10);

		//fade only the goal box
				//		fadeToBlackBy(borderRight+ GOALPOS, (GOALBOXLEN+ SHORTLED + GOALBOXLEN), 10);
		//fade everything in this half
//		fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
		//fade whole field
				//		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
				//		fadeToBlackBy(borderLeft, NUMLEDBORDER, 10);
				//		fadeToBlackBy(goalLeft, SHORTLED, 10);
				*/
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
			borderLeft[beatsin16(i + 7, 0, LONGLED)] |= CHSV(dothue+gHue, 200, 255); // sideline
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
	//	fadeToBlackBy(borderRight, NUMLEDBORDER, 10);
	//	fadeToBlackBy(goalRight, SHORTLED, 10);


	//	for (int i = 0; i < 8; i++) {
	//		borderRight[beatsin16(i + 7, 0, LONGLED)] |= CHSV(dothue, 200, 255); // sideline
	//		fill_solid(goalRight, SHORTLED, borderRight[GOALPOS]); // goalline
	//		fill_solid(borderRight + LONGLED, SHORTLED, borderRight[LONGLED - 1]); // endfield line
	//		 
	//		dothue += 32;
	//		for (uint16_t i = 0; i < LONGLED; i++)
	//		{
	//			borderRight[(NUMLEDBORDER - 1) - i] = borderRight[i];
	//		}
	//	}
	}
	
}