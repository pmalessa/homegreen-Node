/******************************************************************************* 
*   $FILE:  main.c
*   Atmel Corporation:  http://www.atmel.com \n
*   Support email:  touch@atmel.com
******************************************************************************/

/*  License
*   Copyright (c) 2010, Atmel Corporation All rights reserved.
*   
*   Redistribution and use in source and binary forms, with or without
*   modification, are permitted provided that the following conditions are met:
*   
*   1. Redistributions of source code must retain the above copyright notice,
*   this list of conditions and the following disclaimer.
*   
*   2. Redistributions in binary form must reproduce the above copyright notice,
*   this list of conditions and the following disclaimer in the documentation
*   and/or other materials provided with the distribution.
*   
*   3. The name of ATMEL may not be used to endorse or promote products derived
*   from this software without specific prior written permission.
*   
*   THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
*   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
*   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
*   SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
*   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
*   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
*   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*----------------------------------------------------------------------------
                            compiler information
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                                include files
----------------------------------------------------------------------------*/

#include <avr/io.h>
#include <avr/interrupt.h>
#define __delay_cycles(n)     __builtin_avr_delay_cycles(n)
#define __enable_interrupt()  sei()

/*  now include touch api.h with the localization defined above */
#include "touch_api.h"

#ifdef _DEBUG_INTERFACE_
/* Include files for QTouch Studio integration */
#include "QDebug.h"
#include "QDebugTransport.h"
#endif

/*----------------------------------------------------------------------------
                            manifest constants
----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
                            type definitions
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                                prototypes
----------------------------------------------------------------------------*/

/*  initialise host app, pins, watchdog, etc    */
static void init_system( void );
/*  configure timer ISR to fire regularly   */
static void init_timer_isr( void );
/*  Assign the parameters values to global configuration parameter structure    */
static void qt_set_parameters( void );
/*  Configure the sensors */
static void config_sensors(void);

#if defined(_ROTOR_SLIDER_)
    /*  Configure the sensors with rotors/sliders with keys */
    static void config_rotor_sliders(void);
    
    #if (QT_NUM_CHANNELS == 4u)
        /*  Configure the sensors for 4 channel Key Rotor/sliders   */
        static void config_4ch_krs(void);
    #endif
    #if (QT_NUM_CHANNELS == 8u)
        /*  Configure the sensors for 8 channel Key Rotor/sliders   */
        static void config_8ch_krs(void);
    #endif
    #if (QT_NUM_CHANNELS == 12u)
        /*  Configure the sensors for 12 channel Key Rotor/sliders   */
        static void config_12ch_krs(void);
    #endif    
    #if (QT_NUM_CHANNELS == 16u)
        /*  Configure the sensors for 16 channel Key Rotor/sliders  */
        static void config_16ch_krs(void);
    #endif
#else
    /*  Configure the sensors for  Keys configuration */
    static void config_keys(void);
#endif /* _ROTOR_SLIDER_ */



/*----------------------------------------------------------------------------
                            Structure Declarations
----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
                                    macros
----------------------------------------------------------------------------*/


#define GET_SENSOR_STATE(SENSOR_NUMBER) qt_measure_data.qt_touch_status.sensor_states[(SENSOR_NUMBER/8)] & (1 << (SENSOR_NUMBER % 8))
#define GET_ROTOR_SLIDER_POSITION(ROTOR_SLIDER_NUMBER) qt_measure_data.qt_touch_status.rotor_slider_values[ROTOR_SLIDER_NUMBER]

/*----------------------------------------------------------------------------
                                global variables
----------------------------------------------------------------------------*/
/* Timer period in msec. */
uint16_t qt_measurement_period_msec = 25u;

/*----------------------------------------------------------------------------
                                extern variables
----------------------------------------------------------------------------*/
/* This configuration data structure parameters if needs to be changed will be
   changed in the qt_set_parameters function */
extern qt_touch_lib_config_data_t qt_config_data;
/* touch output - measurement data */
extern qt_touch_lib_measure_data_t qt_measure_data;
/* Get sensor delta values */
extern int16_t qt_get_sensor_delta( uint8_t sensor);
#ifdef QTOUCH_STUDIO_MASKS
extern TOUCH_DATA_T SNS_array[2][2];
extern TOUCH_DATA_T SNSK_array[2][2];
#endif

/* Output can be observed in the watch window using this pointer */
qt_touch_lib_measure_data_t *pqt_measure_data = &qt_measure_data;

#ifdef _DEBUG_INTERFACE_
extern uint16_t timestamp1_hword;
extern uint16_t timestamp1_lword;    
extern uint16_t timestamp2_hword;
extern uint16_t timestamp2_lword;
extern uint16_t timestamp3_hword;
extern uint16_t timestamp3_lword;
#endif

/*----------------------------------------------------------------------------
                                static variables
----------------------------------------------------------------------------*/

/* flag set by timer ISR when it's time to measure touch */
static volatile uint8_t time_to_measure_touch = 0u;

/* current time, set by timer ISR */
static volatile uint16_t current_time_ms_touch = 0u;

/*============================================================================
Name    :   main
------------------------------------------------------------------------------
Purpose :   main code entry point
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/

int main( void )
{
    /*status flags to indicate the re-burst for library*/
    uint16_t status_flag = 0u;
    uint16_t burst_flag = 0u;
#ifdef QTOUCH_STUDIO_MASKS /* QTouch mask works with 8 buttons and 0 rotor/slider configuration */
   SNS_array[0][0]=0x55;
   SNS_array[0][1]=0xaa;
   SNS_array[1][0]=0x00;
   SNS_array[1][1]=0x00;
   SNSK_array[0][0]=0x55;
   SNSK_array[0][1]=0xaa;
   SNSK_array[1][0]=0x00;
   SNSK_array[1][1]=0x00; 
#endif    
    /* initialise host app, pins, watchdog, etc */
    init_system();

    /* Configure the Sensors as keys or Keys With Rotor/Sliders in this function */
    config_sensors();

    /* initialise touch sensing */
    qt_init_sensing();

    /*  Set the parameters like recalibration threshold, Max_On_Duration etc in this function by the user */
    qt_set_parameters( );

    /* configure timer ISR to fire regularly */
    init_timer_isr();


    /*  Address to pass address of user functions   */
    /*  This function is called after the library has made capacitive measurements,
    *   but before it has processed them. The user can use this hook to apply filter
    *   functions to the measured signal values.(Possibly to fix sensor layout faults)    */
    qt_filter_callback = 0;

#ifdef _DEBUG_INTERFACE_
    /* Initialize debug protocol */
    QDebug_Init();
#endif

    /* enable interrupts */
    __enable_interrupt();

#ifdef _DEBUG_INTERFACE_
    /* Process commands from PC */
    QDebug_ProcessCommands();    
#endif

    /* loop forever */
    for( ; ; )
    {
        if( time_to_measure_touch )
        {

            /*  clear flag: it's time to measure touch  */
            time_to_measure_touch = 0u;

            do {
                /*  one time measure touch sensors    */
                status_flag = qt_measure_sensors( current_time_ms_touch );
                
                burst_flag = status_flag & QTLIB_BURST_AGAIN;

#ifdef _DEBUG_INTERFACE_                
                /* send debug data */ 
                QDebug_SendData(status_flag);
#endif
                
                /* Time-critical host application code goes here */

            }while (burst_flag) ;
            
#ifdef _DEBUG_INTERFACE_            
            /* Process commands from PC */
            QDebug_ProcessCommands();
#endif            
        }

    /*  Time Non-critical host application code goes here  */
    }
}


/*============================================================================
Name    :   init_timer_isr
------------------------------------------------------------------------------
Purpose :   configure timer ISR to fire regularly
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/

static void init_timer_isr( void )
{
    /*  set timer compare value (how often timer ISR will fire) */
    OCR1A = ( TICKS_PER_MS * qt_measurement_period_msec);

    /*  enable timer ISR */
    TIMSK1 |= (1u << OCIE1A);

    /*  timer prescaler = system clock / 8  */
    TCCR1B |= (1u << CS11);

    /*  timer mode = CTC (count up to compare value, then reset)    */
    TCCR1B |= (1u << WGM12);
}

/*============================================================================
Name    :   set_timer_period
------------------------------------------------------------------------------
Purpose :   set the timer period
============================================================================*/
void set_timer_period(uint16_t qt_measurement_period_msec)
{
	OCR1A = ( TICKS_PER_MS * qt_measurement_period_msec);
}

/*============================================================================
Name    :   init_system
------------------------------------------------------------------------------
Purpose :   initialise host app, pins, watchdog, etc
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/

static void init_system( void )
{
    /*  run at 4MHz (assuming internal clock is set to 8MHz)*/
    CLKPR = 0x80u;
    CLKPR = 0x01u;

    /*  disable pull-ups    */
    MCUCR |= (1u << PUD);

}

/*============================================================================
Name    :   timer_isr
------------------------------------------------------------------------------
Purpose :   timer 1 compare ISR
Input   :   n/a
Output  :   n/a
Notes   :
============================================================================*/

ISR(TIMER1_COMPA_vect)
{
    /*  set flag: it's time to measure touch    */
    time_to_measure_touch = 1u;

    /*  update the current time */
    current_time_ms_touch += qt_measurement_period_msec;
}


/*============================================================================
Name    :   qt_set_parameters
------------------------------------------------------------------------------
Purpose :   This will fill the default threshold values in the configuration 
            data structure.But User can change the values of these parameters .
Input   :   n/a
Output  :   n/a
Notes   :   initialize configuration data for processing
============================================================================*/

static void qt_set_parameters( void )
{
    /*  This will be modified by the user to different values   */
    qt_config_data.qt_di              = DEF_QT_DI;
    qt_config_data.qt_neg_drift_rate  = DEF_QT_NEG_DRIFT_RATE;
    qt_config_data.qt_pos_drift_rate  = DEF_QT_POS_DRIFT_RATE;
    qt_config_data.qt_max_on_duration = DEF_QT_MAX_ON_DURATION;
    qt_config_data.qt_drift_hold_time = DEF_QT_DRIFT_HOLD_TIME;
    qt_config_data.qt_recal_threshold = DEF_QT_RECAL_THRESHOLD;
    qt_config_data.qt_pos_recal_delay = DEF_QT_POS_RECAL_DELAY;
}

/*============================================================================
Name    :   config_sensors
------------------------------------------------------------------------------
Purpose :   Configure the sensors
Input   :   n/a
Output  :   n/a
Notes   :   n/a
============================================================================*/
static void config_sensors(void)
{
#if defined(_ROTOR_SLIDER_)
    config_rotor_sliders();
#else /*    !_ROTOR_SLIDER_ OR ONLY KEYS    */
    config_keys();
#endif /*   _ROTOR_SLIDER_  */
}
/*============================================================================
Name    :   config_keys
------------------------------------------------------------------------------
Purpose :   Configure the sensors as keys only
Input   :   n/a
Output  :   n/a
Notes   :   n/a
============================================================================*/
#ifndef _ROTOR_SLIDER_
static void config_keys(void)
{
    /*  enable sensors 0..3: keys on channels 0..3  */
    qt_enable_key( CHANNEL_0, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_1, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_2, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_3, AKS_GROUP_1, 10u, HYST_6_25 );

#if(QT_NUM_CHANNELS >= 8u)
    /*  enable sensors 4..7: keys on channels 4..7   */
    qt_enable_key( CHANNEL_4, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_5, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_6, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_7, AKS_GROUP_1, 10u, HYST_6_25 );
#endif
#if(QT_NUM_CHANNELS >= 12u)
    /*  enable sensors 8..11: keys on channels 4..7   */
    qt_enable_key( CHANNEL_8, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_9, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_10, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_11, AKS_GROUP_1, 10u, HYST_6_25 );
#endif    
#if (QT_NUM_CHANNELS >= 16u)
    /*  enable sensors 12..15: keys on channels 8..15    */
    qt_enable_key( CHANNEL_12, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_13, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_14, AKS_GROUP_1, 10u, HYST_6_25 );
    qt_enable_key( CHANNEL_15, AKS_GROUP_1, 10u, HYST_6_25 );
#endif

}

#else/*    _ROTOR_SLIDER_  */

/*============================================================================
  Name : config_rotor_sliders
------------------------------------------------------------------------------
Purpose: Configure the Sensors as keys and also as Rotors/Sliders
Input  : n/a
Output : n/a
Notes  : n/a
============================================================================*/
static void config_rotor_sliders(void)
{
#if (QT_NUM_CHANNELS == 4u)
    {
        /*  Call this function if library used is 4 channel library with KRS Configuration  */
        config_4ch_krs();
    }
#endif
#if (QT_NUM_CHANNELS == 8u)
    {
        /*  Call this function if library used is 8 channel library with KRS Configuration  */
        config_8ch_krs();
    }
#endif
#if (QT_NUM_CHANNELS == 12u)
    {
        /*  Call this function if library used is 12 channel library with KRS Configuration  */
        config_12ch_krs();
    }
#endif    
#if (QT_NUM_CHANNELS == 16u)
    {
        /*  Call this function if library used is 16 channel library with KRS Configuration */
        config_16ch_krs();
    }
#endif
}

/*============================================================================
Name    :   config_4ch_krs
------------------------------------------------------------------------------
Purpose :   Configure the Sensors as keys and Rotor/Sliders for 4 channels only
Input   :   n/a
Output  :   n/a
Notes   :   n/a
============================================================================*/
#if (QT_NUM_CHANNELS == 4u)
static void config_4ch_krs(void)
{
    /*  enable sensor 0: a key on channel 0 */
    qt_enable_key( CHANNEL_0, AKS_GROUP_2, 10u, HYST_6_25 );

    /*  enable sensor 1: a slider on channels 0..2  */
    qt_enable_slider( CHANNEL_1, CHANNEL_3, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  );

}
#endif /*QT_NUM_CHANNELS == 4u config_4ch_krs */

/*============================================================================
Name    :   config_8ch_krs
------------------------------------------------------------------------------
Purpose :   Configure the Sensors as keys and Rotor/Sliders for 8 channels only
Input   :   n/a
Output  :   n/a
Notes   :   n/a
============================================================================*/
#if (QT_NUM_CHANNELS == 8u)
static void config_8ch_krs(void)
{
    /*  enable sensor 0: a keys on channel 0    */
    qt_enable_key( CHANNEL_0, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 1: a slider on channels 1..3  */
    qt_enable_slider( CHANNEL_1, CHANNEL_3, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  );
    /*  enable sensor 2: a key on channel 4 */
    qt_enable_key( CHANNEL_4, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 3: a rotor on channels 5..7   */
    qt_enable_rotor( CHANNEL_5, CHANNEL_7, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u );

}
#endif /*   QT_NUM_CHANNELS == 8u config_8ch_krs    */

/*============================================================================
Name    :   config_12ch_krs
------------------------------------------------------------------------------
Purpose :   Configure the Sensors as keys and Rotor/Sliders for 12 channels only
Input   :   n/a
Output  :   n/a
Notes   :   n/a
============================================================================*/
#if (QT_NUM_CHANNELS == 12u)
static void config_12ch_krs(void)
{
    /*  enable sensor 0: a keys on channel 0    */
    qt_enable_key( CHANNEL_0, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 1: a slider on channels 1..3  */
    qt_enable_slider( CHANNEL_1, CHANNEL_3, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  );
    /*  enable sensor 2: a key on channel 4 */
    qt_enable_key( CHANNEL_4, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 3: a rotor on channels 5..7   */
    qt_enable_slider( CHANNEL_5, CHANNEL_7, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u );
    /*  enable sensor 4: a keys on channel 8    */
    qt_enable_key( CHANNEL_8, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 5: a slider on channels 9..11  */
    qt_enable_rotor( CHANNEL_9, CHANNEL_11, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  );    

}
#endif 

/*============================================================================
Name    :   config_16ch_krs
------------------------------------------------------------------------------
Purpose :   Configure the Sensors as keys and Rotor/Sliders for 16 channels only
Input   :   n/a
Output  :   n/a
Notes   :   n/a
============================================================================*/
#if (QT_NUM_CHANNELS == 16u)
static void config_16ch_krs(void)
{
    /*  enable sensor 0: a keys on channel 0    */
    qt_enable_key( CHANNEL_0, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 1: a slider on channels 1..3  */
    qt_enable_slider( CHANNEL_1, CHANNEL_3, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  );
    /*  enable sensor 2: a key on channel 4 */
    qt_enable_key( CHANNEL_4, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 3: a rotor on channels 5..7   */
    qt_enable_rotor( CHANNEL_5, CHANNEL_7, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u );
    /*  enable sensor 4: a keys on channel 8    */
    qt_enable_key( CHANNEL_8, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 5: a slider on channels 9..11  */
    qt_enable_slider( CHANNEL_9, CHANNEL_11, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  ); 
    /*  enable sensor 6: a keys on channel 12    */
    qt_enable_key( CHANNEL_12, AKS_GROUP_2, 10u, HYST_6_25 );
    /*  enable sensor 7: a slider on channels 13..15  */
    qt_enable_rotor( CHANNEL_13, CHANNEL_15, NO_AKS_GROUP, 16u, HYST_6_25, RES_8_BIT, 0u  );   

}
#endif  /*  QT_NUM_CHANNELS == 16u config_16ch_krs  */
#endif /* _ROTOR_SLIDER_ */


