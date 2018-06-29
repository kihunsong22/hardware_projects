#include <stdlib.h>
#include "can.h"
#include "tealib_cfg.h"

#define CAN_BASE 0x0100

#define MODE (*( volatile __xdata unsigned char *) (CAN_BASE + 0x00 ))
#define CMDREG (*( volatile __xdata unsigned char *) (CAN_BASE + 0x01))
#define SREG (*( volatile __xdata unsigned char *) (CAN_BASE + 0x02))
#define INTREG (*( volatile __xdata unsigned char *) (CAN_BASE + 0x03))
#define INTENREG (*( volatile __xdata unsigned char *) (CAN_BASE + 0x04))
#define BTR0 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x06))
#define BTR1 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x07))
#define OCTRLREG (*( volatile __xdata unsigned char *) (CAN_BASE + 0x08))
#define ALC (*( volatile __xdata unsigned char *) (CAN_BASE + 0x0B))
#define ECC (*( volatile __xdata unsigned char *) (CAN_BASE + 0x0C))
#define EWL (*( volatile __xdata unsigned char *) (CAN_BASE + 0x0D))
#define RERRCNT (*( volatile __xdata unsigned char *) (CAN_BASE + 0x0E))
#define TERRCNT (*( volatile __xdata unsigned char *) (CAN_BASE + 0x0F))
#define ACR0 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x10))
#define ACR1 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x11))
#define ACR2 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x12))
#define ACR3 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x13))
#define AMR0 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x14))
#define AMR1 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x15))
#define AMR2 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x16))
#define AMR3 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x17))
#define TXB0 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x10))
#define TXB1 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x11))
#define TXB2 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x12))
#define TXB3 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x13))
#define TXB4 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x14))
#define TXB5 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x15))
#define TXB6 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x16))
#define TXB7 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x17))
#define TXB8 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x18))
#define TXB9 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x19))
#define TXB10 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1A))
#define TXB11 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1B))
#define TXB12 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1C))
#define RXB0 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x10))
#define RXB1 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x11))
#define RXB2 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x12))
#define RXB3 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x13))
#define RXB4 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x14))
#define RXB5 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x15))
#define RXB6 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x16))
#define RXB7 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x17))
#define RXB8 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x18))
#define RXB9 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x19))
#define RXB10 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1A))
#define RXB11 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1B))
#define RXB12 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1C))
#define MC (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1D))
#define RBSA (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1E))
#define CLKDIVREG (*( volatile __xdata unsigned char *) (CAN_BASE + 0x1F))
#define RM (*( volatile __xdata unsigned char *) (CAN_BASE + 0x20))
#define TM (*( volatile __xdata unsigned char *) (CAN_BASE + 0x60))
#define GPR0 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x6D))
#define GPR1 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x6E))
#define GPR2 (*( volatile __xdata unsigned char *) (CAN_BASE + 0x6F))


/*
 * typedefs
 */
typedef struct
{
    volatile unsigned char mode;
    unsigned char frame_info;
    CAN_ID can_id;
    CAN_ID contr_id;
    char *volatile payload;
    volatile int size;
    char *payload_start;
    int payload_size;
} CAN_MSG_INFO;


/*
 * static functions
 */
static void can_send_msg( char mo_nr );


/*
 * local vars
 */
static CAN_MSG_INFO can_mo_queue[CAN_NR_OF_MO] = {{0}};
static volatile char tx_busy = 0;

#ifdef CAN_BAUDRATE
 #if CAN_BAUDRATE+0 == 0
  #undef CAN_BAUDRATE
  #define CAN_BAUDRATE 125000
 #endif
#endif
/*****************************************************************************
 *
 *  FUNCTION:   can_init
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: if CAN_BAUDRATE is defined: none
 *          if CAN_BAUDRATE not defined: baudrate, accept_filt, accept_mask
 *
 *  RETURN VALUE:   actual baudrate on success
 *          CAN_BAUD_ERR on error.
 *
 *  DESCRIPTION:    initialize and start the CAN-controller
 */
#ifdef CAN_BAUDRATE
long can_init( unsigned long accept_filt, unsigned long accept_mask  )
{
 #define baudrate CAN_BAUDRATE
#else
long can_init( long baudrate, unsigned long accept_filt, unsigned long accept_mask )
{
#endif
    unsigned long ret_val = CAN_BAUD_ERR;
    char i;
    char best_quanta = 0;
    long bpr;
    long diff;
//    unsigned char tseg1;
//    unsigned char tseg2;
    unsigned long best_diff = 0xFFFFFFFF;

    /* switch to reset mode to get access to the configuration registers */
    MODE = 1;

    /* calculate best fitting values for bpr and number of time-quanta per bit */
    for ( i = 25; i > 2; i-- )
    {
        /* calculate parameters */
        bpr = ( FOSC / 2 ) / (long)( baudrate * i );
        diff = ( bpr * baudrate * i ) - ( FOSC / 2 );
        if ( diff < 0 )
        {
            diff = -diff;
        }

        if (( diff < best_diff ) && ( bpr < 32 )) /* better fit? */
        {
            best_diff = diff;
            best_quanta = i;
        }
    }

    /* set bit timing registers */
    if ( best_quanta >= 20 )
    {
        /* sample point 4 time-quanta before end */
        bpr = ( FOSC / 2 ) / (long)( baudrate * (long)best_quanta );
        BTR0 = ( char )bpr - 1;

        /* tseg1 = 16 -> TSEG1 = 15 */
        /* tseg2 = best_quanta - tesg1 - 1 -> TSEG2 = best_quanta - 18 */
        BTR1 = 15 + (( best_quanta - 18 ) << 4 );

        ret_val = ( FOSC / 2 ) / (( 3 + ( BTR1 & 0x0F ) + (( BTR1 >> 4 ) & 0x07 )) * ( BTR0 + 1 ));
    }
    else if ( best_quanta >= 6 )
    {
        /* sample point 2 time-quanta before end */
        bpr = ( FOSC / 2 ) / (long)( baudrate * (long)best_quanta );
        BTR0 = ( char )bpr - 1;

        /* tseg1 = best_quanta - 3, TSEG1 = tseg1 - 1 */
        /* tseg2 = 2, TSEG2 = tseg2 - 1 */
        BTR1 = best_quanta - 4 + ( 1 << 4 );

        ret_val = ( FOSC / 2 ) / (( 3 + ( BTR1 & 0x0F ) + (( BTR1 >> 4 ) & 0x07 )) * ( BTR0 + 1 ));
    }
    else if ( best_quanta >= 3 )
    {
        /* sample point 1 time-quantum before end */
        bpr = ( FOSC / 2 ) / (long)( baudrate * (long)best_quanta );
        BTR0 = ( char )bpr - 1;

        /* tseg1 = best_quanta - 2, TSEG1 = tseg1 - 1 */
        /* tseg2 = 1, TSEG2 = tseg2 - 1 */
        BTR1 = best_quanta - 3;

        ret_val = ( FOSC / 2 ) / (( 3 + ( BTR1 & 0x0F ) + (( BTR1 & 0x70) >> 4 )) * ( BTR0 + 1 ));
    }

    if ( ret_val != CAN_BAUD_ERR )
    {

        INTENREG = 0x03;    /* set interrupt register to enable transmit and receive interrupt */
        OCTRLREG = 0x02;    /* set output mode to normal mode */
        EWL = 96;       /* set error warning level to 96 */
        ACR0 = ( accept_filt >> 24 ) & 0x000000FF;
        ACR1 = ( accept_filt >> 16 ) & 0x000000FF;
        ACR2 = ( accept_filt >>  8 ) & 0x000000FF;
        ACR3 = accept_filt & 0x000000FF;
        AMR0 = ( accept_mask >> 24 ) & 0x000000FF;
        AMR1 = ( accept_mask >> 16 ) & 0x000000FF;
        AMR2 = ( accept_mask >>  8 ) & 0x000000FF;
        AMR3 = accept_mask & 0x000000FF;

        /* start the CAN controller */

        /* clear all bits of the mode register except the reset bit ( => 1 acceptance filter, normal mode )*/
        MODE = 1;
        /* clear the reset bit, switch to normal mode */
        MODE = 0;

        EX0 = 1;
        EA = 1;
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_status
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: none
 *
 *  RETURN VALUE:   CAN-bus status:
 *              CAN_OK: OK
 *              CAN_WARNING: warning
 *              CAN_BUS_OFF: bus-off state
 *
 *  DESCRIPTION:    initialize and start the CAN-controller
 */
char can_status( void )
{
    char ret_val = CAN_OK;
    char status = -1;

    /* is the reset bit set? */
    if (( MODE & 0x01 ) == 0x01 )
    {
       /* CAN is stopped */
        ret_val = CAN_STOPPED;
    }
    else
    {
       status = SREG;
       if ( status & 0x80 )
       {
          /* bus-off */
          ret_val = CAN_BUS_OFF;
       }
       else if ( status & 0x40 )
       {
          /* warning */
          ret_val = CAN_WARNING;
       }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:      can_restart
 *
 *  AVAILABILITY:  GLOBAL
 *
 *  PARAMETERS:    none
 *
 *  RETURN VALUE:  CAN_OK if OK
 *                 CAN_ERR if not in reset mode at function entry.
 *
 *  DESCRIPTION:    restart the CAN-controller
 */
char can_restart( void )
{
    char ret_val = CAN_ERR;

    /* is the reset bit set? */
    if (( MODE & 0x01 ) == 0x01 )
    {
        /* clear the reset bit */
        MODE = 0;
        ret_val = CAN_OK;
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_stop
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: none
 *
 *  RETURN VALUE:   none
 *
 *  DESCRIPTION:    stop the CAN-controller
 */
void can_stop( void )
{
    /* set reset bit */
    MODE = 0x01;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_init_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id = msg_id
 *          mode =  1: send
 *              2: receive
 *
 *  RETURN VALUE:   mo number on success
 *          CAN_MO_FULL on error
 *
 *  DESCRIPTION:    initialize an MO
 */
char can_init_mo( CAN_ID id, char mode, int size, char *payload )
{
    char mo_nr;
    char ret_val = CAN_MO_FULL;
    char frame_info;
//    CAN_MSG_INFO mo_copy;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for first empty MO */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if ( mo->mode == CAN_EMPTY_MODE )
        {
//            mo_copy = can_mo_queue[mo_nr];
            /* first empty MO */
            mo->can_id = id;
#ifdef CAN_EXTENDED_ID
            mo->contr_id = id << 3;
            frame_info = 0x80;
#else
            mo->contr_id = id << 5;
            frame_info = 0x00;
#endif
            mo->payload = payload;
            mo->payload_start = payload;

            if ( mode == CAN_RX_MODE )
            {
                /* receive mode */
                mo->mode = CAN_RX_MODE;
                mo->frame_info = frame_info;
                mo->size = 0;
                mo->payload_size = size;
            }
            else if ( mode == CAN_TX_MODE )
            {
                /* transmit mode */
                mo->mode = CAN_TXREADY_MODE;
                mo->frame_info = frame_info;
                mo->size = 0;
                mo->payload_size = 0;
            }
            else if ( mode == CAN_RQ_MODE )
            {
                /* request mode */
                mo->mode = CAN_RQREADY_MODE;
                mo->frame_info = frame_info | 0x40;
                mo->size = 0;
                mo->payload_size = size;
            }
            else if ( mode == CAN_UPD_MODE )
            {
                /* update mode */
                mo->mode = CAN_UPDREADY_MODE;
                mo->frame_info = frame_info;
                mo->size = size;
                mo->payload_size = size;
            }
            ret_val = mo_nr;
//            can_mo_queue[mo_nr] = mo_copy;
            break;
        }
    }

    return ret_val;
}



#ifdef CAN_USE_ID


/*****************************************************************************
 *
 *  FUNCTION:   can_delete_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id = msg_id
 *
 *  RETURN VALUE:   CAN_OK = success
 *          CAN_INV_MO = error
 *
 *  DESCRIPTION:    delete an MO from the MO-list
 */
char can_delete_mo( CAN_ID id )
{
    char mo_nr;
    char ret_val = CAN_INV_MO;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* check if given id exists */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if ( mo->can_id == id )
        {
            mo->mode = CAN_EMPTY_MODE;
            ret_val = CAN_OK;
            break;
        }
    }

    /* if none MO had the given id, return CAN_INV_MO, else return CAN_OK */
    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_send_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id: message id
 *          len: length of the message (0-8 bytes)
 *          msg: pointer to the message
 *
 *  RETURN VALUE:   CAN_OK if success
 *          CAN_INV_MO if error
 *
 *  DESCRIPTION:    Put a message in the send buffer
 */
char can_send_mo( CAN_ID id, int size, char *payload )
{
    char mo_nr;
    char ret_val = CAN_INV_MO;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for MO with given id */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if (( mo->can_id == id ) && ( mo->mode == CAN_TXREADY_MODE ))
        {
            /* right MO found, set message data */
            if ( payload != NULL )
            {
                mo->payload = payload;
            }
            else
            {
                mo->payload = mo->payload_start;
            }
            mo->size = size;

            /* start send process */
            if ( !tx_busy )     /* none of the tx mo's busy? */
            {
                while ( !(SREG & 0x04 ));   /* wait until the tx buffer can be written */
                can_send_msg( mo_nr );      /* start tx chain */
            }
            else
            {
                /* add to running tx chain */
                mo->mode = CAN_TX_MODE;

                /* tx ended before this message was added? */
                if ( !( tx_busy ) && ( mo->mode == CAN_TX_MODE ))
                {
                    /* start tx chain */
                    can_send_msg( mo_nr );
                }
            }
            ret_val = CAN_OK;
            break;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_send_status_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id: id of the message
 *
 *  RETURN VALUE:   CAN_READY:  message is sent
 *          CAN_BUSY:   message is pending
 *          CAN_INV_MO: invalid id
 *
 *  DESCRIPTION:    get the status of a send-MO with the given id
 */
char can_send_status_mo( CAN_ID id )
{
    char ret_val = CAN_INV_MO;
    char mo_nr;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for MO with given id */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if ( mo->can_id == id )
        {
            /* MO found, check status */
            if ( mo->mode == CAN_TX_MODE )
            {
                ret_val = CAN_BUSY;
            }
            else if ( mo->mode == CAN_TXREADY_MODE )
            {
                ret_val = CAN_READY;
            }
            break;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_receive_status_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id: id of the message
 *
 *  RETURN VALUE:   >= 0:         length of received message
 *          CAN_BUSY:     no message available
 *          CAN_INV_MO:   invalid id/mo
 *          CAN_BUFF_OVR: receive buffer overrun
 *
 *  DESCRIPTION:    receive a message with the given id
 */
int can_receive_status_mo( CAN_ID id )
{
    char mo_nr;
    int ret_val = CAN_INV_MO;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for MO with given id */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if ( mo->can_id == id )
        {
            /* MO found, check status */
            if ( mo->mode == CAN_RX_MODE )
            {
                ret_val = CAN_BUSY;
            }
            else if ( mo->mode == CAN_RXOVL_MODE )
            {
                ret_val = CAN_BUF_OVL;
            }
            else if ( mo->mode == CAN_RXREADY_MODE )
            {
                ret_val = mo->size;
            }
            break;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_clear_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id: id of the message
 *
 *  RETURN VALUE:   CAN_OK:     success
 *          CAN_INV_MO: invalid id
 *
 *  DESCRIPTION:    reset the mo with the given id to the initial state
 */
char can_clear_mo( CAN_ID id )
{
    char mo_nr;
    char ret_val = CAN_INV_MO;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for MO with given id */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if ( mo->can_id == id )
        {
//            mo_copy = can_mo_queue[mo_nr];
            /* MO found, check status and reset to initial state */
            if (( mo->mode == CAN_RXREADY_MODE ) ||
                ( mo->mode == CAN_RXOVL_MODE ))
            {
                /* receive-MO */
                mo->mode = CAN_RX_MODE;
            }
            else if (( mo->mode == CAN_TX_MODE ) ||
                 ( mo->mode == CAN_TXLOAD_MODE ))
            {
                /* send-MO */
                mo->mode = CAN_TXREADY_MODE;
            }
            else if (( mo->mode == CAN_RQ_MODE ) ||
                 ( mo->mode == CAN_RQREADY_MODE ))
            {
                /* request-MO */
                mo->mode = CAN_RQREADY_MODE;
            }
            else if (( mo->mode == CAN_UPD_MODE ) ||
                 ( mo->mode == CAN_UPDREADY_MODE ))
            {
                /* update-MO */
                mo->mode = CAN_UPDREADY_MODE;
            }

            mo->payload = mo->payload_start;
            mo->size = 0;

//            can_mo_queue[mo_nr] = mo_copy;

            ret_val = CAN_OK;
            break;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_request
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id: id of the message to request
 *
 *  RETURN VALUE:   CAN_OK  if success
 *          CAN_INV_MO if invalid id
 *
 *  DESCRIPTION:    request a message with the given id
 */
char can_request_mo( CAN_ID id )
{
    char mo_nr;
    char ret_val = CAN_INV_MO;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for MO with given id */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if (( mo->can_id == id ) && ( mo->mode == CAN_RQREADY_MODE ))
        {
            /* start send process */
            if ( !tx_busy )     /* none of the tx mo's busy? */
            {
                while ( !(SREG & 0x04 ));   /* wait until the tx buffer can be written */
                can_send_msg( mo_nr );      /* start tx chain */
            }
            else
            {
                /* add to running tx chain */
                mo->mode = CAN_TX_MODE;

                /* tx ended before this message was added? */
                if ( !( tx_busy ) && ( mo->mode == CAN_TX_MODE ))
                {
                    /* start tx chain */
                    can_send_msg( mo_nr );
                }
            }

            ret_val = CAN_OK;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_update_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: id: id of the message
 *
 *  RETURN VALUE:   0  if success
 *          -1 if error
 *
 *  DESCRIPTION:    request a message with the given id
 */
char can_update_mo( CAN_ID id, int size, char *payload )
{
    char mo_nr;
    char ret_val = CAN_INV_MO;
    CAN_MSG_INFO *mo = can_mo_queue;

    /* search for MO with given id */
    for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++, mo++ )
    {
        if ( mo->can_id == id )
        {
            size = size;
            payload=payload;
            ret_val = CAN_OK;
        }
    }

    return ret_val;
}


#else

/*****************************************************************************
 *
 *  FUNCTION:   can_delete_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: mo_nr =  number of MO to be deleted
 *
 *  RETURN VALUE:   CAN_OK = success
 *          CAN_INV_MO = error
 *
 *  DESCRIPTION:    delete an MO from the MO-list
 */
char can_delete_mo( char mo_nr )
{
    char ret_val = CAN_INV_MO;

    /* check if given MO exists */
    if ( mo_nr < CAN_NR_OF_MO )
    {
        can_mo_queue[mo_nr].mode = CAN_EMPTY_MODE;
        ret_val = CAN_OK;
    }

    /* if MO is not existing CAN_INV_MO, else CAN_OK */
    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_send_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: mo_nr: number of MO
 *          len: length of the message (0-8 bytes)
 *          msg: pointer to the message
 *
 *  RETURN VALUE:   CAN_OK if success
 *          CAN_INV_MO if error
 *
 *  DESCRIPTION:    Put a message in the send buffer
 */
char can_send_mo( char mo_nr, int size, char *payload )
{
    char ret_val = CAN_INV_MO;

    /* check if given MO exists as send_MO */
    if (( mo_nr < CAN_NR_OF_MO ) && ( can_mo_queue[mo_nr].mode == CAN_TXREADY_MODE ))
    {
        if ( payload != NULL )
        {
            can_mo_queue[mo_nr].payload = payload;
        }
        else
        {
            /* no payload address given, use address given at init_mo */
            can_mo_queue[mo_nr].payload = can_mo_queue[mo_nr].payload_start;
        }
        can_mo_queue[mo_nr].size = size;

        if ( !tx_busy )     /* no tx chain running? */
        {
            while ( !(SREG & 0x04 ));   /* wait until the tx buffer can be written */

            can_send_msg( mo_nr );      /* start tx chain */
        }
        else
        {
            can_mo_queue[mo_nr].mode = CAN_TX_MODE;     /* add to tx chain */

            /* tx ended before this message was added? */
            if ( !( tx_busy ) && ( can_mo_queue[mo_nr].mode == CAN_TX_MODE ))
            {
                can_send_msg( mo_nr );  /* start tx chain */
            }
        }
        ret_val = CAN_OK;
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_send_busy
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: mo_nr: number of the MO
 *
 *  RETURN VALUE:   CAN_READY:  message is sent
 *          CAN_BUSY:   message is pending
 *          CAN_INV_MO: invalid mo or MO is not a send-MO
 *
 *  DESCRIPTION:    receive a message with the given id
 */
char can_send_status_mo( char mo_nr )
{
    char ret_val = CAN_INV_MO;

    /* check if given MO exists */
    if ( mo_nr < CAN_NR_OF_MO )
    {
        /* check status */
        if (( can_mo_queue[mo_nr].mode == CAN_TX_MODE ) ||
            ( can_mo_queue[mo_nr].mode == CAN_TX7_MODE ))
        {
            ret_val = CAN_BUSY;
        }
        else if ( can_mo_queue[mo_nr].mode == CAN_TXREADY_MODE )
        {
            ret_val = CAN_READY;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_receive_status_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: mo_nr: number of the MO
 *
 *  RETURN VALUE:   >= 0:         length of received message
 *          CAN_BUSY:     no message available
 *          CAN_INV_MO:   invalid mo or MO is not a receive-MO
 *          CAN_BUFF_OVL: receive buffer overrun
 *
 *  DESCRIPTION:    check if a message with the given id is received
 */
int can_receive_status_mo( char mo_nr )
{
    int ret_val = CAN_INV_MO;

    /* check if given MO exists */
    if ( mo_nr < CAN_NR_OF_MO )
    {
        /* check status */
        switch ( can_mo_queue[mo_nr].mode )
        {
        case CAN_RX_MODE:
            ret_val = CAN_BUSY;
            break;
        case CAN_RXOVL_MODE:
            ret_val = CAN_BUF_OVL;
            break;
        case CAN_RXREADY_MODE:
            ret_val = can_mo_queue[mo_nr].size;
            break;
        default:
            break;
        }
    }

    return ret_val;
}


/*****************************************************************************
 *
 *  FUNCTION:   can_clear_mo
 *
 *  AVAILABILITY:   GLOBAL
 *
 *  PARAMETERS: mo_nr: number of the MO
 *
 *  RETURN VALUE:   CAN_OK:     success
 *          CAN_INV_MO: invalid id
 *
 *  DESCRIPTION:    reset the mo with the given mo-number to the initial state
 */
char can_clear_mo( char mo_nr )
{
    int ret_val = CAN_INV_MO;
    unsigned char mode;

    /* check if given MO exists */
    if ( mo_nr < CAN_NR_OF_MO )
    {
        mode = can_mo_queue[mo_nr].mode;
        /* reset MO-status to initial status */
        if (( mode == CAN_RXREADY_MODE ) ||
            ( mode == CAN_RXOVL_MODE ))
        {
            mode = CAN_RX_MODE;
        }
        else if (( mode == CAN_TX_MODE ) ||
             ( mode == CAN_TXLOAD_MODE ) ||
             ( mode == CAN_TX7_MODE ))
        {
            mode = CAN_TXREADY_MODE;
        }
        can_mo_queue[mo_nr].mode = mode;
        can_mo_queue[mo_nr].payload = can_mo_queue[mo_nr].payload_start;
        can_mo_queue[mo_nr].size = 0;
        ret_val = CAN_OK;
    }

    return ret_val;
}


char can_request_mo( char mo_nr )
{
    int ret_val = CAN_INV_MO;

    /* check if given MO exists */
    if ( mo_nr < CAN_NR_OF_MO )
    {
        ret_val = CAN_OK;
    }

    return ret_val;
}


char can_update_mo( char mo_nr, int size, char *payload )
{
    int ret_val = CAN_INV_MO;

    /* check if given MO exists */
    if ( mo_nr < CAN_NR_OF_MO )
    {
        payload=payload;
        size=size;

        ret_val = CAN_OK;
    }

    return ret_val;
}
#endif


__interrupt(0x03) void can_interrupt( void )
{
    static unsigned char last_mo_nr = 0;
    unsigned char mo_nr;
    char msg_len;
    unsigned char volatile  *msg_buf;  /* pointer to CAN-controller data-buffer */

    /* local copies to speed up the routine: */
    CAN_ID  *contr_id;
    char *payload_buf;
    int payload_size;

    if ( INTREG & 0x02 )
    {
        /* transmit of new CAN message possible */
        mo_nr = last_mo_nr;
        tx_busy = 0;
        do
        {
            last_mo_nr++;
            if ( last_mo_nr == CAN_NR_OF_MO )
            {
                last_mo_nr = 0;
            }
            if (( can_mo_queue[last_mo_nr].mode == CAN_TX_MODE ) ||
                ( can_mo_queue[last_mo_nr].mode == CAN_TX7_MODE ))
            {
                can_send_msg( last_mo_nr );
                break;
            }
        } while ( last_mo_nr != mo_nr );
    }

    if ( INTREG & 0x01 )
    {
        /* receive interrupt */
        contr_id = (CAN_ID  *)&RXB1;

        /* look for MO with id of received message */
        for ( mo_nr = 0; mo_nr < CAN_NR_OF_MO; mo_nr++ )
        {
            if (( can_mo_queue[mo_nr].mode == CAN_RX_MODE ) &&
                ( *contr_id == can_mo_queue[mo_nr].contr_id ))
            {
                /* if found, append message to MO */
                payload_buf = can_mo_queue[mo_nr].payload;
                payload_size = can_mo_queue[mo_nr].size;

                msg_len = (RXB0 & 0x0f);
                msg_buf = &RXB1 + sizeof( CAN_ID );

                if (msg_len != 8 )  /* last meesage for this MO? */
                {
                    if ( *msg_buf++ != 7 )
                    {
                        can_mo_queue[mo_nr].mode = CAN_RXREADY_MODE;
                    }
                    msg_len--;
                }
                /* check for overflow */
                if (( payload_size + msg_len) <= can_mo_queue[mo_nr].payload_size )
                {
                    payload_size += msg_len;

                    while ( --msg_len >= 0 )
                    {
                        *payload_buf++ = *msg_buf++;
                    }

                    can_mo_queue[mo_nr].payload = payload_buf;
                    can_mo_queue[mo_nr].size = payload_size;
                }
                else
                {
                    can_mo_queue[mo_nr].mode = CAN_RXOVL_MODE;  /* OVERFLOW! */
                }
                break;
            }
        }
        CMDREG = 0x04;  /* release receive message buffer in the CAN-controller */
    }
}

static void can_send_msg( char mo_nr )
{
    char msg_len;
    char copy_len;
    unsigned char volatile  *msg_buf;  /* pointer to CAN-controller data-buffer */
    CAN_MSG_INFO *mo = can_mo_queue;

    mo += mo_nr;      /* set pointer to MO */

    *(CAN_ID  *)&TXB1 = mo->contr_id;       /* id */
    msg_buf = &TXB1 + sizeof( CAN_ID );          /* start of data */

    if ( mo->mode == CAN_RQ_MODE )
    {
        /* send request msg */
        msg_len = copy_len = 0;
        mo->mode = CAN_RQREADY_MODE;
    }
    else if ( mo->mode == CAN_TX7_MODE )
    {
        /* send last message */
        *msg_buf++ = mo->size;
        msg_len = 2;
        copy_len = 1;
        mo->mode = CAN_TXREADY_MODE;
    }
    else
    {
        if ( mo->size >= 8 )
        {
            /* more then 7 bytes data to send, more CAN-messages to go */
            msg_len = copy_len = 8;
            mo->size -= 8;
            mo->mode = CAN_TX_MODE;
        }
        else if ( mo->size == 7 )
        {
            /* 7 bytes to send, second last CAN-message */
            *msg_buf++ = 7;
            msg_len = 7;
            copy_len = 6;
            mo->size = 1;   /* last byte in last CAN-message */
            mo->mode = CAN_TX7_MODE;
        }
        else
        {
            /* 6 bytes or less to send, last CAN-message */
            *msg_buf++ = mo->size;
            copy_len = mo->size;
            msg_len = copy_len + 1;
            mo->mode = CAN_TXREADY_MODE;
        }
    }

    /* copy message header and data to the CAN-controller */
    TXB0 = mo->frame_info + msg_len;         /* frame info */
    while ( --copy_len >= 0 )
    {
        *msg_buf++ = *mo->payload++;        /* copy data */
    }
    CMDREG = 0x01;                      /* transmit CAN message */
    tx_busy = 1;                        /* indicate running chain */
}

