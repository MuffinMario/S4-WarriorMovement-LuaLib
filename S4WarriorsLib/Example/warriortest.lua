function new_game()
	MinuteEvents.new_game()
	dbg.stm("Spielstart! Ich heisse dich Willkommen zu dem Showcase von der S4 Script Erweiterung 'WarriorsLib'")
	dbg.stm("---------")
	request_event(gameloop, Events.FIVE_TICKS)

	dbg.aioff(1)
	dbg.aioff(2)
	dbg.aioff(3)
	
	-- Einlagern von Gold
	Buildings.AddBuilding(118, 58, 3, Buildings.STORAGEAREA)
	WarriorsLib.StoreGood (Buildings.GetFirstBuilding(3, Buildings.STORAGEAREA), Goods.GOLDBAR, 1, 3, 1)
end
function showAi()
	dbg.stm(WarriorsLib.getPlayerName(1)..", you are ".. (WarriorsLib.isHuman(1) == 1 and "a Human" or "an AI"))
	dbg.stm(WarriorsLib.getPlayerName(2)..", you are ".. (WarriorsLib.isHuman(2) == 1 and "a Human" or "an AI"))
	dbg.stm(WarriorsLib.getPlayerName(3)..", you are ".. (WarriorsLib.isHuman(3) == 1 and "a Human" or "an AI"))
	dbg.stm("---------")
end

function register_functions()
	reg_func(gameloop)
	MinuteEvents.register_functions()
end

function register_minute_events()
	requestMinuteEvent(turmBesetzen,1)
	requestMinuteEvent(setupTrading,2)
	requestMinuteEvent(recruitBowman,3)
	requestMinuteEvent(setWorkingAreaAndRecruit,4)
	requestMinuteEvent(turmUnBesetzen,5)
end

function setupTrading()
	dbg.stm("Wir setzen eine Handelsroute mit: ")
	dbg.stm("WarriorsLib.SetTradingRoute(SourceBuilding, DestBuilding, player)")
	dbg.stm("WarriorsLib.SetTradingRoute(Buildings.GetFirstBuilding(3, Buildings.MARKETPLACE), Buildings.GetFirstBuilding(1, Buildings.MARKETPLACE), 3)")
	dbg.stm("---------")
	dbg.stm("Festlegen was verschickt werden soll: ")
	dbg.stm("WarriorsLib.TradeGood(SourceBuilding, goodtype, amount, player)")
	dbg.stm("WarriorsLib.TradeGood(Buildings.GetFirstBuilding(3, Buildings.MARKETPLACE), Goods.GOLDBAR, 100, 3)")
	dbg.stm("---------")
	
	b1 = Buildings.GetFirstBuilding(1, Buildings.MARKETPLACE)
	b2 = Buildings.GetFirstBuilding(3, Buildings.MARKETPLACE)
	WarriorsLib.SetTradingRoute(b2, b1, 3)
	WarriorsLib.TradeGood(b2, Goods.GOLDBAR, 100, 3)
end

function gameloop()
	local x = 150
	local y = 200
	if newMinute() == 1 then
		showAi()
	end

	addUnits(1, 85, 125, 1, Settlers.SWORDSMAN_03)
	unit1 = WarriorsLib.SelectWarriors(85, 125, 20, 1, Settlers.SWORDSMAN_03)
	WarriorsLib.Send(unit1,x,y,WarriorsLib.MOVE_FORWARD)

	addUnits(2, 170, 125, 1, Settlers.SWORDSMAN_03)
	unit2 = WarriorsLib.SelectWarriors(170, 125, 20, 2, Settlers.SWORDSMAN_03)
	WarriorsLib.Send(unit2,x,y,WarriorsLib.MOVE_FORWARD)

end

function turmBesetzen()
	dbg.stm("Wir besetzen nun alle Türme")
	dbg.stm("WarriorsLib.GarrisonWarriors(buildingID, player)")
	dbg.stm("WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),1)")
	dbg.stm("---------")
	WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),1)
	WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(2, Buildings.GUARDTOWERSMALL),2)
	WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(3, Buildings.GUARDTOWERSMALL),3)
end

function turmUnBesetzen()
	dbg.stm("Wir holen alle Soldaten aus den Türmen")
	dbg.stm("WarriorsLib.UnGarrisonWarriors(buildingID, column, bowman, player)")
	dbg.stm("WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),-1,1,1)")
	dbg.stm("---------")
	WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),-1,1,1)
	WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(2, Buildings.GUARDTOWERSMALL),-1,1,2)
	WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(3, Buildings.GUARDTOWERSMALL),-1,1,3)
	showAi()
end

function recruitBowman()
	dbg.stm("Wir rekrutieren bei allen Spielern nun 5 Bogenschützen:")
	dbg.stm("WarriorsLib.RecruitWarriors(buildingID, settlertype , amount, player)")
	dbg.stm("WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(1, Buildings.BARRACKS), Settlers.BOWMAN_03, 5, 1)")
	dbg.stm("---------")
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(1, Buildings.BARRACKS), Settlers.BOWMAN_03, 5, 1)
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(2, Buildings.BARRACKS), Settlers.BOWMAN_03, 5, 2)
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(3, Buildings.BARRACKS), Settlers.BOWMAN_03, 5, 3)
end
function setWorkingAreaAndRecruit()
	dbg.stm("Wir rekrutieren bei Spieler 3 nun 5 Boigenschützen und setzen den Arbeitsbereich der Kaderne um")
	dbg.stm("WarriorsLib.SetBuildingWorkarea(buildingID, x, y, player)")
	dbg.stm("WarriorsLib.SetBuildingWorkarea(Buildings.GetFirstBuilding(3, Buildings.BARRACKS),70, 33, 3)")
	dbg.stm("---------")
	WarriorsLib.SetBuildingWorkarea(Buildings.GetFirstBuilding(3, Buildings.BARRACKS),70, 33, 3)
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(3, Buildings.BARRACKS), Settlers.BOWMAN_03, 5, 3)
end


function addUnits(id, x, y, amount, settlertype)
	settlertype = settlertype or Settlers.SWORDSMAN_03
	Settlers.AddSettlers(x, y, id, settlertype, amount)	
end


-- gibt jede Minute einmal 1 zurueck
function newMinute()
	if Vars.Save8 ~= Game.Time() then
		Vars.Save8 = Game.Time()
		return 1
	else
		return 0
	end
end

----
--LIB fuer Minute Events---
-----


----
--LIB fuer Minute Events---
-----

MinuteEvents =  {
  -- table of all events at all minutes in format _minuteEventTable[minute][funcid (from 1 - n; no specific meaning)]
  _minuteEventTable = {}
}

-- calls all function types in table
function MinuteEvents._subroutine_foreachFunction(i,v)
    if type(v) == "function" then
        v();
    end
end
function MinuteEvents.runMinuteEventTick()
  -- true on first tick of new minute
  local currentMinute = Game.Time()
  if Vars.Save9 ~= currentMinute then
    Vars.Save9 = currentMinute  -- minute
    -- calls all functions in table
    if MinuteEvents._minuteEventTable[Vars.Save9] ~= nil then
      foreach(MinuteEvents._minuteEventTable[Vars.Save9],MinuteEvents._subroutine_foreachFunction)
    end
  end

end

-- sets Save9 to 0 on start
function MinuteEvents.initVars()
  Vars.Save9 = 0
end
function MinuteEvents.new_game()
  request_event(MinuteEvents.runMinuteEventTick,Events.TICK)
  request_event(register_minute_events,Events.FIRST_TICK_OF_NEW_OR_LOADED_GAME)
  request_event(MinuteEvents.initVars,Events.FIRST_TICK_OF_NEW_GAME)
end
function MinuteEvents.register_functions()
  reg_func(MinuteEvents.runMinuteEventTick)
  reg_func(MinuteEvents.initVars)
  reg_func(register_minute_events)
end

-- util function to use
function requestMinuteEvent(eventfunc,minute)
  if MinuteEvents._minuteEventTable[minute] == nil then
    MinuteEvents._minuteEventTable[minute] = {}
  end
  tinsert(MinuteEvents._minuteEventTable[minute],eventfunc)
end