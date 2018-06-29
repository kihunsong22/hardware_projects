/* serial test */


#define OWCR (*(__bsfr volatile unsigned char *)0xC8)
#define OWDR (*(__sfr volatile unsigned char *)0xC9)
#define OWIR (*(__sfr volatile unsigned char *)0xCA)
#define OWIER (*(__sfr volatile unsigned char *)0xCB)
#define OWCKD (*(__sfr volatile unsigned char *)0xCC)




static int itim0=0;
static unsigned char spad[10];
static unsigned char tc1,tc2,tcd,fcs0,fcs1,dA0,dA1,dA2,dA3;
static unsigned char frame[32];

void crc(unsigned char  v)
{
    v^=fcs0;
    fcs0=fcs1;
    fcs1=v;

    fcs1^=fcs1<<4;
    fcs0^=fcs1>>4;
    fcs0^=fcs1<<3;
    fcs1^=fcs1>>5;
}

void init_uart(void)
{
    PCON |= 0x80;    //SMOD=1
    TH1 = 0xE5;      //9600 @ 50MHz
    TMOD = 0x20 | (TMOD & 0x0F) ;
    TR1 = 1;
    SCON = 0x50;
}

void init_tim0(void)
{
    TH0 = 0xC0;
    TL0 = 0x00;
    TMOD = 0x01 | (TMOD & 0xf0) ;
    TR0 = 1;
    IE |= 0x82;
}

void send(unsigned char x)
{
    SBUF = x;
    while (TI == 0) ;
    TI = 0;
}

void send_byte(unsigned char v)
{
  if(v==0xC0 || v==0xC1 || v==0x7D) {
    send(0x7D);
    send(v^0x20);
  } else
    send(v);
}

void send_cc(void)
{
    unsigned char  x;
    //send('#');
    x = (0x40 | (P0 & 0x0f));
    send(x);
    crc(x);
}

//void send_hex(unsigned char x)
//{
//    send(0x30 | (x & 0xf0)>>4);
//    send(0x30 | (x & 0x0f));
//    send(0x20);
//}

void init_OWB(void)
{
    OWCKD = 0x91;
    //OWCR  = 0x01;
    //OWDR  = 0xf0;
}



void temperature(void)
{
      int i;
      unsigned char x;

        IE &= 0xFD; // timer0 ofl int off
        //send('R');
        OWCR = 0x01;         ; //reset pulse
        while ((OWIR & 0x01) == 0) ; // wait for PD
        //send('p');
        if ((OWIR & 0x02) == 0)      // test PDR
        {
          //send('c');
          OWDR = 0xCC;               // send skip ROM
          while ((OWIR & 0x04) == 0) ; // wait for TBE
          while ((OWIR & 0x10) == 0) ;  //wait for RBF
          for (i=0;i<200;i++) ; //delay

          x = OWDR;  //send_hex(x);

          //send('d');
          OWDR = 0xBE; // read scratchpad
          while ((OWIR & 0x04) == 0) ; // wait for TBE
          while ((OWIR & 0x10) == 0) ;  //wait for RBF
          x = OWDR;   //send_hex(x);
          //send('b');
          for (i=0;i<9;i++)
          {
             OWDR = 0xFF;         // 0xFF;
             while ((OWIR & 0x04) == 0) ; // wait for TBE
             while ((OWIR & 0x10) == 0) ;  //wait for RBF
             x = OWDR;
             spad[i] = x;
             //send_hex(x);//send_hex(OWIR);
          }

           tc1 = 0x30 | (spad[0]%20)/2;
           tc2 = 0x30 | spad[0]/20;
           tcd = 0x30 | (spad[0]%2)*5;
           send('T');send(tc2);send(tc1);send('.');send(tcd);
           crc('T');crc(tc2);crc(tc1);crc('.');crc(tcd);
        }

        OWCR = 0x01;         ; //reset pulse
        while ((OWIR & 0x01) == 0) ; // wait for PD

        if ((OWIR & 0x02) == 0)      // test PDR
        {
          OWDR = 0xCC;               // send skip ROM
          while ((OWIR & 0x04) == 0) ; // wait for TBE
          while ((OWIR & 0x10) == 0) ;  //wait for RBF
          for (i=0;i<200;i++) ; //delay

          x = OWDR;  //send_hex(x);

          OWDR = 0x44;                 // send convert T
          while ((OWIR & 0x04) == 0) ; // wait for TBE
          while ((OWIR & 0x10) == 0) ;  //wait for RBF
          for (i=0;i<200;i++) ; //delay

          x = OWDR;  //send_hex(x);

          do
          {
             OWDR = 0xFF;
             x = OWDR;  //send_hex(x);
          }while (x != 0xFF) ;

        }
        IE |= 2;    // timer0 ofl int on
}

void __interrupt(11) tim0int(void)
{
   unsigned char i;
   TF0 = 0;
   itim0 += 1;
   if (itim0 > 99)
   {                // send frame ##################
      for (i=0;i<11;i++)
      {
         send(0xC0);
      }
      // init fcs
      fcs0 = 0xFF;
      fcs1 = 0xFF;
      send(0xFF);
      crc(0xFF);
      send(0xF3);      // TEST CMD
      crc(0xF3);

      send('M');       //my addr
      crc('M');
      send('u');
      crc('u');
      send('P');
      crc('P');
      send('D');
      crc('D');

      send(dA0);       //his addr
      crc(dA0);
      send(dA1);
      crc(dA1);
      send(dA2);
      crc(dA2);
      send(dA3);
      crc(dA3);

      //send(2);
      //crc(2);

      //send(1);
      //crc(1);

      //send(0);
      //crc(0);

      send_cc ();
      temperature();

      send_byte(~fcs0);
      send_byte(~fcs1);

      send(0xC1);

      //send(0x0D);send(0x0A);
      itim0 = 0;
   }
}


char recv(void)
{
    char x;
    while (RI==0) ;
    x = SBUF;
    RI = 0;
    return x;
}

void main(void)
{
   unsigned char x,i;
   init_uart();

   //send(0x07);

   init_OWB();

   init_tim0();

   // init mpu outputs
   //P0.0 = 0;       // Ir neg_rx
   //P0.1 = 1;       // Ir Tx_sel
   //P0.2 = 1;       // IrSC
   P0 = 0x06;
   // init frame stuff
   dA0 = 0xFF;
   dA1 = 0xFF;
   dA2 = 0xFF;
   dA3 = 0xFF;
   i=0;
   // real work is done in Timer0 interrupt routine
   for (;;)
   {
      x = recv();
      
      //send(x);      //ECHO BACK
   }

}
