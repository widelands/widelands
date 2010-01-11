/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef TIMEEDIT_H
#define TIMEEDIT_H

#include "button.h"

#include "i18n.h"

#include "ref_cast.h"

namespace UI {

struct TimeEdit : Panel {
	TimeEdit(Panel * parent, Point p, uint32_t time, bool enabled = true);

	bool enabled() const {return m_enabled;}
	void set_enabled(bool on);

	virtual void draw(RenderTarget &);

	uint32_t time() const {
		uint32_t result = m_value_hours_first;
		result *= 10; result += m_value_hours_secnd;
		result *=  6; result += m_value_minutes_first;
		result *= 10; result += m_value_minutes_secnd;
		result *=  6; result += m_value_seconds_first;
		result *= 10; result += m_value_seconds_secnd;
		result *= 10; result += m_value_msecs_first;
		result *= 10; result += m_value_msecs_secnd;
		result *= 10; result += m_value_msecs_third;
		return result;
	}

private:
	uint8_t m_value_msecs_third;
	uint8_t m_value_msecs_secnd;
	uint8_t m_value_msecs_first;
	uint8_t m_value_seconds_secnd;
	uint8_t m_value_seconds_first;
	uint8_t m_value_minutes_secnd;
	uint8_t m_value_minutes_first;
	uint8_t m_value_hours_secnd;
	uint8_t m_value_hours_first;
	bool    m_enabled;

	struct Crease_Button : public UI::Button {
		Crease_Button(TimeEdit & parent, Point, PictureID pic, bool enabled);
	};
	struct Increase_Button : public Crease_Button {
		Increase_Button(TimeEdit & parent, int32_t x, bool enabled);
	};
	struct Decrease_Button : public Crease_Button {
		Decrease_Button(TimeEdit & parent, int32_t x, bool enabled);
	};

	struct Hours_First_Increase : public Increase_Button {
		Hours_First_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent,   0, _enabled and parent.m_value_hours_first < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_hours_first < 9);
			set_enabled(++parent.m_value_hours_first < 9);
			parent.m_hours_first_decrease.set_enabled(true);
		}
	} m_hours_first_increase;
	struct Hours_First_Decrease : public Decrease_Button {
		Hours_First_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent,   0, _enabled and parent.m_value_hours_first)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_hours_first);
			parent.m_hours_first_increase.set_enabled(true);
			set_enabled(--parent.m_value_hours_first);
		}
	} m_hours_first_decrease;
	struct Hours_Second_Increase : public Increase_Button {
		Hours_Second_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent,  22, _enabled and parent.m_value_hours_secnd < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_hours_secnd < 9);
			set_enabled(++parent.m_value_hours_secnd < 9);
			parent.m_hours_secnd_decrease.set_enabled(true);
		}
	} m_hours_secnd_increase;
	struct Hours_Second_Decrease : public Decrease_Button {
		Hours_Second_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent,  22, _enabled and parent.m_value_hours_secnd)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_hours_secnd);
			parent.m_hours_secnd_increase.set_enabled(true);
			set_enabled(--parent.m_value_hours_secnd);
		}
	} m_hours_secnd_decrease;

	struct Minutes_First_Increase : public Increase_Button {
		Minutes_First_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent,  59, _enabled and parent.m_value_minutes_first < 5)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_minutes_first < 5);
			set_enabled(++parent.m_value_minutes_first < 5);
			parent.m_minutes_first_decrease.set_enabled(true);
		}
	} m_minutes_first_increase;
	struct Minutes_First_Decrease : public Decrease_Button {
		Minutes_First_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent,  59, _enabled and parent.m_value_minutes_first)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_minutes_first);
			parent.m_minutes_first_increase.set_enabled(true);
			set_enabled(--parent.m_value_minutes_first);
		}
	} m_minutes_first_decrease;
	struct Minutes_Second_Increase : public Increase_Button {
		Minutes_Second_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent,  81, _enabled and parent.m_value_minutes_secnd < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_minutes_secnd < 9);
			set_enabled(++parent.m_value_minutes_secnd < 9);
			parent.m_minutes_secnd_decrease.set_enabled(true);
		}
	} m_minutes_secnd_increase;
	struct Minutes_Second_Decrease : public Decrease_Button {
		Minutes_Second_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent,  81, _enabled and parent.m_value_minutes_secnd)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_minutes_secnd);
			parent.m_minutes_secnd_increase.set_enabled(true);
			set_enabled(--parent.m_value_minutes_secnd);
		}
	} m_minutes_secnd_decrease;

	struct Seconds_First_Increase : public Increase_Button {
		Seconds_First_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent, 118, _enabled and parent.m_value_seconds_first < 5)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_seconds_first < 5);
			set_enabled(++parent.m_value_seconds_first < 5);
			parent.m_seconds_first_decrease.set_enabled(true);
		}
	} m_seconds_first_increase;
	struct Seconds_First_Decrease : public Decrease_Button {
		Seconds_First_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent, 118, _enabled and parent.m_value_seconds_first)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_seconds_first);
			parent.m_seconds_first_increase.set_enabled(true);
			set_enabled(--parent.m_value_seconds_first);
		}
	} m_seconds_first_decrease;
	struct Seconds_Second_Increase : public Increase_Button {
		Seconds_Second_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent, 140, _enabled and parent.m_value_seconds_secnd < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_seconds_secnd < 9);
			set_enabled(++parent.m_value_seconds_secnd < 9);
			parent.m_seconds_secnd_decrease.set_enabled(true);
		}
	} m_seconds_secnd_increase;
	struct Seconds_Second_Decrease : public Decrease_Button {
		Seconds_Second_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent, 140, _enabled and parent.m_value_seconds_secnd)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_seconds_secnd);
			parent.m_seconds_secnd_increase.set_enabled(true);
			set_enabled(--parent.m_value_seconds_secnd);
		}
	} m_seconds_secnd_decrease;

	struct MSecs_First_Increase : public Increase_Button {
		MSecs_First_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent, 176, _enabled and parent.m_value_msecs_first < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_msecs_first < 9);
			set_enabled(++parent.m_value_msecs_first < 9);
			parent.m_msecs_first_decrease.set_enabled(true);
		}
	} m_msecs_first_increase;
	struct MSecs_First_Decrease : public Decrease_Button {
		MSecs_First_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent, 176, _enabled and parent.m_value_msecs_first)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_msecs_first);
			parent.m_msecs_first_increase.set_enabled(true);
			set_enabled(--parent.m_value_msecs_first);
		}
	} m_msecs_first_decrease;
	struct MSecs_Second_Increase : public Increase_Button {
		MSecs_Second_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent, 198, _enabled and parent.m_value_msecs_secnd < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_msecs_secnd < 9);
			set_enabled(++parent.m_value_msecs_secnd < 9);
			parent.m_msecs_secnd_decrease.set_enabled(true);
		}
	} m_msecs_secnd_increase;
	struct MSecs_Second_Decrease : public Decrease_Button {
		MSecs_Second_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent, 198, _enabled and parent.m_value_msecs_secnd)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_msecs_secnd);
			parent.m_msecs_secnd_increase.set_enabled(true);
			set_enabled(--parent.m_value_msecs_secnd);
		}
	} m_msecs_secnd_decrease;
	struct MSecs_Third_Increase : public Increase_Button {
		MSecs_Third_Increase(TimeEdit & parent, bool const _enabled) :
			Increase_Button
				(parent, 220, _enabled and parent.m_value_msecs_third < 9)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_msecs_third < 9);
			set_enabled(++parent.m_value_msecs_third < 9);
			parent.m_msecs_third_decrease.set_enabled(true);
		}
	} m_msecs_third_increase;
	struct MSecs_Third_Decrease : public Decrease_Button {
		MSecs_Third_Decrease(TimeEdit & parent, bool const _enabled) :
			Decrease_Button
				(parent, 220, _enabled and parent.m_value_msecs_third)
		{}
		void clicked() {
			TimeEdit & parent = ref_cast<TimeEdit, Panel>(*get_parent());
			assert(parent.m_value_msecs_third);
			parent.m_msecs_third_increase.set_enabled(true);
			set_enabled(--parent.m_value_msecs_third);
		}
	} m_msecs_third_decrease;
};

}

#endif
