#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "ai.h"
#include "utils.h"
#include "hashtable.h"
#include "stack.h"

void copy_state(state_t *dst, state_t *src)
{

  //Copy field
  memcpy(dst->field, src->field, SIZE * SIZE * sizeof(int8_t));

  dst->cursor = src->cursor;
  dst->selected = src->selected;
}

/**
 * Saves the path up to the node as the best solution found so far
*/
void save_solution(node_t *solution_node)
{
  node_t *n = solution_node;
  while (n->parent != NULL)
  {
    copy_state(&(solution[n->depth]), &(n->state));
    solution_moves[n->depth - 1] = n->move;

    n = n->parent;
  }
  solution_size = solution_node->depth;
}

/**
 * Frees linked list.
 */
void free_list(node_t *node)
{
  node_t *n = node;
  while (n != NULL)
  {

    node_t *tmp = n;
    n = n->parent;
    free(tmp);
  }
}

node_t *create_init_node(state_t *init_state)
{
  node_t *new_n = (node_t *)malloc(sizeof(node_t));
  new_n->parent = NULL;
  new_n->depth = 0;
  copy_state(&(new_n->state), init_state);
  return new_n;
}

/**
 * Apply an action to node n and return a new node resulting from executing the action
*/
node_t *applyAction(node_t *n, position_s *selected_peg, move_t action)
{
  node_t *new_node = create_init_node(&(n->state));

  //1 Point new node to parent node.
  new_node->parent = n;

  //2 Update the required fields of state.
  new_node->state.cursor = *selected_peg;
  new_node->state.selected = true;

  //3 Increment depth.
  new_node->depth = n->depth + 1;

  //4 Update the action used to create the node.
  new_node->move = action;

  // Execute move on node as specified by the state, cursor and action in the parameter.
  execute_move_t(&(new_node->state), &(new_node->state.cursor), action);
  return new_node;
}

/**
 * Deletes nodes from "n" to "upto".
 */
void delete_nodes_upto(node_t *n, node_t *upto)
{
  // Stop while loop if "n->parent" has equal or less pegs than "upto".
  while (num_pegs(&n->state) <= num_pegs(&upto->state))
  {
    node_t *tmp = n;
    n = n->parent;
    free(tmp);
  }
}

/**
 * Find a solution path as per algorithm description in the handout
 */

void find_solution(state_t *init_state)
{
  HashTable table;

  // Choose initial capacity of PRIME NUMBER.
  // Specify the size of the keys and values you want to store once.
  ht_setup(&table, sizeof(int8_t) * SIZE * SIZE, sizeof(int8_t) * SIZE * SIZE, 16769023);

  // Initialize Stack
  initialize_stack();

  // Create the initial node
  node_t *n = create_init_node(init_state);

  // Push initial node to stack.
  stack_push(n);

  // Update "remaininPegs" with the initial number of pegs.
  int remainingPegs = num_pegs(&(n->state));

  // Initialize variables used to traverse the board.
  // Where "x" and "y" are the coordinates of the board and "i" is the direction of action.
  int x, y, i, flag = 0;

  // While loop through stack and exit if stack is empty.
  while (!is_stack_empty(stack))
  {
    // If no new node was pushed to the stack, it means "n" has no children.
    if (flag)
    {
      // Therefore we can delete nodes up to the parent of the top stack.
      delete_nodes_upto(n, stack[stack_top_idx]);
    }
    // Update flag to 1.
    flag = 1;

    // Grab node at the top of the stack and assign it to "n".
    n = stack_top();

    // Decrements stack_top_index.
    stack_pop();

    // Update global variable "expanded_nodes" which checks for total expanded nodes.
    expanded_nodes += 1;

    // If a state with fewer number of pegs is found,
    if (num_pegs(&(n->state)) < remainingPegs)
    {
      // Save the path to this node,
      save_solution(n);

      // Update "remainingPegs".
      remainingPegs = num_pegs(&(n->state));
    }
    // Traverse every position of the board.
    for (x = 0; x < SIZE; x++)
    {
      for (y = 0; y < SIZE; y++)
      {

        // Create position variable with the x-y coordinate.
        position_s position = {x, y};

        // Loops through every direction.
        for (i = left; i <= down; i++)
        {
          // For every position at any direction, if it is a valid move,
          if (can_apply(&(n->state), &position, i))
          {
            // Apply the action and return the resulting node.
            node_t *new = applyAction(n, &position, i);

            // Increment "generated_nodes".
            generated_nodes++;

            // If the new node is a winning node,
            if (won(&(new->state)))
            {
              // Save the path up to this node.
              save_solution(new);

              // Update "remainingPegs".
              remainingPegs = num_pegs(&(new->state));

              // Destroy and free hash table.
              ht_destroy(&table);
              // Free path of nodes up to this point.
              free_list(new);
              // Free the stack.
              free_stack();

              return;
            }
            // If the created node is not contained in the hashtable,
            if (!ht_contains(&table, &new->state.field))
            {
              // Update the flag since "n" has children,
              flag = 0;
              // Push the new node to the stack.
              stack_push(new);
              // Insert new node into hashtable.
              ht_insert(&table, &new->state.field, &new->state.field);
            }
            // If the created node is not pushed to the stack, it won't have any more children.
            // Meaning we can free it.
            else
            {
              free(new);
            }
          }
        }
      }
    }
    // Check if over budget.
    if (expanded_nodes >= budget)
    {
      // Destroy and free hash table.
      ht_destroy(&table);
      // Free path of nodes up to this point.
      free_list(n);
      // Free the stack.
      free_stack();

      return;
    }
  }
  return;
}
