 /**
 ****************************************************************************************
 *
 * @file app_audio439.c
 *
 * @brief Code related with the SC14439 control.
 *
 * Copyright (C) 2012. Dialog Semiconductor Ltd, unpublished work. This computer 
 * program includes Confidential, Proprietary Information and is a Trade Secret of 
 * Dialog Semiconductor Ltd.  All use, disclosure, and/or reproduction is prohibited 
 * unless authorized in writing. All Rights Reserved.
 *
 * <bluetooth.support@diasemi.com> and contributors.
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
 
#include "rwip_config.h"               // SW configuration

char stop_when_buffer_empty = 0;

#if (BLE_APP_PRESENT && HAS_AUDIO)
#include "app_audio439.h"
#include "spi_439.h"
#include "app_audio_codec.h"
#include "app_stream.h"
#include "pwm.h"

#define USE_IMA
//#define APP_AUDIO439_DEBUG      //DEBUG FUNCTIONS
#define CLICK_STARTUP_CLEAN     //enable the DC_BLOCK filter and drop of packages

    #ifdef APP_AUDIO439_DEBUG
        #include "uart.h"
    #endif

t_app_audio439_env app_audio439_env;

#ifdef CFG_AUDIO439_ADAPTIVE_RATE
volatile app_audio439_ima_mode_t app_audio439_imamode   __attribute__((section("retention_mem_area0"), zero_init));
volatile int app_audio439_imaauto   __attribute__((section("retention_mem_area0"), zero_init));
int app_audio439_imacnt;
int app_audio439_imatst=0;
#endif

static void app_audio439_set_ima_mode(void);

/**
 ****************************************************************************************
 * @brief Initiliaze the 439 state variable.
 * This function is called at every start of new Audio Command.
 *
 * @param[in] void
 *
 * @return void.
 ****************************************************************************************
 */
void app_audio439_init(void)
{
    int i;
    app_audio439_env.audio439SlotWrNr   = 0;
    app_audio439_env.audio439SlotRdNr   = 0;
    app_audio439_env.audio439SlotIdx    = 0;
     app_audio439_env.audio439SlotSize  = 0;

        for (i=0; i<AUDIO439_NR_SLOT; i++) {
        app_audio439_env.audioSlots[i].hasData = 0;
    }
    app_audio439_env.imaState.index           = 0;
    app_audio439_env.imaState.predictedSample = 0;

    app_audio439_env.sbuf_len                 = 0;
    app_audio439_env.sbuf_min                 = AUDIO439_NR_SAMP;   // Number of samples needed to encode in one 20 byte packet, after possible downsampling
    app_audio439_env.sbuf_avail               = AUDIO439_SBUF_SIZE; // Number of bytes available
#ifdef DC_BLOCK
    app_audio439_env.dcBlock.len              = AUDIO439_NR_SAMP;
    app_audio439_env.dcBlock.beta             = APP_AUDIO_DCB_BETA;
    app_audio439_env.dcBlock.xn1              = 0;
    app_audio439_env.dcBlock.yyn1             = 0;
    app_audio439_env.dcBlock.fade_step        = 16;   // about 1000 samples fade-in   
    app_audio439_env.dcBlock.fcnt             = 25;   // block input for first 25 frames of 40 samples
#endif
    app_audio439_env.buffer_errors            = 0;
    app_audio439_env.spi_errors               = 0;
    app_audio439_env.errors_send              = 100;

    app_audio439_set_ima_mode();  // set IMA adpcm encoding parameters
}

#if defined(UART_JS_DEBUG) || defined(APP_AUDIO439_DEBUG)
/**
 ****************************************************************************************
 * @brief Local, Fast uart debug function
 *
 * @param[in] ch
 *
 * @return void
 ****************************************************************************************
 */
void myuart_send_byte(uint16_t ch)
{
    while((GetWord16(UART_LSR_REG)&0x20)==0);       // read status reg to check if THR is empty
    SetWord16(UART_RBR_THR_DLL_REG,(0xFF&ch)); // write to THR register
}
#endif


#ifdef APP_AUDIO439_DEBUG
/**
 ****************************************************************************************
 * @brief Test Function to sent Audio Samples to Uart
 *
 * This test function will process all incoming Audio Packets (40 Samples) and 
 * encode them with selected coder (IMA, ALAW, LIN).
 * This function does not return, it runs indefinetly. 
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_to_uart(void)
{
    int i;
    char msg[] = "AudioData:\r\n";
    uart_write((uint8_t*)msg,strlen(msg),NULL);
#ifdef CFG_AUDIO439_IMA_ADPCM
    uint8_t  ima_bytes[AUDIO439_NR_SAMP];
    t_IMAData imaState;
    imaState.len     = AUDIO439_NR_SAMP;
    imaState.out     = ima_bytes;
    imaState.index   = 0;
    imaState.predictedSample = 0;
#endif    
    app_audio439_start();
    
    while (true) {
        if (app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].hasData == 1) {
            app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].hasData = 0;
#ifdef USE_IMA
            imaState.inp = &app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].samples[AUDIO439_SKIP_SAMP];

            app_ima_enc(&imaState);
            for (i=0;i<AUDIO439_NR_SAMP/2;i++) {
                myuart_send_byte(ima_bytes[i]);
            }
#else
            for (i=0;i<AUDIO439_NR_SAMP;i++) {
                /* Send out all the byte one by one.. */
                int16_t s = app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].samples[i+AUDIO439_SKIP_SAMP];  // SKIP two samples !!
                uint8_t alaw = audio439_aLaw_encode(s);  
                myuart_send_byte(alaw);
            }
#endif
            app_audio439_env.audio439SlotRdNr++;
            if (app_audio439_env.audio439SlotRdNr == AUDIO439_NR_SLOT) {
                app_audio439_env.audio439SlotRdNr = 0;
            }
        }
    }
}
#endif

#ifdef CLICK_STARTUP_CLEAN
int click_packages;
#define DROP_PACKAGES_NO 25             //how many packages to drop
#define DC_BLOCK_PACKAGES_START 10      //when to start DC_BLOCK calculation
#define DC_BLOCK_PACKAGES_STOP 150      //when to stop DC_BLOCK calculation
#endif

#ifdef CFG_APP_STREAM_FIFO_PREDEFINED
/**
 ****************************************************************************************
 * @brief Drop the next package from the FIFO.
 *
 * @param  None
 *
 * @return void
 ****************************************************************************************
 */
static void app_audio439_next_package(void)
{
    app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].hasData = 0;
    app_audio439_env.audio439SlotRdNr++;
    if (app_audio439_env.audio439SlotRdNr >= AUDIO439_NR_SLOT) {
        app_audio439_env.audio439SlotRdNr = 0;
    }
    app_audio439_env.audio439SlotSize--;
}

/**
 ****************************************************************************************
 * @brief Fill work buffer with new packet from SPI439, with optional downsampling
 * This function will add a 439 packet to the work buffer. If downsampling is selective
 * then the a 2x downsampling is performed using FIR filter.
 *
 * @param[in] ptr: pointer to the data in 439-packet.
 *
 * @return void
 ****************************************************************************************
 */
static void app_audio439_fill_buffer(int16_t *ptr)
{
    int i;
    int16_t *dst = &app_audio439_env.sbuffer[app_audio439_env.sbuf_len];
    int tot = AUDIO439_NR_SAMP;

#ifdef CFG_AUDIO439_ADAPTIVE_RATE    
    if (app_audio439_env.sample_mode == 0) {
        for (i=0;i<AUDIO439_NR_SAMP;i++) {
            *dst++ = *ptr++;
        }
    } else {
        audio439_downSample(AUDIO439_NR_SAMP,ptr,dst, app_audio439_env.FilterTaps);
        tot = AUDIO439_NR_SAMP/2;
    }
#elif (IMA_DEFAULT_MODE)!=3 && (IMA_DEFAULT_MODE)!=4
    for (i=0;i<AUDIO439_NR_SAMP;i++) {
        *dst++ = *ptr++;
    }
#else
    audio439_downSample(AUDIO439_NR_SAMP,ptr,dst, app_audio439_env.FilterTaps);
    tot = AUDIO439_NR_SAMP/2;
#endif        

    app_audio439_env.sbuf_len += tot;
    app_audio439_env.sbuf_avail -= tot;
}

/**
 ****************************************************************************************
 * @brief Remove len samples from the work buffer. 
 * The len samples to be removed are always at the start of the buffer. Shift the complete
 * buffer, and update the sbuf_len and sbuf_avail states.
 *
 * @param[in] len: number of samples to remove
 *
 * @return void
 ****************************************************************************************
 */
static void app_audio439_empty_buffer(int len)
{
    int i;

    app_audio439_env.sbuf_len -= len;
    app_audio439_env.sbuf_avail += len;

    int16_t *src = app_audio439_env.sbuffer+len;
    int16_t *dst = app_audio439_env.sbuffer;
    /* Move the buffer.. */
        for (i=0; i<app_audio439_env.sbuf_len; i++) {
        *dst++ = *src++;
    }   
}

/**
 ****************************************************************************************
 * @brief Encode the audio, read one packet from buffer, encode and store in stream buffer
 * This function is the interface between the raw sample packets from the 439 (stored in
 * spi439 fifo in groups of 40 samples) and streaming packets (with compressed audio) which
 * are currently 20 bytes (but this could change).
 * The function uses an internal working buffer (app_audio439_env.sbuffer) for this purpose.
 * - if sbuffer has enough space, add spi439 sample block.
 * - if sbuffer has enough samples, compress a block of samples to make one HID stream packet.
 *   The amount of samples needed for one HID packet depends on the IMA compression mode
 *   selected. Default 4 bits/sample = 40 samples makes 20 bytes. For 3 bits/sample, we 
 *   need 50 samples (5 IMA codes will occupy 2 bytes).
 * 
 * @param[in] maxNr maximum number of packets to encode
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_encode(int maxNr)
{
    int i;
    for (i=0; i<maxNr; i++) {
        /* First check if there is enough space in our Sbuffer
         *  to put in one 439 sample block (40 samples) */
        if ((app_audio439_env.sbuf_avail >= AUDIO439_NR_SAMP) &&
            (app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].hasData == 1)) {
#ifdef DC_BLOCK
        /* 
        ** Additional DC Blocking Filter 
        ** It will do the whole block and stores output inplace (same array as input).
        */
#ifdef CLICK_STARTUP_CLEAN
            if ((click_packages < DC_BLOCK_PACKAGES_STOP) &&
                (click_packages > DC_BLOCK_PACKAGES_START))
#endif
            {
                app_audio439_env.dcBlock.inp = &app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].samples[AUDIO439_SKIP_SAMP];
                app_audio439_env.dcBlock.out = app_audio439_env.dcBlock.inp;  /* IN-PLACE operation !! */
                app_audio_dcblock(&app_audio439_env.dcBlock);
            }
#endif   
#ifdef CLICK_STARTUP_CLEAN            
            if (click_packages < DROP_PACKAGES_NO) {
                //if DROP_PACKAGES_NO>DC_BLOCK_PACKAGES_START DC-blocking updated but not used
                click_packages++;
                app_audio439_next_package();
                continue;
            } else if (click_packages < DC_BLOCK_PACKAGES_STOP) {
                click_packages++;
            }      
#endif
            /* Now add the sample block to our SBuffer */
            app_audio439_fill_buffer(&app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].samples[AUDIO439_SKIP_SAMP]);
            app_audio439_next_package();
#ifdef CFG_AUDIO439_ADAPTIVE_RATE 
            app_audio439_imacnt++;
#endif
        }
    
    /*
            ** Check if we have enough samples in the Sbuffer.
            ** sbuf_min is number of samples needed for encoding 20 output bytes (one ble packet).
    ** For IMA-ADPCM @ 8/16 Khz, this value is 40. For IMA-ADPCM 3 bits, this is 52.
    */
        if ( app_audio439_env.sbuf_len >= app_audio439_env.sbuf_min) {
#ifdef CFG_AUDIO439_IMA_ADPCM
        /*
        ** IMA Adpcm, compress 40 samples into 40 bytes
        */
            if (app_stream_fifo_check_next()) {
                /*
                ** The app_stream_fifo buffer is full.
                                ** THis means that not all packets can be sent in time,
                                ** e.g. due to lack of suffficient bandwidth.
                ** Start skipping samples.
                */
                app_audio439_env.buffer_errors++;
                app_audio439_empty_buffer(app_audio439_env.sbuf_len);
                continue;
            }
            app_audio439_env.imaState.inp = app_audio439_env.sbuffer; //  &app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].samples[AUDIO439_SKIP_SAMP];
            app_audio439_env.imaState.out = (uint8_t*)app_stream_fifo_get_next_dataptr();
            app_ima_enc(&app_audio439_env.imaState);
#else
            /*
            ** For ALAW, we encode 40 samples into 2 packets of 20 bytes.
            */
            int16_t *src = app_audio439_env.sbuffer; //  &app_audio439_env.audioSlots[app_audio439_env.audio439SlotRdNr].samples[AUDIO439_SKIP_SAMP];
            uint8_t *dst = app_stream_fifo_get_next_dataptr();
            for (i=0;i<AUDIO439_NR_SAMP/2;i++) {
                *dst++ = audio439_aLaw_encode(*src++);
            }
            app_stream_fifo_commit_pkt();
            for (i=0;i<AUDIO439_NR_SAMP/2;i++) {
                *dst++ = audio439_aLaw_encode(*src++);
            }
#endif
            app_stream_fifo_commit_pkt();
            app_audio439_empty_buffer(app_audio439_env.sbuf_min);
        }
        else {
            // The intermediate buffer is empty, go to next iteration so it can be filled up..
        }
    }
    /*
    ** If buffer errors have been detected, send out "enable" notification with error values..
    */
    app_audio439_env.errors_send++;
    if ((app_audio439_env.errors_send > 100) &&
        (app_audio439_env.buffer_errors || app_audio439_env.spi_errors)) {
        char data[APP_STREAM_PACKET_SIZE];
        memset(data,0,APP_STREAM_PACKET_SIZE);
        data[1] = 3;   // TYPE of message = warning message.
        data[4] = 3; 
        data[5] = (char)app_audio439_env.buffer_errors;
        data[6] = (char)app_audio439_env.spi_errors;
        data[7] = (char)app_audio439_env.audio439SlotRdNr;
        data[8] = (char)app_audio439_env.audio439SlotWrNr;
        data[9] = (char)(uint8)app_stream_env.fifo_size;
        app_audio439_env.buffer_errors = 0;
        app_audio439_env.spi_errors = 0;
        app_audio439_env.errors_send = 0;
        app_stream_send_enable_data(data);
    }
#ifdef CFG_AUDIO439_ADAPTIVE_RATE 
    /* Change the IMA rate, every xx seconds. 400= 1 second. */   
    if ((app_audio439_imacnt > 400) && (app_audio439_imaauto == 1) && (app_stream_fifo_check_next() == 0) ) {
        //app_audio439_imamode = (app_audio439_imamode+1) & 0x03;  // Iterate from 0,1,2,3,0,1,2,3,..
        app_audio439_imatst = (app_audio439_imatst+1) & 0x03;  // Iterate from 0,1,2,3,0,1,2,3,..
        char *data = (char*)app_stream_fifo_get_next_dataptr();
        memset(data,0,APP_STREAM_PACKET_SIZE);
        data[1] = 4;   // TYPE of message = RATE
        data[2] = (uint8)app_audio439_imatst;
        data[4] = 4;
        data[5] = (uint8)app_audio439_imatst;
        data[6] = (uint8)app_audio439_env.audio439SlotSize;
        data[7] = (uint8)app_stream_env.fifo_size;
        app_stream_fifo_commit_enable_pkt();
        app_audio439_imamode = (app_audio439_ima_mode_t)app_audio439_imatst;
        app_audio439_set_ima_mode();
    }
#endif
#endif
}


void SWTIM_Callback (void);
/**
 ****************************************************************************************
 * @brief Local function for stoping the Timer
 *
 * @param  None
 *
 * @return void
 ****************************************************************************************
 */
static inline void swtim_stop(void)
{
    timer0_stop();
}

/**
 ****************************************************************************************
 * @brief Local function for configuring the Timer
 *
 * @param[in] ticks
 * @param[in] mode
 *
 * @return void
 ****************************************************************************************
 */
static inline void swtim_configure(const int ticks, const int mode)
{
     //Enables TIMER0,TIMER2 clock
    set_tmr_enable(CLK_PER_REG_TMR_ENABLED);
    //Sets TIMER0,TIMER2 clock division factor to 8
    set_tmr_div(CLK_PER_REG_TMR_DIV_1);
    
    timer0_disable_irq();
    // initilalize PWM with the desired settings
    timer0_init(TIM0_CLK_FAST, PWM_MODE_ONE, TIM0_CLK_NO_DIV);
    // set pwm Timer0 On, Timer0 'high' and Timer0 'low' reload values
    timer0_set(1000, (ticks/2)-1, (ticks-ticks/2)-1);
    // register callback function for SWTIM_IRQn irq
    timer0_register_callback(SWTIM_Callback);
}

/**
 ****************************************************************************************
 * @brief Local function for starting the Timer
 *
 * @param  None
 *
 * @return void
 ****************************************************************************************
 */
static inline void swtim_start(void)
{
    // start pwm0
    timer0_start();
    while (!NVIC_GetPendingIRQ(SWTIM_IRQn));
    NVIC_ClearPendingIRQ(SWTIM_IRQn);
    // enable SWTIM_IRQn irq
    timer0_enable_irq();
}


#ifdef CFG_SPI_439_BLOCK_BASED
    #define AUDIO439_SYSTICK_TIME 40000   // 40000 is 16 Khz/40 samples, NOTE, you must set systick to N-1 to get it exactly every N cycles
#else
    #define AUDIO439_SYSTICK_TIME 999    // 1000 is 16 Khz/1 samples. NOTE, you must set systick to N-1 to get it exactly every N cycles
#endif
/*
** Function to get the samples from the 439 over SPI, should run at 16 Khz/32 (block based).
*/
/**
 ****************************************************************************************
 * @brief Systick Handler, Interrupt handler for starting Audio Fetching
 * 
 * This function will either run at 16 Khz for Sample Based processing, 
 * or at 16Khs/40, to read block of data (40) samples from 439.
 * 
 *
 * @return void
 ****************************************************************************************
 */
uint8_t session_swtim_ints=0;
//count the first two ticks of the timer.

void SWTIM_Callback(void)
{
#ifdef CFG_SPI_439_SAMPLE_BASED
    app_audio439_env.audio439SlotIdx++;
        if (app_audio439_env.audio439SlotIdx == AUDIO439_NR_SAMP) {
        app_audio439_env.audioSlots[app_audio439_env.audio439SlotWrNr].hasData = 1;
        app_audio439_env.audio439SlotIdx = 0;
        app_audio439_env.audio439SlotWrNr++;
        if (app_audio439_env.audio439SlotWrNr == AUDIO439_NR_SLOT) {
                app_audio439_env.audio439SlotWrNr = 0;
        }
    }
    spi_439_buf_ptr = (int16_t*)&app_audio439_env.audioSlots[app_audio439_env.audio439SlotWrNr].samples[app_audio439_env.audio439SlotIdx];
    spi_439_get_codec_sample();
#endif
    
#ifdef CFG_SPI_439_BLOCK_BASED
    if (session_swtim_ints > 1) {
        //this if statement drops two packages 
        //the first when the session starts is empty and never filled from 439
        //the second package has unkwown data (0-10 samples caused by spi_439_codec_restart and "click" data )
        //the second package is also required to re-allign the DMA read operation of 439 and the timer 
        app_audio439_env.spi_errors += app_audio439_env.audioSlots[app_audio439_env.audio439SlotWrNr].hasData;  // To monitor possible buffer Overflows...
        app_audio439_env.audioSlots[app_audio439_env.audio439SlotWrNr].hasData = 1;
        app_audio439_env.audio439SlotWrNr++;
        if (app_audio439_env.audio439SlotWrNr == AUDIO439_NR_SLOT) {
            app_audio439_env.audio439SlotWrNr = 0;
        }
    }
    spi_439_buf_ptr = (int16_t*)&app_audio439_env.audioSlots[app_audio439_env.audio439SlotWrNr].samples;
    spi_439_getblock(app_audio439_env.audio439SlotWrNr);
    app_audio439_env.audio439SlotSize++;
    
#endif
    if (session_swtim_ints < 2) {
        session_swtim_ints++;   //increase this way to avoid overflow.
    }
}
    


int app_audio439_timer_started = 0;
/**
 ****************************************************************************************
 * @brief Start the Audio processing from 439
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_start(void)
{
    // myuart_send_byte('S');

    stop_when_buffer_empty=0;
    if (app_audio439_timer_started == 1) {
        return ;
    }
    if (app_get_sleep_mode()) {
        app_force_active_mode();
    }
      
    spi_439_init();         //initialize 439
  
    app_audio439_init();    //clear the state data before setting the timer0
    
    app_stream_fifo_init();

    swtim_configure(AUDIO439_SYSTICK_TIME,6);   //initialize the timer
    spi_439_codec_restart();    //SET THE DMA TO A GIVEN OFFSET -- May introduce artifacts on the first packet
    swtim_start();              //start the timer
    session_swtim_ints =0;
    app_audio439_timer_started = 1;
#ifdef CLICK_STARTUP_CLEAN
    click_packages=0;   //changing logic
#endif
    /* Start the streaming... */
}

/**
 ****************************************************************************************
 * @brief Stop the Audio processing from 439
  *
 * @return void
 ****************************************************************************************
 */
void app_audio439_stop(void)
{
    swtim_stop();
    // myuart_send_byte('x');
        
    if (app_audio439_timer_started) {
        app_restore_sleep_mode();
    }

    app_audio439_timer_started = 0;

    spi_439_release();
}

/**
 ****************************************************************************************
 * @brief Configuration for Audio processing from 439
  *
 * @return void
 ****************************************************************************************
 */
#pragma O0
void app_audio439_config(uint8_t *param)
{
#ifdef CFG_AUDIO439_ADAPTIVE_RATE        
    // myuart_send_byte('C');
    int tp = (int)param[1];
    
    // myuart_send_byte(param[0]+'0');
    // myuart_send_byte(param[1]+'0');
    // myuart_send_byte(param[2]+'0');

    if (tp == 6) {
        app_audio439_imamode = (app_audio439_ima_mode_t)0;
        app_audio439_imaauto = 1;
    } else if (tp > 1) {
        // myuart_send_byte('2');
        app_audio439_imamode = (app_audio439_ima_mode_t)(tp - 2);
        app_audio439_imaauto = 0;
    }
#endif    
}
#pragma O3

void app_audio439_configure_ima_mode(app_audio439_ima_mode_t mode)
{
#ifdef CFG_AUDIO439_ADAPTIVE_RATE    
    app_audio439_imamode = mode;
#endif    
}

/**
 ****************************************************************************************
 * @brief Set the correct IMA encoding parameters 
 * The ima-mode is stored in global retention value app_audio439_ima_mode;
 * The Data Rate will be:
 * 0: 64 Kbit/s = ima 4Bps, 16 Khz.
 * 1: 48 Kbit/s = ima 3Bps, 16 Khz.
 * 2: 32 Kbit/s = ima 4Bps, 8 Khz (downsample).
 * 3: 24 Kbit/s = ima 3Bps, 8 Khz (downsample).
 * @return void
 ****************************************************************************************
 */
#pragma O0
void app_audio439_set_ima_mode(void)
{
    int AUDIO_IMA_SIZE;    
    
#ifdef CFG_AUDIO439_ADAPTIVE_RATE    
    switch (app_audio439_imamode) {
        case IMA_MODE_24KBPS_3_8KHZ:
        case IMA_MODE_32KBPS_4_8KHZ:
            app_audio439_env.sample_mode = 1;
            break;
        case IMA_MODE_48KBPS_3_16KHZ:
        case IMA_MODE_64KBPS_4_16KHZ:
            app_audio439_env.sample_mode = 0;
            break;
        default:
            break;
    }
    app_audio439_env.ima_mode  = app_audio439_imamode;
    app_audio439_imacnt = 0;
#endif

#ifdef CFG_AUDIO439_ADAPTIVE_RATE    
    switch (app_audio439_imamode) {
#else        
    switch (IMA_DEFAULT_MODE) {
#endif
    case IMA_MODE_24KBPS_3_8KHZ:
    case IMA_MODE_48KBPS_3_16KHZ:
        AUDIO_IMA_SIZE=3;
        break;
    case IMA_MODE_64KBPS_4_16KHZ:
    case IMA_MODE_32KBPS_4_8KHZ:
    default:
        AUDIO_IMA_SIZE=4;
        break;
    }


    app_audio439_env.imaState.imaSize = AUDIO_IMA_SIZE;
    app_audio439_env.imaState.imaAnd  = 0xF- ((1 << (4-AUDIO_IMA_SIZE)) -1);
    app_audio439_env.imaState.imaOr   = (1 << (4-AUDIO_IMA_SIZE)) -1;
    app_audio439_env.sbuf_min         = 160/AUDIO_IMA_SIZE;   
    app_audio439_env.imaState.len     = 160/AUDIO_IMA_SIZE; // 20*8/AUDIO_IMA_SIZE - Number of samples needed to encode in one 20 byte packet, after possible downsampling
  
    app_audio439_env.imaState.index           = 0;
    app_audio439_env.imaState.predictedSample = 0;  
}

#endif // (BLE_APP_PRESENT)
 
/// @} APP
