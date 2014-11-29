#include<stdio.h>
#include<stdlib.h>

#define YES 1
#define NO 0
#define WIDTH 7
#define HEIGHT 8

enum type{up, down, left, right};
typedef enum type type;

typedef struct cell{
    int alive;
    int left;
    int right;
    int up;
    int down;
}cell;

typedef struct version{
    cell grid[HEIGHT][WIDTH];
    int target_row;
    int target_column;
    struct version *next;
    struct version *parent;
    //Could I have a dynamically allocated array of child boards, allocated when number of new child boards was calculated
}version;

version* create_initial_board(void);
void print_board(version *board);
void print_list(version *board);
void make_move(version *board);
void copy_cell_array(cell **original, cell **copy);
void make_child(version *parent, version *child);
int check(version *board, int row, int column, type direction);
void add_to_list(version *board, version *tmp_board);
int search_board_list(version *board);
int compare_grid(version *board, version *tmp_board);
version* get_start_board(version *board);
void move(version *board, int row, int column, type direction); 

int main(void){
    version *board;
    board = create_initial_board();
    make_move(board);
    print_list(board);
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

version* get_start_board(version *board){
    
    if(board->parent == NULL){
        return board;
    }
    else{
        return get_start_board(board->parent);
    }  

}

int search_board_list(version *board){

    version *tmp_board;
    int found = NO;
    tmp_board = get_start_board(board);
    while(tmp_board != NULL && found == NO){
        found = compare_grid(board, tmp_board);
        tmp_board = tmp_board->next;
    }
    return found;    
    
}
  
int compare_grid(version *board, version *tmp_board){
    int row, column;
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            if( board->grid[row][column].alive != tmp_board->grid[row][column].alive){
                return NO;
            }
        }
    }
    return YES;
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
    printf("\n\n");
    
}    

void print_list(version *board){
    print_board(board);
    if(board->next != NULL){
        print_list(board->next);
    }
}

void make_move(version *board){
    int row, column;
    type direction;
    version *tmp_board;
    tmp_board = (version*)malloc(sizeof(version));
    
    make_child(board, tmp_board);
    
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            for(direction = up; direction <= right; direction++){
            
                if(check(board, row, column, direction) == YES){
                    move(tmp_board, row, column, direction);
                    
                    if (search_board_list(tmp_board) != YES){
                        add_to_list(board, tmp_board);
                    }
                    //print_board(tmp_board);
                    tmp_board = (version*)malloc(sizeof(version)); //When and how do I free these mallocs?
                    make_child(board, tmp_board);
                }
            }
        }
    }
    
}
void move(version *board, int row, int column, type direction){
    switch (direction){
        case up:
            board->grid[row][column].alive = NO;
            board->grid[row - 1][column].alive = NO;
            board->grid[row - 2][column].alive = YES;
            break;
        case down:
            board->grid[row][column].alive = NO;
            board->grid[row + 1][column].alive = NO;
            board->grid[row + 2][column].alive = YES;
            break;
        case left:
            board->grid[row][column].alive = NO;
            board->grid[row][column - 1].alive = NO;
            board->grid[row][column - 2].alive = YES;
            break;
        case right:
            board->grid[row][column].alive = NO;
            board->grid[row][column + 1].alive = NO;
            board->grid[row][column + 2].alive = YES;
            break;
    } 
}

int check(version *board, int row, int column, type direction){
    if(board->grid[row][column].alive == NO){
        return NO;
    }
    //ARE THE 1's AND 2's MAGIC NUMBERS?
    switch (direction){
        case up:
            if(row < 2){
                return NO;
            }
            else{
                if(board->grid[row - 1][column].alive == YES && board->grid[row - 2][column].alive == NO){
                    return YES;
                }
            }
            break;
        case down:
            if(row > HEIGHT - 3){
                return NO;
            }
            else{
                if(board->grid[row + 1][column].alive == YES && board->grid[row + 2][column].alive == NO){
                    return YES;
                }
            }
            break;
        case left:
            if(column < 2){
                return NO;
            }
            else{
                if(board->grid[row][column - 1].alive == YES && board->grid[row][column - 2].alive == NO){
                    return YES;
                }
            }
            break;
        case right:
           if(column > WIDTH - 3){
                return NO;
            }
            else{
                if(board->grid[row][column + 1].alive == YES && board->grid[row][column + 2].alive == NO){
                    return YES;
                }
            }
            break;
    } 
    return NO;
    
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
    child->target_row = parent->target_row;
    child->target_column = parent->target_column;
    child->parent = parent;
}
    
void add_to_list(version *board, version *tmp_board){
    if(board->next == NULL){
        board->next = tmp_board;
    }
    else{
        add_to_list(board->next, tmp_board);
    }
}
    
    
    
    
    
       

