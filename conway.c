#include<stdio.h>
#include<stdlib.h>

#define YES 1
#define NO 0
#define WIDTH 7
#define HEIGHT 8

typedef struct cell{
    int alive;
}cell;

typedef struct version{
    cell grid[HEIGHT][WIDTH];
    struct version *parent;
    //Could I have a dynamically allocated array of child boards, allocated when number of new child boards was calculated
}version;

version* create_initial_board(void);
void print_board(version *board);

int main(void){
    version *board;
    board = create_initial_board();
    print_board(board);
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

