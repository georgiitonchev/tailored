// loop through all characters
//      sets current cell as occupied (obstacle)
//      picks a target based on distance
//
//      if within range
//          -> perform attacks
//      else
//          picks cell neighbour of target based on direction
//          pass that cell and current sell to pathfinding
//          get result path's first cell and set as occupied
//          -> navigate to cell