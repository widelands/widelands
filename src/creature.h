/*
 * Copyright (C) 2002 by Florian Bluemel
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

#ifndef __CREATURE_H
#define __CREATURE_H
/*
class Growable_Array;
class Field;

class Creature
{
public:
	enum TaskAction
	{
		TASK_GO = 1,		// param: field pointer
		TASK_GOHOME,		// param: none
		TASK_WORK,			// param: none
		TASK_WORKONROAD,	// param: road pointer
		TASK_PRODUCE,		// param: none
		TASK_TRANSPORT,		// param: none
		TASK_DROP,			// param: none
		TASK_FORCE_DWORD = 0x7FFFFFF
	};
private:
	struct Task
	{
		TaskAction	action;
		void*		param;
					Task(TaskAction a, void* p=NULL);
	};

	CreatureDesc*	desc;
	int				carrying;
	uint			taskTime;
	//muss nicht unbedingt GrowableArray sein, aber for now einfachste loesung
	Growable_Array*	tasks;
	Field*			field;
	//Road*			road;
					Creature(CreatureDesc* d);
	void			work();
	int				produce(uint timekey);
public:
					~Creature();
	void			add_task(TaskAction action, void* param=NULL);
	void			next_task();
	void			clear_tasks();
	void			live(uint timekey);
	Field*			find_bob_to_consume();
};
*/
#endif __CREATURE_H
