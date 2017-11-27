include "scripting/formatting.lua"

set_textdomain("texts")

function a(text)
   return "</p><p font-size=14 font-decoration=underline>" .. text
end

function a2(text)
   return "</p><p font-size=14 font-decoration=underline>" .. text .. "</p><p font-size=14 color=ff4444>"
end

function i(text)
   return "</p><p font-size=14 font-style=italic>" .. text
end

return {
   title = _"License",
   text = rt(
      title(_"Licensing information for Widelands") ..

      "<rt>" .. h2("<br>" ..
      _"Copyright 2002 - 2016 by the Widelands Development Team.") ..
   h2(_"This game is Free and Open Source (FOSS), licensed under the GNU General Public License (GPL) V2.0.")..

      --"<p font-size=8><br></p" ..

      p(_"You can find more information on FOSS and the GPL by visiting the following webpage: %s"):bformat(a("https://www.gnu.org/licenses/old-licenses/gpl-2.0.html")) ..
      p(_"You can find the full text of the license there as well as further information about its philosophy and the legal implications.") ..

      p(_"We are also shipping the GPL as a text document with Widelands itself.") ..
      p(_"On Linux, you can find the file called COPYING in the root of the source or standalone binary package, or in the installation directory (like ‘%1%’)."):bformat(i("/usr/share/games/widelands")) ..
      p(_"On Windows, you can find the file called COPYING.txt in the installation folder, and the Widelands Start menu entry provides a link to this file.") ..
      p(_"On MacOS, you can find the file called COPYING in the archive you downloaded from the website.") ..

      h2(_"This game comes as-is and without any warranty.") ..

      "<p font-size=14>" .. (_"For more information and support you can find us at %1% (Website, Wiki, Forum for questions or general support), %2% (Bugtracker), and %3% (Translations)."):
         bformat(
            a2("widelands.org"),
            a2("widelands.org/wiki/ReportingBugs"),
            a2("widelands.org/wiki/TranslatingWidelands"))
         .. "</p>"
   )
}
