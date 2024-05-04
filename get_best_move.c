#include<stdio.h>
#include<limits.h>
#include <string.h>
#include "chess_types.h"
#include "evalute_position.h"
#include "get_best_move.h"
#include "init_functions.h"
#include "make_move.h"
#include "move_generation.h"
#include "piece_movement.h"
#include "player_interface.h"
#include "zobrist_hashing.h"
#include "position_hash_table.h"

enum bool min_max(ChessBoard *board, const LookupTable *tbls, const Keys *keys, HashTable *t, int *out_move, long long  *out_eval, int depth, long long alpha, long long beta, int original_depth)
{
    MoveList list = init_move_list();

    ///generate position moves
    generate_position_moves(board, tbls, &list);
    sieve_moves(&list, board, tbls);

    ///Move ordering speeds up the code by making capturing moves be seen first, therefor sooner alpha beta cuts accurr.
    move_ordering_by_capture(&list);

    int best_move = -1;
    long long best_eval = (board->turn == white) ? -CHECK_MATE_V-1 : CHECK_MATE_V+1;
    
    ///if the position is mate and there are no moves
    *out_eval = best_eval;

    //if (depth == 2) {
    //    enum bool flag = false;
    //    for (unsigned int move_index = 0; move_index < list.count; move_index ++){
    //        enum piece ispr = DECODE_MOVE_PROMOTED_PIECE(list.moves[move_index]);
    //        enum piece iscap = DECODE_MOVE_CAPTURE(list.moves[move_index]);
    //        if (ispr != empty && iscap == empty) {
    //            flag = true;
    //            print_move(list.moves[move_index]);
    //        }
    //    }
    //    if (flag) {
    //        printf("&***********************************\n");
    //    }
    //}


    for(unsigned int move_index = 0; move_index < list.count; move_index ++)
    {
        long long curr_eval = 0;
        int curr_move = list.moves[move_index]; 

        /// copy board before the move is done
        Bitboard pieces[12], occupied[3];
        ChessBoard saveboard;
        memcpy(&saveboard, board, sizeof(*board));

        play_move(curr_move, board);
          
        /// hash key after the played move
        Board_hash new_key = get_bord_hash(board, keys);
        insert_item(t, new_key);

        ///calculate new position evaluation
        if(depth == 0)
            curr_eval = evaluate_position(board, tbls, t, new_key, curr_move, depth, original_depth);
        int a = check_for_mate_or_path(board, tbls, t, new_key);
        if(a == 1)
            printf("Math!!!\n");
        if(a == 2)
            printf("Path!!!\n");
        ///change the turn
        board->turn = !board->turn;

        /// going one depth further
        if(depth)
            min_max(board, tbls, keys, t, out_move, &curr_eval, depth - 1, alpha, beta, original_depth);

        ///set to original
        board->turn = !board->turn;
        
        ///if the current position is better then the current best update
        if((board->turn == white && best_eval < curr_eval) || (board->turn == black && best_eval > curr_eval))
        {
            (board->turn == white) ? (alpha = curr_eval) : (beta = curr_eval);
            best_eval = curr_eval;
            best_move = curr_move;
        }

        /// undo the move

        memcpy(board, &saveboard, sizeof(*board));

        /// removing the position if it is seen for the first time, or decresing the seen counter
        remove_item(t, new_key);

        if(beta <= alpha) 
        {
            //printf("Alpha Beta prunning accurr depth: %d\n", depth);
            break;
        }
    }

    if(best_move == -1)
    {
		*out_eval = (board->turn == white) ? -CHECK_MATE_V : CHECK_MATE_V;
		return false;
	}
    
    *out_move = best_move;
    *out_eval = best_eval;
    
    return true;
}