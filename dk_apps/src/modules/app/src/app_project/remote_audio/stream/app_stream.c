 /**
 ****************************************************************************************
 *
 * @file app_stream.c
 *
 * @brief Stream application.
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
#if (BLE_APP_PRESENT) && (BLE_APP_STREAM)
#include "app_task.h"                // application task definitions
#include "l2cc_task.h"
#include "l2cm.h"
#include "app_stream.h"


/*
 * GLOBAL VARIABLE DECLARATION
 ****************************************************************************************
 */

t_app_stream_env app_stream_env;

int min_vendor_hndl __attribute__((section("retention_mem_area0"), zero_init));
int max_vendor_hndl __attribute__((section("retention_mem_area0"), zero_init));

const int min_vendor_repnr = 6;
const int max_vendor_repnr = 8;
#define STREAM_ENABLE_REPNR 5

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */



void app_stream_init(void)
{
        app_stream_env.stream_enabled = false;
    app_stream_fifo_init();
}


/**
 ****************************************************************************************
 * Streamer Application Functions
 ****************************************************************************************
 */


/**
 ****************************************************************************************
 * @brief Enable the Streaming profile.
 * 
 * Called from main project, in app_connection_func.
 * Sends an "enable_req" message to STREAMDATAD profile.
 *
 * @param[in]   void.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_enable(void)
{
    app_stream_env.stream_enabled = false;
}

/**
 ****************************************************************************************
 * @brief Called after STREAM-ON was received.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_start(void)
{
    if (app_stream_env.stream_enabled == 0) {
        // Re-initialize the streamer 
        app_stream_init();
    }
    app_stream_env.stream_enabled = 1;
}

void app_stream_stop(void)
{
    app_stream_env.stream_enabled = false;
    app_stream_fifo_init();             //drop all packages when you stop.
}

short app_stream_get_enable(void)
{
	return app_stream_env.stream_enabled;
}

void app_stream_send_enable(short value)
{
    uint16_t* existing_value;
    uint16_t len;
    uint16 handle = hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + 3] + 1;
    attmdb_att_get_value(handle, &len, (uint8_t**) &existing_value);        
    if (*existing_value != value) {
        // Update the value in the attribute database
        attmdb_att_set_value(handle, sizeof(uint16_t), (uint8_t*) &value);        
        streamdatad_streamonoff_hogpd();
    }
}

void app_stream_send_enable_data ( char *data)
{
    struct l2cc_pdu_send_req *pkt = KE_MSG_ALLOC_DYN(L2CC_PDU_SEND_REQ,
                                                         KE_BUILD_ID(TASK_L2CC, app_env.conidx),
                                                         TASK_APP, l2cc_pdu_send_req,
                                                     APP_STREAM_PACKET_SIZE);
    if (!pkt) {
        return;
    }
	// Set attribute channel ID
    pkt->pdu.chan_id   = L2C_CID_ATTRIBUTE;
    // Set packet opcode.
    pkt->pdu.data.code = L2C_CODE_ATT_HDL_VAL_NTF;

    pkt->pdu.data.hdl_val_ntf.handle    = (hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + 4] + 1); 
    pkt->pdu.data.hdl_val_ntf.value_len = APP_STREAM_PACKET_SIZE;
    /* copy the content to value */
    memcpy(&(pkt->pdu.data.hdl_val_ntf.value[0]), data, APP_STREAM_PACKET_SIZE);
    ke_msg_send(pkt);
}

/**
 ****************************************************************************************
 * @brief Send a notification with the enable data.
 *
 * @param[in]   val: the value of enable.
 *
 * @return      void.
 ****************************************************************************************
 */
void send_enable ( char val )
{
    char data[APP_STREAM_PACKET_SIZE];
    memset (data,0,APP_STREAM_PACKET_SIZE);
    data[0]=val;
    app_stream_send_enable_data(data);
}

/**
 ****************************************************************************************
 * @brief Send a notification with the enable flag.
 *
 * @param[in]   value: the value of enable.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_send_enable_not(short value)
{
    uint16_t* existing_value;
    uint16_t len;
    uint16 handle = hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + 3] + 1;
    attmdb_att_get_value(handle, &len, (uint8_t**) &existing_value); 
    
    if (*existing_value != value) {
        // Update the value in the attribute database
        // the value is not set, because we are waiting for a response from the central
        // this way is a "simple" way to synch the state of RCU and central device
        // Send notification
        send_enable(value);
    }
}

#define STREAM_HOGPD_ENABLE_REPORT_NR 5

__INLINE uint16_t hogpd_report_handle(uint8 report_nb)
{
    ASSERT_WARNING(report_nb<HOGPD_NB_REPORT_INST_MAX);
    
    return hogpd_env.shdl[0] + hogpd_env.att_tbl[0][HOGPD_REPORT_CHAR + report_nb-1] + 1;    
}


/**
 ****************************************************************************************
 * @brief Send directly a notfication to L2CC for HID vendore specific report.
 *
 * @param[in]   kreq: the key report to send.
 *
 * @return      void.
 ****************************************************************************************
 */
void app_stream_send_keyreport(struct hogpd_report_info *kreq)
{
    struct l2cc_pdu_send_req *pkt = KE_MSG_ALLOC_DYN(L2CC_PDU_SEND_REQ,
                                                     KE_BUILD_ID(TASK_L2CC, app_env.conidx),
                                                     TASK_APP, l2cc_pdu_send_req,
                                                     APP_STREAM_PACKET_SIZE);
    pkt->pdu.chan_id   = L2C_CID_ATTRIBUTE;
    // Set packet opcode.
    pkt->pdu.data.code = L2C_CODE_ATT_HDL_VAL_NTF;
    pkt->pdu.data.hdl_val_ntf.handle = hogpd_report_handle(STREAM_HOGPD_ENABLE_REPORT_NR);
    pkt->pdu.data.hdl_val_ntf.value_len = APP_STREAM_PACKET_SIZE;
    /* copy the content to value */
    memset (pkt->pdu.data.hdl_val_ntf.value,0,APP_STREAM_PACKET_SIZE);
    pkt->pdu.data.hdl_val_ntf.value[1] = 2;  // TYPE of message == Key Report
    pkt->pdu.data.hdl_val_ntf.value[2] = kreq->report_length;
    memcpy(&(pkt->pdu.data.hdl_val_ntf.value[3]), kreq->report, kreq->report_length);
    ke_msg_send(pkt);
}

#define MAX_TX_BUFS (18)

#define MAX_BUFS_PCON_INT (7)
#define MIN_AVAILABLE (2)
#define MAX_PACKETS_COMP_EVT (2)

#if (HAS_BMI055)
#define STREAM_HOGPD_MOTION_REPORT_NR 78
void app_stream_send_motionreport(void * motiondata)
{
    int available, already_in;
    available=l2cm_get_nb_buffer_available();
    already_in=MAX_TX_BUFS-available;
    if (already_in < 3) {
        struct l2cc_pdu_send_req *pkt = KE_MSG_ALLOC_DYN(L2CC_PDU_SEND_REQ,
                                                         KE_BUILD_ID(TASK_L2CC, app_env.conidx),
                                                         TASK_APP, l2cc_pdu_send_req,
                                                         APP_STREAM_PACKET_SIZE);
    pkt->pdu.chan_id   = L2C_CID_ATTRIBUTE;
    // Set packet opcode.
    pkt->pdu.data.code = L2C_CODE_ATT_HDL_VAL_NTF;
    pkt->pdu.data.hdl_val_ntf.handle = STREAM_HOGPD_MOTION_REPORT_NR;
    pkt->pdu.data.hdl_val_ntf.value_len = APP_STREAM_PACKET_SIZE;
    /* copy the content to value */
    memcpy (pkt->pdu.data.hdl_val_ntf.value,motiondata,APP_STREAM_PACKET_SIZE);

    ke_msg_send(pkt);
    }
}
#endif // HAS_BMI055

/*
 * APP_STREAM FIFO DataStructure
 ****************************************************************************************
 *
 * 
 */

#define MEMORY_OPTIMIZATION1
#define MEMORY_OPTIMIZATION2

typedef struct s_app_stream_pkt {
#ifndef MEMORY_OPTIMIZATION1    
    void   *datapt;
#endif
        
#ifndef CFG_APP_STREAM_FIFO_PREDEFINED
    void   (*p_callback) (void* , int);
    uint8  len;
#endif
  uint8  used_hndl;   // if 0, stream packet not used, else it contains the handle/reportnr
} t_app_stream_pkt;

#if MAX_BUFFER_CONF
#define MAX_FIFO_LEN (350)
#define MULTIPLE_ARRAYS
#ifdef MULTIPLE_ARRAYS
#define MAX_FIFO_LEN0 160
#define MAX_FIFO_LEN1 120
#define MAX_FIFO_LEN2 70
#if (MAX_FIFO_LEN!=(MAX_FIFO_LEN0+MAX_FIFO_LEN1+MAX_FIFO_LEN2))
#error "FIX THE FIFO SIZES."
#endif
#endif
#endif

#define MAX_FIFO_LEN (60)
#define MULTIPLE_ARRAYS
#ifdef MULTIPLE_ARRAYS
#define MAX_FIFO_LEN0 20
#define MAX_FIFO_LEN1 20
#define MAX_FIFO_LEN2 20
#if (MAX_FIFO_LEN!=(MAX_FIFO_LEN0+MAX_FIFO_LEN1+MAX_FIFO_LEN2))
#error "FIX THE FIFO SIZES."
#endif
#endif

#ifdef MULTIPLE_ARRAYS
uint8 pkts0 [MAX_FIFO_LEN0][APP_STREAM_PACKET_SIZE] __attribute__((section("pkts0_area"),zero_init));
uint8 pkts1 [MAX_FIFO_LEN1][APP_STREAM_PACKET_SIZE] __attribute__((section("pkts1_area"),zero_init)); 
uint8 pkts2 [MAX_FIFO_LEN2][APP_STREAM_PACKET_SIZE] __attribute__((section("pkts2_area"),zero_init)); 
#endif

typedef struct s_app_stream_fifo {
    t_app_stream_pkt   pkt_fifo[MAX_FIFO_LEN];
        
#if (MAX_FIFO_LEN<256)
    uint8  fifo_read;
    uint8  fifo_write;
#else
    uint16 fifo_read;
    uint16 fifo_write;
#endif
    
#ifdef MEMORY_OPTIMIZATION2 
  int16    rd_handle;
#endif    
    
#ifdef CFG_APP_STREAM_FIFO_PREDEFINED
    /* Pre-allocated Packets */
#ifndef MULTIPLE_ARRAYS
    uint8  pkts[MAX_FIFO_LEN][APP_STREAM_PACKET_SIZE];
#endif
    int16  hnd;
    int    overflow_errors;
#endif
} t_app_stream_fifo;

#ifdef MEMORY_OPTIMIZATION1 
uint8* datapt (int16 handle)
{
#ifndef MULTIPLE_ARRAYS
    return (app_stream_fifo.pkts[handle]);
#else
    if (handle < MAX_FIFO_LEN0) {
        return (pkts0[handle]);
    } else if (handle < (MAX_FIFO_LEN0 + MAX_FIFO_LEN1)) {
        return (pkts1[handle-MAX_FIFO_LEN0]);
    } else {
        return (pkts2[handle - (MAX_FIFO_LEN0 + MAX_FIFO_LEN1)]);
    }
#endif    
}
#endif

/**
 * Global Variable that holds the Steam Application FIFO data
 */
t_app_stream_fifo app_stream_fifo;

void app_stream_fifo_init (void)
{
    memset (app_stream_fifo.pkt_fifo, 0, sizeof(t_app_stream_pkt)*MAX_FIFO_LEN);
    app_stream_fifo.fifo_read = 0;
    app_stream_fifo.fifo_write = 0;
    app_stream_env.fifo_size = 0;   
    app_stream_fifo.hnd = min_vendor_repnr;
    
#ifndef MULTIPLE_ARRAYS
    #ifdef CFG_APP_STREAM_FIFO_PREDEFINED
        memset(app_stream_fifo.pkts,0,sizeof(app_stream_fifo.pkts));
        #ifndef MEMORY_OPTIMIZATION1 
        for (int i=0; i<MAX_FIFO_LEN; i++) {
            app_stream_fifo.pkt_fifo[i].datapt = app_stream_fifo.pkts[i];
            app_stream_fifo.pkt_fifo[i].used_hndl = 0;
        }
        #endif
    #endif
#elif defined(CFG_APP_STREAM_FIFO_PREDEFINED)
    memset(pkts0,0,sizeof(pkts0));
    memset(pkts1,0,sizeof(pkts1));
    memset(pkts2,0,sizeof(pkts2));

    #ifndef MEMORY_OPTIMIZATION1 
    for (int i=0; i<MAX_FIFO_LEN0;i++) {
        app_stream_fifo.pkt_fifo[i].datapt = pkts0[i];
    }
    for (int i=0; i<MAX_FIFO_LEN1;i++) {
        app_stream_fifo.pkt_fifo[MAX_FIFO_LEN0+i].datapt = pkts1[i];
    }
    for (int i=0; i<MAX_FIFO_LEN2;i++) {
        app_stream_fifo.pkt_fifo[(MAX_FIFO_LEN0+MAX_FIFO_LEN1)+i].datapt = pkts2[i];
    }
    #endif // MEMORY_OPTIMIZATION1
#endif
}

#ifdef CFG_APP_STREAM_FIFO_PREDEFINED
uint8_t *app_stream_fifo_get_next_dataptr(void)
{
#ifndef MEMORY_OPTIMIZATION1
    return app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].datapt;
#else
    return datapt(app_stream_fifo.fifo_write);
#endif
}

uint8 app_stream_fifo_check_next(void)
{
    return app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used_hndl;
}

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
void app_stream_fifo_commit_repnr_pkt(uint8 repnr)
{
    #ifdef APP_STREAM_OVERWRITE_PACKETS
    if (app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used_hndl != 0) {
        /* Packet already in use so fifo overflows... do not commit packet, just skip..*/
        app_stream_fifo.overflow_errors++;
        return;
    }
    #endif
   
    /* 
    ** Put the HANDLE number in the used_hndl field. If used_hndl != 0, then packet is used.
    ** Otherwise, the used_hndl field is the packet number, which will be used later to 
    ** obtain the correct Handle number.
    */        
    app_stream_env.fifo_size++;

    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used_hndl = repnr;
    app_stream_fifo.fifo_write++;  
    if (app_stream_fifo.fifo_write >= MAX_FIFO_LEN) {
        app_stream_fifo.fifo_write = 0;
    }    
}

void app_stream_fifo_commit_pkt(void)
{
    int repnr = app_stream_fifo.hnd;
    app_stream_fifo.hnd++;
    if (app_stream_fifo.hnd > max_vendor_repnr) {
        app_stream_fifo.hnd = min_vendor_repnr;
    }

    app_stream_fifo_commit_repnr_pkt(repnr);
}


void app_stream_fifo_commit_enable_pkt(void)
{
    /* Add some debugging to this packet  */
    uint8 *pkt = app_stream_fifo_get_next_dataptr();
    pkt[7] = (uint8)app_stream_env.fifo_size;
    pkt[8] = (uint8)app_stream_fifo.fifo_write;
    pkt[9] = (uint8)app_stream_fifo.fifo_read;
#ifdef ADD_PACKET_DEBUG
    int i;
    pkt += 10;
    uint8 vuse = app_stream_fifo.pkt_fifo[0].used_hndl;
    int idx = 0;
    for (i=0;i<MAX_FIFO_LEN;i++)
    {
        if (  ((app_stream_fifo.pkt_fifo[i].used_hndl != 0) && (vuse == 0))
           || ((app_stream_fifo.pkt_fifo[i].used_hndl == 0) && (vuse != 0))
           )
        {
            vuse = app_stream_fifo.pkt_fifo[i].used_hndl;
            *pkt++ = (uint8)i;
            *pkt++ = vuse;
            idx++;
            if (idx > 5) break; // cannot store more ..
        }
    }
#endif
    
    app_stream_fifo_commit_repnr_pkt(STREAM_HOGPD_ENABLE_REPORT_NR);
}


/**
 ****************************************************************************************
 * @brief Add data to the Stream Fifo
 * @param[in] datapt: pointer to the data to be added
 * @param[in] len: number of data bytes  
 * @param[in] handle: handle  
 * @param[in] p_callback: callback function. 
 *
 * The Callback, if not NULL, will be called after packet is sent, and data in dataptr
 * is not needed anymore. Cann be used to free the dataptr.
 *
 * @return void
 ****************************************************************************************
 */
#ifndef CFG_APP_STREAM_FIFO_PREDEFINED
int app_stream_fifo_add (void* datapt, uint8 len, int handle, void (*p_callback) (void* , int))
{
    uint8 ret_val=0;
    if (app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used != 0)
      return -1;  //buffer is full
  
    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used_hndl  = (uint8)handle;
    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].datapt     = datapt;
    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].len        = len;
 //     app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].handle     = handle;
    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].p_callback = p_callback;
    ret_val = app_stream_fifo.fifo_write;
    app_stream_fifo.fifo_write++;
    if (app_stream_fifo.fifo_write>=MAX_FIFO_LEN) app_stream_fifo.fifo_write=0;
    return (ret_val);
}
#endif

/**
 ****************************************************************************************
 * @brief Check if entry with given index in FIFO is in use
 *
 *
 * @return void
 ****************************************************************************************
 */
int app_stream_fifo_check (uint16 idx)
{
  if (idx>=MAX_FIFO_LEN)
    return -1;
  else
    return ((int)app_stream_fifo.pkt_fifo[idx].used_hndl);
}

/**
 ****************************************************************************************
 * @brief Signal an overflow error
 *
 * @return void
 ****************************************************************************************
 */
int app_stream_fifo_set_overflow(void)
{
    app_stream_fifo.overflow_errors++;
    return app_stream_fifo.overflow_errors; 
}


int tstnr = 0;

#ifndef MEMORY_OPTIMIZATION1 
static void send_pkt_to_l2cc(t_app_stream_pkt *strpkt)
{
     struct l2cc_pdu_send_req *pkt = KE_MSG_ALLOC_DYN(L2CC_PDU_SEND_REQ,
                                                      KE_BUILD_ID(TASK_L2CC, app_env.conidx),
                                                      TASK_APP, l2cc_pdu_send_req,
                                                      APP_STREAM_PACKET_SIZE);
    if (!pkt) {
        return;
    }
    // Set attribute channel ID
    pkt->pdu.chan_id   = L2C_CID_ATTRIBUTE;
    // Set packet opcode.
    pkt->pdu.data.code = L2C_CODE_ATT_HDL_VAL_NTF;
    // Set the handle number, which is derived from the report nr stored in used_hndl
    pkt->pdu.data.hdl_val_ntf.handle    = hogpd_report_handle(strpkt->used_hndl);
 
#ifndef CFG_APP_STREAM_FIFO_PREDEFINED
    pkt->pdu.data.hdl_val_ntf.value_len = strpkt->len;
#else
    pkt->pdu.data.hdl_val_ntf.value_len = APP_STREAM_PACKET_SIZE;
#endif

    // copy the content to value 
    memcpy(&(pkt->pdu.data.hdl_val_ntf.value[0]), strpkt->datapt,
           pkt->pdu.data.hdl_val_ntf.value_len);
    ke_msg_send(pkt);

    // Set used_hndl to 0, to denote that packet is available
    strpkt->used_hdnl = 0;  
#ifndef CFG_APP_STREAM_FIFO_PREDEFINED
    if (strpkt->p_callback) {
        strpkt->p_callback(strpkt->datapt,strpkt->handle);
    }
#endif
}
#else // MEMORY_OPTIMIZATION1
static void send_pkt_to_l2cc(int16 packetIdx)
{
    struct l2cc_pdu_send_req *pkt = KE_MSG_ALLOC_DYN(L2CC_PDU_SEND_REQ,
                                                     KE_BUILD_ID(TASK_L2CC, app_env.conidx),
                                                     TASK_APP, l2cc_pdu_send_req,
                                                     APP_STREAM_PACKET_SIZE);
    if (!pkt) {
        return;
    }
    // Set attribute channel ID
    pkt->pdu.chan_id   = L2C_CID_ATTRIBUTE;
    // Set packet opcode.
    pkt->pdu.data.code = L2C_CODE_ATT_HDL_VAL_NTF;
    // Set the handle number, which is derived from the report nr stored in used_hndl
    pkt->pdu.data.hdl_val_ntf.handle    = hogpd_report_handle(app_stream_fifo.pkt_fifo[packetIdx].used_hndl);
 

#ifndef CFG_APP_STREAM_FIFO_PREDEFINED
        pkt->pdu.data.hdl_val_ntf.value_len = app_stream_fifo.pkt_fifo[packetIdx].len;
#else
     pkt->pdu.data.hdl_val_ntf.value_len = APP_STREAM_PACKET_SIZE;
#endif

    // copy the content to value
    memcpy(&(pkt->pdu.data.hdl_val_ntf.value[0]), datapt(packetIdx),
           pkt->pdu.data.hdl_val_ntf.value_len);
    ke_msg_send(pkt);

#ifdef ADD_PACKET_DEBUG
    if (app_stream_fifo.pkt_fifo[packetIdx].used_hndl != 5) {
        pkt->pdu.data.hdl_val_ntf.value[0] = (uint8)app_stream_fifo.fifo_write;
        pkt->pdu.data.hdl_val_ntf.value[1] = (uint8)app_stream_fifo.fifo_read;
        pkt->pdu.data.hdl_val_ntf.value[2] = (uint8)app_stream_env.fifo_size;
        pkt->pdu.data.hdl_val_ntf.value[3] = (uint8)app_audio439_env.audio439SlotSize;
    }
#endif

    ke_msg_send(pkt);

    // Set used_hndl to 0, to denote that packet is available
    app_stream_fifo.pkt_fifo[packetIdx].used_hndl = 0;  
        app_stream_env.fifo_size--;
}
#endif // MEMORY_OPTIMIZATION1

extern char stop_when_buffer_empty;

static inline int stream_queue_data_until (uint8 min_avail)
{
    int max_count=0;
	int available;
    int retval=0;
  
    if (!app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_read].used_hndl) {
        if (stop_when_buffer_empty) {
            app_stream_send_enable_not(0);
            stop_when_buffer_empty = false;
            app_stream_stop();
        }
        return retval; //nothing to send quick check in order not to spend time
    }
  
    available=l2cm_get_nb_buffer_available();
    if (available <= 1) {
        return retval;       //never place the device into busy state
    }
	  
    max_count=available-min_avail;  //maximum number of packets to add 
    
    while (app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_read].used_hndl && (max_count > 0)) {
#ifndef MEMORY_OPTIMIZATION1
        send_pkt_to_l2cc (&app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_read]);
#else
        send_pkt_to_l2cc (app_stream_fifo.fifo_read);
#endif
        app_stream_fifo.fifo_read++;
        if (app_stream_fifo.fifo_read >= MAX_FIFO_LEN) {
            app_stream_fifo.fifo_read = 0;
        }
        retval++;
        max_count--;
    }
    return (retval);  
}

bool asynch_flag = false;
int str_count=18;
int min_val=1;

static int stream_queue_more_data_during_tx(void)
{
    int retval=0;
    int until=min_val;
    int available, already_in;
    available=l2cm_get_nb_buffer_available();
    already_in=MAX_TX_BUFS-available;
  
    if (asynch_flag) { //previous connection event ended with asynch, current frees the buffers
        until=available-(str_count-1);
    }
    if (until < 1) {
        until = 1;
    }

    if (already_in > str_count) {   //send more than we want to, there will not be enough time 
        asynch_flag = true;     //to add more on the complete event we will add and flag 
    } else {                        //to complete event NOT to try to add more to get synched again
        asynch_flag = false;
    }

    retval=stream_queue_data_until (until);
    return (retval);
}

static int stream_queue_more_data_end_of_event(void)
{
    int retval=0;
	  
    int available, already_in, until;
    available=l2cm_get_nb_buffer_available();
    already_in=MAX_TX_BUFS-available;

    if (already_in >= str_count) {
        return retval; //no need to send anything
    }
  
    if ((available < 17) && asynch_flag) {
        return retval;  //try to get synched again
    }
   
    until=MAX_TX_BUFS-str_count;
    if (!until && (str_count != 18)) {
        until=1;
    }
    
    retval=stream_queue_data_until (until);
    return retval;
}

extern int transmitting_data;
extern volatile char cpt_event;

int stream_queue_more_data(void)
{
    int retval=0;
      
    if (cpt_event == 1) { 
        retval=stream_queue_more_data_end_of_event();
        transmitting_data=0;
        cpt_event=2;
    } else if ((transmitting_data == 1) && (cpt_event == 2)) {
        retval=stream_queue_more_data_during_tx();
        transmitting_data=0;
        if (cpt_event == 2) {
            //if transmission stopped from remote the cpt_event may be already there
            cpt_event=0;
        }
    }
    return (retval);
}
//#else // !defined(CFG_APP_STREAM_FIFO_PREDEFINED)
//int app_stream_fifo_add(void* datapt, uint8 len, int handle, void (*p_callback)(void*, int))
//{
//    uint8 ret_val = 0;
//    if (app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used) {
//        return -1;  //buffer is full
//    }

//    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].used       = true;
//    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].datapt     = datapt;
//    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].len        = len;
//    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].handle     = handle;
//    app_stream_fifo.pkt_fifo[app_stream_fifo.fifo_write].p_callback = p_callback;
//    ret_val = app_stream_fifo.fifo_write;
//    app_stream_fifo.fifo_write++;
//    if (app_stream_fifo.fifo_write >= MAX_FIFO_LEN) {
//        app_stream_fifo.fifo_write = 0;
//    }
//    return (ret_val);
//}
#endif // CFG_APP_STREAM_FIFO_PREDEFINED
#endif // (BLE_APP_PRESENT)

/// @} APP
