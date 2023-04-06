include "scripting/richtext.lua"

function p_sp(text)
   return p(vspace() .. text)
end

push_textdomain("texts")
set_fs_style(true)

local r = {
   rt(
      pagetitle(_("Licensing information for Widelands")) ..
      -- TRANSLATORS: Placeholder is copyright end year
      subtitle(_("Copyright 2002 - %1% by the Widelands Development Team.")):bformat(2023) ..

      h2(_("This game is Free and Open Source (FOSS), licensed under the GNU General Public License (GPL) V2.0.")) ..

      p_sp(_("You can find more information on FOSS and the GPL by visiting the following webpage: %s"):bformat(a("https://www.gnu.org/licenses/old-licenses/gpl-2.0.html", "url", "https://www.gnu.org/licenses/old-licenses/gpl-2.0.html"))) ..

      p_sp(_("You can find the full text of the license there as well as further information about its philosophy and the legal implications.")) ..

      p_sp(_("We are also shipping the GPL as a text document with Widelands itself.")) ..
      p_sp(_("On Linux, you can find the file called COPYING in the root of the source or standalone binary package, or in the installation directory (like ‘%1%’)."):bformat(i("/usr/share/games/widelands"))) ..
      p_sp(_("On Windows, you can find the file called COPYING.txt in the installation folder, and the Widelands Start menu entry provides a link to this file.")) ..
      p_sp(_("On MacOS, you can find the file called COPYING in the archive you downloaded from the website.")) ..

      h2(_("This game comes as-is and without any warranty.")) ..
      p(_("For more information and support you can find us at %1% (Website, Wiki, Forum for questions or general support), %2% (Bugtracker), and %3% (Translations)."):
         bformat(
            a("widelands.org", "url", "https://www.widelands.org"),
            a("widelands.org/wiki/ReportingBugs", "url", "https://www.widelands.org/wiki/ReportingBugs"),
            a("widelands.org/wiki/TranslatingWidelands", "url", "https://www.widelands.org/wiki/TranslatingWidelands")))
   )
}

set_fs_style(false)
pop_textdomain()
return r
