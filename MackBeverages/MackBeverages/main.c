/*
 * MackBeverages.c
 *
 * Created: 12/21/2017 8:11:58 PM
 * Author : MWESIGYE0704066878
 */ 
/**
	Bwayo Jude					15/U/4718/EVE
	Namatovu Damalie			15/U/886
	Mwesigye Robert				15/U/771
	Muyambi Julius				15/U/8610/PS
	Mawanda Henry				15/U/7496/PS
	Nansubuga Joyce Euzebia		15/U/10807/EVE
	Mukiibi Kelly Alvin			15/U/689
	Noah Kusasira				12/U/7421/EVE
*/

//avr includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include<avr/eeprom.h>
#include <util/delay.h>
// c includes
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
//usart communication
/*
	1. using Atmega128
	2. F_CPU 8MHz
	3. Baud rate 9600
*/

//---F_CPU and Baud rate---//
#define F_CPU 8000000UL
#define  FOCS 8000000	//--the frequency of the clock
#define BAUD  9600		//--the baud rate
#define  MYUBRR (FOCS/16/BAUD-1)	//--value for UBRR

//---constants---//
#define TOTAL_FRUITS 3000
#define TX_NEW_LINE {usart_transmitByte(0x0d); usart_transmitByte(0x0a);}
//--- prototypes ---//
void usart_init(unsigned int ubrr);									//--usart0 initialisation
void usart_transmitByte(char data);									//--usart0 transmit
char usart_receiveByte(void);										//--usart0 receive
void usart_transmit_message(char *message);							//--usart0 transmit string
char *usart_receive_string(void);									//--usart0 receive string
void usart_transmit_string(char *string);							//--usart0 transmit received string

void init_external_INT4(void);										//--initialize and enable external interrupts on INT4
void init_external_INT5(void);										//--initialize and enable external interrupts on INT5

void start_timer0(void);											//--start timer0 in normal mode
void start_timer1(void);											//--start_timer1 in ctc mode
void stop_timer1(void);												//--stop_timer1
void start_timer3(void);											//--start timer3 in CTC mode
void stop_timer3(void);												//--stop timer3
void start_main_timer(void);										//--start the main timer for the plant HH : MM : SS
void start_pausing_timer(void);										//--count the time for pausing the system
void stop_pausing_timer(void);										//--stop the pausing timer, when system is running it should not be counting

unsigned int convert_to_integer_value(char *string);				//--get integer from character array
char *get_string_from_int(int integer_value);						//--get string from int
char *get_string_from_float(float float_number, int afterpoint);	//--get string from floats to the specified dp
int intToStr(int x, char str[], int d);								//--used by get_string_from_float
void reverse(char *str, int len);									//--function to  reverses a string 'str' of length 'len' used by intToStr
int convert_numeric_char_to_int(char numeric_char);					//--convert numeric char to int
void message(char message[]);										//--transmit message to the terminal
void error_msg(char error[]);										//--transmit error message to the terminal
void warning_msg(char warning[]);									//--transmit warning message to the terminal

int calculate_number_of_mixed_fruits(int percentage);				//--compute the number of fruits in the entered percentages
int left_fruit(int fruit_mixed, int orignal_fruit);					//--compute the number of fruits left after mixing for a certain fruit
float get_volume_in_bottles(unsigned int bottle_capacity, unsigned int bottles);	//--get the volume in the given number of bottles of a given bottle capacity
unsigned int get_bottles_in_volume(float volume, unsigned int bottle_capacity);		//--get the number of bottles in a given volume of drink
float get_left_volume_in_tank(float original_volume, float dispensed_volume);		//--get left volume after setting bottles to fill with the drink
void capture_fruits_in_store(void);													//--enter the fruits in store
void capture_fruits_percentage(void);												//--enter the percentages for all the 4 fruits to be mixed
void confirm_number_of_fruits(int percentage, int fruits_in_store, int *mixed_fruit); //--confirm that the mixed fruits make up to 3000 fruits = 150l
void set_total_liters_in_tank(unsigned int *total_mixed_fruits);					 //--configure liters in the tank
void get_number_of_bottles(float volume_in_tank);									//--function to capture number of bottles to dispense with the drink
void set_bottle_capacity(void);														//--choose a bottle capacity
void operation_menu(void);															//--main menu for the operation
void change_bottle_capacity(void);													//--change the boottle capacity
void show_entire_operation_time(void);												//--show time the plant has worked
void show_time_for_pausing(void);													//--show time the plant has rested
void show_left_fruits_in_store(void);												//--show the left fruits in store after mixing
void show_liters_in_storage_tank(void);												//--show the left volume in the tank
void show_bottles_filled(void);														//show the boottles filled with the drink since dispension started	

void on_system_restart(void);														//--ask operator for new operation or start from previous settings
void save_system_settings(void);													//--save system settings to eeprom.
void recover_system_settings(void);													//recover system settings
void pause_system(void);															//--pause the system
void resume_system(void);															//--resume the system	
void enable_stepper_motor_drive(void);												//--enable motor drive
void disable_stepper_motor_drive(void);												//--disable motor drive				
void drive_stepper_motor(void);														//--drive stepper motor	
void stop_stepper_motor(void);														//--stop the stepper motor				

void test_action(void);																//testing the success of an action
	

//---global declarations---//
unsigned char main_option;						//for the menus
char received_string[20];						//---the string received from the virtual terminal
unsigned int integer_value;						//--the integer value from a string
char string_from_int_buffer[10];				//--will hold the string got from an int
char string_from_float_buffer[20];				//--will hold the string got from a float
int leftt_percentage = 100;						//--the total percentage = 100% = 3000 fruits = 150 litres
unsigned int total_mixed_fruits = 0;			//--The total mixed fruits NOTE at the end of percentage entry this value should be 3000
float total_litres_mixed = 0;					//--the total litres mixed in the tank


char *input_percentage;				//new input percentage but still a string
int new_percentage = 0;				//new percentage in case the previously entered percentage would not accommodate the available fruit in store
//fruit in store captured from user input
char *mangoes;
char *oranges;
char *pineapples;
char *guavas;
char *apples;
//fruits in store
unsigned int guavas_in_store;
unsigned int mangoes_in_store;
unsigned int oranges_in_store;
unsigned int pineapples_in_store;
unsigned int apples_in_store;
//these hold the percentage entered as a string from the virtual terminal for each fruit
char *mangoes_percentage;
char *oranges_percentage;
char *pineapples_percentage;
char *guavas_percentage;
char *apples_percentage;
//these are the fruit percentages
unsigned int percentage_of_oranges = 0;
unsigned int percentage_of_mangoes = 0;
unsigned int percentage_of_pineapples = 0;
unsigned int percentage_of_apples = 0;
unsigned int percentage_of_guavas = 0;

//these are the fruits in the mixture
int mixed_guavas = 0;
int mixed_oranges = 0;
int mixed_mangoes = 0;
int mixed_pineapples = 0;
int mixed_apples = 0;

//these are the fruits left after mixing
int left_mangoes;
int left_oranges;
int left_guavas;
int left_pineapples;
int left_apples;

//the bottles section variables
unsigned int bottle_capacity;								//the bottle capacity
unsigned int entered_bottles;								//the number of bottles entered
float entered_volume = 0; 									//this holds the volume worth the entered bottles in mls
volatile unsigned int bottles_to_dispense = 0;				//the bottles that will be filled with the drink
float volume_to_dispense;									//this will be the volume to be dispensed after everything is set
float volume_left_in_tank = 0; 								//this will be the volume left in the tank when all the bottles_to_dispense are filled;
volatile unsigned int bottles_filled = 0;					//this is the number of bottels filled so far with the drink since dispension started
volatile float left_volume_in_tank;							//this is the volume in the tank after each bottle is dispensed
/**
	Timer variables
*/
//main timer
volatile unsigned int main_seconds = 0;						//the counter for the seconds of the main timer
volatile unsigned int main_minutes = 0;						//the counter for the minutes of the main timer
volatile unsigned int main_hours = 0;						//the counter for the hours of the main timer
//paused timer
volatile unsigned int paused_seconds = 0;					//the counter for the seconds when system is paused
volatile unsigned int paused_minutes = 0;					//the counter for the minutes when system is paused
volatile unsigned int paused_hours = 0;						//the counter for the hours when the system is paused
volatile unsigned int total_timer0_overflows;				//counts the total over flows for timer 0

//motor timer controller
volatile int motor_drive_control = 0;						//this variable controls the driving of the motor by seconds that have elapsed

//the settings block struct
struct Settings_t{
	unsigned int main_hours_;
	unsigned int main_minutes_;
	unsigned int main_seconds_;
	
	unsigned int paused_hours_;
	unsigned int paused_minutes_;
	unsigned int paused_seconds_;
	
	uint8_t motor_port_;
	uint8_t countdown_timer_port_;
	uint8_t led_port_;
	uint8_t button_pins_;
	
	unsigned int left_apples_;
	unsigned int left_guavas_;
	unsigned int left_mangoes_;
	unsigned int left_oranges_;
	unsigned int left_pineapples_;
	
	unsigned int bottles_to_dispense_;
	unsigned int bottles_filled_;
	unsigned int bottle_capacity_;
	float left_volume_in_tank_;
	int motor_drive_control_;
	unsigned int total_timer0_overflows_;
	
}settings;
/*********************************************************************
///////////////////////////////////////////////////////////////////////
							THE MAIN PROGRAM
//////////////////////////////////////////////////////////////////////
********************************************************************/
int main(void){
	/***************************************************
		start the  timers from this point
	***************************************************/
	start_timer0();										//start the timer for the motor
	start_timer1();										//start the main timer for the plant
	start_timer3();										//start the timer for pausing the system though not enabled not until start_pausing_timer() is called
	
	/**************************************************
		initialize external interrupts
	**************************************************/
	init_external_INT4();								//pausing and resuming the system
	init_external_INT5();								//starting to fill the bottles, actually driving the motor
	
	/**************************************************
		initialize usart for the virtual terminal
	**************************************************/
	usart_init(MYUBRR);									//--initializing USART
	
	/***************************************************
		SETTING UP DDR REGISTERS
	***************************************************/
	
/**STATUS LEDS*/ 
	/*	
		************PORTA**********
		LED 0 system paused			  (PA0)
		LED 1 system dispensing juice (PA1)
		LED 2 system is on 			  (PA2)
	*/
	DDRA |= (1 << PA0) | (1 << PA1) | (1 << PA2);	//set pin0 to 2 for output on port A 
	
/**BUTTON AND SWITCHES*/ 
	/*
		BUTTON for pausing and resuming system PE4
		BUTTON for starting the motor PE5
	*/
		PINE |= (1 << PE4) | (1 << PE5);					    //first make sure pin4 an 5 are not input	
	
/** put the system ON LED on***/
	PORTA |= (1 << PA2);
	
/**STEPPER MOTOR**/
	DDRC = 0xFF;								//use entire port C for output
	
/**COUNT-DOWN TIMERS**/
	DDRF = 0xFF;								//use entire port F for output
	
	usart_transmit_message("Initializing Mack beverages....");	//--send a set of characters (string)
	_delay_ms(3000);
	
	message("############## MACK BEVERAGES #############");		//--welcome message
	TX_NEW_LINE;												//--command to jump to next line
	on_system_restart();										//start of operation
	//main loop
	while(1){
		
		operation_menu();			//display the operation menu to the operator
	
	}//end main loop
}

/*////////////////////////////////////////////////////////////////////////////////
							EXTERNAL INTERRUPTS FUNCTIONS
////////////////////////////////////////////////////////////////////////////////*/

//function to initialize and enable external interrupts on INT4 pin
void init_external_INT4(void){
	/*
		THIS FUNCTION ENABLES INT4 AN INTERRUPT IS FIRED FOR BOTH LEVELS (low and high)
		THE BUTTON ON PE4 (port E) IS USED AS A SWITCH TO PAUSE AND RESUME THE SYSTEM
		HIGH LEVEL IS FOR RESUME AND LOW LEVEL IS FOR PAUSE
	*/
	
	/*** set the way how the interrupt will be caused***/
	EICRB |= (1 << ISC40);		//enable interrupts due to logical change on the INT4 pin
	
	/*** enable interrupts on INT4***/
	EIMSK |= (1 << INT4);
	
	sei();						//enable global interrupts
	
	
}
//function to initialize and enable external interrupts on INT5 pin
void init_external_INT5(void){
	/*
		THIS FUNCTION ENABLES INT5, AN INTERRUPT IS FIRED FOR BOTH LEVELS (low and high)
		THE BUTTON ON PE5 (port E) IS USED AS A SWITCH TO START AND RESUME THE SYSTEM
		HIGH LEVEL IS FOR STOPPING TO DISPENSING JUICE AND LOW LEVEL IS FOR STARTING TO
		DISPENSE JUICE
	*/
	
	/*** set the way how the interrupt will be caused***/
	EICRB |= (1 << ISC50);		//enable interrupts due to logical change on the INT5 pin
	
	/*** enable interrupts on INT5***/
	EIMSK |= (1 << INT5);
	
	sei();						//enable global interrupts
}

/*
///////////////////////////////////////////////////////////////////////////////
							USART0 functions
///////////////////////////////////////////////////////////////////////////////
*/
//function for initialization of usart0
void usart_init(unsigned int ubrr){
	/*set the baud rate*/
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	/*enable receiver and transmitter*/
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);		//--Transmit and receive enable for usart 0
	UCSR0B &= ~(1 << UCSZ02);					//set the 8 bit data mode
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);	//--set 8 bit data
	UCSR0C |= (1 << USBS0);						//two stop bits
	
}
//function to transmit through usart0
void usart_transmitByte(char data){
	UDR0 = data;								//--move the data to the UDR data register
	while(!(UCSR0A & (1 << UDRE0)));			//--check whether UDR is empty
}
//function to receive through usart0
char usart_receiveByte(void){
	while(!(UCSR0A & (1 << RXC0)));			//--check whether receive is complete
	return(UDR0);							//return data from UDR
}
//function to transmit a string
void usart_transmit_message(char *message){
	while(*message != 0){					//--check null
		usart_transmitByte(*message++);		//--increment the pointer
	}
}
//function to transmit the string to the terminal
void usart_transmit_string(char *string){
	while(*string){
		UDR0 = *string++;
		while(!(UCSR0A & (1 << UDRE0)));
	}
}
//function to receive the string from the terminal
char *usart_receive_string(){
	char received_character;
	int char_counter = 0;
	while((received_character = usart_receiveByte()) != 13){	//receive the characters until enter is pressed ascii for enter is 13
		received_string[char_counter++] = received_character;
	}
	received_string[char_counter] = '\0';						//terminate the string
	
	return received_string;
}

/*
///////////////////////////////////////////////////////////////////////
							TIMER FUNCTIONS
//////////////////////////////////////////////////////////////////////
*/
//function to start timer0 in normal mode 
void start_timer0(void){
	total_timer0_overflows = 0;
	TCNT0 = 0x00;							//--initialize the counting register
	TCCR0 |= (1 << CS02) | (1 << CS01);		//--start the oscillator with prescaler of 256.
	
	sei();									//--enable global interrupts.
}
//function to stop timer0
void stop_timer0(void){
	/**
		COMPLETELY STOP TIMER/CONTER0
	*/
	TCCR0 = 0X00;			//stop the oscillator immediately for timer 0
	
	TCNT0 = 0x00;			//clear the counter for timer0
	
	TIMSK &= ~(1 << TOIE0);	//disable interrupts on timer/counter0 overflow
	/*****reset the pausing timer so that u start afresh when u pause the system again****/
	paused_seconds = 0;			//set seconds to 0
	paused_minutes = 0;			//set minutes to 0
	paused_hours = 0;			//set hours to 0
}
//function to start timer 1 and initialize it in ctc mode
void start_timer1(){
	TCNT1 = 0x0000;				//--initialize the counter
	TCCR1A |= 0x00;				//--not required since WGM11 and WGM10 = 0 in CTC mode
	TCCR1B |= (1 << WGM12);		//--set CTC mode
	TCCR1B |= (1 << CS12);		//--set prescaler of 256
	
	OCR1A = 0x7A11;				//--load the compare match register with 31249dec which gives a delay of 1 second
	
	TIMSK |= (1 << OCIE1A);		//--enable interrupts on compare match
	sei();						//--enable global interrupts
}
//function to stop timer1
void stop_timer1(void){
	TCCR1B = 0x00;				//immediately stop the timer
	
	TCNT1 = 0x0000;				//clear the counter
	OCR1A = 0x0000;				//clear the compare match register for timer 1
	
	TIMSK &= ~(1 << OCIE1A);	//disable interrupts on compare match
	/*****reset the main timer so that u start afresh when u start timing the system again****/
	main_seconds = 0;			//set seconds to 0
	main_minutes = 0;			//set minutes to 0
	main_hours = 0;				//set hours to 0
}
//function to start timer 3 and initialize it in ctc mode
void start_timer3(void){
	/**
		THIS TIMER USES TIMER/COUNTER3 AND INCREMENTS BY 1 SECOND GENERATED IN CTC MODE 
		BY PLACING 0x7A11 = 31249dec IN OCR3A TO GENERATE A DELAY OF 1 SECOND
		SECONDS ARE COUNTED BY THE INTERRUPT ON COMPARE MATCH 
	
	 */
	
	TCNT3 = 0x0000;				//--initialize the counting register
	TCCR3A |= 0x00;				//--not required since WG31 and WGM30 = 0 in CTC mode
	TCCR3B |= (1 << WGM32);		//--set CTC mode for output compare ie WGM33:30 = 4dec
	TCCR3B |= (1 << CS32);		//set a prescaler of 256 and start the oscillator
	
	OCR3A = 0x7A11;				//--load the output compare register with 31249dec which gives a delay of 1 second
	/*
		first disable the interrupts on compare match not until you decide to start the motor by calling
		enable_stepper_motor_drive() function
	*/
	ETIMSK &= ~(1 << OCIE3A);	
	 
	sei();						//--enable global interrupts
}
//function to stop timer3
void stop_timer3(){
	/**
		COMPLETELY STOP TIMER/CONTER3
	
	 */
	TCCR3B = 0x00;				//immediately stop the oscillator for timer 3
	
	TCNT3 = 0x0000;				//clear the counter for timer3
	OCR3A = 0x0000;				//clear the output compare register for timer3
	
	ETIMSK &= ~(1 << OCIE3A);	//disable interrupts on compare match
	/*****reset the motor controller vaeriable so that u begin driving the motor afresh the next time u start the timer3****/
	motor_drive_control = 0;			//set the motor drive controller variable to 0
}

//function for counting the main time this timer is accurate to the nearest second
void start_main_timer(void){
	//count the minutes
	if (main_seconds >= 59)
	{
		main_seconds = 0;	//reset the seconds for the main timer
		main_minutes++;		//count it as 1 minute actually 60s = 1minute
	}
	//count the hours
	if(main_minutes >= 59){
		main_minutes = 0;	//reset the minutes for the main timer
		main_hours++;		//count it as an hour actually 60min = 1hr
	}
	
}
//function for counting the time when the system is paused this timer is accurate to the nearest second
void start_pausing_timer(void){
	/**
		START THE PAUSING TIMER BY ENABLING INTERRUPTS ON TIMER0 overflow
	 */
	 TIMSK |= (1 << TOIE0);	//--enable interrupts on overflow in TCNT0
	 
	 /******turn off the LED that shows system dispensing juice status**********/
	 PORTA &= ~(1 << PA1);
	 
	 /* i suppose to read the time from eeprom
	   such that we start from where we last paused but,
	   this implementation will be in the future lets
	   first make sure we can start and stop the pausing timer
	*/
	
}
//function to stop the pausing timer, this timer stops immediately the system is running and started when the system is paused
void stop_pausing_timer(void){
	/**
		STOP THE PAUSING TIMER BY DISABLING INTERRUPTS ON TIMER0 OVERFLOW
	 */
	TIMSK &= ~(1 << TOIE0);	//disable the interrupts due to timer0 overflow
	
	total_timer0_overflows = 0;		//reset the total overflows back to 0
	/******turn off the LED that shows system paused status**********/
	PORTA &= ~(1 << PA0);
	/******turn on the LED that shows system dispensing juice status**********/
	
	
	/* 
		i suppose for now that once we stop the pausing timer we start afresh when we pause the system again
		you can uncomment out the reset pausing timer section if u want to start from where u ended timming the pausing period
	*/
	/*****reset the pausing timer so that u start afresh when u pause the system again****/
	// paused_seconds = 0;			//set seconds to 0
	// paused_minutes = 0;			//set minutes to 0
	// paused_hours = 0;			//set hours to 0
	
	/* i suppose to save the time to eeprom
	   such that we start from where we last paused but,
	   this implementation will be in the future lets
	   first make sure we can start and stop the pausing timer
	*/
} 

/*
///////////////////////////////////////////////////////////////////////
							UTILITY FUNCTIONS
//////////////////////////////////////////////////////////////////////
*/
//function to send a message to the terminal
void message(char message[]){
	TX_NEW_LINE;	//go to new line
	usart_transmit_message(message);	//transmit the message
}
//function to print an error due to user input
void error_msg(char error[]){
	TX_NEW_LINE;	//go to new line
	usart_transmit_message("ERROR: ");
	usart_transmit_message(error);	//transmit the error
}
//function to print a warning
void warning_msg(char warning[]){
	TX_NEW_LINE;	//go to new line
	usart_transmit_message("WARNING: ");
	usart_transmit_message(warning);	//transmit the warning
}
//function to get string from integer value
char *get_string_from_int(int integer_value){
	itoa(integer_value, string_from_int_buffer, 10);
	return string_from_int_buffer;
}
//function to get a string from a float
char *get_string_from_float(float float_number, int afterpoint){
	// Extract integer part
	int int_part = (int)float_number;
	
	// Extract floating part
	float float_part = float_number - (float)int_part;
	
	// convert integer part to string
	int i = intToStr(int_part, string_from_float_buffer, 1);
	
	// check for display option after point
	if (afterpoint != 0)
	{
		string_from_float_buffer[i] = '.';  // add dot
		
		// Get the value of fraction part upto given no.
		// of points after dot. The third parameter is needed
		// to handle cases like 233.007
		float_part = float_part * pow(10, afterpoint);
		
		intToStr((int)float_part, string_from_float_buffer + i + 1, afterpoint);
	}
	return string_from_float_buffer;
}
// Converts a given integer x to string str[].  d is the number of digits required in output. If d is more than the number of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d){
	int i = 0;
	while (x)
	{
		str[i++] = (x%10) + '0';
		x = x/10;
	}
	
	// If number of digits required is more, then
	// add 0s at the beginning
	while (i < d)
	str[i++] = '0';
	
	reverse(str, i);
	str[i] = '\0';
	return i;
}
//function to  reverses a string 'str' of length 'len'
void reverse(char *str, int len){
	int i = 0, j = len-1, temp;
	while (i<j)
	{
		temp = str[i];
		str[i] = str[j];
		str[j] = temp;
		i++; j--;
	}
}
//function to change a numeric char to int
int convert_numeric_char_to_int(char numeric_char){
	if(isdigit(numeric_char)){
		return (numeric_char - '0');
	}
	else{
		return -1;
	}
}
//function to get integer from character array
unsigned int convert_to_integer_value(char *string){
	int counter = 0;
	char int_string[strlen(string)];
	for(counter = 0; counter < strlen(string); counter++){	/* loop thru the entered strings*/
		if(isdigit(string[counter])){						/* pick out the digit value */
			int_string[counter] = string[counter];
		}
	}
	integer_value = atoi(int_string);		// convert the integer string to integer
	return integer_value;
}
//this function is helping to trace an error...
void test_action(){
	DDRB |= (1 << PB0);
	PORTB |= (1 << PB0);	//turn on
	_delay_ms(3000);
	PORTB &= ~(1 << PB0);	//turn off
	
}

/*
///////////////////////////////////////////////////////////////////////
							FRUITS FUNCTION
//////////////////////////////////////////////////////////////////////
*/
//function to calculate the number of fruits
int calculate_number_of_mixed_fruits(int percentage){
	return (float)((percentage) / (float)100) * TOTAL_FRUITS;
}
//function to calculate the fruits left in store after mixing
int left_fruit(int fruit_mixed, int orignal_fruit){
	return (orignal_fruit - fruit_mixed);
}
//function to get the volume worth the entered number of bottles
float get_volume_in_bottles(unsigned int bottle_capacity, unsigned int bottles){
	return ((float)bottle_capacity * (float)bottles);
}
//function to get the number of bottles in the entered volume;
unsigned int get_bottles_in_volume(float volume, unsigned int bottle_capacity){
	return (volume / (float)bottle_capacity);
}
//function to get the left volume in the tank after dispensing
float get_left_volume_in_tank(float original_volume, float dispensed_volume){
	return (original_volume - dispensed_volume);
}
//function to confirm that the number of fruits in store are enough
void confirm_number_of_fruits(int percentage, int fruits_in_store, int *mixed_fruit){
	
	*mixed_fruit = calculate_number_of_mixed_fruits(percentage);
	do{
		if(*mixed_fruit > fruits_in_store){
			//enter a lower %
			error_msg("please enter a lower percentage to  ");
			message("accommodate the available fruits in store:>> ");
			input_percentage = usart_receive_string();	//get another percentage from the user input as a string
			new_percentage = convert_to_integer_value(input_percentage);	//convert string percentage to int;
			percentage = new_percentage;
			mixed_fruit = calculate_number_of_mixed_fruits(new_percentage);//calculate the mixed fruit again using the newly entered percentage
		} 
	} while(*mixed_fruit > fruits_in_store);
	
	leftt_percentage = leftt_percentage - percentage;
	//print the remaining %
	usart_transmit_message("The remaining percentage to make 100% is: ");
	usart_transmit_message(get_string_from_int(leftt_percentage)); usart_transmit_message("%");//add the % symbol
}
//function to enter the fruits in store
void capture_fruits_in_store(){
	message("       ENTER THE NUMBER OF  EACH FRUIT AVAILABLE");
	int fruit_entry = 0;											//this is the number of times i enter the fruits
	while(fruit_entry < 5){
		
		if(fruit_entry == 0){
			message(" Please enter number of Mangoes in store: ");
			mangoes = usart_receive_string();						//enter the number of mangoes in store
			mangoes_in_store = convert_to_integer_value(mangoes);	//convert to integer
			fruit_entry++;
		}
		else if(fruit_entry == 1){
			message(" Please enter number of Oranges in store: ");
			oranges = usart_receive_string();						//enter the number of oranges in store
			oranges_in_store = convert_to_integer_value(oranges);	//convert to integer
			fruit_entry++;
		}
		else if(fruit_entry == 2){
			message(" Please enter number of Guavas in store: ");
			guavas = usart_receive_string();						//enter the number of guavas in store
			guavas_in_store = convert_to_integer_value(guavas);		//convert to integer
			fruit_entry++;
		}
		else if(fruit_entry == 3){
			message(" Please enter number of Pineapples in store: ");
			pineapples = usart_receive_string();						//enter the number of pineapples in store
			pineapples_in_store = convert_to_integer_value(pineapples);	//convert to integer
			fruit_entry++;
		}
		else if(fruit_entry == 4){
			message(" Please enter number of Apples in store: ");
			apples = usart_receive_string();							//enter the number of apples in store
			apples_in_store = convert_to_integer_value(apples);			//convert to integer
			fruit_entry++;
		}
		else{
			fruit_entry++;
		}
		
	}
	//before we mix the fruit, left fruit of each kind in equal to the entered fruit in store
	left_apples = apples_in_store;
	left_guavas = guavas_in_store;
	left_mangoes = mangoes_in_store;
	left_oranges = oranges_in_store;
	left_pineapples = pineapples_in_store;
}
//function to capture the fruit percentages
void capture_fruits_percentage(){
	int option = -1;
	int total_entries = 0;			//keeps track of the number of fruits in the mixture, they should be four
	char input_option;				//the option input from the terminal
	message("           ENTER THE PERCENTAGE OF EACH FRUIT");
	while(option != 0){
		//menu
		TX_NEW_LINE;
		message(" 1. percentage of mangoes");
		message(" 2. percentage of oranges");
		message(" 3. percentage of pineapples");
		message(" 4. percentage of apples");
		message(" 5. percentage of guavas");
		message(" 0. to exit entry of fruit percentages");
		
		message(" Choose a fruit percentage. Enter 0 to stop>> ");
		input_option = usart_receiveByte();					//get the option from user input
		_delay_ms(1000);									//wait a bit
		option = convert_numeric_char_to_int(input_option);	//change entered char to integer
		
		if(total_entries < 4){
			switch(option){
				case 1:{
					//make sure you don't enter the same fruit more than once
					if(percentage_of_mangoes == 0){
						/* ensure the percentage is correct*/
						do{
							
							message("Enter the percentage of mangoes:>> ");
							mangoes_percentage = usart_receive_string();							//get the percentage of mangoes from the terminal as a string
							percentage_of_mangoes = convert_to_integer_value(mangoes_percentage);	//convert string percentage to int
							if(percentage_of_mangoes > 100){
								error_msg("percentage ends at 100, please try again");
							}
							else{
								mixed_mangoes = calculate_number_of_mixed_fruits(percentage_of_mangoes);	//get mixed mangoes
								confirm_number_of_fruits(percentage_of_mangoes, mangoes_in_store, &mixed_mangoes);//the failure is due to this function
							}
						} while(percentage_of_mangoes > 100);
						left_mangoes = left_fruit(mixed_mangoes, mangoes_in_store);			//get the left mangoes after mixing
						//get the total number of fruits in the mixture
						total_mixed_fruits += mixed_mangoes;
						total_entries++;
						} else {
						warning_msg("You have already entered the percentage of"); message("mangoes! Choose another fruit");
					}
					
				}
				break;
				case 2:{
					//make sure you don't enter the same fruit more than once
					if(percentage_of_oranges == 0){
						/* ensure the percentage is correct*/
						do{
							message("Enter the percentage of oranges:>> ");
							oranges_percentage = usart_receive_string();	//get percentage of oranges from terminal as string
							percentage_of_oranges = convert_to_integer_value(oranges_percentage); //convert string percentage to int
							if(percentage_of_oranges > 100){
								error_msg("percentage ends at 100, please try again");
							}
							else{
								mixed_oranges = calculate_number_of_mixed_fruits(percentage_of_oranges);	//get mixed oranges
								confirm_number_of_fruits(percentage_of_oranges, oranges_in_store, &mixed_oranges);
							}
						} while(percentage_of_oranges > 100);
						left_oranges = left_fruit(mixed_oranges, oranges_in_store);	//get the left oranges after mixing
						//get the total number of fruits in the mixture
						total_mixed_fruits += mixed_oranges;
						total_entries++;
						} else {
						warning_msg("You have already entered the percentage of"); message("oranges! Choose another fruit");
					}
				}
				break;
				case 3:{
					if(percentage_of_pineapples == 0){
						/* ensure the percentage is correct*/
						do{
							message("Enter the percentage of pineapples:>> ");
							pineapples_percentage = usart_receive_string();	//get percentage of pineapples from terminal as string
							percentage_of_pineapples = convert_to_integer_value(pineapples_percentage); //convert string percentage to int;
							if(percentage_of_pineapples > 100){
								error_msg("percentage ends at 100, please try again");// i stopped here!!!!!!!
							}
							else{
								mixed_pineapples = calculate_number_of_mixed_fruits(percentage_of_pineapples);	//get mixed pineapples
								confirm_number_of_fruits(percentage_of_pineapples, pineapples_in_store, &mixed_pineapples);
							}
						} while(percentage_of_pineapples > 100);
						left_pineapples = left_fruit(mixed_pineapples, pineapples_in_store);	//get the left pineapples after mixing
						//get the total number of fruits in the mixture
						total_mixed_fruits += mixed_pineapples;
						total_entries++;
						} else {
						warning_msg("You have already entered the percentage of"); message("pineapples! Choose another fruit");
					}
				}
				break;
				case 4:{
					if(percentage_of_apples == 0){
						/* ensure the percentage is correct*/
						do{
							message("Enter the percentage of apples:>> ");
							apples_percentage = usart_receive_string();	//get percentage of apples from terminal as string
							percentage_of_apples = convert_to_integer_value(apples_percentage); //convert string percentage to int;
							if(percentage_of_apples > 100){
								error_msg("percentage ends at 100, please try again");
							}
							else{
								mixed_apples = calculate_number_of_mixed_fruits(percentage_of_apples);
								confirm_number_of_fruits(percentage_of_apples, apples_in_store, &mixed_apples);
							}
						} while(percentage_of_apples > 100);
						left_apples = left_fruit(mixed_apples, apples_in_store);	//get the left apples after mixing
						//get the total number of fruits in the mixture
						total_mixed_fruits += mixed_apples;
						total_entries++;
						} else {
						warning_msg("You have already entered the percentage of"); message("apples! Choose another fruit");
					}
				}
				break;
				case 5:{
					if(percentage_of_guavas == 0){
						/* ensure the percentage is correct*/
						do{
								message("Enter the percentage of guavas:>> ");
								guavas_percentage = usart_receive_string();	//get percentage of guavas from terminal as string
								percentage_of_guavas = convert_to_integer_value(guavas_percentage); //convert string percentage to int;
								if(percentage_of_guavas > 100){
									error_msg("percentage ends at 100, please try again");
								}
								else{
									mixed_guavas = calculate_number_of_mixed_fruits(percentage_of_guavas);
									confirm_number_of_fruits(percentage_of_guavas, guavas_in_store, &mixed_guavas);//stopped here use pass by ref
								}
							} while(percentage_of_guavas > 100);
						left_guavas = left_fruit(mixed_guavas, guavas_in_store);	//get the left guavas after mixing
						//get the total number of fruits in the mixture
						total_mixed_fruits += mixed_guavas;
						total_entries++;
						} else {
						warning_msg("You have already entered the percentage of"); message("guavas! Choose another fruit");
					}
				}
				break;
				default:
					error_msg("pick among the options 0 - 5 only");
				break;
			}//end switch
			
			} else{
				warning_msg("The mixture should have four different"); message("fruits and you have already entered them.");
				TX_NEW_LINE;
				break;
		}
		
	}//end while
	
}
//function to set the total number of liters in the tank
void set_total_liters_in_tank(unsigned int *total_mixed_fruits){
	//ensure that the mixture has 3000 fruits = 150 liters
	do{
		if(*total_mixed_fruits != TOTAL_FRUITS){
			error_msg("The total liters  should be 150 ltr."); message("please enter the fruit percentages correctly again");
			warning_msg("You will not be able to proceed if you"); message(" don't enter all the 4 different fruits per mixture.");
			TX_NEW_LINE;
			//reinitialize the percentage of each fruit so that we enter everything afresh
			percentage_of_apples = 0;
			percentage_of_guavas = 0;
			percentage_of_mangoes = 0;
			percentage_of_oranges = 0;
			percentage_of_pineapples = 0;
			leftt_percentage = 100;
			*total_mixed_fruits = 0;
			capture_fruits_percentage();	//enter the fruits percentages again
		}
	} while(*total_mixed_fruits != TOTAL_FRUITS);
	total_litres_mixed = 150; 				//set the 150 liters = 150000 mls
	left_volume_in_tank = 150000;			//set the left volume in the tanks
	message("You have successfully mixed 150 liters of the drink.");
	message("Please proceed and configure the size"); message("of the bottle to fill with the drink.");
	TX_NEW_LINE;TX_NEW_LINE;	//put  empty line here
}
//function to set the bottle capacity
void set_bottle_capacity(){
	char bottle_option;
	message("CHOOSE A BOTTLE CAPACITY TO FILL WITH THE DRINK");
	do{
		//menu
		message(" 1. 500 ml bottle.");
		message(" 2. 300 ml bottle.");
		TX_NEW_LINE;
		message("Choose a bottle capacity>> ");
		bottle_option = usart_receiveByte();	//get an option from terminal
		switch(bottle_option){
			case '1':{
				bottle_capacity = 500;	//set bottle capacity to 500 ml
				message("bottle capacity of 500 ml is set");
				
			}
			break;
			case '2':{
				bottle_capacity = 300;	//set bottle capacity to 300 ml
				message("bottle capacity of 300 ml is set");
			}
			break;
			default:{
				bottle_option = '0';	//reset the bottle option
				error_msg("Choose either 1 or 2.");
			}
			break;
		}//end switch
		
	}
	while(bottle_option == '0');
}
//function to capture number of bottles to dispense with the drink
void get_number_of_bottles(float volume_in_tank){
	char *bottles_entered;	//this holds the string read from the terminal as entered bottles
	//1 ask operator to enter number of bottles
	//2 calculate the volume worth the entered bottles (entered volume)
	//3 compare the entered volume with the volume in the tank
	//if volume in tank is enough (ie >= ), dispense that volume (ie set volume to dispense = entered volume)and
	//show the volume that will be left
	//4 else if volume in tank is < than entered volume, calculate dispensed bottles,
	//set volume to dispense = volume in tank show dispensed bottles and the left bottles
	TX_NEW_LINE;
	message("Please enter the number of bottles you"); message(" want to fill the juice with>> ");
	bottles_entered = usart_receive_string(); //get the number of bottles to be dispensed
	entered_bottles = convert_to_integer_value(bottles_entered);	//convert string entered bottles to int
	
	entered_volume = get_volume_in_bottles(bottle_capacity, entered_bottles);	//calculate the mls in the entered bottles
	//show the volume in the entered_bottles
	TX_NEW_LINE;
	usart_transmit_message(bottles_entered);usart_transmit_message(" bottles of ");usart_transmit_message(get_string_from_int(bottle_capacity));
	usart_transmit_message(" mls give "); message(get_string_from_float((entered_volume/1000),1));
	usart_transmit_message(" ltrs of the drink");
	
	if(volume_in_tank >= entered_volume){
		volume_to_dispense = entered_volume;	//set volume to dispense equal entered volume
		volume_left_in_tank = get_left_volume_in_tank(volume_in_tank, volume_to_dispense);	//get the volume that will be left in the tank
		bottles_to_dispense = entered_bottles;		//set the number of bottles to be dispensed
		//show the left volume in the tank
		TX_NEW_LINE;
		usart_transmit_message("The volume will be left in the tank is: ");usart_transmit_message(get_string_from_float((volume_left_in_tank / 1000),1));
		usart_transmit_message(" liters.");
		TX_NEW_LINE;
		message("Switch on the \"button for starting the motor\" ");message("to start dispensing the bottels with the juice.");
		TX_NEW_LINE;
	}
	else if(volume_in_tank < entered_volume){
		warning_msg("Limited drink in the tank....");
		TX_NEW_LINE;
		volume_to_dispense = volume_in_tank;	//set volume to dispense equal to the volume available in the tanks
		bottles_to_dispense = get_bottles_in_volume(volume_to_dispense, bottle_capacity);	//get the bottles that can be filled with the available volume of the drink
		//show the bottles that can be filled by the available drink
		usart_transmit_message("The available volume in the tank can only fill: ");
		message(get_string_from_int(bottles_to_dispense)); usart_transmit_message(" bottles of ");
		usart_transmit_message(get_string_from_int(bottle_capacity));usart_transmit_message(" mls");
		message("and will be dispensed");
		TX_NEW_LINE;
		TX_NEW_LINE;
		message("Switch on the \"button for starting the motor\" "); message("to start dispensing the bottles with the juice.");
		TX_NEW_LINE;
	}
	
}
//ask opertor whether to resume from previous settings or prepare a new juice on system restart
void on_system_restart(void){
	char on_start_option;
	do{
		//ask operator to choose
		message("CHOOSE AN OPTION");
		//menu
		message(" 1. Start from previous settings?");
		message(" 2. Start afresh(prepare new juice)?");
		TX_NEW_LINE;
		message("Choose an option (1 or 2)>> ");	
		on_start_option = usart_receiveByte();			//get the option from the terminal
		//switch the option
		switch(on_start_option){
			case '1':{
				recover_system_settings();				//access system settings from eeprom
				
				if(left_volume_in_tank > 0){
					message("There are: "); usart_transmit_message(get_string_from_float((left_volume_in_tank/1000),1));
					usart_transmit_message(" ltrs in the tank");
					//check if all the bottles were fully dispensed otherwise ask for new bottles to be filled
					if(bottles_to_dispense == 0){
						message("OOps all bottles were filled..");
						message("Enter bottles to be filled.");
						get_number_of_bottles(left_volume_in_tank);			//get more bottles in case the previos were fully filled
					}
					message("Continuing from previous settings!!!");
					message("Press the start motor button if its not on.");
				}
				else{
					warning_msg("There's no juice in the tank");
					message("Mix fresh juice.");
					//start afresh
					capture_fruits_in_store();									//get the fruits in store
					capture_fruits_percentage();								//get the percentages for all the fruits
					set_total_liters_in_tank(&total_mixed_fruits);				//mix the total liters in the tank
					set_bottle_capacity();										//set the bottle capacity
					get_number_of_bottles(150000);
				}
				//first check if eeprom is empty
					//yes:inform the operator that he has no settings and ask him to prepare a new juice and get out of switch
					//no:read all other settings  and get out of switch
				
			}
			break;
			case '2':{
				//start afresh
				capture_fruits_in_store();									//get the fruits in store
				capture_fruits_percentage();								//get the percentages for all the fruits
				set_total_liters_in_tank(&total_mixed_fruits);				//mix the total liters in the tank
				set_bottle_capacity();										//set the bottle capacity
				get_number_of_bottles(150000);
			}
			break;
			default:{
				//ask him/her again on wrong entry
				on_start_option = '0'; //reset the option
				error_msg("Unknown entry!! choose either 1 or 2.");
			}
			break;
		}
		
	}
	while(on_start_option == '0');
}
//function to save settings by writting to eeprom
void save_system_settings(void){
	//the timers
	settings.main_hours_ = main_hours;
	settings.main_minutes_ = main_minutes;
	settings.main_seconds_ = main_seconds;
	
	settings.paused_hours_ = paused_hours;
	settings.paused_minutes_ = paused_minutes;
	settings.paused_seconds_ = paused_seconds;
	
	//port values
		//motor state
		settings.motor_port_ = PORTC;
		//7 segment status
		settings.countdown_timer_port_ = PORTF;
		//leds
		settings.led_port_ = PORTA;
	//pin values
	settings.button_pins_ = PINE;
	
	//fruits left in store 
	settings.left_apples_ = left_apples;
	settings.left_oranges_ = left_oranges;
	settings.left_mangoes_ = left_mangoes;
	settings.left_pineapples_ = left_pineapples;
	settings.left_guavas_ = left_guavas;
	
	//bottles remaining to be filled 
	settings.bottles_to_dispense_ = bottles_to_dispense;
	//bottles filled yet 
	settings.bottles_filled_ = bottles_filled;
	//bottle capacity 18
	settings.bottle_capacity_ = bottle_capacity;
	//volume of juice left in the tank
	settings.left_volume_in_tank_ = left_volume_in_tank;
	//motor control variable
	settings.motor_drive_control_ = motor_drive_control;
	//total overflows of timer0
	settings.total_timer0_overflows_ = total_timer0_overflows;
	
	//save the entire settings to eeprom
	eeprom_write_block((const void*)&settings, (void*)0, sizeof(settings));
	
}
//function to recover settings on system restart
void recover_system_settings(void){
	//fetch the settings from eeprom on system restart
	eeprom_read_block((void*)&settings, (void*)0, sizeof(settings));
	
	//read the previously paused time
	paused_hours = settings.paused_hours_;
	paused_minutes = settings.paused_minutes_;
	paused_seconds = settings.paused_seconds_;
	
	PORTC = settings.motor_port_;					//get where the motor stoped from
	PORTF = settings.countdown_timer_port_;			//get where the countdown timers for sealing and filling stopped
	//PORTA = settings.led_port_;						//get the status of the LEDs
	PINE = settings.button_pins_;					//get the previous status of the buttons
	
	//the left fruits
	left_apples = settings.left_apples_;
	left_guavas = settings.left_guavas_;
	left_mangoes = settings.left_mangoes_;
	left_pineapples = settings.left_pineapples_;
	left_oranges = settings.left_oranges_;
	
	bottles_to_dispense = settings.bottles_to_dispense_;		//get the remaing bottles to be filled
	bottle_capacity = settings.bottle_capacity_;				//get the configered boottle capacity
	bottles_filled = settings.bottles_filled_;					//get the bottles that were filled before
	left_volume_in_tank = settings.left_volume_in_tank_;		//get the volume which was left before the system was turnned off
	 
	total_timer0_overflows = settings.total_timer0_overflows_;	//pausing timer control variable
	
}
//function to pause the system
void pause_system(void){
	//only pause the system when the button for pausing is pressed on
	if((PINE & (1 << PE4)) == 0){
		start_pausing_timer();		    //start the pausing timer (time the pausing session)
		disable_stepper_motor_drive();	//disable motor driver
		PORTA &= ~(1 << PA1);			//dispensing LED Off
	}
	
}
//function to resume the system from the pausing state
void resume_system(void){
	stop_pausing_timer();			//stop timing the pausing session
	
	if((PINE & (1 << PE5)) == 0){
		enable_stepper_motor_drive();	//enable motor driver
		PORTA |= (1 << PA1);			//dispensing LED On
	}
	
	
}
//function for the operational menu
void operation_menu(void){
	
	/**this is the operation menu*/
	char volatile menu_option;
	TX_NEW_LINE;
	message("***************OPERATION MENU******************");
	//the menu
	TX_NEW_LINE;
	message(" 1. Show amount of fruit left in store.");
	message(" 2. Show bottles filled with the drink so far.");
	message(" 3. Show liters of drink in storage tank.");
	message(" 4. Change bottle capacity.");
	message(" 5. Show time the plant has worked.");
	message(" 6. Show time the plant has rested.");
	
	message("Choose an option(1 - 6):>> ");
	menu_option = usart_receiveByte();					//get the option from the operator
	
	//the selection
	switch(menu_option){
		//show the amount of fruits left in the store
		case '1':{
			
			show_left_fruits_in_store();
		}
		break;
		//show the bottles filled with the drink so far
		case '2':{
			show_bottles_filled();
		}
		break;
		//show the liters of the drink left in the tank
		case '3':{
			show_liters_in_storage_tank();
		}
		break;
		//change the boottle capacity
		case '4':{
			change_bottle_capacity();
		}
		break;
		//show time the plant has worked
		case '5':{
			show_entire_operation_time();
		}
		break;
		//show time the plant has rested
		case '6':{
			show_time_for_pausing();
		}
		break;
		default:{
			TX_NEW_LINE;
			error_msg("Unknown entry! ");
		}
		break;
		
	}
}

/********** operation menu functions */
//function to change the bottel capacity
void change_bottle_capacity(void){
	char new_bottle_option;
	//First tell the operator about the bottle capacity that was previously set
	message("You had configered bottel capacity to: ");
	usart_transmit_message(get_string_from_int(bottle_capacity));
	usart_transmit_message(" ml");
	
	do{
		message("CHANGE THE BOTTLE CAPACITY TO FILL WITH THE DRINK");
		//menu
		message(" 1. 500 ml bottle.");
		message(" 2. 300 ml bottle.");
		TX_NEW_LINE;
		message("Choose a new bottle capacity>> ");
		new_bottle_option = usart_receiveByte();	//get a new capacity option from terminal
			switch(new_bottle_option){
				case '1':{
					bottle_capacity = 500;	//change bottle capacity to 500 ml
					message("Bottle capacity is changed to 500 ml");
					
				}
				break;
				case '2':{
					bottle_capacity = 300;	//change bottle capacity to 300 ml
					message("Bottle capacity is changed to 300 ml");
				}
				break;
				default:{
					new_bottle_option = '0';	//reset the bottle option
					error_msg("Choose either 1 or 2.");
				}
				break;
			}//end switch
	} while(new_bottle_option == '0');
}
//function to show the time the plant has worked
void show_entire_operation_time(void){
	TX_NEW_LINE;
	message("The plant has worked for:  ");
	usart_transmit_message(get_string_from_int((int)main_hours));	//the hours
	usart_transmit_message(":");
	usart_transmit_message(get_string_from_int((int)main_minutes));	//the minutes
	usart_transmit_message(":");
	usart_transmit_message(get_string_from_int((int)main_seconds));	//the seconds
	usart_transmit_message(" (H:M:S)");
	TX_NEW_LINE;
}
//function to show the time the plant has rested/paused
void show_time_for_pausing(void){
	TX_NEW_LINE;
	message("The plant has rested for:  ");
	usart_transmit_message(get_string_from_int((int)paused_hours));		//the hours
	usart_transmit_message(":");
	usart_transmit_message(get_string_from_int((int)paused_minutes));	//the minutes
	usart_transmit_message(":");
	usart_transmit_message(get_string_from_int((int)paused_seconds));	//the seconds
	usart_transmit_message(" (H:M:S)");
	TX_NEW_LINE;
}
//function to display the number of left fruits of each type in the store after mixing
void show_left_fruits_in_store(void){
	TX_NEW_LINE;
	message("THE LEFT FRUITS IN STORE.");
	message("The left mangoes are:  "); usart_transmit_message(get_string_from_int(left_mangoes));
	message("The left oranges are:  "); usart_transmit_message(get_string_from_int(left_oranges));
	message("The left apples are:  "); usart_transmit_message(get_string_from_int(left_apples));
	message("The left guavas are:  "); usart_transmit_message(get_string_from_int(left_guavas));
	message("The left pineapples are: "); usart_transmit_message(get_string_from_int(left_pineapples));
	TX_NEW_LINE;
}
//function to show the volume in litres of the drink left in the storage tank
void show_liters_in_storage_tank(void){
	TX_NEW_LINE;
	message("The storage tank has:  ");
	usart_transmit_message(get_string_from_float((left_volume_in_tank/1000),1));		//left volume in the tank
	usart_transmit_message(" ltrs of juice");
	TX_NEW_LINE;
}
//function to show the bottles filled with the drink so far since the dispension started
void show_bottles_filled(void){
	TX_NEW_LINE;
	message("The plant has filled:  ");
	usart_transmit_message(get_string_from_int((int)bottles_filled));		//the boottles filled
	usart_transmit_message(" boottles.");
	TX_NEW_LINE;
}

/*
///////////////////////////////////////////////////////////////////////
							STEPPER MOTOR DRIVE FUNCTIONS
//////////////////////////////////////////////////////////////////////
*/

//function to enable motor driver
void enable_stepper_motor_drive(void){
	//drive the motor
	ETIMSK |= (1 << OCIE3A);			//--enable interrupts on compare match for timer3
}
//function to disable motor driver
void disable_stepper_motor_drive(void){
	//stop motor
	ETIMSK &= ~(1 << OCIE3A);			//--disable interrupts on compare match for timer3
}
//function to drive stepper motor in half drive stepping
void drive_stepper_motor(void){
	//the stepper motor is on port C of atmega 128
	PORTC = 0x01;
	_delay_ms(750);
	PORTC = 0x03;
	_delay_ms(750);
	PORTC = 0x02;
	_delay_ms(750);
	PORTC = 0x06;
	_delay_ms(750);
	PORTC = 0x04;
	_delay_ms(750);
	PORTC = 0x0C;
	_delay_ms(750);
	PORTC = 0x08;
	_delay_ms(750);
	PORTC = 0x09;
	_delay_ms(750);
	
}
//function to stop stepper motor
void stop_stepper_motor(void){
	PORTC = 0x00;				//remove energy from the port completely so that it stops moving
	
}


/*
///////////////////////////////////////////////////////////////////////
							INTERRUPTS SECTION
//////////////////////////////////////////////////////////////////////
*/

					/*****timer interrupts****/
//timer/counter0 timer/counter0 overflow interrupt
ISR(TIMER0_OVF_vect){
	total_timer0_overflows++;
	//122 overflows generate a delay of 1 second
	if (total_timer0_overflows >= 122)
	{
		total_timer0_overflows = 0;		//reset the total overflows for timer0
		
		paused_seconds++;				//count it as a second
	
		//count the minutes for pausing
		if (paused_seconds >= 59)
		{
			paused_seconds = 0;		//reset the seconds for the paused timer
			paused_minutes++;		//count it as 1 minute actually 60s = 1 minute
		}
		//count the hours
		if (paused_minutes >= 59)
		{
			paused_minutes = 0;		//reset the minutes for the paused timer
			paused_hours++;			//count it as an hour actually 60min = 1hour
		}
		PORTA ^= (1 << PA0);		//blink the LED for system paused
	}
	
	
}
//timer/counter1 compare match  interrupt
ISR(TIMER1_COMPA_vect){
	main_seconds++;			//increment seconds by one
	start_main_timer();		//count the main time (time the plant has took since started)
}
//timer/counter3 compare match interrupt
ISR(TIMER3_COMPA_vect){
	motor_drive_control++;		//increment the seconds which control the motor drive-stop logic
	
	
	//drive the motor through 2 rotations to drive the bottel to storage area in the due course another bottel will be available for filling
	if(motor_drive_control <= 2)
	{
		//countdown timers are 00  when the bottle is being driven to the storage area
		PORTF = 0x00;
		
		drive_stepper_motor();	//drive the motor
		
	}
	//wait for 2 seconds for the boottle to be filled
	else if ((motor_drive_control > 2) && (motor_drive_control <= 4))
	{
		stop_stepper_motor();	//keep the motor stopped for 2 seconds
		
		//count down timer for filling the bottle
		if(motor_drive_control == 3){
			PORTF = 0x02;	//show a 2 on the count down timer
			test_action();
		}
		else if(motor_drive_control == 4){
			PORTF = 0x01;	//show a 1 on the countdown timer for filling
		}
		else{
			PORTF = 0x00;	//show a zero otherwise		
		}
	}
	//drive the motor through one rotation to drive the bottel to the sealing stage
	else if ((motor_drive_control > 4) && (motor_drive_control <= 5))
	{
		//countdown timer for filling the boottle value is shown as zero when we drive the motor to set the bottle for sealing
		PORTF = 0x00;
		
		drive_stepper_motor();	//keep driving the motor for 1 second
		
	}
	//stop the motor for 1 second to seal the bottel
	else if ((motor_drive_control > 5) && (total_timer0_overflows <= 6))
	{
		//start the count down timer for sealing the bottle for 
		PORTF = 0x10;
		
		stop_stepper_motor();	//keep the motor stopped for 1s to seal boottle and then go back to re
		
		/*
			actually after 6 seconds a bottle will be fully completed and driven the storage area
			therefore to keep track of the filled bottels and the left volume in the tank the section below helps
		*/
		if(motor_drive_control == 6){
			if(bottles_to_dispense == 0){	//when all the bottels are filled stop the dispension of juice, stop the motor
				//When all bottles are filled reset the countdown timers for both sealing and filling the bottles to 00
				PORTF = 0x00;
				
				disable_stepper_motor_drive();
				stop_stepper_motor();
				//dispension complete....
				//save the left volume  
			}
			else{
				left_volume_in_tank = left_volume_in_tank - (float)bottle_capacity;		//get the left volume in tank
				
				bottles_to_dispense--;		//reduce bottles_to_dispense by 1
				bottles_filled++;			//count that bottle as a filled bottle
				save_system_settings();	//save the system settings after each second
				
			}
			motor_drive_control = 0;		//reset the motor_drive_control variable
		}
			
	}
	else{
		stop_stepper_motor();
	}
	
	
}
					/*****external interrupts****/
//External interrupts due to INT4  change in voltage level
ISR(INT4_vect){
	//when the button is pressed on, start the pausing timer and pause the system
	if ((PINE &(1 << PE4)) == 0)
	{
		//pause the system function goes here.....................
		pause_system();
	}
	//when the button is pressed off, stop the pausing timer and resume the system
	else if((PINE &(1 << PE4)) > 0){
		//resume the system function goes here.....................
		resume_system();
	}
	
}
//external interrupts due to INT5 change in voltage level
ISR(INT5_vect){
	//when the button is pressed on
	if ((PINE &(1 << PE5)) == 0)
	{
		//start motor function goes here
		enable_stepper_motor_drive();	//drive the motor
		PORTA |= (1 << PA1);			//dispensing LED On
	}
	else{
		stop_stepper_motor();			//turn off the motor completely
		disable_stepper_motor_drive();
		PORTA &= ~(1 << PA1);			//dispensing LED Off
	}
}

