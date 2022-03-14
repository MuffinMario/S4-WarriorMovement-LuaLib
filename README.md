# S4-WarriorMovement-LuaLib
 Mod that adds additional lua functions to control "controllable" units and adds some helpful function for warrior control in general
  ! BE CAREFUL WITH MULTIPLAYER - STILL IN TESTING

# API

The API consists of a few functions and an enum that contains all possible types of movement

The enum:
- Movementtype values:
    - WarriorsLib.MOVE_FORWARDS: move units as you would do via right click
    - WarriorsLib.MOVE_PATROL: send units on a patrol between their starting point and the point given
    - WarriorsLib.MOVE_ACCUMULATE: Create paths for units to move (MOVE_FORWARD queue, basically)
    - WarriorsLib.MOVE_WATCH: move units as you would do via ALT + right click
    - WarriorsLib.MOVE_STOP: Stop a group of units, x,y passed to this action do not matter.

The functions: 
- WarriorsLib.SelectWarriors(x, y, r, party, settlertype)
   - Selects all entities of type <settlertype> (e.g. Settlers.SWORDSMAN_01) of a <party> around a circle with the center of <x>/<y> and radius <r>
   - Returns an index with a reference to the selection

- WarriorsLib.Send(group, x, y, movementtype)
   - Sends a <group> of entities to the coordinate <x>/<y>. The behavior in which they walk towards this point is determined by <movementtype>

- WarriorsLib.GarrisonWarriors(buildingID, party)
   - Garrison a building. This makes a military building to search nearby units and when found command them to enter the specified building to completely occupy all spots in the building.
   - Only works for human players, not AIs
 
- WarriorsLib.UnGarrisonWarriors(buildingID, column, bowman, party)
   - Ungarrison a building. This makes a military building eject units
   - Only works for human players, not AIs
   - column: The column of the unit to eject. Use 0 (zero) to eject the leftmost unit (in regards to the side panel of the selected building) . Use -1 to eject as much units as possible. This value is usually in the range -1..5.
   - bowman: If TRUE a bowman is ejected. Otherwise a swordman is ejected. This parameter is ignored if column is -1.

- WarriorsLib.RecruitWarriors(buildingID, settlertype, amount, party)
   - Let a building start or stop recruiting settlers. 
   - Only works for human players, not AIs
   - settlertype: for example Settlers.SWORDSMAN_01 or Settlers.BOWMAN_03
   - amount: 
	
| Value | Description                                                  |
| ----- | ------------------------------------------------------------ |
| -5    | Subtract five unit from the queue.                           |
| -1    | Subtract one unit from the queue.                            |
| 0     | Clears the queue. This will stop the building from producing further units. |
| 1     | Add one unit to the queue.                                   |
| 5     | Add five units to the queue.                                 |
| 100   | The game will interpret this as infinity and will never decrease the queue. |

- WarriorsLib.AiGarrisonWarriors(buildingID, party)
   - Same an GarrisonWarriors, but ONLY works for AIs

- WarriorsLib.AiUnGarrisonWarriors(buildingID, column, bowman, party)
   - Same an UnGarrisonWarriors, but ONLY works for AIs

- WarriorsLib.AiRecruitWarriors(buildingID, settlertype, amount, party)
   - Same an RecruitWarriors, but ONLY works for AIs



