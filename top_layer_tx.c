#include <altera_avalon_sgdma.h>
#include <altera_avalon_sgdma_descriptor.h>
#include <altera_avalon_sgdma_regs.h> //include the sgdma registers
#include <altera_avalon_pio_regs.h> //include the PIO registers
#include <unistd.h>
#include <stdio.h>

#include "sys/alt_stdio.h"
#include "sys/alt_irq.h"
#include "system.h"
//#include "altera_up_avalon_character_lcd.h"



// Function Prototypes
void tx_ethernet_isr (void *context);

//int check_length(char a[])
//{
	//int j,count=1;
	//for(j=0;a[j]!='/0';j++)
	//{
		//count++;
	//}
	//return count;
//}
// Create a receive frame
//unsigned char rx_frame[1024] = { 0 };
unsigned char tx_frame[1024] = {
0x00,0x00, // for 32-bit alignment
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // destination address (broadcast)
0x01,0x60,0x6E,0x11,0x02,0x0F, // source address
0x00,0x2E, // length or type of the payload data
0x45,0x30,0x00,0x28,
0x00,0x01,0x40,0x00,
0x01,0x11,0x95,0x6A,
0xC0,0xA8,0x01,0x01,// source ip addr
0xA9,0xFE,0xAD,0xD6,// dest ip addr

'\0'
};

//create UDP packet
unsigned int udp_frame[1024];
unsigned int pse_frame[1024];
//Other variables
int in=0, in2=0,print;
// Create sgdma transmit and receive devices
alt_sgdma_dev * sgdma_tx_dev;
//alt_sgdma_dev * sgdma_rx_dev;

// Allocate descriptors in the descriptor_memory (onchip memory)
alt_sgdma_descriptor tx_descriptor		__attribute__ (( section ( ".descriptor_memory" )));
alt_sgdma_descriptor tx_descriptor_end	__attribute__ (( section ( ".descriptor_memory" )));

//alt_sgdma_descriptor rx_descriptor  	__attribute__ (( section ( ".descriptor_memory" )));
//alt_sgdma_descriptor rx_descriptor_end  __attribute__ (( section ( ".descriptor_memory" )));


/********************************************************************************
 * This program demonstrates use of the Ethernet in the DE2i-150 board.
********************************************************************************/
int main(void){

//	alt_up_character_lcd_dev * char_lcd_dev;
	// open the Character LCD port
//	char_lcd_dev = alt_up_character_lcd_open_dev ("/dev/Char_LCD_16x2");
	//if ( char_lcd_dev == NULL)
//	alt_printf ("Error: could not open character LCD device\n");
	//else
//	alt_printf ("Opened character LCD device\n");
	/* Initialize the character display */
//	alt_up_character_lcd_init (char_lcd_dev);
	/* Write "Welcome to" in the first row */
//	alt_up_character_lcd_string(char_lcd_dev, "Welcome to");
	/* Write "the DE2 board" in the second row */
//	char second_row[] = "the DE2 board\0";
//	alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);
//	alt_up_character_lcd_string(char_lcd_dev, second_row);


	/****************************************************************************************
	 * THe main
	****************************************************************************************/


	int i,j=0,sum=0,flow;
	for(i=0;i<4;i++)     //set ip source add
		{
	pse_frame[i]=0x01;
		}
	for(i=4;i<8;i++)     //set ip des add
		{
	pse_frame[i]=0xE2;
		}
	for(i=8;i<9;i++)     //set pseudo placeholder
		{
	pse_frame[i]=0x00;
		}
	for(i=9;i<10;i++)     //set protocol
		{
	pse_frame[i]=0x11;
		}
	for(i=10;i<12;i++)     //set head length
		{
	pse_frame[i]=0x40;
		}
	for(i=0;i<2;i++)     //set udp source add
	{
		udp_frame[i]=0xA8;
	}
	for(i=2;i<4;i++)    //set udp desti add
	{
		udp_frame[i]=0xA9;
	}
	     //set udp length

		udp_frame[4]=0x00;
		udp_frame[5]=0x60;


	for(i=0;i<4;i++)      //push UDP source add
	{tx_frame[i+36]=udp_frame[i];
	}
	for(i=0;i<4;i++)      //push UDP desti add
	{tx_frame[i+40]=udp_frame[i+4];
	}
	int udp_group[10];
	for(i=0;i<12;i=i+2)     //checksum group
	{
		udp_group[j]=pse_frame[i]*0x100+pse_frame[i+1];
		j++;
	//	alt_printf("%x%x \n",pse_frame[4],pse_frame[5]);
	}
	for(i=0;i<6;i=i+2)     // checksum group
		{
			udp_group[j]=udp_frame[i]*0x100+udp_frame[i+1];
			//	alt_printf("%x\n",udp_group[j]);
				j++;
		}

	for(i=0;i<9;i++)     //set checksum
		{
			sum=sum+udp_group[i];
			//alt_printf("%x\n",sum);
		}
     flow=sum/0x10000;
     sum=sum-flow*0x10000;
     sum=flow+sum;
 //  printf("%x\n",sum);
     int first;
     first=sum/0x100;
     int second;
     second=sum-first*0x100;
     //set udp checksum

 	udp_frame[6]=first;
 	udp_frame[7]=second;
 //	printf("%x%x\n",udp_frame[6],udp_frame[7]);
 	//set udp data
    printf("input data");
 	scanf("%x%x%x%x%x%x%x%x",&udp_frame[8],&udp_frame[9],&udp_frame[10],&udp_frame[11],&udp_frame[12],&udp_frame[13],&udp_frame[14],&udp_frame[15]);

 	for(i=0;i<10;i++)   //set ip data
 	{
 		tx_frame[44+i]=udp_frame[6+i];
 	}
	//printf("input data 4 digits");
	//scanf ("%c%c%c%c",&udp_frame[47],&udp_frame[48],&udp_frame[49],&udp_frame[50]);
    //alt_printf("aaa");
	// Open the sgdma transmit device
	sgdma_tx_dev = alt_avalon_sgdma_open ("/dev/sgdma_tx");
	if (sgdma_tx_dev == NULL) {
		alt_printf ("Error: could not open scatter-gather dma transmit device\n");
		//return -1;
	} else alt_printf ("Opened scatter-gather dma transmit device\n");

	// Open the sgdma receive device
	//sgdma_rx_dev = alt_avalon_sgdma_open ("/dev/sgdma_rx");
	//if (sgdma_rx_dev == NULL) {
		//alt_printf ("Error: could not open scatter-gather dma receive device\n");
		//return -1;
	//} else alt_printf ("Opened scatter-gather dma receive device\n");



	// Set interrupts for the sgdma receive device
	//alt_avalon_sgdma_register_callback( sgdma_rx_dev, (alt_avalon_sgdma_callback) rx_ethernet_isr, 0x00000014, NULL );
	alt_avalon_sgdma_register_callback( sgdma_tx_dev, (alt_avalon_sgdma_callback) tx_ethernet_isr, 0x00000014, NULL );

	// Create sgdma receive descriptor
	//alt_avalon_sgdma_construct_stream_to_mem_desc( &rx_descriptor, &rx_descriptor_end, (alt_u32 *)rx_frame, 0, 0 );



	// Triple-speed Ethernet MegaCore base address
	volatile int * tse = (int *) TSE_BASE;

	// Specify the addresses of the PHY devices to be accessed through MDIO interface
	*(tse + 0x0F) = 0x10;

	// Disable read and write transfers and wait
	*(tse + 0x02) = *(tse + 0x02) | 0x00800220;
	while ( *(tse +  0x02) != ( *(tse + 0x02 ) | 0x00800220 ) );


	//MAC FIFO Configuration
	*(tse + 0x09 ) = TSE_TRANSMIT_FIFO_DEPTH-16;
	*(tse + 0x0E ) = 3;
	*(tse + 0x0D ) = 8;
	*(tse + 0x08 ) =TSE_RECEIVE_FIFO_DEPTH-16;
	*(tse + 0x0C ) = 8;
	*(tse + 0x0B ) = 8;
	*(tse + 0x0A ) = 0;
	*(tse + 0x08 ) = 0;

	// Initialize the MAC address
	*(tse + 0x03  ) = 0x0000CB4A;
	*(tse + 0x04 ) = 0x17231C00;

	// MAC function configuration
	*(tse + 0x05) = 1518;
	*(tse + 0x17) = 12;
	*(tse + 0x06) = 0xFFFF;
	*(tse + 0x02) = 0x00800220;


	// Software reset the PHY chip and wait
	*(tse + 0x02) =  0x00802220;
	while ( *(tse + 0x02 ) != ( 0x00800220 ) );

	// Enable read and write transfers, gigabit Ethernet operation and promiscuous mode
	
	*(tse + 0x02 ) = *(tse + 0x02) | 0x0080023B;

	while ( *(tse + 0x02) != ( *(tse + 0x02) | 0x0080023B ) );



	while (1) {

		print=in;
		in= IORD_ALTERA_AVALON_PIO_DATA(0x102490); //read the input from the switch
		IOWR_ALTERA_AVALON_PIO_DATA(LED_BASE, in); //switch on or switch off the LED

		if (in==1){
			if (print != in){
				alt_printf( "Switch on LED \n" );		
				alt_avalon_sgdma_construct_mem_to_stream_desc( &tx_descriptor, &tx_descriptor_end, (alt_u32 *)tx_frame, 62, 0, 1, 1, 0  );
					// Set up non-blocking transfer of sgdma receive descriptor
			    alt_avalon_sgdma_do_async_transfer( sgdma_tx_dev, &tx_descriptor );
					printf("Data input is :%x %x %x %x %x %x %x %x\n",tx_frame[46],tx_frame[47],tx_frame[48],tx_frame[49],tx_frame[50],tx_frame[51],tx_frame[52],tx_frame[53]);
					//Wait until tramsmitter descriptor transfer is complete
						while (alt_avalon_sgdma_check_descriptor_status(&tx_descriptor) != 0)
				;
			}
		}
		else{
			if (print != in) {
				alt_printf( "Switch off LED \n" );
				printf("Checksum: %x%x ", tx_frame[44],tx_frame[45]);
//				printf("The number of receive frames with CRC error.%d\n",*(tse+0x1C));
			}
		}
	}

	return 0;
}

/****************************************************************************************
 * Subroutine to read incoming Ethernet frames
****************************************************************************************/
void tx_ethernet_isr (void *context)
{

	//Include your code to show the values of the source and destination addresses of the received frame. For example:
	if(in){
		alt_printf( "transmitting...: \n"  );
	//	alt_printf( "Destination address:  \n" );

	}



	// Wait until receive descriptor transfer is complete
	//while (alt_avalon_sgdma_check_descriptor_status(&rx_descriptor) != 0)
		;

	// Create new receive sgdma descriptor
	//alt_avalon_sgdma_construct_stream_to_mem_desc( &rx_descriptor, &rx_descriptor_end, (alt_u32 *)rx_frame, 0, 0 );

	// Set up non-blocking transfer of sgdma receive descriptor
	//alt_avalon_sgdma_do_async_transfer( sgdma_rx_dev, &rx_descriptor );
}
