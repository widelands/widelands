include "scripting/richtext.lua"

push_textdomain("texts")

local r = {
   rt(fs_color(
      p_font("align=center", "size=28 color=2F9131", _("Licensing information for Widelands")) ..
      -- TRANSLATORS: Placeholder is copyright end year
      p_font("align=center", "size=14 italic=1 color=D1D1D1", _("Copyright 2002 - %1% by the Widelands Development Team.")):bformat(2022) ..

      h2(_("This game is Free and Open Source (FOSS), licensed under the GNU General Public License (GPL) V2.0.")) ..

      p(vspace(6) .. _("You can find more information on FOSS and the GPL by visiting the following webpage: %s")):bformat(a("https://www.gnu.org/licenses/old-licenses/gpl-2.0.html", "url", "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html")) ..

      p(vspace(6) .. _("You can find the full text of the license there as well as further information about its philosophy and the legal implications.")) ..

      p(vspace(6) .. _("We are also shipping the GPL as a text document with Widelands itself.")) ..
      p(vspace(6) .. _("On Linux, you can find the file called COPYING in the root of the source or standalone binary package, or in the installation directory (like ‘%1%’).")):bformat(i("/usr/share/games/widelands")) ..
      p(vspace(6) .. _("On Windows, you can find the file called COPYING.txt in the installation folder, and the Widelands Start menu entry provides a link to this file.")) ..
      p(vspace(6) .. _("On MacOS, you can find the file called COPYING in the archive you downloaded from the website.")) ..

      h2(_("This game comes as-is and without any warranty.")) ..
      p(_("For more information and support you can find us at %1% (Website, Wiki, Forum for questions or general support), %2% (Bugtracker), and %3% (Translations).")):
         bformat(
            a("widelands.org", "url", "https://www.widelands.org"),
            a("widelands.org/wiki/ReportingBugs", "url", "https://www.widelands.org/wiki/ReportingBugs"),
            a("widelands.org/wiki/TranslatingWidelands", "url", "https://www.widelands.org/wiki/TranslatingWidelands"))
   ))
}
pop_textdomain()
return r
