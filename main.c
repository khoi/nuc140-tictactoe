#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h" 
#include "LCD.h" 
#include "string.h"
#include "Draw2D.h"

#define COMPUTERMOVE 'O' 
#define HUMANMOVE 'X' 
#define EMPTYMOVE ' '
#define BOARD_SIZE 3
#define BOUNCING_DLY 1500000

typedef enum {game_rules, human_turn, computer_turn, game_end} STATES;

void KeyPadEnable(void); 
uint8_t KeyPadScanning(void);
void System_Config(void);
void LCD_start(void);
void LCD_command(unsigned char temp);
void SPI3_Config(void);

void reset_board(char board[BOARD_SIZE][BOARD_SIZE]) {
	uint8_t i, j;
	for (i = 0; i < BOARD_SIZE; ++i) {
		for (j = 0; j < BOARD_SIZE; ++j) {
			board[i][j] = EMPTYMOVE;
		}
	}
}
void draw_board(char board[BOARD_SIZE][BOARD_SIZE]);

bool rowCrossed(char board[BOARD_SIZE][BOARD_SIZE]) { 
		uint8_t i;
    for (i=0; i < BOARD_SIZE; i++) 
    { 
        if (board[i][0] == board[i][1] && 
            board[i][1] == board[i][2] &&  
            board[i][0] != EMPTYMOVE) 
            return (true); 
    } 
    return false; 
} 

bool colCrossed(char board[BOARD_SIZE][BOARD_SIZE]) { 
		uint8_t i;
    for (i=0; i < BOARD_SIZE; i++) 
    { 
        if (board[0][i] == board[1][i] && 
            board[1][i] == board[2][i] &&  
            board[0][i] != EMPTYMOVE) 
            return true; 
    } 
    return false; 
} 
bool diagonalCrossed(char board[BOARD_SIZE][BOARD_SIZE]) { 
    if (board[0][0] == board[1][1] && 
        board[1][1] == board[2][2] &&  
        board[0][0] != EMPTYMOVE) 
        return true; 
          
    if (board[0][2] == board[1][1] && 
        board[1][1] == board[2][0] && 
         board[0][2] != EMPTYMOVE) 
        return true; 
  
    return(false); 
} 


bool gameOver(char board[BOARD_SIZE][BOARD_SIZE]) { 
    return rowCrossed(board) || colCrossed(board) || diagonalCrossed(board) ; 
} 


int main(void) {
		STATES game_state = game_rules;
		uint16_t i, j;
		uint8_t key_pressed = 0;	
		uint8_t x, y = 0;
		uint8_t move_count = 0;
		uint8_t winner = 0; // 0: tie, 1: human: 2: computer
		

		// initialize board
		char board[BOARD_SIZE][BOARD_SIZE];
		

		System_Config(); 

    KeyPadEnable();

    SPI3_Config();
		LCD_start();
		clear_LCD();
		
    while (1) { 
			switch(game_state){
				case game_rules:
					printS_5x7(1, 0, "Tic Tac Toe - NUC140"); 
					printS_5x7(1, 8, "1: HUMAN FIRST"); 
          printS_5x7(1, 16, "2: COMPUTER FIRST"); 
					printS_5x7(1, 56, "choose who moves first!");
			
					while(key_pressed == 0) key_pressed = KeyPadScanning(); 
				
					CLK_SysTickDelay(BOUNCING_DLY); // debouncing
				
					clear_LCD();
				
					// reset all variables to its initial state
					reset_board(board);
					move_count = 0;
					winner = 0;
				
					if (key_pressed == 1) {
						game_state = human_turn;
					}
					else if (key_pressed == 2) {
						game_state = computer_turn;
					}
					else {
						game_state = game_rules;
					}
					
					key_pressed = 0;
					
					break;
				case human_turn:
					draw_board(board);

					while (key_pressed == 0) {
						key_pressed = KeyPadScanning();
					}						
					CLK_SysTickDelay(BOUNCING_DLY);
					
					x = (key_pressed - 1) / BOARD_SIZE;
					y = (key_pressed - 1) % BOARD_SIZE;
					
					key_pressed = 0;

					if (board[x][y] != EMPTYMOVE) {
						break; // go back to the human state again
					}
					
					board[x][y] = HUMANMOVE;
					
					draw_board(board);
					
					if (gameOver(board)) {
						winner = 1;
						game_state = game_end;
					} 
					else if (++move_count == (BOARD_SIZE * BOARD_SIZE)) {
						winner = 0;
						game_state = game_end;
					}  
					else {
						game_state = computer_turn;
					}
					break;

				case computer_turn:
					draw_board(board);
					
					do {
						x = rand() % BOARD_SIZE;
						y = rand() % BOARD_SIZE;
					} while (board[x][y] != EMPTYMOVE);
					
					board[x][y] = COMPUTERMOVE;
					
					draw_board(board);

					if (gameOver(board)) {
						winner = 2;
						game_state = game_end;
					} 
					else if (++move_count == (BOARD_SIZE * BOARD_SIZE)) {
						winner = 0;
						game_state = game_end;
					}  
					else {
						game_state = human_turn;
					}
					
					break;

				case game_end:
					if (winner == 0) {
						printS_5x7(65, 0, "TIE!");
					}
					else if (winner == 1){
						printS_5x7(65, 0, "YOU WIN!");
					}
					else if (winner == 2) {
						printS_5x7(65, 0, "YOU LOST!");
					}
					
					printS_5x7(1, 56, "press any key to continue!");
				
					while(key_pressed==0) key_pressed = KeyPadScanning(); 
          key_pressed=0;
          clear_LCD();
          game_state = game_rules;	
					
					break;
				default: break;
			}
    } 
}

void draw_board(char board[BOARD_SIZE][BOARD_SIZE]) {
	draw_Line(0, 0, 0, 60, 1, 0);
	printC_5x7(8, 8, board[0][0]);
	
	draw_Line(20, 0, 20, 60, 1, 0);
	printC_5x7(28, 8, board[0][1]);
	
	draw_Line(40, 0, 40, 60, 1, 0);
	printC_5x7(48, 8, board[0][2]);

	draw_Line(60, 0, 60, 60, 1, 0);

	draw_Line(0, 0, 60, 0, 1, 0);
	printC_5x7(8, 28, board[1][0]);

	
	draw_Line(0, 20, 60, 20, 1, 0);
	printC_5x7(28, 28, board[1][1]);

	
	draw_Line(0, 40, 60, 40, 1, 0);
	printC_5x7(48, 28, board[1][2]);

	
	draw_Line(0, 60, 60, 60, 1, 0);
	
	printC_5x7(8, 48, board[2][0]);
	printC_5x7(28, 48, board[2][1]);
	printC_5x7(48, 48, board[2][2]);
}
void KeyPadEnable(void) {
    GPIO_SetMode(PA, BIT0, GPIO_MODE_QUASI);
    GPIO_SetMode(PA, BIT1, GPIO_MODE_QUASI);
    GPIO_SetMode(PA, BIT2, GPIO_MODE_QUASI);
    GPIO_SetMode(PA, BIT3, GPIO_MODE_QUASI);
    GPIO_SetMode(PA, BIT4, GPIO_MODE_QUASI);
    GPIO_SetMode(PA, BIT5, GPIO_MODE_QUASI);
}

uint8_t KeyPadScanning(void) {
    PA0 = 1;
    PA1 = 1;
    PA2 = 0;
    PA3 = 1;
    PA4 = 1;
    PA5 = 1;
    if (PA3 == 0) return 1;
    if (PA4 == 0) return 4;
    if (PA5 == 0) return 7;
    PA0 = 1;
    PA1 = 0;
    PA2 = 1;
    PA3 = 1;
    PA4 = 1;
    PA5 = 1;
    if (PA3 == 0) return 2;
    if (PA4 == 0) return 5;
    if (PA5 == 0) return 8;
    PA0 = 0;
    PA1 = 1;
    PA2 = 1;
    PA3 = 1;
    PA4 = 1;
    PA5 = 1;
    if (PA3 == 0) return 3;
    if (PA4 == 0) return 6;
    if (PA5 == 0) return 9;
    return 0;
}

void System_Config(void) {
    SYS_UnlockReg(); // Unlock protected registers
    CLK -> PWRCON |= (0x01ul << 0);
    while (!(CLK -> CLKSTATUS & (1ul << 0)));

    //PLL configuration starts
    CLK -> PLLCON &= ~(1ul << 19); //0: PLL input is HXT
    CLK -> PLLCON &= ~(1ul << 16); //PLL in normal mode
    CLK -> PLLCON &= (~(0x01FFul << 0));
    CLK -> PLLCON |= 48;
    CLK -> PLLCON &= ~(1ul << 18); //0: enable PLLOUT
    while (!(CLK -> CLKSTATUS & (0x01ul << 2)));
    //PLL configuration ends
    //clock source selection
    CLK -> CLKSEL0 &= (~(0x07ul << 0));
    CLK -> CLKSEL0 |= (0x02ul << 0); //clock frequency division
    CLK -> CLKDIV &= (~0x0Ful << 0);
    //enable clock of SPI3
    CLK -> APBCLK |= 1ul << 15;
    SYS_LockReg(); // Lock protected registers
}

void LCD_start(void) {
    LCD_command(0xE2); // Set system reset
    LCD_command(0xA1); // Set Frame rate 100 fps
    LCD_command(0xEB); // Set LCD bias ratio E8~EB for 6~9 (min~max)
    LCD_command(0x81); // Set V BIAS potentiometer
    LCD_command(0xA0); // Set V BIAS potentiometer: A0 ()
    LCD_command(0xC0);
    LCD_command(0xAF); // Set Display Enable
}

void LCD_command(unsigned char temp) {
    SPI3 -> SSR |= 1ul << 0;
    SPI3 -> TX[0] = temp;
    SPI3 -> CNTRL |= 1ul << 0;
    while (SPI3 -> CNTRL & (1ul << 0));
    SPI3 -> SSR &= ~(1ul << 0);
}

void SPI3_Config(void) {
    SYS -> GPD_MFP |= 1ul << 11; //1: PD11 is configured for alternative function
    SYS -> GPD_MFP |= 1ul << 9; //1: PD9 is configured for alternative function
    SYS -> GPD_MFP |= 1ul << 8; //1: PD8 is configured for alternative function
    SPI3 -> CNTRL &= ~(1ul << 23); //0: disable variable clock feature

    SPI3 -> CNTRL &= ~(1ul << 22); //0: disable two bits transfer mode
    SPI3 -> CNTRL &= ~(1ul << 18); //0: select Master mode
    SPI3 -> CNTRL &= ~(1ul << 17); //0: disable SPI interrupt
    SPI3 -> CNTRL |= 1ul << 11; //1: SPI clock idle high
    SPI3 -> CNTRL &= ~(1ul << 10); //0: MSB is sent first
    SPI3 -> CNTRL &= ~(3ul << 8); //00: one transmit/receive word will be executed in one data transfer
    SPI3 -> CNTRL &= ~(31ul << 3); //Transmit/Receive bit length
    SPI3 -> CNTRL |= 9ul << 3; //9: 9 bits transmitted/received per data transfer
    SPI3 -> CNTRL |= (1ul << 2); //1: Transmit at negative edge of SPI CLK
    SPI3 -> DIVIDER = 0; // SPI clock divider. SPI clock = HCLK / ((DIVIDER+1)*2). HCLK = 50 MHz
}
