include "txts/help/common_helptexts.lua"

push_textdomain("texts")

local r = {
   rt(fs_color(
      p_font("align=center", "size=38 face=Widelands/Widelands.ttf color=2F9131", img("images/logos/wl-ico-32.png") .. "Widelands") ..
      p_font("align=center", "size=14 italic=1 color=D1D1D1", _("an open source strategy game")) ..
      help_introduction() ..
      p(vspace(3)) .. inline_header(_("Check out the Widelands project homepage:"), a("widelands.org", "url", "https://www.widelands.org")) .. p(vspace(3)) ..
      p(_("Widelands is licensed under the GNU General Public License (GPL). For more information, see the file ‘COPYING’.")) ..
      h2(_("Status")) ..
      p(_("Widelands is nearly feature complete and is much fun to play alone and even more in multiplayer with others. Still, there is always more work to be done in all areas. If you are interested in contributing – be it by making graphics, fixing bugs or adding new features, translating the game into your language or creating new maps – get in touch on our homepage.")) ..

      h2(_("Widelands Help")) ..
      -- TRANSLATORS: %1% is a key on the keyboard
      p((_("You can find help about gameplay or the editor by using the help button on the bottom menu, or by pressing %1%."))
         -- TRANSLATORS: This is a key on the keyboard
         :bformat(_("F1"))) ..
      help_online_help() ..

      h2(_("Reporting Bugs")) ..
      p(_("If you encounter a bug, please report it to our bugtracker:")) ..
      p(a("widelands.org/wiki/ReportingBugs", "url", "https://www.widelands.org/wiki/ReportingBugs")) ..
      p(_("Please provide enough background information. Tell us:")) ..
      p(
          li(_("Detailed steps on how to trigger the bug, if possible.")) ..
          li(_("What you expected to happen when performing the steps and what actually happened.")) ..
          li(_("Which version of Widelands you are running (i.e. either the build number or the revision number if you are running a development version or a daily build.)")) ..
          li(_("Please also include which operating system you are running Widelands on as some bugs can be platform specific.")) ..
          li(_("Which map you were playing when the bug occurred.")) ..
          li(_("If the bug concerns something not being displayed correctly or if it helps demonstrate the issue, please include one or more screenshots.")) ..
          li(_("If you have a save game or replay demonstrating the issue (for instance crashes where it can be hard to tell exactly what is triggering it), please include that too. For more information, see the section on Replays below."))) ..
      help_replays()
   ))
}
pop_textdomain()
return r
