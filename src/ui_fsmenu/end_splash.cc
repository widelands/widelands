/*
 * Copyright (C) 2024 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "ui_fsmenu/end_splash.h"

#include "base/i18n.h"

const std::vector<EndSplashOption> kEndSplashOptionEntries = {
   {EndSplashOption::kHard,
    /** TRANSLATORS: Dropdown entry for time to end splashscreen */
    gettext_noop("Immediate"),
    /** TRANSLATORS: Tooltip for an end splashscreen time option */
    gettext_noop("End splash screen as soon as possible, without fading")},

   {EndSplashOption::kSoft,
    /** TRANSLATORS: Dropdown entry for time to end splashscreen */
    gettext_noop("Early"),
    /** TRANSLATORS: Tooltip for an end splashscreen time option */
    gettext_noop("Fade to main menu as soon as possible")},

   {EndSplashOption::kWaitIntroMusic,
    /** TRANSLATORS: Dropdown entry for time to end splashscreen */
    gettext_noop("End of intro music"),
    /** TRANSLATORS: Tooltip for an end splashscreen time option */
    gettext_noop("Wait until intro music ends, then fade to main menu")},

   {EndSplashOption::kUserMenuMusic,
    /** TRANSLATORS: Dropdown entry for time to end splashscreen */
    gettext_noop("Click, change music"),
    /** TRANSLATORS: Tooltip for an end splashscreen time option */
    gettext_noop("Wait for mouse click or keypress. Play main menu music after intro music ends.")},

   {EndSplashOption::kUserSilent,
    /** TRANSLATORS: Dropdown entry for time to end splashscreen */
    gettext_noop("Click, end music"),
    /** TRANSLATORS: Tooltip for an end splashscreen time option */
    gettext_noop("Wait for mouse click or keypress. No music after intro music ends.")}};
