#include "chess_types.h"

#ifndef CHESS_BIT_BOARDS_EVALUTE_POSITION_H
#define CHESS_BIT_BOARDS_EVALUTE_POSITION_H

long long evaluate_position(ChessBoard *board, const LookupTable *tbls, HashTable *t, Board_hash hash_key, int move, int original_depth, int curr_depth);
int check_for_mate_or_path(ChessBoard *board, const LookupTable *tbls, HashTable *t, Board_hash hash_key);
#endif //CHESS_BIT_BOARDS_EVALUTE_POSITION_H
