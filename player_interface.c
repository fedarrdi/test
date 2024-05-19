#include <stdio.h>
#include "chess_types.h"
#include "move_generation.h"
#include "position_hash_table.h"
#include "zobrist_hashing.h"
#include "piece_movement.h"
#include "init_functions.h"
#include "make_move.h"

int mod(int a) { return a < 0 ? -a : a; }

void print_piece(int piece_type)
{
    switch (piece_type)
    {
        case 0: printf(" \u265F |"); break;
        case 1: printf(" \u265E |"); break;
        case 2: printf(" \u265D |"); break;
        case 3: printf(" \u265C |"); break;
        case 4:printf(" \u265B |"); break;
        case 5:printf(" \u265A |"); break;
        case 6: printf(" \u2659 |"); break;
        case 7: printf(" \u2658 |"); break;
        case 8: printf(" \u2657 |"); break;
        case 9: printf(" \u2656 |"); break;
        case 10: printf(" \u2655 |"); break;
        case 11: printf(" \u2654 |"); break;
        default: printf("   |"); break;
    }
}

void print_chess_board(ChessBoard *board)
{
    printf("\n");
    for(int y = 0;y < 8;y++)
    {
        printf("    |");
        for (int x = 0; x < 8; x++)
            printf("---+");

        printf("\n");
        printf("%d ->|", 8 - y);
        for(int x = 0;x < 8;x++)
        {
            int square = y * 8 + x;
            int piece = empty; 
            if((board->occupied[both] & (1ULL << square)))
                for(int piece_index = 0;piece_index < 12; piece_index++)
                    if((board->pieces[piece_index] & (1ULL << square)))
                    {
                        piece = piece_index;
                        break;
                    }
            print_piece(piece);
        }

        if (y == 8 - 1)
        {
            printf("\n"); printf("    |");
            for (int x = 0; x < 8; x++)
                printf("---+");
        }
        printf("\n");
    }

    printf("      A   B   C   D   E   F   G   H");
    printf("\n\n");
}

void print_move(int move)
{

    const char *square_string[] = {"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
                                   "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
                                   "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
                                   "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                                   "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
                                   "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
                                   "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
                                   "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

    const char *piece_to_symbol = "PHBRQKphbrqk0";

    printf("MOVE from: %s\n", square_string[DECODE_MOVE_FROM(move)]);
    printf("MOVE to: %s\n", square_string[DECODE_MOVE_TO(move)]);
    printf("MOVE piece: %c\n", piece_to_symbol[DECODE_MOVE_PIECE(move)]);
    printf("MOVE promotion: %c\n", piece_to_symbol[DECODE_MOVE_PROMOTED_PIECE(move)]);
    printf("MOVE capture piece: %c\n", piece_to_symbol[DECODE_MOVE_CAPTURE(move)]);
    printf("MOVE is double pawn push: %d\n", DECODE_MOVE_DOUBLE_PAWN_PUSH(move) ? 1 : 0);
    printf("MOVE is en passant: %d\n", DECODE_MOVE_ENPASSANT(move) ? 1 : 0);
    printf("MOVE is castle: %d\n", DECODE_MOVE_CASTLING(move) ? 1 : 0);

    printf("\n MOVE VALUE: %d\n\n", move);

}

void player_make_move(ChessBoard *board, const LookupTable *tbls, const Keys *keys, HashTable *t)
{
    enum color side = board->turn;
    int enpassant = 0;
    int rank;
    char file;

    back:;

    printf("Please enter file.\n");
    scanf(" %c", &file);

    while(file < 'A' || file > 'H')
    {
        printf("File incorrect!\n");
        printf("Please enter file.\n");
        scanf(" %c", &file);
    }

    printf("Please enter rank.\n");
    scanf(" %d", &rank);

    while(rank < 1 || rank > 8)
    {
        printf("Rank incorrect!\n");
        printf("Please enter rank.\n");
        scanf(" %d", &rank);
    }

    int from_square = (8 - rank) * 8 + file - 'A';
    
    if(!(board->occupied[side] & (1ULL << from_square)))
    {
        printf("Invalid position!!!\n");
        goto back;
    }

    printf("Please enter file.\n");
    scanf(" %c", &file);

    while(file < 'A' || file > 'H')
    {
        printf("File incorrect!\n");
        printf("Please enter file.\n");
        scanf(" %c", &file);
    }

    printf("Please enter rank.\n");
    scanf(" %d", &rank);

    while(rank < 0 || rank > 8)
    {
        printf("Rank incorrect!\n");
        printf("Please enter Rank.\n");
        scanf("%d", &rank);
    }

    int to_square = (8 - rank) * 8 + file - 'A';

    int curr_piece = empty;
    for(enum piece i = w_pawn;i <= b_king; i++)
    {
        if(board->pieces[i] & (1ULL << from_square))
        {
            curr_piece = i;
            break;
        }
    }

    if(curr_piece == empty)
    {
        printf("No Piece!!!\n");
        goto back;
    }

    Bitboard moves = get_piece_move(curr_piece, 1ULL << from_square, board->occupied[side], board->occupied[!board->turn], tbls);

    if(!(moves & (1ULL << to_square)))
    {
        printf("This move is invlaid!!!\n");
        goto back;
    }


    int capture = empty;
    for(enum piece i = w_pawn; i <= b_king; i++)
    {
        if(board->pieces[i] & (1ULL << to_square))
        {
            capture = i;
            break;
        }
    }

    int promotion = empty;
    if(curr_piece == w_pawn && ((1ULL << to_square) & tbls->MaskRank[RANK_8]))
    {
        printf("To what piece you whant to promote\n");
        for(int i = w_knight; i <= w_queen; i++)
        {
            printf(" %d: ", i);
            print_piece(i);
        }
        scanf(" %d", &promotion);
    }

    if(curr_piece == b_pawn && ((1ULL << to_square) & tbls->MaskRank[RANK_1]))
    {
        printf("To what piece you want to promote\n");
        for(int i = b_knight; i <= b_queen; i++)
        {
            printf(" %d: ", i);
            print_piece(i);
        }
        scanf(" %d", &promotion);
    }
    int en_passant_flag = 1;
    if((1ULL << to_square) & board->en_passant[side])
    {
        en_passant_flag = 1;
    }

    int double_push_pawn_flag = 0;
    if((curr_piece == w_pawn || curr_piece == b_pawn) && mod(to_square - from_square) == 16)
    {
        double_push_pawn_flag = 1;
    }

    int castle_flag = 0;
    if((curr_piece == w_king || curr_piece == b_king) && mod(to_square - from_square) == 2)
    {
        castle_flag = 1;
    }

    int move = ENCODE_MOVE(from_square, to_square, curr_piece, promotion,capture, double_push_pawn_flag, enpassant, castle_flag);

    MoveList list = init_move_list();

    generate_position_moves(board, tbls, &list);
    sieve_moves(&list, board, tbls);

    int a = 0;
    for(int i = 0;i < list.count;i++)
    {
        if(move == list.moves[i])
        {
            a = 1;
            break;
        }
    }

    if(!a)
    {
        printf("Invalid move!!!\n");
    }

    play_move(move, board, keys, t);

}

void print_move_list(const MoveList *list)
{
    printf("\n\n\n**************** Position moves ****************\n\n\n");
    printf("list size: %d\n", list->count);
    for(int i = 0;i < list->count;i++)
        if(list->moves[i])
            print_move(list->moves[i]);

    printf("\n\n\n************************************************\n\n\n");
}
