/**
 ****************************************************************************************
 *
 * @file app_audio439.h
 *
 * @brief AudioStreamer Application entry point
 *
 * Copyright (C) RivieraWaves 2009-2012
 *
 * $Rev: $
 *
 ****************************************************************************************
 */

#ifndef APP_AUDIO439_H_
#define APP_AUDIO439_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief AudioStreamer Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "rwble_config.h"

#if (HAS_AUDIO)

#include <stdint.h>          // standard integer definition
#include <co_bt.h>
#include "app_audio_codec.h"

#include "datasheet.h"
#include "core_cm0.h"
#include "gpio.h"


#if !defined(CFG_AUDIO439_ADAPTIVE_RATE) && !defined(IMA_DEFAULT_MODE)
#error "IMA_DEFAULT_MODE must be defined"
#endif

/*
 * APP_AUDIO439 Env DataStructure
 ****************************************************************************************
 */
 
/**
 ****************************************************************************************
 * Local buffers for storing Audio Samples
 * Audio is fetched in blocks of 40 samples.
 *
 ****************************************************************************************
 */

#ifdef CFG_SPI_439_BLOCK_BASED
  #define AUDIO439_SKIP_SAMP 2
#else
  #define AUDIO439_SKIP_SAMP 0
#endif

#define AUDIO439_NR_SAMP 40
typedef struct s_audio439 {
    int16_t  samples[AUDIO439_NR_SAMP+AUDIO439_SKIP_SAMP];  // Extra dummy value at start of buffer...
    int16      hasData;
} t_audio439_slot;

#define AUDIO439_NR_SLOT   10
#define AUDIO439_SBUF_SIZE 100

typedef enum {
    IMA_MODE_64KBPS_4_16KHZ = 0,
    IMA_MODE_48KBPS_3_16KHZ = 1,
    IMA_MODE_32KBPS_4_8KHZ  = 2,
    IMA_MODE_24KBPS_3_8KHZ  = 3,
} app_audio439_ima_mode_t;

typedef  struct s_app_audio439_env
{
    t_audio439_slot audioSlots[AUDIO439_NR_SLOT];
    int audio439SlotWrNr;
    int audio439SlotIdx;
    int audio439SlotRdNr;
    int audio439SlotSize;
#ifdef CFG_AUDIO439_IMA_ADPCM
    t_IMAData imaState;
    unsigned int errors_send;
    t_DCBLOCKData dcBlock;
    int spi_errors;
    int buffer_errors;
    /* States for internal working buffer, to deal with changing rates */
    int     sbuf_len;
    int     sbuf_min;
    int     sbuf_avail;
    int16_t sbuffer[AUDIO439_SBUF_SIZE];
#if defined(CFG_AUDIO439_ADAPTIVE_RATE) || (IMA_DEFAULT_MODE)==3 || (IMA_DEFAULT_MODE)==4
    int16_t FilterTaps[FILTER_LENGTH];  
#endif    
#ifdef CFG_AUDIO439_ADAPTIVE_RATE    
    bool sample_mode;
    app_audio439_ima_mode_t ima_mode;
#endif    
#endif // CFG_AUDIO439_IMA_ADPCM
} t_app_audio439_env;

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */
extern t_app_audio439_env app_audio439_env;


/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initiliaze the 439 state variable.
 * This function is called at every start of new Audio Command.
 *
 * @param[in] None
 *
 * @return void.
 ****************************************************************************************
 */
void app_audio439_init(void);

/**
 ****************************************************************************************
 * @brief Set IMA mode
 *
 * @param[in] mode
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_configure_ima_mode(app_audio439_ima_mode_t mode);

#ifdef APP_AUDIO439_DEBUG
/**
 ****************************************************************************************
 * @brief Test Function to sent Audio Samples to Uart
 *
 * This test function will process all incoming Audio Packets (40 Samples) and 
 * encode them with selected coder (IMA, ALAW, LIN).
 * This function does not return, it runs indefinetly. 
 *
 * @param[in] None
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_to_uart(void);
#endif

/**
 ****************************************************************************************
 * @brief Encode the audio 
 *
 * This test function will process all incoming Audio Packets (40 Samples) and 
 * encode them with selected coder (IMA, ALAW, LIN).
 * This function does not return, it runs indefinetly. 
 *
 * @param[in] maxNr
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_encode(int maxNr);

/**
 ****************************************************************************************
 * @brief Start the Audio processing from 439
 *
 * @param[in] None
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_start(void);

/**
 ****************************************************************************************
 * @brief Stop the Audio processing from 439
  *
 * @param[in] None
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_stop(void);

/**
 ****************************************************************************************
 * @brief Configuration for Audio processing from 439
  *
 * @param param
 *
 * @return void
 ****************************************************************************************
 */
void app_audio439_config(uint8_t *param);

extern int app_audio439_timer_started;

__INLINE void declare_audio_mute_gpios(void)
{    
#ifdef HAS_AUDIO_MUTE
    RESERVE_GPIO( AUDIO_MUTE, AUDIO_MUTE_PORT, AUDIO_MUTE_PIN, PID_439_MUTE_LDO);
#endif
}

__INLINE void init_audio_mute_gpios(bool enable_power)
{    
#ifdef HAS_AUDIO_MUTE
    GPIO_SetPinFunction( AUDIO_MUTE_PORT, AUDIO_MUTE_PIN, ((enable_power && (AUDIO_MUTE_POLARITY)==GPIO_ACTIVE_HIGH) || (!enable_power && (AUDIO_MUTE_POLARITY)==GPIO_ACTIVE_LOW))?INPUT_PULLUP:INPUT_PULLDOWN, PID_GPIO);
#endif
}

__INLINE void declare_audio_vddio_control_gpios(void)
{
#ifdef HAS_AUDIO_VDDIO_CONTROL
    RESERVE_GPIO( AUDIO_VDDIO_CONTROL, AUDIO_VDDIO_CONTROL_PORT, AUDIO_VDDIO_CONTROL_PIN, PID_AUDIO_MUTE);
#endif
}    
    
__INLINE void init_audio_vddio_control_gpios(bool enable_power)
{
#ifdef HAS_AUDIO_VDDIO_CONTROL
    GPIO_SetPinFunction( AUDIO_VDDIO_CONTROL_PORT, AUDIO_VDDIO_CONTROL_PIN, ((enable_power && (AUDIO_VDDIO_CONTROL_POLARITY)==GPIO_ACTIVE_HIGH) || (!enable_power && (AUDIO_VDDIO_CONTROL_POLARITY)==GPIO_ACTIVE_LOW))?INPUT_PULLUP:INPUT_PULLDOWN, PID_GPIO);
#endif
}    

    /* RESERVE_GPIO( SC14439_MUTE_LDO,  MUTE_AUDIO_PORT, MUTE_AUDIO_PIN, PID_439_MUTE_LDO);    \
    ** Reserve the GPIO pins for 16 Mhz clock output.
    ** Clock is on P0_5, which also prohibits use of P0_6, P0_7, P1_0
    */


#endif //HAS_AUDIO

__INLINE void declare_audio439_gpios(void)
{                                 
#ifdef HAS_AUDIO
    declare_audio_mute_gpios();
    declare_audio_vddio_control_gpios();
    RESERVE_GPIO( CLK_16MHZOUT,   GPIO_PORT_0,        GPIO_PIN_5,         PID_16MHZ_CLK);
    RESERVE_GPIO( CLK_16MHZOUT,   GPIO_PORT_0,        GPIO_PIN_6,         PID_16MHZ_CLK);
    RESERVE_GPIO( CLK_16MHZOUT,   GPIO_PORT_0,        GPIO_PIN_7,         PID_16MHZ_CLK);
    RESERVE_GPIO( CLK_16MHZOUT,   GPIO_PORT_1,        GPIO_PIN_0,         PID_16MHZ_CLK);
    RESERVE_GPIO( AUDIO_SPI_EN,   AUDIO_SPI_EN_PORT,  AUDIO_SPI_EN_PIN,   PID_AUDIO_SPI_EN);  
    RESERVE_GPIO( AUDIO_SPI_CLK,  AUDIO_SPI_CLK_PORT, AUDIO_SPI_CLK_PIN,  PID_AUDIO_SPI_CLK); 
    RESERVE_GPIO( AUDIO_SPI_DO,   AUDIO_SPI_DO_PORT,  AUDIO_SPI_DO_PIN,   PID_AUDIO_SPI_DO);  
    RESERVE_GPIO( AUDIO_SPI_DI,   AUDIO_SPI_DI_PORT,  AUDIO_SPI_DI_PIN,   PID_AUDIO_SPI_DI);  
#endif
}
  
__INLINE void init_audio439_gpios(bool enable_power)   
{                                       
#ifdef HAS_AUDIO
    GPIO_SetPinFunction( AUDIO_SPI_EN_PORT,  AUDIO_SPI_EN_PIN,  INPUT_PULLDOWN, PID_GPIO);    
    GPIO_SetPinFunction( AUDIO_SPI_CLK_PORT, AUDIO_SPI_CLK_PIN, INPUT_PULLDOWN, PID_GPIO);    
    GPIO_SetPinFunction( AUDIO_SPI_DO_PORT,  AUDIO_SPI_DO_PIN,  INPUT_PULLDOWN, PID_GPIO);    
    GPIO_SetPinFunction( AUDIO_SPI_DI_PORT,  AUDIO_SPI_DI_PIN,  INPUT_PULLDOWN, PID_GPIO);    
    GPIO_SetPinFunction( AUDIO_CLK_PORT,     AUDIO_CLK_PIN,     INPUT_PULLDOWN, PID_GPIO);    
    init_audio_mute_gpios(enable_power);
    init_audio_vddio_control_gpios(enable_power);
#endif
}

/// @} APP

#endif // APP_AUDIO439_H_
