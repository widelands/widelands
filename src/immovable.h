/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef included_immovable_h
#define included_immovable_h

#include "animation.h"
#include "geometry.h"
#include "instances.h"

class Economy;
class Flag;
class Profile;
class Tribe_Descr;
class Worker;

/*
BaseImmovable is the base for all non-moving objects (immovables such as trees,
buildings, flags, roads).

The immovable's size influences building capabilities around it.
If size is NONE, the immovable can simply be removed by placing something on it
(this is usually true for decorations).
For more information, see the Map::recalc_* functions.
*/
struct BaseImmovable : public Map_Object {
	enum {
		NONE,   //  not robust
		SMALL,
		MEDIUM,
		BIG
	};

	BaseImmovable(const Map_Object_Descr &);
	virtual ~BaseImmovable();

	virtual int32_t  get_size    () const throw () = 0;
	virtual bool get_passable() const throw () = 0;
	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point)
		= 0;
	virtual std::string const & name() const throw ();

protected:
	void set_position(Editor_Game_Base *g, Coords c);
	void unset_position(Editor_Game_Base *g, Coords c);
};


class Immovable;
class ImmovableProgram;
struct ImmovableAction;

/*
Immovable represents a standard immovable such as trees or stones.
*/
struct Immovable_Descr : public Map_Object_Descr {
	friend class Widelands_Map_Immovabledata_Data_Packet; // For writing (get_program)

	typedef std::map<std::string, ImmovableProgram*> ProgramMap;

	Immovable_Descr
		(const Tribe_Descr * const, const std::string & immovable_name);
	~Immovable_Descr();

	const std::string & name() const throw () {return m_name;}
	__attribute__ ((deprecated)) const char * get_name() const throw () {return m_name.c_str();}
	int32_t get_size() const throw () {return m_size;}
	const char* get_picture() const {return m_picture.c_str();}
	const ImmovableProgram* get_program(std::string programname) const;
	const EncodeData& get_default_encodedata() const {return m_default_encodedata;}

	void parse(const char *directory, Profile *s);
	void parse_program(std::string directory, Profile* prof, std::string programname);
	uint32_t parse_animation(std::string directory, Profile* prof, std::string name);
	void parse_playFX(std::string directory, Profile* prof, std::string name);
	Immovable *create(Editor_Game_Base *g, Coords coords);

	const Tribe_Descr* get_owner_tribe() const throw () {return m_owner_tribe;}

protected:
	std::string m_picture;
	const std::string         m_name;
	int32_t           m_size;
	EncodeData    m_default_encodedata;

	ProgramMap    m_programs;
	const Tribe_Descr * const m_owner_tribe; // 0 if world immovable
};

class Immovable : public BaseImmovable {
	friend class Immovable_Descr;
	friend class ImmovableProgram;
	friend class Widelands_Map_Immovabledata_Data_Packet; // for writing (obsolete since build-11)

	MO_DESCR(Immovable_Descr);

public:
	Immovable(const Immovable_Descr &);
	~Immovable();

	Coords get_position() const {return m_position;}

	virtual int32_t  get_type    () const throw ();
	virtual int32_t  get_size    () const throw ();
	virtual bool get_passable() const throw ();
	std::string const & name() const throw ();
	__attribute__ ((deprecated)) std::string get_name() const {return name();}

	void init(Editor_Game_Base *g);
	void cleanup(Editor_Game_Base *g);
	void act(Game *g, uint32_t data);

	virtual void draw
		(const Editor_Game_Base &, RenderTarget &, const FCoords, const Point);

	void switch_program(Game* g, std::string programname);

	const Tribe_Descr * get_owner_tribe() const throw ()
	{return descr().get_owner_tribe();}

protected:
	void set_program_animation(Editor_Game_Base* g);
	void run_program(Game* g, bool killable);

private:
	bool run_animation(Game* g, bool killable, const ImmovableAction& action);
	bool run_transform(Game* g, bool killable, const ImmovableAction& action);
	bool run_remove(Game* g, bool killable, const ImmovableAction& action);
	bool run_playFX(Game* g, bool killable, const ImmovableAction& action);

protected:
	Coords                   m_position;

	uint32_t                     m_anim;
	int32_t                      m_animstart;

	const ImmovableProgram * m_program;
	uint32_t m_program_ptr; //  index of next instruction to execute
	int32_t                      m_program_step; //  time of next step


	// Load/save support
protected:
	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead&);
		virtual void load_pointers();
		virtual void load_finish();
	};

public:
	// Remove as soon as we fully support the new system
	virtual bool has_new_save_support() {return true;}

	virtual void save(Editor_Game_Base*, Widelands_Map_Map_Object_Saver*, FileWrite&);
	static Map_Object::Loader* load(Editor_Game_Base*, Widelands_Map_Map_Object_Loader*, FileRead&);
};


/**
 * PlayerImmovable is an immovable owned by a player that belongs to an economy:
 * building, flag or road
 *
 * A PlayerImmovable can also house a number of workers, which are automatically
 * turned into fugitives when the immovable is destroyed, and their economy is also
 * adjusted automatically.
 */
struct PlayerImmovable : public BaseImmovable {
	PlayerImmovable(const Map_Object_Descr &);
	virtual ~PlayerImmovable();

	Player *get_owner() const {return m_owner;}
	Player & owner() const {return *m_owner;}
	Economy * get_economy() const throw () {return m_economy;}
	Economy & economy() const throw () {return *m_economy;}

	virtual Flag *get_base_flag() = 0;

	virtual void set_economy(Economy *e);

	virtual void add_worker(Worker *w);
	virtual void remove_worker(Worker *w);

	const std::vector<Worker*>& get_workers() const {return m_workers;}

	void log_general_info(Editor_Game_Base*);

protected:
	void set_owner(Player *owner);

	virtual void init(Editor_Game_Base *g);
	virtual void cleanup(Editor_Game_Base *g);

private:
	Player              * m_owner;
	Economy             * m_economy;

	std::vector<Worker *> m_workers;
};


#endif // included_immovable_h
