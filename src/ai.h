#ifndef __AI__
#define __AI__

#include <stdint.h>
#include <unistd.h>
#include "utils.h"

void copy_state(state_t *dst, state_t *src);
void save_solution(node_t *solution_node);
void free_list(node_t *node);
node_t *create_init_node(state_t *init_state);
node_t *applyAction(node_t *n, position_s *selected_peg, move_t action);
void delete_nodes_upto(node_t *n, node_t *upto);
void find_solution( state_t* init_state );

#endif
