/* view.c -- view module for clock project (the V in MVC)
 *
 * Darren Provine, 17 July 2009
 * Modified by S. Orsini Nov 19 2020
 * Copyright (C) Darren Provine, 2009-2019, All Rights Reserved
 */

#include "clock.h"
#include "view.h"

/* see "view.h" for list of bits that set properties */

int view_props = 0x00; // default is 24-hour mode, plain text

// returns old properties so you can save them if needed
void set_view_properties(int viewbits)
{
    view_props = viewbits;
}

int get_view_properties()
{
    return view_props;
}


void do_test(struct tm *dateinfo)
{
    // turn display bits on and off
    
    display();
    fflush(stdout);
}

#define MAX_TIMESTR 40 // big enough for any valid data
// make_timestring
// returns a string formatted from the "dateinfo" object.
char * make_timestring (struct tm *dateinfo, int dividers)
{
    char *timeformat; // see strftime(3)

    if ( view_props & DATE_MODE ) {
        // if dividers is true:
        //   make a string such as "10/31/12 dt" or " 3/17/12 dt"
        //   (note: no leading zero on month!)
        // if dividers is false:
        //   make a string such as "103112d" or " 31712d"
        //   (note: no leading zero on month!)
        // 
	if(dividers){
            timeformat = "%-m/%d/%Y dt";
	}
	else{
	    timeformat = "%-m%d%Yd";    
	}
    } else {
        // if dividers is true:
        //   am/pm: make a string such as "11:13:52 am" or " 4:21:35 pm"
        //          (note: no leading zero on hour!)
        //   24 hr: make a string such as "14:31:25 24"
        //          (include leading zero on hour)
        // if dividers is false:
        //   am/pm: make a string such as "111352a" or " 42135p"
        //          (note: no leading zero on hour!)
        //   24 hr: make a string such as "1431252"
        //          (include leading zero on hour)
        // see strftime(3) for details
        if ( dividers ) {
            if ( view_props & AMPM_MODE ) {
		timeformat = "%l:%M:%S %p";
            } 
	    else {
                timeformat = "%H:%M:%S 24";
            }
        } 
	else {
            if(view_props & AMPM_MODE){
		timeformat = "%l%M%S";
	    }
	    else {
		timeformat = "%H%M%S";
	    }
	}
    }

    // make the timestring and return it
    static char timestring[MAX_TIMESTR];
    strftime(timestring, MAX_TIMESTR, timeformat, dateinfo);
    return timestring;
}

/* We get a pointer to a "struct tm" object, put it in a string, and
 * then send it to the screen.
 */
void show_led(struct tm *dateinfo)
{

    digit *where = get_display_location();
    int i;
    digit  bitvalues = 0;
    int hour;
    int indicator;

    if ( view_props & TEST_MODE ) {
        do_test(dateinfo);
        return;
    }
    
    for (i = 0; i < 6; i++) {
        switch ( make_timestring(dateinfo, 0)[i] ) {
            case ' ': bitvalues = 0x00; break;
            case '1': bitvalues = 0x42; break;
            case '2': bitvalues = 0x37; break;
            case '3': bitvalues = 0x67; break;
            case '4': bitvalues = 0x4b; break;
            case '5': bitvalues = 0x6d; break;
            case '6': bitvalues = 0x7d; break;
            case '7': bitvalues = 0x46; break;
            case '8': bitvalues = 0x7f; break;
            case '9': bitvalues = 0x6f; break;
            case '0': bitvalues = 0x7e; break;
        }
        where[i] = bitvalues;
    }
    display();
    fflush(stdout);
}

void show_text(struct tm *dateinfo)
{
    printf("\r%s ", make_timestring(dateinfo, 1));
    fflush(stdout);
}


void show(struct tm *dateinfo)
{
    if ( view_props & LED_MODE )
        show_led(dateinfo);
    else
        show_text(dateinfo);        
}
