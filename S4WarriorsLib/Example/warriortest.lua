
function vcc()
	
end

function addUnits(id, x, y, amount, settlertype)
	settlertype = settlertype or Settlers.SWORDSMAN_03
	Settlers.AddSettlers(x, y, id, settlertype, amount)	
end

function new_game()
	MinuteEvents.new_game()
	dbg.stm("Spielstart! Ich heisse dich Willkommen zu dem Showcase von der S4 Script Erweiterung 'WarriorsLib'")
	request_event(gameloop, Events.FIVE_TICKS)
	request_event(vcc,Events.VICTORY_CONDITION_CHECK)
	dbg.stm("Hello "..WarriorsLib.getPlayerName(1)..", you are a ".. (WarriorsLib.isHuman(1) == 1 and "Human" or "AI"))
	dbg.stm("Hello "..WarriorsLib.getPlayerName(2)..", you are a ".. (WarriorsLib.isHuman(2) == 1 and "Human" or "AI"))
	dbg.stm("Hello "..WarriorsLib.getPlayerName(3)..", you are a ".. (WarriorsLib.isHuman(3) == 1 and "Human" or "AI"))
	dbg.aioff(1)
	dbg.aioff(2)
	dbg.aioff(3)
		
	--WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(1, Buildings.BARRACKS), Settlers.SWORDSMAN_03, 100)
	b1 = Buildings.GetFirstBuilding(1, Buildings.MARKETPLACE)
	b2 = Buildings.GetFirstBuilding(3, Buildings.MARKETPLACE)
	WarriorsLib.SetTradingRoute(b2, b1, 3, 1)
	WarriorsLib.TradeGood(b2, Goods.GOLDBAR, 100, 3, 1)
	WarriorsLib.SetBuildingWorkarea(Buildings.GetFirstBuilding(3, Buildings.BARRACKS),70, 33, 3, 1)
	--WarriorsLib.ShowMessageBox(500,500,200,50,"testheader","testnachricht")
	
	Buildings.AddBuilding(118, 58, 3, Buildings.STORAGEAREA)
	WarriorsLib.StoreGood (Buildings.GetFirstBuilding(3, Buildings.STORAGEAREA), Goods.GOLDBAR, 1, 3, 1)
end

function register_functions()
	reg_func(gameloop)
	reg_func(vcc)
	MinuteEvents.register_functions()
end

function register_minute_events()
	requestMinuteEvent(turmBesetzen,4)
	requestMinuteEvent(turmUnBesetzen,5)
	requestMinuteEvent(sp1_5l3,1)
	requestMinuteEvent(sp2_5l3,2)
	requestMinuteEvent(sp3_20l3,3)
end

function gameloop()
	local x = 150
	local y = 200
	-- dbg.stm("1")

	addUnits(1, 85, 125, 1, Settlers.SWORDSMAN_03)
	unit1 = WarriorsLib.SelectWarriors(85, 125, 20, 1, Settlers.SWORDSMAN_03)
	WarriorsLib.Send(unit1,x,y,WarriorsLib.MOVE_FORWARD)

	addUnits(2, 170, 125, 1, Settlers.SWORDSMAN_03)
	unit2 = WarriorsLib.SelectWarriors(170, 125, 20, 2, Settlers.SWORDSMAN_03)
	WarriorsLib.Send(unit2,x,y,WarriorsLib.MOVE_FORWARD)
	
	-- addUnits(3, 100, 70, 1, Settlers.SWORDSMAN_03)
	-- unit3 = WarriorsLib.SelectWarriors(100, 70, 20, 3, Settlers.SWORDSMAN_03)
	-- WarriorsLib.Send(unit3,x,y,WarriorsLib.MOVE_FORWARD)

end

function turmBesetzen()
	dbg.stm("Alle Türme besetzen")
	WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),1)
	WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(2, Buildings.GUARDTOWERSMALL),2)
	WarriorsLib.GarrisonWarriors(Buildings.GetFirstBuilding(3, Buildings.GUARDTOWERSMALL),3, 1)
end

function turmUnBesetzen()
	dbg.stm("Alle Türme unbesetzen")
	WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(1, Buildings.GUARDTOWERSMALL),-1,1,1)
	WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(2, Buildings.GUARDTOWERSMALL),-1,1,2)
	WarriorsLib.UnGarrisonWarriors(Buildings.GetFirstBuilding(3, Buildings.GUARDTOWERSMALL),-1,1,3, 1)
end

function sp1_5l3()
	dbg.stm("SP1 5x L3 SWORDSMAN_03")
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(1, Buildings.BARRACKS), Settlers.SWORDSMAN_03, 5, 1)
end
function sp2_5l3()
	dbg.stm("SP2 5x L3 BOWMAN_03")
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(2, Buildings.BARRACKS), Settlers.BOWMAN_03, 5, 2)
end
function sp3_20l3()
	dbg.stm("SP3 30x L3 BOWMAN_03")
	WarriorsLib.RecruitWarriors(Buildings.GetFirstBuilding(3, Buildings.BARRACKS), Settlers.BOWMAN_03, 30, 3, 1)
end







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