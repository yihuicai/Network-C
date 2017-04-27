#include <altera_avalon_sgdma.h>
#include <altera_avalon_sgdma_descriptor.h>
#include <altera_avalon_sgdma_regs.h>
#include <altera_avalon_pio_regs.h>
#include <unistd.h>
#include <stdio.h>

#include "sys/alt_stdio.h"
#include "sys/alt_irq.h"
#include "system.h"


// Function Prototypes
void rx_ethernet_isr (void *context);


// Create a receive frame
unsigned char rx_frame[1024] = { 0 };

// Address
alt_u32 sw = 0x001024f0;
alt_u32 sw1 = 0x001024e0;
alt_u32 sw2 = 0x001024d0;
alt_u32 sw3 = 0x001024c0;
alt_u32 sw4 = 0x001024b0;
alt_u32 sw5 = 0x001024a0;
alt_u32 sw6 = 0x00102490;
alt_u32 sw7 = 0x00102480;
alt_u32 led = 0x00102500;

// Other variables
int data=8;
int in=0, print;
int in1=0, print1;
int in2=0, print2;
int in3=0, print3;
int in4=0, print4;
int in5=0, print5;
int in6=0, print6;
int in7=0, print7;

// Create sgdma transmit and receive devices
alt_sgdma_dev * sgdma_tx_dev;
alt_sgdma_dev * sgdma_rx_dev;

// Allocate descriptors in the descriptor_memory (onchip memory)
alt_sgdma_descriptor tx_descriptor		__attribute__ (( section ( ".descriptor_memory" )));
alt_sgdma_descriptor tx_descriptor_end	__attribute__ (( section ( ".descriptor_memory" )));

alt_sgdma_descriptor rx_descriptor  	__attribute__ (( section ( ".descriptor_memory" )));
alt_sgdma_descriptor rx_descriptor_end  __attribute__ (( section ( ".descriptor_memory" )));


/********************************************************************************
 * This program demonstrates use of the Ethernet in the DE2i-150 board.
********************************************************************************/
int main(void){



	// Open the sgdma receive device
	sgdma_rx_dev = alt_avalon_sgdma_open ("/dev/sgdma_rx");
	if (sgdma_rx_dev == NULL) {
		alt_printf ("Error: could not open scatter-gather dma receive device\n");
		//return -1;
	} else alt_printf ("Opened scatter-gather dma receive device\n");



	// Set interrupts for the sgdma receive device
	alt_avalon_sgdma_register_callback( sgdma_rx_dev, (alt_avalon_sgdma_callback) rx_ethernet_isr, 0x00000014, NULL );

	// Create sgdma receive descriptor
	alt_avalon_sgdma_construct_stream_to_mem_desc( &rx_descriptor, &rx_descriptor_end, (alt_u32 *)rx_frame, 0, 0 );

	// Set up non-blocking transfer of sgdma receive descriptor
	alt_avalon_sgdma_do_async_transfer( sgdma_rx_dev, &rx_descriptor );



	// Triple-speed Ethernet MegaCore base address
	volatile int * tse = (int *) TSE_BASE;

	// Specify the addresses of the PHY devices to be accessed through MDIO interface
	*(tse + 0x0F) = 0x10;

	// Disable read and write transfers and wait
	*(tse + 0x02) = *(tse + 0x02) | 0x00800220;
	while ( *(tse + 0x02) != ( *(tse + 0x02) | 0x00800220 ) );


	//MAC FIFO Configuration
	*(tse + 0x09 ) = TSE_TRANSMIT_FIFO_DEPTH-16;
	*(tse + 0x0E ) = 3;
	*(tse + 0x0D ) = 8;
	*(tse + 0x09 ) =TSE_RECEIVE_FIFO_DEPTH-16;
	*(tse + 0x0C ) = 8;
	*(tse + 0x0B ) = 8;
	*(tse + 0x0A ) = 0;
	*(tse + 0x08 ) = 0;

	// Initialize the MAC address
	*(tse + 0x03 ) = 0x17231C00;
	*(tse + 0x04 ) = 0x0000CB4A;

	// MAC function configuration
	*(tse + 0x05) = 1518;
	*(tse + 0x17) = 12;
	*(tse + 0x06) = 0xFFFF;
	*(tse + 0x02) = 0x00800220;


	// Software reset the PHY chip and wait
	*(tse + 0x02) =  0x00802220;
	while ( *(tse + 0x02) != ( 0x00800220 ) );

	// Enable read and write transfers, gigabit Ethernet operation and promiscuous mode
	
	*(tse + 0x02) = *(tse + 0x02) | 0x0080023B;

	while ( *(tse + 0x02) != ( *(tse + 0x02) | 0x0080023B ));



	// Application
	while (1) {

		data = (int)rx_frame[46];

		switch (data){
		case 0:
			print=in;
		    in = IORD_ALTERA_AVALON_PIO_DATA(sw);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 1:
			print=in;
		    in = IORD_ALTERA_AVALON_PIO_DATA(sw1);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 2:
			print=in;
		    in = IORD_ALTERA_AVALON_PIO_DATA(sw2);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 3:
			print=in;
			in = IORD_ALTERA_AVALON_PIO_DATA(sw3);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 4:
			print=in;
			in = IORD_ALTERA_AVALON_PIO_DATA(sw4);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 5:
			print=in;
			in = IORD_ALTERA_AVALON_PIO_DATA(sw5);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 6:
			print=in;
			in = IORD_ALTERA_AVALON_PIO_DATA(sw6);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		case 7:
			print=in;
			in = IORD_ALTERA_AVALON_PIO_DATA(sw7);
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in);
			break;
		default:
			in = 0;
			IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,0);
			break;
		}

		//print=in;
		//in = IORD_ALTERA_AVALON_PIO_DATA(sw); //read the input from the switch
		//IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE,in); //switch on or switch off the LED

		/*if (in==1){
			if (print != in){
				alt_printf( "Switch on LED \n" );		
			}
		}
		else{
			if (print != in) {
				alt_printf( "Switch off LED \n" );
				alt_printf("Number of successful frames:%d\n",*(tse+0x1B));
				alt_printf("The number of receive frames with CRC error:%d\n",*(tse+0x1C));
			}
		}*/
	}

	return 0;
}

/****************************************************************************************
 * Subroutine to read incoming Ethernet frames
****************************************************************************************/
void rx_ethernet_isr (void *context)
{

	static int x=0,n;


	//Include your code to show the values of the source and destination addresses of the received frame. For example:
	if(1){
		//alt_printf("%x,%x\n",rx_frame[0],rx_frame[1]);
		alt_printf( "Destination address: %x,%x,%x,%x,%x,%x\n", rx_frame[7], rx_frame[6],rx_frame[5],rx_frame[4],rx_frame[3],rx_frame[2]);
		alt_printf( "Source address: %x,%x,%x,%x,%x,%x\n", rx_frame[13], rx_frame[12],rx_frame[11],rx_frame[10],rx_frame[9],rx_frame[8]);
		x++;
		alt_printf("Frame received:%x\n",x);
		alt_printf("Data:%x\n",rx_frame[46]);
		/*
		alt_printf( " %x,%x,%x,%x,%x,%x,%x\n", rx_frame[14], rx_frame[15],rx_frame[16],rx_frame[17],rx_frame[18],rx_frame[19],rx_frame[20]);
		alt_printf( " %x,%x,%x\n", rx_frame[21], rx_frame[22],rx_frame[23]);
		alt_printf( " %x,%x,%x,%x,%x,%x,%x\n", rx_frame[24], rx_frame[25],rx_frame[26],rx_frame[27],rx_frame[28],rx_frame[29],rx_frame[30]);
		alt_printf( " %x,%x,%x,%x,%x,%x\n", rx_frame[31], rx_frame[32],rx_frame[33],rx_frame[34],rx_frame[35],rx_frame[36]);
		alt_printf("Check:%x\n",rx_frame[37]);
		alt_printf("%x,%x\n",rx_frame[61],rx_frame[62]);
        alt_printf("Data:%x,%x\n",rx_frame[63],rx_frame[64]);


	    n = IORD_ALTERA_AVALON_PIO_DATA(0x102480); //read the input from the switch
	*/

	}





	// Wait until receive descriptor transfer is complete
	while (alt_avalon_sgdma_check_descriptor_status(&rx_descriptor) != 0);

	// Create new receive sgdma descriptor
	alt_avalon_sgdma_construct_stream_to_mem_desc( &rx_descriptor, &rx_descriptor_end, (alt_u32 *)rx_frame, 0, 0 );

	// Set up non-blocking transfer of sgdma receive descriptor
	alt_avalon_sgdma_do_async_transfer( sgdma_rx_dev, &rx_descriptor );
}


