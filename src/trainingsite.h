/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#ifndef TRAININGSITE_H
#define TRAININGSITE_H

#include "player.h"
#include "productionsite.h"
#include "soldier.h"

class TrainingSite_Window;

class TrainingSite_Descr : public ProductionSite_Descr {
public:

	TrainingSite_Descr(Tribe_Descr* tribe, const char* name);
	virtual ~TrainingSite_Descr();

	virtual void parse(const char* directory, Profile* prof,
		const EncodeData* encdata);
	virtual Building* create_object();

	virtual bool is_only_production_site(void) { return true; }

	inline  int  get_max_number_of_soldiers(void) { return m_num_soldiers; }
	inline	bool get_train_hp(void) { return m_train_hp; }
	inline	bool get_train_attack(void) { return m_train_attack; }
	inline	bool get_train_defense(void) { return m_train_defense; }
	inline	bool get_train_evade(void) { return m_train_evade; }

	int	get_min_level (tAttribute);
	int	get_max_level (tAttribute);
private:
	int	m_num_soldiers;
	bool	m_train_hp;
	bool	m_train_attack;
	bool	m_train_defense;
	bool	m_train_evade;

	// Minium train levels
	int	m_min_hp;
	int	m_min_attack;
	int	m_min_defense;
	int	m_min_evade;

	// Maxium train levels
	int	m_max_hp;
	int	m_max_attack;
	int	m_max_defense;
	int	m_max_evade;

		// Re-use of m_inputs to get the resources

//	TrainingMap m_programs;
};

class TrainingSite : public ProductionSite {
   friend class Widelands_Map_Buildingdata_Data_Packet;
	MO_DESCR(TrainingSite_Descr);
	friend class TrainingSite_Window;
public:

	TrainingSite(TrainingSite_Descr* descr);
	virtual ~TrainingSite();

	virtual int get_building_type(void) { return Building::TRAININGSITE; }
	virtual std::string get_statistics_string();

	virtual void init(Editor_Game_Base* g);
	virtual void cleanup(Editor_Game_Base* g);
	virtual void act(Game* g, uint data);

	inline	bool get_build_heros (void) { return m_build_heros; }

	inline	void set_build_heros (bool b_heros) { m_build_heros = b_heros; }

	virtual void set_economy(Economy* e);

 	virtual std::vector<Soldier*>* get_soldiers(void) { return &m_soldiers; }

 	inline	void switch_heros (void) {
						m_build_heros = !m_build_heros;
						molog("BUILD_HEROS: %s", m_build_heros ? "TRUE" : "FALSE");
						}
	inline TrainingSite_Descr* get__descr() { return get_descr(); }

   virtual void drop_soldier(uint nr);
	uint    get_pri (enum tAttribute atr);
	void    add_pri (enum tAttribute atr);
	void    sub_pri (enum tAttribute atr);
	uint    get_capacity () 	{ return m_capacity; }
	virtual	void soldier_capacity_up ()		{ change_soldier_capacity(1); }
	virtual	void soldier_capacity_down ()	{ change_soldier_capacity(-1); }
protected:
	virtual void	change_soldier_capacity (int);
	virtual UIWindow* create_options_window(Interactive_Player* plr,
		UIWindow** registry);


private:
	void request_soldier(Game* g);
	static void request_soldier_callback(Game* g, Request* rq, int ware,
		Worker* w, void* data);

	void program_start (Game *g, std::string name);
	void program_end (Game *g, bool success);
	void find_next_program (Game *g);
	void calc_list_upgrades (Game *g);

	void call_soldiers(Game *g);
	void drop_soldier(Game *g, uint nr);
	void drop_unupgradable_soldiers (Game *g);

	void modif_priority (enum tAttribute, int);
private:
	std::vector<Request*> m_soldier_requests; // Requests of soldiers
	std::vector<Soldier*> m_soldiers;

	// Soldiers that the user want at house (modificable)
	uint m_capacity;
	uint m_total_soldiers;	// Soldiers in house + soldiers requested

	std::vector<std::string> m_list_upgrades;

	bool	m_build_heros;	// True, ALLWAYS begin trying to upgrade at higher levels
							// False, ALLWAY begin trying to upgrade at lesser levels

	// Priorities of upgrades (0 to 2) 0 = don't upgrade
	uint	m_pri_hp;
	uint	m_pri_attack;
	uint	m_pri_defense;
	uint	m_pri_evade;

	// Modificators to priorities. To prevent an infinite loop while aren't possible any
	// inmediate upgrade
	int	m_pri_hp_mod;
	int	m_pri_attack_mod;
	int	m_pri_defense_mod;
	int	m_pri_evade_mod;

	bool	m_success;
	std::string	m_prog_name;
};

#endif
