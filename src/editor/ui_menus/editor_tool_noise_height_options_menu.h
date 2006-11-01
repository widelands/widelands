/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__EDITOR_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H
#define __S__EDITOR_TOOL_NOISE_HEIGHT_OPTIONS_MENU_H

#include "editor_tool_options_menu.h"

class Editor_Interactive;
class Editor_Noise_Height_Tool;
namespace UI {struct Textarea;};

struct Editor_Tool_Noise_Height_Options_Menu : public Editor_Tool_Options_Menu {
      Editor_Tool_Noise_Height_Options_Menu(Editor_Interactive*, int, Editor_Noise_Height_Tool*, UI::UniqueWindow::Registry*);
      virtual ~Editor_Tool_Noise_Height_Options_Menu() { }

   private:
      UI::Textarea* m_textarea_lower;
      UI::Textarea* m_textarea_upper;
      UI::Textarea* m_set;
      Editor_Noise_Height_Tool* m_nht;

      void button_clicked(int);
      void update();
};

#endif
