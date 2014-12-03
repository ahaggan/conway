/*
*
*   Conway's Soldiers
*   Created by Ashley Haggan
*   University of Bristol
*   2014
*
*   Program takes a target row and column for the user, creates a linked list of possible possitions the board can take
*   until it finds the required solution. It then prints the solution in SDL.
*
*
*/


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "neillsdl2.h"

#define YES 1
#define NO 0
#define WIDTH 7
#define HEIGHT 8
#define CLEAR_SCREEN "\033[2J\033[1;1H"
#define RECT_SIZE 20

enum type{up, down, left, right};
typedef enum type type;

typedef struct cell{
    int alive;
}cell;

typedef struct version{
    cell grid[HEIGHT][WIDTH];
    int target_row;
    int target_column;
    int found;
    int counter;
   
    struct version *next;
    struct version *parent;
    //Could I have a dynamically allocated array of child boards, allocated when number of new child boards was calculated
}version;

typedef struct reverse_list{
    version *board;
    struct reverse_list *previous;
}reverse_list;

typedef struct stack{
    reverse_list *pointer_to_stack;
}stack;

version* create_initial_board(void);
void print_board(version *board, SDL_Simplewin *sw);
void print_list(version *board, SDL_Simplewin *sw);
void prepare_solution(version *board, stack *pointer);
void print_solution(version *board, SDL_Simplewin *sw);
version* make_move(version *board);
void copy_cell_array(cell **original, cell **copy);
void make_child(version *parent, version *child);
int check(version *board, int row, int column, type direction);
void add_to_list(version *board, version *tmp_board);
int search_board_list(version *board);
int compare_grid(version *board, version *tmp_board);
version* get_start_board(version *board);
void move(version *board, int row, int column, type direction); 
version* find_target(version *board);
int check_input(int argc, char **argv, version *board);
void initialise_stack(stack *pointer);
void push(stack *pointer, version *board);
version* pop(stack *pointer);
void free_space(version *board);
void print_error(void);
void print_target(version *board, SDL_Simplewin *sw);

int main(int argc, char **argv){
    version *board, *solution;
    board = create_initial_board();
    
    if(check_input(argc, argv, board) == YES){
        solution = find_target(board);
        SDL_Simplewin sw;
        Neill_SDL_Init(&sw);
        
        if(solution->found == YES){
            //print_list(board, &sw);
            print_solution(solution, &sw);
            printf("\nSolution found!");
            printf("\nClick anywhere in the window to exit program.\n");
        }
        else{
            printf("\nMove not possible");
        }
        free_space(board);
        
        do{
            Neill_SDL_Events(&sw);
        }while(!sw.finished);
        
        atexit(SDL_Quit);
        return 0;
    }
    else{
        print_error();
        return 1;
    }
    
}

void print_error(void){
    printf("\n***************************************************************************");
    printf("\n* Your input is incorrect,                                                *");
    printf("\n* You need to enter the executable followed by the width then the height. *");
    printf("\n* Width is between 1 and 7.                                               *");
    printf("\n* Height is between 1 and 8.                                              *");
    printf("\n***************************************************************************\n\n");
}

void free_space(version *board){
    
    if(board->next != NULL){
        free_space(board->next);
    }
    free(board);
}

int check_input(int argc, char **argv, version *board){
    int column, row;

    if(argc >= 3){
    //HOW CAN I CHECK THE INPUTS ARE INTEGERS?
        column = atoi(argv[1]);
        row = atoi(argv[2]);
    }
    else{
        return NO;
    }
    
    if(column > 0 && column <= 7 && row > 0 && row <= 8){
        //User enters a number between 1 and the height and width of the board,
        //but the values used in the program are between 0 and height and width -1.
        board->target_row = row - 1;    
        board->target_column = column - 1;
        printf("\nColumn: %d Row: %d \n", column, row);
        return YES;
        }
    else{
        return NO;
    }
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
            
void print_board(version *board, SDL_Simplewin *sw){

    int row, column;
    SDL_Rect rectangle;
    rectangle.w = RECT_SIZE;
    rectangle.h = RECT_SIZE;
    
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            rectangle.x = column * RECT_SIZE;
            rectangle.y = row * RECT_SIZE;
            if(board->grid[row][column].alive == YES){
                Neill_SDL_SetDrawColour(sw, 0, 255, 0);
                SDL_RenderFillRect(sw->renderer, &rectangle);
                //printf("#");
            }
            else{
                Neill_SDL_SetDrawColour(sw, 0, 0 , 0);
                rectangle.y = row * RECT_SIZE;
                SDL_RenderFillRect(sw->renderer, &rectangle);
                Neill_SDL_SetDrawColour(sw, 255, 255, 255);
                SDL_RenderDrawRect(sw->renderer, &rectangle);
                //printf("0");
            }
        }
        //printf("\n");
    }
    
    print_target(board, sw);
   
    SDL_RenderPresent(sw->renderer);
    SDL_UpdateWindowSurface(sw->win); 
    SDL_Delay(1000);
}    

void print_target(version *board, SDL_Simplewin *sw){
    SDL_Rect rectangle;
    rectangle.w = RECT_SIZE;
    rectangle.h = RECT_SIZE;

    if(board->found == NO){
        Neill_SDL_SetDrawColour(sw, 255, 0, 255);
    }
    else{
        Neill_SDL_SetDrawColour(sw, 0, 0, 255);
    }

    rectangle.x = board->target_column * RECT_SIZE;
    rectangle.y = board->target_row * RECT_SIZE;
    SDL_RenderFillRect(sw->renderer, &rectangle);
}
void print_list(version *board, SDL_Simplewin *sw){
    printf(CLEAR_SCREEN);
    sleep(1);
    print_board(board, sw);
    if(board->next != NULL){
        print_list(board->next, sw);
    }
}

void print_solution(version *board, SDL_Simplewin *sw){
    stack pointer;
    version *tmp_board;
    initialise_stack(&pointer);
    prepare_solution(board, &pointer);
    do{
        tmp_board = pop(&pointer);
        print_board(tmp_board, sw);
    }while(tmp_board->next != NULL);
}
    
void prepare_solution(version *board, stack *pointer){
    push(pointer, board);
    
    if(board->parent != NULL){
        prepare_solution(board->parent, pointer);
    }
}

void push(stack *pointer, version *board)
{
    reverse_list *new;
    new = (reverse_list*)malloc(sizeof(reverse_list));
    new->previous = pointer->pointer_to_stack;
    pointer->pointer_to_stack->board = board;
    pointer->pointer_to_stack = new;
}

version* pop(stack *pointer)
{
    pointer->pointer_to_stack = pointer->pointer_to_stack->previous;
    return pointer->pointer_to_stack->board;
}

void initialise_stack(stack *pointer){
    pointer->pointer_to_stack = (reverse_list*)malloc(sizeof(reverse_list));
    pointer->pointer_to_stack->previous = NULL;
}

version* find_target(version *board){
    version *tmp_board;
    tmp_board = board;
    
    while(tmp_board->found == NO){
        tmp_board = make_move(tmp_board);
    }
    return tmp_board;
}

version* make_move(version *board){
    int row, column;
    type direction;
    version *tmp_board;
    tmp_board = (version*)malloc(sizeof(version));
    
    make_child(board, tmp_board);
    //printf("\nNext Board");
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            for(direction = up; direction <= right; direction++){
            
                if(check(board, row, column, direction) == YES){
                    move(tmp_board, row, column, direction);
                    if (search_board_list(tmp_board) == NO){
                        add_to_list(board, tmp_board);
                        //board->counter += 1;
                        //printf("\n%d", board->counter);
                        if(tmp_board->found == YES){
                            return tmp_board;
                        }
                    }
                    else{
                        free(tmp_board);
                    }
                    tmp_board = (version*)malloc(sizeof(version)); //When and how do I free these mallocs?
                    make_child(board, tmp_board);
                }
            }
        }
    }
    board->next->counter = board->counter;
    return board->next; //will be the next board of moves to check
}

void move(version *board, int row, int column, type direction){
    switch (direction){
        case up:
            board->grid[row][column].alive = NO;
            board->grid[row - 1][column].alive = NO;
            board->grid[row - 2][column].alive = YES;
            if(board->target_row == row - 2 && board->target_column == column){
                board->found = YES;
            }
            break;
        case down:
            board->grid[row][column].alive = NO;
            board->grid[row + 1][column].alive = NO;
            board->grid[row + 2][column].alive = YES;
            if(board->target_row == row + 2 && board->target_column == column){
                board->found = YES;
            }
            break;
        case left:
            board->grid[row][column].alive = NO;
            board->grid[row][column - 1].alive = NO;
            board->grid[row][column - 2].alive = YES;
            if(board->target_row == row && board->target_column == column - 2){
                board->found = YES;
            }
            break;
        case right:
            board->grid[row][column].alive = NO;
            board->grid[row][column + 1].alive = NO;
            board->grid[row][column + 2].alive = YES;
            if(board->target_row == row && board->target_column == column + 2){
                board->found = YES;
            }
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
        }
    }
    child->target_row = parent->target_row;
    child->target_column = parent->target_column;
    child->found = parent->found;
    child->counter = parent->counter;
    
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
    
    
    
    
    
       

