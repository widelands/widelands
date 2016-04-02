include "scripting/formatting.lua"
include "txts/help/common_helptexts.lua"

set_textdomain("texts")

return {
   title = _"Readme",
   text =
rt("image=images/logos/wl-logo-64.png image-align=center text-align=center") ..
rt("text-align=center",
   p("font-size=38 font-face=Widelands/Widelands font-color=#2F9131", [[Widelands]]) ..
   p("font-size=14 font-style=italic text-align=center", _[[an open source strategy game]])) ..
rt(
   help_introduction() ..
   h3(_[[Check out the Widelands project homepage:]]) ..
   p(a([[https://wl.widelands.org]])) ..
   p(_[[Widelands is licensed under the GNU General Public License (GPL). For more information, see the file ‘COPYING’.]]) ..
   h3(_[[Status]]) ..
   p(_[[Widelands is nearly feature complete and is much fun to play alone and even more in multiplayer with others. Still, there is always more work to be done in all areas. If you are interested in contributing – be it by making graphics, fixing bugs or adding new features, translating the game into your language or creating new maps – get in touch on our homepage.]]) ..

   h2(_[[Widelands Help]]) ..
   p(_"You can find general help about gameplay or the editor by using the help button in the main menu.") ..
   help_online_help() ..

   h2(_[[Reporting Bugs]]) ..
   p(_[[If you encounter a bug, please report it to our bugtracker:]]) ..
   p(a([[https://bugs.launchpad.net/widelands]])) ..
   p(_[[Please provide enough background information. Tell us:]]) ..
   p(
       listitem_bullet(_[[Detailed steps on how to trigger the bug, if possible.]]) ..
       listitem_bullet(_[[What you expected to happen when performing the steps and what actually happened.]]) ..
       listitem_bullet(_[[Which version of Widelands you are running (i.e. either the build number or the revision number if you are running a development version or a daily build.)]]) ..
       listitem_bullet(_[[Please also include which operating system you are running Widelands on as some bugs can be platform specific.]]) ..
       listitem_bullet(_[[Which map you were playing when the bug occurred.]]) ..
       listitem_bullet(_[[If the bug concerns something not being displayed correctly or if it helps demonstrate the issue, please include one or more screenshots.]]) ..
       listitem_bullet(_[[If you have a save game or replay demonstrating the issue (for instance crashes where it can be hard to tell exactly what is triggering it), please include that too. For more information, see the section on Replays below.]])) ..
   help_replays()
)
}
