#include "pch.h"
#include "S4Entity.h"
///////////////////////////////////////////////////////////////////////////////
// GNU Lesser General Public License v3 (LGPL v3) 
//
// Copyright (c) 2020 nyfrk <nyfrk@gmx.net>
//
// This file is part of S4ModApi.
//
// S4ModApi is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// S4ModApi is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with S4ModApi. If not, see <https://www.gnu.org/licenses/lgpl-3.0>.
///////////////////////////////////////////////////////////////////////////////

extern DWORD g_s4Base;
S4_ENTITY_ENUM IEntity::GetClass() {
	switch ((DWORD)GetVtable() - g_s4Base) {
		// todo: the following offsets work only for version with md5 c13883cbd796c614365ab2d670ead561
	case 0xc5821c: return S4_ENTITY_ENUM::S4_ENTITY_LANDANIMAL;
	case 0xc585e4: return S4_ENTITY_ENUM::S4_ENTITY_BUILDING;
	case 0xc59710: return S4_ENTITY_ENUM::S4_ENTITY_CART;
	case 0xc59814: return S4_ENTITY_ENUM::S4_ENTITY_CATAPULT_MAYA;
	case 0xc59918: return S4_ENTITY_ENUM::S4_ENTITY_CATAPULT_TROJAN;
	case 0xc59a1c: return S4_ENTITY_ENUM::S4_ENTITY_CATAPULT_ROMAN;
	case 0xc59b20: return S4_ENTITY_ENUM::S4_ENTITY_CATAPULT_VIKING;
	case 0xc59d10: return S4_ENTITY_ENUM::S4_ENTITY_DECO;
	case 0xc59e4c: return S4_ENTITY_ENUM::S4_ENTITY_HIVE;
	case 0xc59ec8: return S4_ENTITY_ENUM::S4_ENTITY_MUSHROOM;
	case 0xc59f44: return S4_ENTITY_ENUM::S4_ENTITY_PLANT;
	case 0xc59fc0: return S4_ENTITY_ENUM::S4_ENTITY_SHADOW_HERB;
	case 0xc5a3b8: return S4_ENTITY_ENUM::S4_ENTITY_MANAKOPTER;
	case 0xc5a7bc: return S4_ENTITY_ENUM::S4_ENTITY_PILE;
	case 0xc5bf20: return S4_ENTITY_ENUM::S4_ENTITY_SETTLER;
	case 0xc5c60c: return S4_ENTITY_ENUM::S4_ENTITY_FERRY;
	case 0xc5c81c: return S4_ENTITY_ENUM::S4_ENTITY_TRANSPORTSHIP;
	case 0xc5c8fc: return S4_ENTITY_ENUM::S4_ENTITY_WARSHIP_TROJAN;
	case 0xc5ca10: return S4_ENTITY_ENUM::S4_ENTITY_WARSHIP_VIKING;
	case 0xc5cb0c: return S4_ENTITY_ENUM::S4_ENTITY_WARSHIP_MAYA;
	case 0xc5cc20: return S4_ENTITY_ENUM::S4_ENTITY_WARSHIP_ROMAN;
	case 0xc5cde8: return S4_ENTITY_ENUM::S4_ENTITY_STONE;
	case 0xc5ce68: return S4_ENTITY_ENUM::S4_ENTITY_TREE;
	default: return S4_ENTITY_ENUM::S4_ENTITY_UNKNOWN;
	}
}
S4_ROLE_ENUM CPersistance::GetRoleClass()
{

	// todo: the following offsets work only for version with md5 c13883cbd796c614365ab2d670ead561
	switch ((DWORD)GetVtable() - g_s4Base) {
		// Pile Roles
	case 0xc5a66c: return S4_ROLE_ENUM::S4_ROLE_PILE_ANYWHERE;
	case 0xc5a714: return S4_ROLE_ENUM::S4_ROLE_PILE_BUILDING_SITE;
	case 0xc5a768: return S4_ROLE_ENUM::S4_ROLE_PILE_DELIVERY;
	case 0xc5a8fc: return S4_ROLE_ENUM::S4_ROLE_PILE_PRODUCTION;
	case 0xc5a8a8: return S4_ROLE_ENUM::S4_ROLE_PILE_STORAGE;
	case 0xc5a950: return S4_ROLE_ENUM::S4_ROLE_PILE_TRADE;

		// Building Roles
	case 0xc58e84: return S4_ROLE_ENUM::S4_ROLE_BUILDING_WORKUP;  // needs confirmation
	case 0xc5837c: return S4_ROLE_ENUM::S4_ROLE_BUILDING_ANIMALRANCH;
	case 0xc58f8c: return S4_ROLE_ENUM::S4_ROLE_BUILDING_MILITARY; // needs confirmation
	case 0xc58c74: return S4_ROLE_ENUM::S4_ROLE_BUILDING_CASTLE;
	case 0xc5958c: return S4_ROLE_ENUM::S4_ROLE_BUILDING_TRADING;
	case 0xc58484: return S4_ROLE_ENUM::S4_ROLE_BUILDING_BARRACK;
	case 0xc58508: return S4_ROLE_ENUM::S4_ROLE_BUILDING_BIGTEMPLE;
	case 0xc591bc: return S4_ROLE_ENUM::S4_ROLE_BUILDING_SITE;
	case 0xc58ae8: return S4_ROLE_ENUM::S4_ROLE_BUILDING_MUSHROOMFARM;
	case 0xc59284: return S4_ROLE_ENUM::S4_ROLE_BUILDING_DARKTEMPLE;
	case 0xc58b6c: return S4_ROLE_ENUM::S4_ROLE_BUILDING_EYECATCHER;
	case 0xc58cf8: return S4_ROLE_ENUM::S4_ROLE_BUILDING_FARM;
	case 0xc58f08: return S4_ROLE_ENUM::S4_ROLE_BUILDING_GATHER;
	case 0xc58d7c: return S4_ROLE_ENUM::S4_ROLE_BUILDING_LOOKOUTTOWER;
	case 0xc593d0: return S4_ROLE_ENUM::S4_ROLE_BUILDING_MANAKOPTERHALL;
	case 0xc59484: return S4_ROLE_ENUM::S4_ROLE_BUILDING_MINE;
	case 0xc59094: return S4_ROLE_ENUM::S4_ROLE_BUILDING_PRODUCTION;
	case 0xc59010: return S4_ROLE_ENUM::S4_ROLE_BUILDING_RESIDENCE;
	case 0xc58e00: return S4_ROLE_ENUM::S4_ROLE_BUILDING_SIMPLE;
	case 0xc58bf0: return S4_ROLE_ENUM::S4_ROLE_BUILDING_SMALLTEMPLE;
	case 0xc59508: return S4_ROLE_ENUM::S4_ROLE_BUILDING_STORAGE;
	case 0xc59610: return S4_ROLE_ENUM::S4_ROLE_BUILDING_WORKSHOP;

		// Settler Roles
	case 0xc5a9d8: return S4_ROLE_ENUM::S4_ROLE_SETTLER_BUILDER;
	case 0xc5ab08: return S4_ROLE_ENUM::S4_ROLE_SETTLER_CARRIER;
	case 0xc5ad04: return S4_ROLE_ENUM::S4_ROLE_SETTLER_DARKGARDENER;
	case 0xc5ad8c: return S4_ROLE_ENUM::S4_ROLE_SETTLER_DIGGER;
	case 0xc5ae90: return S4_ROLE_ENUM::S4_ROLE_SETTLER_DONKEY;
	case 0xc5af38: return S4_ROLE_ENUM::S4_ROLE_SETTLER_DOOR;
	case 0xc5afc0: return S4_ROLE_ENUM::S4_ROLE_SETTLER_FLEE;
	case 0xc5b2cc: return S4_ROLE_ENUM::S4_ROLE_SETTLER_FREEWORKER;
	case 0xc5b8b0: return S4_ROLE_ENUM::S4_ROLE_SETTLER_HOUSEWORKER;
	case 0xc5bb18: return S4_ROLE_ENUM::S4_ROLE_SETTLER_HUNTER;
	case 0xc5bbf8: return S4_ROLE_ENUM::S4_ROLE_SETTLER_MUSHROOMFARMER;
	case 0xc5c254: return S4_ROLE_ENUM::S4_ROLE_SETTLER_SLAVE;
	case 0xc5c584: return S4_ROLE_ENUM::S4_ROLE_SETTLER_TOWERSOLDIER;

		// The following are selectables
	case 0xc5b51c: return S4_ROLE_ENUM::S4_ROLE_SETTLER_GARDENER;
	case 0xc5b5f8: return S4_ROLE_ENUM::S4_ROLE_SETTLER_GEOLOGIST;
	case 0xc5bc94: return S4_ROLE_ENUM::S4_ROLE_SETTLER_PIONEER;
	case 0xc5bd1c: return S4_ROLE_ENUM::S4_ROLE_SETTLER_PRIEST;
	case 0xc5c4b4: return S4_ROLE_ENUM::S4_ROLE_SETTLER_THIEF;

		// The following roles have a WarriorBehaviour attached. Note that 
		// war machines/ships do not have a role but inherit directly from WarriorBehaviour!
	case 0xc5c3ac: return S4_ROLE_ENUM::S4_ROLE_SETTLER_SOLDIER;
	case 0xc5b7f0: return S4_ROLE_ENUM::S4_ROLE_SETTLER_SQUADLEADER;  // needs confirmation
	case 0xc5b734: return S4_ROLE_ENUM::S4_ROLE_SETTLER_HJB;  // needs confirmation
	case 0xc5c14c: return S4_ROLE_ENUM::S4_ROLE_SETTLER_SHAMAN;
	case 0xc5bdbc: return S4_ROLE_ENUM::S4_ROLE_SETTLER_SABOTEUR;
	default: return S4_ROLE_ENUM::S4_ROLE_UNKNOWN;
	};
}
