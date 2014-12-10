/*
*
*   Conway's Soldiers
*   Created by Ashley Haggan
*   University of Bristol
*   2014
*
*   Program takes a target row and column for the user, creates a hash table of boards, 
*   this table is idexed by the hash_value calculated by each layout of the board.
*   Calculating this hash_value takes about as long as searching each of the boards, grid by grid
*   It then prints the solution in SDL.
*
*
*/


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include "neillsdl2.h"

#define ALIVE_VALUE 19
#define DEAD_VALUE 3
#define YES 1
#define NO 0
#define WIDTH 7
#define HEIGHT 8
#define CLEAR_SCREEN "\033[2J\033[1;1H"
#define RECT_SIZE 20
#define HASH_PRIME 999983 
#define NO_CELLS 56

enum type{up, down, left, right};
typedef enum type type;

enum increment{add = 1, subtract = -1};
typedef enum increment increment;

typedef struct cell{
    int alive;
    int prime;
}cell;

typedef struct version{
    struct version **hash_table; //An array of version pointers
    int hash_value;     //Hash value that has been calculated from the boards grid
    cell grid[HEIGHT][WIDTH];
    int target_row;
    int target_column;
    int found;
    int counter;
    int no_boards;
    struct version *hash_next;
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
void print_plain_board(version *board);
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
void calculate_hash_value(version *board);
int is_prime(int n);

int main(int argc, char **argv){
    //printf("\n1");
    version *hash_table[HASH_PRIME] = {NULL};
    version *board, *solution;
    board = create_initial_board();
    board->hash_table = hash_table;
    
    
    if(check_input(argc, argv, board) == YES){
        printf("\nInput correct");
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
            print_board(board, &sw);
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
    //printf("\n1");
}

version* create_initial_board(void){
    //printf("\n2");
    int row, column, i = 0, j = 2;
    int primes[NO_CELLS];
    
    //Fill the primes array with 56 unique prime numbers to be allocated to the the cells of the grid in turn.
    while(i < NO_CELLS){
        if(is_prime(j) == YES){
            primes[i] = j;
            i++;
        }
        j++;
    }
    //allocate the lower prime numbers to the cels that contain pegs the most to reduce the size of the hash value being worked with.
       
    version *new;
    new = (version*)malloc(sizeof(version));
    new->counter = 0;
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            if(row > 3){
                new->grid[row][column].alive = YES;
                new->counter += 1;
            }
            else{
                new->grid[row][column].alive = NO;
            }
            new->grid[row][column].prime = primes[i];
            i--;
        }
    }
    new->next = NULL; //To avoid being full of garbage.
    calculate_hash_value(new);
    //printf("\n2");
    return new;
}

int is_prime(int n)
{
	int i; //will count the number of non-trivial factors of a given number n	
	for (i = 2; i < n; i++)
	{
		if (n % i == 0)
		{
			return NO;
		}
	}
	return YES;
	
}

int check_input(int argc, char **argv, version *board){
    int column, row;
    //printf("\nStart check input");
    if(argc >= 3){
    //HOW CAN I CHECK THE INPUTS ARE INTEGERS?
        column = atoi(argv[1]);
        row = atoi(argv[2]);
    }
    else{
        //printf("\nEnd check input");
        return NO;
    }
    
    if(column > 0 && column <= 7 && row > 0 && row <= 8){
        //User enters a number between 1 and the height and width of the board,
        //but the values used in the program are between 0 and height and width -1.
        board->target_row = row - 1;    
        board->target_column = column - 1;
        //printf("\nEnd check input");
        return YES;
        }
    else{
        //printf("\nEnd check input");
        return NO;
    }
    
}

void print_error(void){
    printf("\n***************************************************************************");
    printf("\n* Your input is incorrect,                                                *");
    printf("\n* You need to enter the executable followed by the width then the height. *");
    printf("\n* Width is between 1 and 7.                                               *");
    printf("\n* Height is between 1 and 8.                                              *");
    printf("\n* The co-ordinates 1 1 represent the top left corner                      *");
    printf("\n* The co-ordinates 7 8 represent the bottom right corner                  *");
    printf("\n***************************************************************************\n\n");
}

version* find_target(version *board){
    //printf("\nStart find target");
    version *tmp_board;
    tmp_board = board;
    //printf("\ntmp_board = board\n");
    while(tmp_board != NULL && tmp_board->found == NO){
        //printf("\nInside while");
        //printf("\nCell 4 5 = %d", tmp_board->grid[4][5].alive);
        tmp_board = make_move(tmp_board);
        //printf("\nAfter make move");
    }
    //printf("\nEnd find target");
    return tmp_board;
}

version* make_move(version *board){
    //printf("\nStart make move");
    int row, column;
    type direction;
    version *tmp_board;
    tmp_board = (version*)malloc(sizeof(version));
    //printf("\nAllocated memory to tmp_board");
    make_child(board, tmp_board);
    //printf("\nNext Board");
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            for(direction = up; direction <= right; direction++){
            
                if(check(board, row, column, direction) == YES){
                    //printf("\nFinish check\n");
                    move(tmp_board, row, column, direction);
                    //printf("\nFinish move");
                    tmp_board->counter -= 1;
                    calculate_hash_value(tmp_board);
                //    printf("\nEnd calculating hash.");
                    if (search_board_list(tmp_board) == NO){
              //          printf("\nAfter search.");
                        add_to_list(board, tmp_board);
                        //printf("\nAdded to list");
                        board->no_boards += 1;
                        printf("\nNumber of boards %d", board->no_boards);
                        //print_plain_board(tmp_board);
                        if(tmp_board->found == YES){
            //                printf("\nEnd Make move");
                            return tmp_board;
                        }
                    }
                    else{
                        free(tmp_board);
                    }
                    tmp_board = (version*)malloc(sizeof(version)); //When and how do I free these mallocs?
                    make_child(board, tmp_board);
                    //printf("\nMade child board");
                    //SDL_Delay(100);
                }
                else{
                    //printf("\nnothing can move");
                    //SDL_Delay(100);
                }
            }
        }
    }
    //board->next->counter = board->counter;
    board->next->no_boards = board->no_boards;
    //printf("\nEnd make move");
    //printf("\nBefore if statement");
    /*
    if(board->next == NULL){
        printf("\nFinish make move");
        
        return board;
    }
    else{
        *///printf("\nFinish make move");
        //printf("\nCell 4 5 = %d", board->next->grid[4][5].alive);
        return board->next; //will be the next board of moves to check
    //}
}

void make_child(version *parent, version *child){
    //printf("\nStart make child");
    int row, column;
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            child->grid[row][column].alive = parent->grid[row][column].alive;
            child->grid[row][column].prime = parent->grid[row][column].prime;
        }
    }
    child->target_row = parent->target_row;
    child->target_column = parent->target_column;
    child->found = parent->found;
    child->counter = parent->counter;
    child->hash_table = parent->hash_table;
    child->next = NULL;
    child->parent = parent;
    //printf("\nEnd make child");
    
}

int check(version *board, int row, int column, type direction){
    //printf("\nStart Check");
    //printf("\nrow: %d column: %d\n", row, column);
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
            //printf("\nright");
            if(column > WIDTH - 3){
                //printf("\nend right no");
                return NO;
            }
            else{
                if(board->grid[row][column + 1].alive == YES && board->grid[row][column + 2].alive == NO){
                    //printf("\nend right yes");
                    return YES;
                }
            }
            
            break;
    } 
    //printf("\nEnd of check, return NO.");
    return NO;    
}


void move(version *board, int row, int column, type direction){
    //printf("\nstart move");
    switch (direction){
        case up:
            //printf("\nMove up");
            board->grid[row][column].alive = NO;
            board->grid[row - 1][column].alive = NO;
            board->grid[row - 2][column].alive = YES;
            if(board->target_row == row - 2 && board->target_column == column){
                board->found = YES;
            }
            //printf("\nFinish up");
            break;
        case down:
            //printf("\nMove down");
            board->grid[row][column].alive = NO;
            board->grid[row + 1][column].alive = NO;
            board->grid[row + 2][column].alive = YES;
            if(board->target_row == row + 2 && board->target_column == column){
                board->found = YES;
            }
            break;
        case left:
            //printf("\nMove right");
            board->grid[row][column].alive = NO;
            board->grid[row][column - 1].alive = NO;
            board->grid[row][column - 2].alive = YES;
            if(board->target_row == row && board->target_column == column - 2){
                board->found = YES;
            }
            break;
        case right:
            //printf("\nMove left");
            board->grid[row][column].alive = NO;
            board->grid[row][column + 1].alive = NO;
            board->grid[row][column + 2].alive = YES;
            if(board->target_row == row && board->target_column == column + 2){
                board->found = YES;
            }
            break;
    } 
    //printf("\nEnd move");
}

void calculate_hash_value(version *board){
    //printf("\nStart calculating hash value");
    int row, column;
    board->hash_value = 1;
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            //Only multiply by the cell's prime number if alive - should create unique hash values for every board, ont the mod should create duplicates
            //printf("\nIn hashing loop");
            //printf("\nCell prime = %d", board->grid[row][column].prime );
            //printf("\nHash value = %d", board->hash_value);
            if(board->grid[row][column].alive == YES){
                board->hash_value *= board->grid[row][column].prime;
                board->hash_value = board->hash_value % HASH_PRIME;     
            }
        }
    }
    //printf("\nEnd hashing loop");
    
    //printf("Hash value: %d", board->hash_value);
    //printf("\nEnd calculating hash value");
}

void free_space(version *board){
    //printf("\n9");
    if(board->next != NULL){
        free_space(board->next);
    }
    free(board);
    //printf("\n9");
}

version* get_start_board(version *board){
    //printf("\n10");
    if(board->parent == NULL){
        //printf("\n10");
        return board;
    }
    else{
        //printf("\n10");
        return get_start_board(board->parent);
    }  
}

int search_board_list(version *board){
    //printf("\nStart search");
    version *tmp_board, *start_board;
    int found = NO;
    start_board = get_start_board(board);
    //printf("\nBefore if");
    if(start_board->hash_table[board->hash_value] == NULL){
            start_board->hash_table[board->hash_value] = (version*)malloc(sizeof(version));
            board->hash_next = NULL;
            start_board->hash_table[board->hash_value] = board;
            //print_plain_board(start_board->hash_table[board->hash_value]);
     //       printf("\nEnd search - Empty slot");
            return NO;
    }
    tmp_board = start_board->hash_table[board->hash_value];
   // printf("\n Collision, before while loop");
    while(tmp_board != NULL && found == NO){
            
           
    //            printf("\n Collision, before board checked");
                //printf("\nTmp hash value: %d", tmp_board->hash_value);
                //print_plain_board(tmp_board);
                //printf("\nboard hash value: %d", tmp_board->hash_value);
                //print_plain_board(board);
                
                    if(board->counter == tmp_board->counter){
                        found = compare_grid(board, tmp_board);
                    }
                
                
  //                  printf("\nBOARD IN NULL!!");
                
                //print_plain_board(tmp_board);
//                printf("\n after found =");
                //print_plain_board(tmp_board->hash_next);
                
                //
                //
                //PROBLEM HERE
                //
                //
                //if(tmp_board->hash_next != NULL){
                    tmp_board = tmp_board->hash_next;
                    //printf("\nafter tmp board allocated to next.");
                //}
                //else{
                  //  break;  //HOW ELSE CAN I DO THIS??
                //}
    }
    //printf("\nAfter while loop");
    if(found == NO){
        //printf("\nIn if.");
        tmp_board = (version*)malloc(sizeof(version));
        board->hash_next = NULL;
        tmp_board = board;
        //printf("\nENd if.");
    }
    //printf("\nEnd search with collision, board found = %d", found);
    return found;    
    
}

void add_to_list(version *board, version *tmp_board){
    if(board->next == NULL){
        board->next = tmp_board;
    }
    else{
        add_to_list(board->next, tmp_board);
    }
}

int compare_grid(version *board, version *tmp_board){
    //printf("\nStart Compare");
    int row, column;
    for(row = 0; row < HEIGHT; row++){
        for(column = 0; column < WIDTH; column++){
            if( board->grid[row][column].alive != tmp_board->grid[row][column].alive){
      //          printf("\nmismatch");
                //print_plain_board(board);
                //print_plain_board(tmp_board);
                return NO;
            }
        }
    }
    //printf("\nEnd Compare");
    return YES;
}

void print_plain_board(version *board){
    printf("\n13");
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
    
    
    SDL_Delay(100);
    printf("\n13");
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
                SDL_RenderFillRect(sw->renderer, &rectangle);
                //printf("0");
            }
            Neill_SDL_SetDrawColour(sw, 255, 255, 255);
            SDL_RenderDrawRect(sw->renderer, &rectangle);
        }
        //printf("\n");
    }
    
    print_target(board, sw);
   
    SDL_RenderPresent(sw->renderer);
    SDL_UpdateWindowSurface(sw->win); 
    SDL_Delay(1000);
}    

void print_target(version *board, SDL_Simplewin *sw){
    //printf("\n9");
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
   // printf("\n9");
}

void print_list(version *board, SDL_Simplewin *sw){
    //printf("\n10");
    printf(CLEAR_SCREEN);
    sleep(1);
    print_board(board, sw);
    if(board->next != NULL){
        print_list(board->next, sw);
    }
    //printf("\n10");
}

void print_solution(version *board, SDL_Simplewin *sw){
    //printf("\n11");
    stack pointer;
    version *tmp_board;
    initialise_stack(&pointer);
    prepare_solution(board, &pointer);
    do{
        tmp_board = pop(&pointer);
        print_board(tmp_board, sw);
    }while(tmp_board->next != NULL);
    //printf("\n11");
}
    
void prepare_solution(version *board, stack *pointer){
    //printf("\n12");
    push(pointer, board);
    
    if(board->parent != NULL){
        prepare_solution(board->parent, pointer);
    }
    //printf("\n12");
}

void push(stack *pointer, version *board)
{
    //printf("\n13");
    reverse_list *new;
    new = (reverse_list*)malloc(sizeof(reverse_list));
    new->previous = pointer->pointer_to_stack;
    pointer->pointer_to_stack->board = board;
    pointer->pointer_to_stack = new;
    //printf("\n13");
}

version* pop(stack *pointer)
{
    //printf("\n14");
    pointer->pointer_to_stack = pointer->pointer_to_stack->previous;
    return pointer->pointer_to_stack->board;
    //printf("\n14");
}

void initialise_stack(stack *pointer){
    //printf("\n15");
    pointer->pointer_to_stack = (reverse_list*)malloc(sizeof(reverse_list));
    pointer->pointer_to_stack->previous = NULL;
    //printf("\n15");
}

  
    
       

