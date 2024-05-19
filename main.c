#include <stdio.h>
#include <limits.h>
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

//// Imposible end games, check player function for entering moves, game tackticks

HashTable t;
ChessBoard board;
LookupTable tbls;
Keys keys;
MoveList list;

void player_vs_player()
{
    while(1)
    {
        print_chess_board(&board);
        board.turn == white ? printf("WHITE\n") : printf("BLACK\n");
        player_make_move(&board, &tbls, &keys, &t);
        Board_hash new_hash = get_bord_hash(&board, &keys);
        int state = check_for_mate_or_path(&board, &tbls, &t, new_hash);

        if(state == 1)
        {
            printf("End game mate!!!\n");
            break;
        }

        if(state == 2)
        {
            printf("End game path!!!\n");
            break;
        }

        board.turn = !board.turn;

    }

}

void player_vs_bot()
{
    int difficulty_bot;
    printf("Select difficulty level for Bot:\n");
    printf("1. Low\n");
    printf("2. Medium\n");
    printf("3. High\n");
    printf("Enter choice: ");
    scanf("%d", &difficulty_bot);
    if (scanf("%d", &difficulty_bot) != 1 || difficulty_bot < 1 || difficulty_bot > 3) 
    {
        printf("Invalid input for Bot 1 difficulty level. Exiting.\n");
        return;
    }

    int turn;
    printf("For white 1, for black 2\n");
    scanf(" %d", &turn);
    
    if(turn != 1 && turn != 2)
    {
        printf("Invalid input!!!\n");
        return;
    }
        
    int depth = difficulty_bot * 2; 

    while(1)
    {
        long long eval = 0;
        int move;
        board.turn == white ? printf("WHITE\n") : printf("BLACK\n");
        printf("depth: %d\n", depth);
        
        if(turn == 1)
        {
            player_make_move(&board, &tbls, &keys, &t);
        }

        if(turn == 2)
        {
            if (!min_max(&board, &tbls, &keys, &t, &move, &eval, depth, LLONG_MIN, LLONG_MAX, depth)) 
            {
                printf("Could not find best move, terminating.\n");
                break;
            }

            print_move(move);
            play_move(move, &board, &keys, &t);
        }
        
        printf("===============================================================================\n");
        printf("eval: %lli\n", eval);
        print_chess_board(&board);
        board.turn  = !board.turn;
    }

}

void bot_vs_bot()
{
    int difficulty_bot1, difficulty_bot2;
    printf("Select difficulty level for Bot 1:\n");
    printf("1. Low\n");
    printf("2. Medium\n");
    printf("3. High\n");
    printf("Enter choice: ");
    if (scanf(" %d", &difficulty_bot1) != 1 || difficulty_bot1 < 1 || difficulty_bot1 > 3) 
    {
        printf("Invalid input for Bot 1 difficulty level. Exiting.\n");
        return;
    }

    printf("\nSelect difficulty level for Bot 2:\n");
    printf("1. Low\n");
    printf("2. Medium\n");
    printf("3. High\n");
    printf("Enter choice: ");
    if (scanf(" %d", &difficulty_bot2) != 1 || difficulty_bot2 < 1 || difficulty_bot2 > 3) 
    {
        printf("Invalid input for Bot 2 difficulty level. Exiting.\n");
        return;
    }   

    int depth1 = difficulty_bot1 * 2; 
    int depth2 = difficulty_bot2 * 2;
    
    print_chess_board(&board);
    for(int i = 0; 1 ; i++)
    {
        long long eval;
        int move;
        board.turn == white ? printf("WHITE\n") : printf("BLACK\n");
        int depth = (board.turn == white) ? depth1 : depth2; 
        printf("depth: %d\n", depth);
        
        if (!min_max(&board, &tbls, &keys, &t, &move, &eval, depth, LLONG_MIN, LLONG_MAX, depth)) 
        {
            printf("Could not find best move, terminating.\n");
            break;
        }

        printf("eval: %lli\n", eval);

        play_move(move, &board, &keys, &t);
        print_chess_board(&board);
        print_move(move);
        printf("===============================================================================\n");
        board.turn  = !board.turn;
    }
}

int check_for_mate_or_path(ChessBoard *board, const LookupTable *tbls, HashTable *t, Board_hash hash_key); 

int main()
{
    for(int i = w_pawn;i <= b_king; i++)
        print_piece(i);

    printf("\n");

    printf(""
    "-----------------------------------\n"
    "       Introduction to Chess      \n"
    "-----------------------------------\n"
    "Chess is a timeless board game that has been played and enjoyed for\n"
    "centuries. It's a game of strategy, tactics, and skill, often referred\n"
    "to as \"the game of kings.\" In chess, two players face off against each\n"
    "other on a square board with 64 squares arranged in an 8x8 grid.\n\n"
    "Objective\n"
    "The objective of chess is to checkmate your opponent's king. Checkmate\n"
    "occurs when the opponent's king is in a position to be captured (in\n"
    "check), and there is no legal move to escape the threat. The game ends\n"
    "immediately when a player's king is checkmated, and that player loses\n"
    "the game.\n\n"
    "Setup\n"
    "Before starting the game, the board is set up in a specific way:\n"
    "- Each player has 16 pieces: one king, one queen, two rooks, two\n"
    "knights, two bishops, and eight pawns.\n"
    "- The pieces are arranged on the first two rows (ranks) closest to each\n"
    "player.\n"
    "- The back row (rank 1 for white, rank 8 for black) contains, from left\n"
    "to right: rook, knight, bishop, queen, king, bishop, knight, and rook.\n"
    "- The front row (rank 2 for white, rank 7 for black) contains all eight\n"
    "pawns.\n\n"
    "Movement of Pieces\n"
    "Each type of piece moves differently:\n"
    "1. King: Can move one square in any direction.\n"
    "2. Queen: Can move any number of squares horizontally, vertically, or\n"
    "   diagonally.\n"
    "3. Rook: Can move any number of squares horizontally or vertically.\n"
    "4. Bishop: Can move any number of squares diagonally.\n"
    "5. Knight: Moves in an L-shape (two squares in one direction and then\n"
    "   one square perpendicular).\n"
    "6. Pawn: Moves forward one square, but captures diagonally. On its first\n"
    "   move, a pawn can move forward two squares.\n\n"
    "Special Moves\n"
    "- Check: When a player's king is under attack by an opponent's piece.\n"
    "- Checkmate: When a player's king is in check and there is no legal\n"
    "  move to escape the threat.\n"
    "- Stalemate: When a player's king is not in check but has no legal\n"
    "  moves to make. The game ends in a draw.\n"
    "- En passant: A special pawn capture that occurs if a pawn moves two\n"
    "  squares forward from its starting position and lands next to an\n"
    "  opponent's pawn, which has the option to capture it as if it only\n"
    "  moved one square forward.\n"
    "- Castling: A special move involving the king and one of the rooks. It\n"
    "  allows the king to move two squares towards a rook on its initial\n"
    "  square, and that rook to move to the square next to the king.\n\n"
    "Conclusion\n"
    "Chess is a game that combines strategic thinking, tactical planning,\n"
    "and careful execution. It's a game of skill, patience, and practice.\n"
    "Learning the fundamentals of chess opens the door to endless\n"
    "possibilities and challenges, making it a rewarding and intellectually\n"
    "stimulating pursuit for players of all ages and skill levels.\n"
    "-----------------------------------\n");

    create_table(&t, 10000);
//	board = parse_FEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	board = parse_FEN("8/8/5q2/8/8/2k5/K7/8 b");
    tbls  = fill_lookup_table();
    keys  = init_random_keys();
    list  = init_move_list();

//    print_chess_board(&board);
//    return 0;
    int choice;
    printf("Welcome to Console Chess!\n");
    printf("1. Player vs. Player\n");
    printf("2. Player vs. Bot\n");
    printf("3. Bot vs. Bot\n");
    printf("Choose an option: ");
    scanf("%d", &choice);

    switch (choice)
    {
        case 1:
            player_vs_player();
            break;
        case 2:
            player_vs_bot();
            break;
        case 3:
            bot_vs_bot();
            break;
        default:
            printf("Invalid choice\n");
            return 1;
    }
    

    return 0;
}
