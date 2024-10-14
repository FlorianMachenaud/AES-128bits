#ifndef AES_H
#define AES_H
#define DATA_SIZE 16
#define STATE_ROW_SIZE 4
#define STATE_COL_SIZE 4
#define ROUND_COUNT 10
#include <stdint.h>

// the round that will t r i g g e r
extern uint8_t targeted_round;

void AESEncrypt (uint8_t ciphertext[DATA_SIZE], uint8_t plaintext[DATA_SIZE], uint8_t key[DATA_SIZE]); // OK

void AddRoundKey (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t roundkey[STATE_ROW_SIZE][STATE_COL_SIZE]); // OK

void SubBytes (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]); // OK

void ShiftRows (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]); // OK

void MixColumns (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]); // OK

void KeyGen (uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t master_key[STATE_ROW_SIZE][STATE_COL_SIZE]); // OK

// fill the first c o l u m n of a given round key
void ColumnFill ( uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], int round); // OK

// fill the other 3 c o l u m n s of a given round key
void OtherColumnsFill ( uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], int round); // OK

void GetRoundKey (uint8_t roundkey [STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], int round); // OK

void MessageToState (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t message [DATA_SIZE]); // OK

void StateToMessage (uint8_t message [DATA_SIZE], uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]); // OK

void MCMatrixColumnProduct ( uint8_t colonne [STATE_COL_SIZE]); // OK

uint8_t gmul (uint8_t a, uint8_t b) ; // OK

extern const uint8_t rcon [10]; // OK

extern const uint8_t sboxtab [256]; // OK

extern const uint8_t invsbox [256];

# endif


