#ifndef CAN_H
#define CAN_H

//#define CAN_EXTENDED_ID
//#define CAN_USE_ID
//#define CAN_BAUDRATE
#define CAN_NR_OF_MO 10


/*
 * DO NOT EDIT NEXT LINES!
 */


enum { CAN_EMPTY_MODE = 0,
       CAN_RX_MODE = 1,
       CAN_TX_MODE = 2,
       CAN_RQ_MODE = 3,
       CAN_UPD_MODE = 4,
       CAN_RXREADY_MODE,
       CAN_RXOVL_MODE,
       CAN_TXREADY_MODE,
       CAN_TXLOAD_MODE,
       CAN_TX7_MODE,
       CAN_RQREADY_MODE,
       CAN_UPDREADY_MODE
     };

#define CAN_OK              0
#define CAN_ERR             -1
#define CAN_STOPPED         -2
#define CAN_INV_MO          -3
#define CAN_BUS_OFF         -4
#define CAN_WARNING         -5
#define CAN_MO_FULL         -6
#define CAN_BAUD_ERR        -7
#define CAN_READY           -8
#define CAN_BUSY            -9
#define CAN_BUF_OVL         -10


#ifdef CAN_EXTENDED_ID
typedef unsigned long CAN_ID;
#else
typedef unsigned int CAN_ID;
#endif


#ifdef CAN_BAUDRATE 
extern long can_init( unsigned long accept_filt, unsigned long accept_mask  );
#else
extern long can_init( long baudrate, unsigned long accept_filt, unsigned long accept_mask );
#endif
extern char can_status( void );
extern char can_restart( void );
extern void can_stop( void );
extern char can_init_mo( CAN_ID id, char mode, int size, char *payload );

#ifdef CAN_USE_ID
extern char can_delete_mo( CAN_ID id );
extern char can_send_mo( CAN_ID id, int size, char *payload );
extern char can_send_status_mo( CAN_ID id );
extern int can_receive_status_mo( CAN_ID id );
extern char can_clear_mo( CAN_ID id );
extern char can_request_mo( CAN_ID id );
extern char can_update_mo( CAN_ID id, int size, char *payload );
#else
extern char can_delete_mo( char mo_nr );
extern char can_send_mo( char mo_nr, int size, char *payload );
extern char can_send_status_mo( char mo_nr );
extern int can_receive_status_mo( char mo_nr );
extern char can_clear_mo( char mo_nr );
extern char can_request_mo( char mo_nr );
extern char can_update_mo( char mo_nr, int size, char *payload );
#endif

#endif
