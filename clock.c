/* clock.c -- controller part of the clock project
 *
 * Darren Provine, 17 July 2009
 *
 * Copyright (C) Darren Provine, 2009-2019, All Rights Reserved
 */

#include "clock.h"

/* CONTROLLER */

static char bugaddress[]="kilroy@elvis.rowan.edu";

// version -- say which version this is and exit
// (note simple output; you could also print the rcsid too)
void version()
{
    fprintf(stderr, "simple clock version 2020.11\n");
    exit(0);
}

// usage -- print brief summary and instructions
void usage(char *progname)
{
    fprintf(stderr, "This program displays a realtime clock.\n");
    fprintf(stderr, "Usage: %s [-advh] [-o number]\n", progname);
    fprintf(stderr, "  -a    : am/pm instead of 24 hour\n");
    fprintf(stderr, "  -d    : show date instead of time\n");
    fprintf(stderr, "  -l    : use simulated LED display\n");
    fprintf(stderr, "  -o #  : offset the time by # seconds \n");
    fprintf(stderr, "  -v    : show version information\n");
    fprintf(stderr, "  -h    : this help message\n");
    fprintf(stderr, "report bugs to %s \n", bugaddress);
    exit (0);
}

// These store timestamps for when the different modes end
int test_mode_end;
int date_mode_end;

void process_key(keybits KeyCode)
{
    void stop_clock(void);
    int KeyRow, KeyCol;
    int view_props;
    time_t now;
    
    if ( ( KeyCode & 0xff00 ) == 0 ) {  // no ASCII code, so mouse hit

        // TODO: figure out KeyRow and KeyCol

        KeyRow = KeyCode & 0xf; KeyCol = (KeyCode & 0xf0) >> 4;

        if (KeyRow == 0) {
            switch (KeyCol) {
                case 0: // 24-hour mode
                    view_props = get_view_properties();
                    view_props &= ( ~ AMPM_MODE );
                    set_view_properties (view_props);
                    KeyCode = 0;
                    break;
                case 1:
                    view_props = get_view_properties();
                    view_props |= ( AMPM_MODE );
                    set_view_properties (view_props);
                    KeyCode = 0;
                    break;
                case 2:
                    view_props = get_view_properties();
                    view_props |= (DATE_MODE);
                    set_view_properties(view_props);
                    date_mode_end = (int) time(NULL) + 5; //5 second pause
                    KeyCode = 0;
                    break;
                case 3:
                    view_props = get_view_properties();                    
                    view_props |= (TEST_MODE); //isnt this for the title bar? if so, wheres the var for test mode
                    set_view_properties(view_props);
                    test_mode_end = (int) time(NULL) + 5; //5 second pause as well
                    KeyCode = 0;
                    break;
                case 4:
                    stop_clock();
                    break;
            }
        } else if (KeyRow == 1) {
            switch (KeyCol) {
                case 0:
                    break;
            }
        }        
    } else { // keystroke
        // figure out ASCII value from first 8 bits
        // right now any key goes to 24-hour mode
        KeyCode >>= 8;
        switch( KeyCode ) {
            case '2':
                view_props = get_view_properties();
                view_props &= ( ~ AMPM_MODE );
                set_view_properties (view_props);
                KeyCode = 0;
                break;
            case 'a':
                view_props = get_view_properties();
                view_props |= ( AMPM_MODE );
                set_view_properties (view_props);
                KeyCode = 0;
                break;
            case 'd':
                view_props = get_view_properties();
                view_props |= (DATE_MODE);
                set_view_properties (view_props);
                date_mode_end = (int) time(NULL) + 5; //5 second pause
                KeyCode = 0;
                break;
            case 't':
                view_props = get_view_properties();
                view_props |= (TEST_MODE); //isnt this for the title bar? if so, wheres the var for test mode
                set_view_properties (view_props);
                test_mode_end = (int) time(NULL) + 5; //5 second pause
                KeyCode = 0;
                break;
            case 'q':
                stop_clock();
                break;
        }
    }

    // force update when keys are hit
    tick(0);
}

void stop_clock()
{
    end_display();
    exit(0);
}

int main(int argc, char *argv[])
{
    int letter;  // option character

    // next three are for setting view properties
    int view_props;
    int ampm = 0;     // default to 24hr
    int date = 0;     // default to time
    int LED  = 0;     // default to text
    

    // loop through all the options; getopt() can handle together or apart
    while ( ( letter = getopt(argc, argv, "adlo:vh")) != -1 ) {
        // *INDENT-OFF*
        switch (letter) {
            case 'a':  ampm = 1;               break;
            case 'd':  date = 1;               break;                
            case 'l':  LED  = 1;               break;
            case 'o':  set_offset (atoi(optarg));  break;
            case 'v':  version();              break;
            case 'h':  usage(argv[0]);         break;

            case '?':  // unknown flag; fall through to next case
            default:   // shouldn't happen, but Just In Case
                       // note that getopt() warns about the unknown flag
                  fprintf(stderr, "run \"%s -h\" for help\n", argv[0]);
                  exit(1);
        }
        // *INDENT-ON*
    }

    // All the flags have been scanned.
    // "optind" is now the index of the first non-option item

    // set view properties; default is "24 hour text mode"
    view_props = 0;  // all bits off
    if ( ampm )
        view_props = AMPM_MODE;
    if ( date )
        view_props |= DATE_MODE;  // note |= to switch on a bit
    if ( LED )
        view_props |= LED_MODE;

    set_view_properties (view_props);

    if (LED) { // set up the fancy display
        start_display();
        // has to be exactly 78 chars
        set_title_bar("----------------------------"
                      " Steven was here at: "
                      "----------------------------");
        register_keyhandler(process_key);

        // turn on some keys in row 2
    }

    /* get the model running */
    start_timer();

    while (1) {
        if (LED) {
            get_key();
        } else {
            pause(); // wait for signal
        }
    }

    /* no return because never reached */
}


/* This function is called is called by the model when a new
 * time is ready for display.
 */
void new_time(struct tm *dateinfo)
{
    int view_props;

    time_t now = time( NULL );

    // handle date mode
    if ( now > date_mode_end ) {
        view_props = get_view_properties();
        view_props &= (~DATE_MODE);
        set_view_properties(view_props);
    }
    if ( now > test_mode_end ) {
       view_props = get_view_properties();
       view_props &= (~TEST_MODE);
       set_view_properties(view_props);
    }    

    show(dateinfo);
}
