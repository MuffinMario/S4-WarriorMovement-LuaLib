# S4-WarriorMovement-LuaLib
 Mod that adds additional lua functions to control "controllable" units 
  ! DOES NOT YET WORK IN MULTIPLAYER; CALLING FUNCTIONS OF THIS LIBRARY MAX FREEZE THE LOBBY

# API

The API consists of two functions and an enum that contains all possible types of movement

The enum:
- Movementtype values:
    - WarriorsLib.MOVE_FORWARDS: move units as you would do via right click
    - WarriorsLib.MOVE_PATROL: send units on a patrol between their starting point and the point given
    - WarriorsLib.MOVE_ACCUMULATE: Create paths for units to move (MOVE_FORWARD queue, basically)
    - WarriorsLib.MOVE_WATCH: move units as you would do via ALT + right click
    - WarriorsLib.MOVE_STOP: Stop a group of units, x,y passed to this action do not matter.

The functions: 
- WarriorsLib.SelectWarriors(x,y,r,party,settlertype)
   - Selects all entities of type <settlertype> (e.g. Settlers.SWORDSMAN_01) of a <party> around a circle with the center of <x>/<y> and radius <r>
   - Returns a table with all entity IDs (example return values would be {182, 192, 193} or {} if no one has been found)
- WarriorsLib.Send(group,x,y,movementtype)
   - Sends a <group> of entities to the coordinate <x>/<y>. The behavior in which they walk towards this point is determined by <movementtype>
 
 
