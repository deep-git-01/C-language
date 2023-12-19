// A simple variant of the game Snake
//
// Used for teaching in classes
//
// Author:
// Franz Regensburger
// Ingolstadt University of Applied Sciences
// (C) 2011
//
// The worm model

#include <curses.h>
#include "worm.h"
#include "board_model.h"
#include "worm_model.h"


// *****************************************************
// Functions concerning the management of the worm data
// *****************************************************

// START WORM_DETAIL
// The following functions all depend on the model of the worm

// Initialize the worm
enum ResCodes initializeWorm(struct worm* aworm, int len_max, int len_cur, struct pos headpos, enum WormHeading dir, enum ColorPairs color) {
    // Initialize indices
    aworm->maxindex = len_max - 1;
    // Current last usable index in array. May grow upto maxindex
    aworm->cur_lastindex = len_cur - 1;
    aworm->headindex = 0;

    for (int i = 0; i <= aworm->maxindex; i++) {
        aworm->wormpos[i].y = UNUSED_POS_ELEM;
        aworm->wormpos[i].x = UNUSED_POS_ELEM;
    }

    // Initialize position of worms head
    aworm->wormpos[aworm->headindex] = headpos;

    // Initialize the heading of the worm
    setWormHeading(aworm, dir);

    // Initialze color of the worm
    aworm->wcolor = color;

    return RES_OK;
}

void growWorm(struct worm * aworm, enum Boni growth) {
        // Play it safe and inhibit surpassing the bound
        if (aworm->cur_lastindex + growth <= aworm->maxindex) {
                  aworm->cur_lastindex += growth;
        } else {
                aworm->cur_lastindex = aworm->maxindex;
        }
}

// Show the worms's elements on the display
void showWorm(struct board* aboard, struct worm* aworm) {
    // Upadate body of the worm
    int tailindex = (aworm->headindex + 1) % (aworm->cur_lastindex + 1);
        for(int i = 0; i <= aworm->cur_lastindex; i++){ 
      if(aworm->wormpos[i].y == -1 && aworm->wormpos[i].x == -1){
        break;
      }
      if(i == aworm->headindex){
        placeItem(
            aboard, 
            aworm->wormpos[i].y, 
            aworm->wormpos[i].x, 
            BC_USED_BY_WORM, 
            SYMBOL_WORM_HEAD_ELEMENT, 
            aworm->wcolor);
        continue;
      }
      else if(i == tailindex){
        placeItem(
            aboard, 
            aworm->wormpos[i].y, 
            aworm->wormpos[i].x, 
            BC_USED_BY_WORM, 
            SYMBOL_WORM_TAIL_ELEMENT, 
            aworm->wcolor);
        continue;
      }
      placeItem(
          aboard, 
          aworm->wormpos[i].y, 
          aworm->wormpos[i].x, 
          BC_USED_BY_WORM, 
          SYMBOL_WORM_INNER_ELEMENT, 
          aworm->wcolor);
    }
}

void cleanWormTail(struct board* aboard, struct worm* aworm) {
    int tailindex = (aworm->headindex + 1) % (aworm->cur_lastindex + 1);
    if (getContentAt(aboard, aworm->wormpos[tailindex]) == BC_USED_BY_WORM) {
        placeItem(aboard,
                  aworm->wormpos[tailindex].y,
                  aworm->wormpos[tailindex].x,
                  BC_FREE_CELL,
                  SYMBOL_FREE_CELL,
                  COLP_FREE_CELL);
    }
}

void moveWorm(struct board* aboard, struct worm* aworm, enum GameStates* agame_state) {
    // Compute and store new head position according to current heading.
    struct pos new_headpos;
    new_headpos.y = aworm->wormpos[aworm->headindex].y + aworm->dy;
    new_headpos.x = aworm->wormpos[aworm->headindex].x + aworm->dx;

    // Check if we would leave the display if we move the worm's head according
    // to worm's last direction.
    // We are not allowed to leave the display's window.
    if (new_headpos.x < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (new_headpos.x > getLastColOnBoard(aboard)) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (new_headpos.y < 0) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else if (new_headpos.y > getLastRowOnBoard(aboard)) {
        *agame_state = WORM_OUT_OF_BOUNDS;
    } else {
        // We will stay within bounds.
        // Check if the worms head hits any items at the new position on the board.
         // Hitting food is good, hitting barriers or worm elements is bad.
         switch ( getContentAt(aboard, new_headpos) ) {
             case BC_FOOD_1:
                 *agame_state = WORM_GAME_ONGOING;
                  // Grow worm according to food item digested
                  growWorm(aworm, BONUS_1);
                  decrementNumberOfFoodItems(aboard);
                  break;
             case BC_FOOD_2:
                  *agame_state = WORM_GAME_ONGOING;
                  growWorm(aworm, BONUS_2);
                  decrementNumberOfFoodItems(aboard);
                  break;
             case BC_FOOD_3:
                  *agame_state = WORM_GAME_ONGOING;
                  growWorm(aworm, BONUS_3);
                  decrementNumberOfFoodItems(aboard);
                  break;
             case BC_BARRIER:
                   // That's bad: stop game
                   *agame_state = WORM_CRASH;
                   break;
             case BC_USED_BY_WORM:
                   // That's bad: stop game
                   *agame_state = WORM_CROSSING;
                   break;
            default:
            // Without default case we get a warning message.
            {;} // Do nothing. C syntax dictates some statement, here.
    }
  }
      
        // Check the status of *agame_state
        // Go on if nothing bad happened
        if ( *agame_state == WORM_GAME_ONGOING ) {
            // So all is well: we did not hit anything bad and did not leave the
            // window. --> Update the worm structure.
            // Increment headindex
            // Go round if end of worm is reached (ring buffer)
            aworm->headindex++;
        if (aworm->headindex > aworm->cur_lastindex) {
            aworm->headindex = 0;
        }
        // Store new coordinates of head element in worm structure
        aworm->wormpos[aworm->headindex] = new_headpos;
  }
}

// Setters
void setWormHeading(struct worm* aworm, enum WormHeading dir) {
    switch(dir) {
        case WORM_UP :// User wants up
            aworm->dx=0;
            aworm->dy=-1;
            break;
        case WORM_DOWN:// User wants down
            aworm->dx=0;
            aworm->dy=1;
            break;
        case WORM_LEFT:// User wants left
            aworm->dx=-1;
            aworm->dy=0;
            break;
        case WORM_RIGHT:// User wants right
            aworm->dx=1;
            aworm->dy=0;
            break;
    }
}


// Getters
struct pos getWormHeadPos (struct worm* aworm) {
        // Structures are passed by value!
        // ->f we return a copy here
        return aworm->wormpos[aworm->headindex];
}
int getWormLength(struct worm* aworm){
        return aworm->cur_lastindex + 1;
}

