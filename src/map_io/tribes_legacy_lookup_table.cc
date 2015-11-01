/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "map_io/tribes_legacy_lookup_table.h"

// NOCOM(#codereview): Why do we need this table? It seems it should only be
// needed for savegames - which we break anyways. Did you find any maps in the
// wild that requires this?
TribesLegacyLookupTable::TribesLegacyLookupTable() :
	// Workers
	workers_
	{std::make_pair(
		"atlanteans", std::map<std::string, std::string>
		{
			{"armorsmith", "atlanteans_armorsmith"},
			{"baker", "atlanteans_baker"},
			{"blackroot_farmer", "atlanteans_blackroot_farmer"},
			{"builder", "atlanteans_builder"},
			{"carrier", "atlanteans_carrier"},
			{"charcoal_burner", "atlanteans_charcoal_burner"},
			{"farmer", "atlanteans_farmer"},
			{"fish_breeder", "atlanteans_fishbreeder"},
			{"fisher", "atlanteans_fisher"},
			{"forester", "atlanteans_forester"},
			{"geologist", "atlanteans_geologist"},
			{"horse", "atlanteans_horse"},
			{"horsebreeder", "atlanteans_horsebreeder"},
			{"hunter", "atlanteans_hunter"},
			{"miller", "atlanteans_miller"},
			{"miner", "atlanteans_miner"},
			{"sawyer", "atlanteans_sawyer"},
			{"scout", "atlanteans_scout"},
			{"shipwright", "atlanteans_shipwright"},
			{"smelter", "atlanteans_smelter"},
			{"smoker", "atlanteans_smoker"},
			{"soldier", "atlanteans_soldier"},
			{"spiderbreeder", "atlanteans_spiderbreeder"},
			{"stonecutter", "atlanteans_stonecutter"},
			{"toolsmith", "atlanteans_toolsmith"},
			{"trainer", "atlanteans_trainer"},
			{"weaponsmith", "atlanteans_weaponsmith"},
			{"weaver", "atlanteans_weaver"},
			{"woodcutter", "atlanteans_woodcutter"},
		}),
	std::make_pair(
		"barbarians", std::map<std::string, std::string>
		{
			{"baker", "barbarians_baker"},
			{"blacksmith", "barbarians_blacksmith"},
			{"brewer", "barbarians_brewer"},
			{"builder", "barbarians_builder"},
			{"carrier", "barbarians_carrier"},
			{"cattlebreeder", "barbarians_cattlebreeder"},
			{"charcoal_burner", "barbarians_charcoal_burner"},
			{"chief-miner", "barbarians_miner_chief"},
			{"farmer", "barbarians_farmer"},
			{"fisher", "barbarians_fisher"},
			{"gamekeeper", "barbarians_gamekeeper"},
			{"gardener", "barbarians_gardener"},
			{"geologist", "barbarians_geologist"},
			{"helmsmith", "barbarians_helmsmith"},
			{"hunter", "barbarians_hunter"},
			{"innkeeper", "barbarians_innkeeper"},
			{"lime-burner", "barbarians_lime_burner"},
			{"lumberjack", "barbarians_lumberjack"},
			{"master-blacksmith", "barbarians_blacksmith_master"},
			{"master-brewer", "barbarians_brewer_master"},
			{"master-miner", "barbarians_miner_master"},
			{"miner", "barbarians_miner"},
			{"ox", "barbarians_ox"},
			{"ranger", "barbarians_ranger"},
			{"scout", "barbarians_scout"},
			{"shipwright", "barbarians_shipwright"},
			{"smelter", "barbarians_smelter"},
			{"soldier", "barbarians_soldier"},
			{"stonemason", "barbarians_stonemason"},
			{"trainer", "barbarians_trainer"},
			{"weaver", "barbarians_weaver"},
		}),
	std::make_pair(
		"empire", std::map<std::string, std::string>
		{
			{"armorsmith", "empire_armorsmith"},
			{"baker", "empire_baker"},
			{"brewer", "empire_brewer"},
			{"builder", "empire_builder"},
			{"carpenter", "empire_carpenter"},
			{"carrier", "empire_carrier"},
			{"charcoal_burner", "empire_charcoal_burner"},
			{"donkey", "empire_donkey"},
			{"donkeybreeder", "empire_donkeybreeder"},
			{"farmer", "empire_farmer"},
			{"fisher", "empire_fisher"},
			{"forester", "empire_forester"},
			{"geologist", "empire_geologist"},
			{"hunter", "empire_hunter"},
			{"innkeeper", "empire_innkeeper"},
			{"lumberjack", "empire_lumberjack"},
			{"master-miner", "empire_miner_master"},
			{"miller", "empire_miller"},
			{"miner", "empire_miner"},
			{"pig-breeder", "empire_pigbreeder"},
			{"scout", "empire_scout"},
			{"shepherd", "empire_shepherd"},
			{"shipwright", "empire_shipwright"},
			{"smelter", "empire_smelter"},
			{"soldier", "empire_soldier"},
			{"stonemason", "empire_stonemason"},
			{"toolsmith", "empire_toolsmith"},
			{"trainer", "empire_trainer"},
			{"vinefarmer", "empire_vinefarmer"},
			{"weaponsmith", "empire_weaponsmith"},
			{"weaver", "empire_weaver"},
		}),
	},
	// Wares
	wares_
	{std::make_pair(
		"atlanteans", std::map<std::string, std::string>
		{
			{"advanced_shield", "shield_advanced"},
			{"blackrootflour", "blackroot_flour"},
			{"bread", "atlanteans_bread"},
			{"bucket", "buckets"},
			{"cornflour", "cornmeal"},
			{"double_trident", "trident_double"},
			{"golden_tabard", "tabard_golden"},
			{"goldore", "gold_ore"},
			{"goldyarn", "gold_thread"},
			{"heavy_double_trident", "trident_heavy_double"},
			{"ironore", "iron_ore"},
			{"light_trident", "trident_light"},
			{"long_trident", "trident_long"},
			{"spideryarn", "spider_silk"},
			{"steel_shield", "shield_steel"},
			{"steel_trident", "trident_steel"},
			{"stone", "granite"},
		}),
	 std::make_pair(
		 "barbarians", std::map<std::string, std::string>
		{
			{"battleax", "ax_battle"},
			{"broadax", "ax_broad"},
			{"bronzeax", "ax_bronze"},
			{"felling_ax", "felling_ax"},
			{"goldore", "gold_ore"},
			{"helm", "helmet"},
			{"ironore", "iron_ore"},
			{"mask", "helmet_mask"},
			{"pittabread", "barbarians_bread"},
			{"raw_stone", "granite"},
			{"sharpax", "ax_sharp"},
			{"strongbeer", "beer_strong"},
			{"thatchreed", "thatch_reed"},
			{"warhelm", "helmet_warhelm"},
			{"warriorsax", "ax_warriors"},
		}),
	 std::make_pair(
		 "empire", std::map<std::string, std::string>
		{
			{"advanced_lance", "spear_advanced"},
			{"ax", "felling_ax"},
			{"bread", "empire_bread"},
			{"chain_armor", "armor_chain"},
			{"goldore", "gold_ore"},
			{"heavy_lance", "spear_heavy"},
			{"helm", "armor_helmet"},
			{"ironore", "iron_ore"},
			{"lance", "spear"},
			{"marblecolumn", "marble_column"},
			{"plate_armor", "armor_gilded"},
			{"stone", "granite"},
			{"war_lance", "spear_war"},
			{"wood_lance", "spear_wooden"},
			{"wood", "planks"},
		}),
	 },
	// Immovables
	immovables_
	{std::make_pair(
		"atlanteans", std::map<std::string, std::string>
		{
			{"blackrootfield_b", "blackrootfield_ripe"},
			{"blackrootfield_m", "blackrootfield_medium"},
			{"blackrootfield_s", "blackrootfield_small"},
			{"blackrootfield_t", "blackrootfield_tiny"},
			{"cornfield_b", "cornfield_ripe"},
			{"cornfield_m", "cornfield_medium"},
			{"cornfield_s", "cornfield_small"},
			{"cornfield_t", "cornfield_tiny"},
			{"resi_granite1", "resi_stones1"},
			{"resi_granite2", "resi_stones2"},
			{"shipconstruction", "atlanteans_shipconstruction"},
		}),
	  std::make_pair(
		  "barbarians", std::map<std::string, std::string>
		{
			{"field0", "field_medium"},
			{"field0s", "field_small"},
			{"field0t", "field_tiny"},
			{"field1", "field_ripe"},
			{"field2", "field_harvested"},
			{"reed0", "reed_medium"},
			{"reed0s", "reed_small"},
			{"reed0t", "reed_tiny"},
			{"reed1", "reed_ripe"},
			{"resi_granite1", "resi_stones1"},
			{"resi_granite2", "resi_stones2"},
			{"shipconstruction", "barbarians_shipconstruction"},
		}),
	 std::make_pair(
		 "empire", std::map<std::string, std::string>
		{
			{"field0", "field_medium"},
			{"field0s", "field_small"},
			{"field0t", "field_tiny"},
			{"field1", "field_ripe"},
			{"field2", "field_harvested"},
			{"wine0", "grapevine_medium"},
			{"wine0s", "grapevine_small"},
			{"wine0t", "grapevine_tiny"},
			{"wine1", "grapevine_ripe"},
			{"resi_granite1", "resi_stones1"},
			{"resi_granite2", "resi_stones2"},
			{"shipconstruction", "empire_shipconstruction"},
		}),
	 }
{}

std::string TribesLegacyLookupTable::lookup_worker(const std::string& tribe,
																	  const std::string& worker) const {
	if (workers_.count(tribe)) {
		const std::map<std::string, std::string>& tribe_workers = workers_.at(tribe);
		const auto& i = tribe_workers.find(worker);
		if (i != tribe_workers.end()) {
			return i->second;
		}
	}
	return worker;
}

std::string TribesLegacyLookupTable::lookup_ware(const std::string& tribe, const std::string& ware) const {
	if (wares_.count(tribe)) {
		const std::map<std::string, std::string>& tribe_wares = wares_.at(tribe);
		const auto& i = tribe_wares.find(ware);
		if (i != tribe_wares.end()) {
			return i->second;
		}
	}
	return ware;
}

std::string TribesLegacyLookupTable::lookup_immovable(const std::string& tribe,
																		  const std::string& immovable) const {
	if (immovables_.count(tribe)) {
		const std::map<std::string, std::string>& tribe_immovables = immovables_.at(tribe);
		const auto& i = tribe_immovables.find(immovable);
		if (i != tribe_immovables.end()) {
			return i->second;
		}
	}
	return immovable;
}
