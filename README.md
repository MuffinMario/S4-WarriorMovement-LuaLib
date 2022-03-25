# S4-WarriorLib
 Mod that adds additional lua functions to control "controllable" units and adds some helpful function for warrior in general
  ! BE CAREFUL WITH MULTIPLAYER - STILL IN TESTING

# API

The API consists of a few functions and an enum that contains all possible types of movement

The enum:
- Movementtype values:
    - WarriorLib.MOVE_FORWARDS: move units as you would do via right click
    - WarriorLib.MOVE_PATROL: send units on a patrol between their starting point and the point given
    - WarriorLib.MOVE_ACCUMULATE: Create paths for units to move (MOVE_FORWARD queue, basically)
    - WarriorLib.MOVE_WATCH: move units as you would do via ALT + right click
    - WarriorLib.MOVE_STOP: Stop a group of units, x,y passed to this action do not matter.

The functions: 
- WarriorLib.SelectWarriors(x, y, r, party, settlertype)
   - Selects all entities of type <settlertype> (e.g. Settlers.SWORDSMAN_01) of a <party> around a circle with the center of <x>/<y> and radius <r>
   - Returns an index with a reference to the selection

- WarriorLib.Send(group, x, y, movementtype)
   - Sends a <group> of entities to the coordinate <x>/<y>. The behavior in which they walk towards this point is determined by <movementtype>

- WarriorLib.AiGarrisonWarriors(buildingID, party)
   - Garrison a building. This makes a military building to search nearby units and when found command them to enter the specified building to completely occupy all spots in the building.
   - ONLY works for AIs
 
- WarriorLib.AiUnGarrisonWarriors(buildingID, column, bowman, party)
   - Ungarrison a building. This makes a military building eject units
   - ONLY works for AIs
   - column: The column of the unit to eject. Use 0 (zero) to eject the leftmost unit (in regards to the side panel of the selected building) . Use -1 to eject as much units as possible. This value is usually in the range -1..5.
   - bowman: If TRUE a bowman is ejected. Otherwise a swordman is ejected. This parameter is ignored if column is -1.

- WarriorLib.AiRecruitWarriors(buildingID, settlertype, amount, party)
   - Let a building start or stop recruiting settlers. 
   - ONLY works for AIs
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

# Tipp
Example to check within new_game() if WarriorLib is installed by the user

```lua
function new_game()
	if WarriorLib then
		dbg.stm("WarriorLib has been found")
	else
		dbg.stm("WarriorLib is not installed")
	end
end

```