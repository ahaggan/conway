#include<stdio.h>
#include<stdlib.h>

#define YES 1
#define NO 0
#define WIDTH 7
#define HEIGHT 8

typedef struct cell{
    int alive;
    int left;
    int right;
    int up;
    int down;
}cell;

typedef struct version{
    cell grid[HEIGHT][WIDTH];
    struct version *parent;
    //Could I have a dynamically allocated array of child boards, allocated when number of new child boards was calculated
}version;

version* create_initial_board(void);
void print_board(version *board);
void make_move(version *board);
void copy_cell_array(cell **original, cell **copy);
void make_child(version *parent, version *child);

int main(void){
    version *board;
    board = create_initial_board();
    make_move(board);
    //print_board(board);
}

version* create_initial_board(void){
    int row, column;
    version *new;
    new = (version*)malloc(sizeof(version));
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            if(row > 3){
                new->grid[row][column].alive = YES;
            }
            else{
                new->grid[row][column].alive = NO;
            }
        }
    }
    return new;
}
                
void print_board(version *board){
    int row, column;
    //version tmp_board[HEIGHT][WIDTH];
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            if(board->grid[row][column].alive == YES){
                printf("#");
            }
            else{
                printf("0");
            }
        }
        printf("\n");
    }
}    

void make_move(version *board){
    //int row, column;
    version tmp_board;
    make_child(board, &tmp_board);
    print_board(&tmp_board);
    tmp_board.grid[0][0].alive = YES;
    print_board(&tmp_board);
    print_board(board);
    /*
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
        }
    }*/
}
    
void make_child(version *parent, version *child){
    int row, column;
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            child->grid[row][column].alive = parent->grid[row][column].alive;
            child->grid[row][column].left = parent->grid[row][column].left;
            child->grid[row][column].right = parent->grid[row][column].right;
            child->grid[row][column].up = parent->grid[row][column].up;
            child->grid[row][column].down = parent->grid[row][column].down;
        }
    }
    child->parent = parent;
}
    
    
    
    
    
    
    
       

