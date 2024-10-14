#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "aes.h"

// Tableau S-BOX
const uint8_t sboxtab[256] = {
  //0     1    2      3     4    5     6     7      8    9     A      B    C     D     E     F
  0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
  0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
  0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
  0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
  0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
  0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
  0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
  0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
  0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
  0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
  0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
  0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
  0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
  0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
  0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
  0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16 };

const uint8_t rcon[10] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1B, 0x36};

static const uint8_t MixCMatrix[STATE_ROW_SIZE][STATE_COL_SIZE] = {
    {0x02, 0x03, 0x01, 0x01},
    {0x01, 0x02, 0x03, 0x01},
    {0x01, 0x01, 0x02, 0x03},
    {0x03, 0x01, 0x01, 0x02}
  };

void ShowState(uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]){
  for (int i=0; i<STATE_ROW_SIZE; i++){
    for (int j=0; j<STATE_COL_SIZE; j++){
      printf("%02X ", state[i][j]);
    }
    printf("\n");
  }
}

void ColumnFill ( uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], int round){
  // Extract de la dernière colonne
  uint8_t first_col[STATE_COL_SIZE];
  for (int i=0; i< STATE_ROW_SIZE; i++){
    first_col[i] = roundkeys[round-1][i][STATE_ROW_SIZE-1];
  }

  // RotWord
  uint8_t tmp = first_col[0];
  for (int i=0; i<STATE_COL_SIZE-1; i++){
    first_col[i] = first_col[i+1];
  }
  first_col[STATE_COL_SIZE-1] = tmp;

  // Remplacement avec la sboxtab
  for (int i=0; i<STATE_COL_SIZE; i++){
    uint8_t left_digit = (first_col[i]>> 4) & 0x0F;
    uint8_t right_digit = first_col[i] & 0x0F; 
    first_col[i] = sboxtab[left_digit * DATA_SIZE + right_digit];
    if (i==0){
      first_col[i] ^= roundkeys[round-1][i][0] ^ rcon[round-1];
    }else{
      first_col[i] ^= roundkeys[round-1][i][0] ^ 0x00;
    }
    roundkeys[round][i][0] = first_col[i]; // copie dans la première colonne de la roundkey
  }
}

void OtherColumnsFill ( uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], int round){
  for (int i=0; i<STATE_COL_SIZE; i++){
      for (int j=1; j<STATE_ROW_SIZE; j++){
        roundkeys[round][i][j] = roundkeys[round][i][j-1] ^ roundkeys[round-1][i][j]; 
      }
    }
}

void KeyGen (uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t master_key[STATE_ROW_SIZE][STATE_COL_SIZE]){
  // Initialisation de la roundkey, copie de la masterkey
  for (int i=0; i<STATE_ROW_SIZE; i++){
    for (int j=0; j<STATE_COL_SIZE; j++){
      roundkeys[0][i][j] = master_key[i][j];
    }
  }
  for (int n=1; n<=ROUND_COUNT; n++){ // Boucle principale
    ColumnFill(roundkeys, n);
    OtherColumnsFill(roundkeys, n);
  }
}

void GetRoundKey (uint8_t roundkey [STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t roundkeys [][STATE_ROW_SIZE][STATE_COL_SIZE], int round){
  for (int i=0; i<STATE_COL_SIZE; i++){
    for (int j=0; j<STATE_ROW_SIZE; j++){
      roundkey[i][j] = roundkeys[round][i][j];
    }
  }
}

void AddRoundKey (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t roundkey[STATE_ROW_SIZE][STATE_COL_SIZE]){
  for (int i=0; i<STATE_ROW_SIZE; i++){
    for (int j=0; j<STATE_COL_SIZE; j++){
      state[i][j] = state[i][j] ^ roundkey[i][j];
    }
  }
}

void SubBytes (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]){
  for (int i=0; i<STATE_ROW_SIZE; i++){
    for (int j=0; j<STATE_COL_SIZE; j++){
      // Extraction des 2 digits
      uint8_t left_digit = (state[i][j] >> 4) & 0x0F;
      uint8_t right_digit = state[i][j] & 0x0F; 
      state[i][j]= sboxtab[left_digit * DATA_SIZE + right_digit]; // Remplacement avec la sboxtab
    }
  }
}

void ShiftRows (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]){
  for (int i=1; i<STATE_ROW_SIZE; i++){
    for (int k=0; k<i; k++){
      uint8_t tmp = state[i][0];
      for (int j=0; j<STATE_COL_SIZE-1; j++){
        state[i][j] = state[i][j+1];
      }
      state[i][STATE_COL_SIZE-1] = tmp;
    }
  }
}

uint8_t gmul(uint8_t a, uint8_t b) {
    if (b == 0x01){
        return a;
    }else if (b == 0x02){
        if (a & 0x80){ //1 en bit de poid fort
            return 0xFF & ((a << 1) ^ (0x1B));
         }else{
            return a<<1;
         } 
    }else{
        return gmul(a, 0x02) ^ a;
    }
}

void MCMatrixColumnProduct ( uint8_t colonne [STATE_COL_SIZE]){
  uint8_t tmp[4] = {0};
  for (int i=0; i<STATE_ROW_SIZE; i++){
    for (int k=0; k<STATE_COL_SIZE; k++){
      tmp[i] ^= gmul(colonne[k], MixCMatrix[i][k]);
    }
  }
  for (int i=0; i<STATE_ROW_SIZE; i++){
    colonne[i] = tmp[i];
  }
}

void MixColumns (uint8_t state[STATE_ROW_SIZE][STATE_COL_SIZE]) {
  uint8_t tmp_colonne[STATE_ROW_SIZE];  // Tableau temporaire pour une colonne
  for (int i = 0; i < STATE_COL_SIZE; i++) {
    for (int j = 0; j < STATE_ROW_SIZE; j++) {
      tmp_colonne[j] = state[j][i];
    }

    MCMatrixColumnProduct(tmp_colonne);

    for (int j = 0; j < STATE_ROW_SIZE; j++) {
      state[j][i] = tmp_colonne[j];
    }
  }
}

void MessageToState (uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE], uint8_t message [DATA_SIZE]){
  for (int k = 0; k < DATA_SIZE; k++) {
    int i = k % STATE_ROW_SIZE;
    int j = k / STATE_ROW_SIZE;
    state[i][j] = message[k];
  }
}

void StateToMessage (uint8_t message [DATA_SIZE], uint8_t state [STATE_ROW_SIZE][STATE_COL_SIZE]){
  for (int k = 0; k < DATA_SIZE; k++) {
    int i = k % STATE_ROW_SIZE;
    int j = k / STATE_COL_SIZE;
    message[k] = state[i][j];
  }
}

void AESEncrypt (uint8_t ciphertext[DATA_SIZE], uint8_t plaintext[DATA_SIZE], uint8_t key[DATA_SIZE]){
  uint8_t cipher_state[STATE_COL_SIZE][STATE_ROW_SIZE];
  uint8_t key_state[STATE_COL_SIZE][STATE_ROW_SIZE];
  uint8_t roundkeys[ROUND_COUNT][STATE_COL_SIZE][STATE_ROW_SIZE];

  
  MessageToState(cipher_state, plaintext);
  MessageToState(key_state, key);

  KeyGen(roundkeys, key_state);

  for (int n=0; n<=ROUND_COUNT; n++){
    if (n==0){
      AddRoundKey(cipher_state, roundkeys[n]);
    }else if (n==ROUND_COUNT){
      SubBytes(cipher_state);
      ShiftRows(cipher_state);
      AddRoundKey(cipher_state, roundkeys[n]);
    }else{
      SubBytes(cipher_state);
      ShiftRows(cipher_state);
      MixColumns(cipher_state);
      AddRoundKey(cipher_state, roundkeys[n]);
    }
  }

  StateToMessage(ciphertext, cipher_state);
}

int main(int argc, char** argv){
  uint8_t state[STATE_ROW_SIZE][STATE_COL_SIZE] = {
    {0x32, 0x88, 0x31, 0xE0},
    {0x43, 0x5A, 0x31, 0x37},
    {0xF6, 0x30, 0x98, 0x07},
    {0xA8, 0x8D, 0xA2, 0x34}
  };

  uint8_t masterkey[STATE_ROW_SIZE][STATE_COL_SIZE] = {
    {0x2B, 0x28, 0xAB, 0x09},
    {0x7E, 0xAE, 0xF7, 0xCF},
    {0x15, 0xD2, 0x15, 0x4F},
    {0x16, 0xA6, 0x88, 0x3C}
  };

  printf("\n----------\nÉtat initial:\n");
  ShowState(state);

  printf("\n----------\nMasterKey:\n");
  ShowState(masterkey);


  /*
  // Appel AddRoundKey
  AddRoundKey(state, masterkey);

  printf("\nÉtat après AddRoundKey:\n");
  ShowState(state);

  // Appel SubBytes
  SubBytes(state);

  printf("\nÉtat après SubBytes:\n");
  ShowState(state);

  // Appel ShiftRows
  ShiftRows(state);

  printf("\nÉtat après ShiftRows:\n");
  ShowState(state);

  // Appel MixColumns
  MixColumns(state);

  printf("\nÉtat après MixColumns:\n");
  ShowState(state);

  printf("\nTest MCMatrixColumns :\n");
  uint8_t column[4] = {0xD4, 0xBF, 0x5D, 0x30};
  MCMatrixColumnProduct(column);
  for (int i=0; i<4; i++){
    printf("%02X\n", column[i]);
  }
  
  // TEST KeyGen
  uint8_t roundkey[ROUND_COUNT][STATE_COL_SIZE][STATE_ROW_SIZE];
  KeyGen(roundkey, masterkey);

  for (int i=0; i<=10; i++){
    printf("\nRoundKey %d:\n", i);
    ShowState(roundkey[i]);
  }

  printf("\nTest Message to State:\n");
  uint8_t newTxt[STATE_COL_SIZE][STATE_ROW_SIZE];
  uint8_t mess[DATA_SIZE] = {
    0x00, 0x01, 0x02, 0x03,
    0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B,
    0x0C, 0x0D, 0x0E, 0x0F };
 
  MessageToState(newTxt, mess);

  ShowState(newTxt);
  */

  uint8_t plaintext[DATA_SIZE];
  uint8_t ciphertext[DATA_SIZE];
  uint8_t key[DATA_SIZE];

  StateToMessage(plaintext, state);
  StateToMessage(key, masterkey);

  AESEncrypt(ciphertext, plaintext, key);

  uint8_t cipher_state[STATE_COL_SIZE][STATE_ROW_SIZE];
  MessageToState(cipher_state, ciphertext);
  
  printf("\n----------\nResultat:\n");
  ShowState(cipher_state);
  return 0;
}
