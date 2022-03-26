# S4-WarriorsLib
 Mod that adds additional lua functions to control "controllable" units and adds some helpful function for warrior in general
  ! BE CAREFUL WITH MULTIPLAYER - STILL IN TESTING

# API
Our Wiki you can find here:
English: https://docs.settlers-united.com/v/s4-warriorslib-en
German: https://docs.settlers-united.com/v/s4-warriorslib-de

The API consists of a few functions and an enum that contains all possible types of movement

The enum:
- Movementtype values:
    - WarriorsLib.MOVE_FORWARDS: move units as you would do via right click
    - WarriorsLib.MOVE_PATROL: send units on a patrol between their starting point and the point given
    - WarriorsLib.MOVE_ACCUMULATE: Create paths for units to move (MOVE_FORWARD queue, basically)
    - WarriorsLib.MOVE_WATCH: move units as you would do via ALT + right click
    - WarriorsLib.MOVE_STOP: Stop a group of units, x,y passed to this action do not matter.

The functions: 
- WarriorsLib.SelectWarriors(x, y, r, player, settlertype)
   - Selects all entities of type <settlertype> (e.g. Settlers.SWORDSMAN_01) of a <player> around a circle with the center of <x>/<y> and radius <r>
   - Returns an index with a reference to the selection

- WarriorsLib.Send(group, x, y, movementtype)
   - Sends a <group> of entities to the coordinate <x>/<y>. The behavior in which they walk towards this point is determined by <movementtype>

- WarriorsLib.getPlayerName(player)
   - Return the PlayerName of the given Player. 

- WarriorsLib.RecruitWarriors(buildingID, settlertype, amount, player)
   - Let a building start or stop recruiting settlers. 
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


- WarriorsLib.GarrisonWarriors(buildingID, player)
   - Garrison a building. This makes a military building to search nearby units and when found command them to enter the specified building to completely occupy all spots in the building.
 
- WarriorsLib.UnGarrisonWarriors(buildingID, column, bowman, player)
   - Ungarrison a building. This makes a military building eject units
   - column: The column of the unit to eject. Use 0 (zero) to eject the leftmost unit (in regards to the side panel of the selected building) . Use -1 to eject as much units as possible. This value is usually in the range -1..5.
   - bowman: If TRUE a bowman is ejected. Otherwise a swordman is ejected. This parameter is ignored if column is -1.

- WarriorsLib.SetTradingRoute(sourceBuildingID, destinationBuildingID, player)
   - Set the target market/harbor of a market/harbor to establish a trading route.
 
 - WarriorsLib.TradeGood(buildingID, goodtype, amount, player)
   - Specifies what goods and how many a market or harbor shall transport. Use WarriorsLib.SetTradingRoute to establish the destination building for the trade.
 
 - WarriorsLib.StoreGood(buildingID, goodtype, enable, player)
   - Specifies what goods a storagearea store.
   - enable: If TRUE the building will start to store goods of the specified type. If FALSE it will stop storing them.
 
 - WarriorsLib.SetBuildingWorkarea(buildingID, x, y, player)
   - Set the work area for a building. This is the same event that players can issue by clicking the target icon in the side panel of a selected building.
 
# Tipp
Example to check within new_game() if WarriorsLib is installed by the user

```lua
function new_game()
	if WarriorsLib then
		dbg.stm("WarriorsLib has been found")
	else
		dbg.stm("WarriorsLib is not installed")
	end
end

```