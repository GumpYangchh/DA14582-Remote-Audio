/**
 ****************************************************************************************
 *
 * @file app_stream.h
 *
 * @brief AudioStreamer Application entry point
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

#ifndef APP_STREAM_H_
#define APP_STREAM_H_

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
#include "arch.h"
#include "rwip_config.h"

/************************************************************************
 * Define CFG_APP_STREAM_FIFO_PREDEFINED to use pre-allocated buffers   *
 * for stream FIFO                                                      *
 ************************************************************************/
#if (BLE_APP_STREAM)
    #define CFG_APP_STREAM_FIFO_PREDEFINED  1
    #include "hogpd_task.h"

/*
 * APP_STREAM Env DataStructure
 ****************************************************************************************
 *
 * NOTE that number and size of payload packets is similar to number and size of 
 * notifications in StreamData profile. 
 * STREAMDATAD_PACKET_SIZE = 20, and STREAMDATAD_MAX=10
 */
 
#define APP_STREAM_PACKET_SIZE 20
 
typedef  struct s_app_stream_env
{  
    bool stream_enabled;
    int fifo_size;
} t_app_stream_env;

/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */
extern t_app_stream_env app_stream_env;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initialize AudioStreamer Application
 *
 * Initializes the global stream data struct. 
 * Called from main project. Also called when new connection has been made.
 *
 * @return void
 ****************************************************************************************
 */
void app_stream_init(void);

/**
 ****************************************************************************************
 * @brief Called after STREAM-ON was received.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_start(void);

/**
 ****************************************************************************************
 * @brief Stop the stream.
 *
 * Called when STREAMOFF is received, or button released.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_stop(void);

/**
 ****************************************************************************************
 * @brief Idle handler. 
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_idle_handler(void);

/**
 ****************************************************************************************
 * @brief Enable the Streaming profile.
 * 
 * Called from main project, in app_connection_func.
 * Sends an "enable_req" message to STREAMDATAD profile.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_enable(void);

/**
 ****************************************************************************************
 * @brief Get the value of streamdata Enable value
 * 
 * Called when STREAMOFF is received, or button released.
 *
 * @return      the value of the STREAMDATAD_IDX_ENABLE_VAL.
 ****************************************************************************************
 */
short app_stream_get_enable(void);

/**
 ****************************************************************************************
 * @brief Update the value of the enable in either STREAMDATA or HOGPD profile
 * 
 * Called when STREAMOFF is received, or button released.
 *
 * @param value the value of enable.
 *
 * @return      void
 ****************************************************************************************
 */
void app_stream_send_enable(short value);

/**
 ****************************************************************************************
 * @brief Send a notification directly to L2CC with the data of the enable field.
 *
 * Used for speed.
 *
 * @param data
 *
 * @return void      
 ****************************************************************************************
 */
void app_stream_send_enable_data(char *data);

/**
 ****************************************************************************************
 * @brief Send a notification with the enable flag.
 *
 * @param value: the value of enable.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_send_enable_not(short value);


/**
 ****************************************************************************************
 * @brief Send directly a notfication to L2CC for HID vendore specific report.
 *
 * @param[in]   motiondata: the motion data to send.
 *
 * @return      void.
 ****************************************************************************************
 */
#if (HAS_BMI055)
void app_stream_send_motionreport(void * motiondata);
#endif
                        
/**
 ****************************************************************************************
 * @brief Stream Fifo initialization
 *
 * This function initializes the FIFO. 
 * If CFG_APP_STREAM_FIFO_PREDEFINED is defined, then FIFO uses pre-allocated buffers,
 * which will also be initialized.
 *
 * @return void
 ****************************************************************************************
 */
void app_stream_fifo_init (void);

#ifdef CFG_APP_STREAM_FIFO_PREDEFINED
/**
 ****************************************************************************************
 * @brief Get datapointer of the next packet in Stream Fifo
 *
 * @return The datapointer
 ****************************************************************************************
 */
uint8_t *app_stream_fifo_get_next_dataptr(void);

/**
 ****************************************************************************************
 * @brief Check if next packet in Stream Fifo is used
 *
 * @return The handle number of the packet if it is used, 0 if not
 ****************************************************************************************
 */
uint8 app_stream_fifo_check_next(void);

/**
 ****************************************************************************************
 * @brief Commit a packet in the FIFO
 *
 * Commit packet in stream_fifo. Call this function after you have updated the data
 * in the packet dataptr (get pointer with app_stream_fifo_get_next_pkt). This
 * function will then set the used flag to 1, and increment the Fifo Write index.
 *
 * @return void
 ****************************************************************************************
 */
void app_stream_fifo_commit_pkt(void);
#else // CFG_APP_STREAM_FIFO_PREDEFINED
/**
 ****************************************************************************************
 * @brief Add data to the Stream FIFO
 * @param[in] datapt: pointer to the data to be added
 * @param[in] len: number of data bytes  
 * @param[in] handle: handle  
 * @param[in] p_callback: callback function. 
 *
 * The Callback, if not NULL, will be called after packet is sent, and data in dataptr
 * is not needed anymore. Cann be used to free the dataptr.
 *
 * @return -1 if buffer is full, otherwise the position of the packet in the FIFO
 ****************************************************************************************
 */
int app_stream_fifo_add(void* datapt, uint8 len, int handle, void (*p_callback) (void* , int));

#endif // CFG_APP_STREAM_FIFO_PREDEFINED

/**
 ****************************************************************************************
 * @brief Commit an audio enable packet in the FIFO
 *
 * Commit an enablepacket in stream_fifo. Call this function after you have updated the data
 * in the packet dataptr (get pointer with app_stream_fifo_get_next_pkt). This
 * function will then set the used flag to 1, and increment the Fifo Write index.
 *
 * @return void
 ****************************************************************************************
 */
void app_stream_fifo_commit_enable_pkt(void);


/**
 ****************************************************************************************
 * @brief Send data from the FIFOs to L2CC
 *
 * @return void
 ****************************************************************************************
 */
int stream_queue_more_data(void);

/**
 ****************************************************************************************
 * @brief Send directly a notfication to L2CC for HID vendore specific report.
 *
 * @param[in]   kreq: the key report to send.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_send_keyreport(struct hogpd_report_info *kreq);

#endif //BLE_APP_STREAM

/// @} APP

#endif // APP_STREAM_H_
