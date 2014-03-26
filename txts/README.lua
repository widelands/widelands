include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("texts")

return {
   title = _ "README",
   text =
rt("image=pics/wl-logo-64.png image-align=center text-align=center") ..
rt("text-align=center",
   p("font-size=38 font-face=Widelands/Widelands font-color=#2F9131",
[[Widelands]]
   ) .. p("font-size=14 font-style=italic text-align=center", _
[[an open source strategy game]]
   )
) .. rt(
   h2(_
[[Introduction]]
   ) .. p(_
[[Widelands is a free, open source real-time strategy game with singleplayer
campaigns and a multiplayer mode. The game was inspired by Settlers II but has
significantly more variety and depth to it. Still, it is easy to get started
through playable tutorials.]]
   ) .. p(_
[[In this game, you start out on a small piece of land with just a few
resources. Using these, you’ll create an empire with many thousands of
inhabitants. On your way towards this goal, you will have to establish an
economic infrastructure, explore the land around you and face rivaling
tribes.]]
   ) .. h3(_
[[Check out the Widelands project homepage:]]
   ) .. p(
[[http://wl.widelands.org]]
   ) .. p(_
[[Widelands is licensed under the GNU General Public License (GPL). For more
information, see the file ‘COPYING’.]]
   ) .. h3(_
[[Status]]
   ) .. p(_
[[Widelands is nearly feature complete and is much fun to play alone and even
more in multiplayer with others. Still, there is always more work to be done in
all areas. If you are interested in contributing – be it by making graphics,
fixing bugs or adding new features, translating the game into your language or
creating new maps – get in touch on our homepage.]])
   .. h2(_
[[General game control]]
   ) .. p(_
[[To close an open window, right-click on it. To minimize/maximize a window, click
on the title bar of that window either with the middle mouse button or with the left
mouse button while holding down CTRL.]]
   ) .. p(_
[[If you hold CTRL while building a road, flags will be placed automatically.
If you hold CTRL while removing a road, all flags up to the first junction are
removed.]]
   ) .. p(_
[[You can skip confirmation windows for irreversible actions if you hold CTRL
while clicking on the action button.]]
   ) .. h2(_
[[Keyboard shortcuts (in-game)]]
   ) .. p(
_"PAGEUP: increases game speed" .. "<br>"
.. _"PAGEDOWN: decreases game speed" .. "<br>"
.. _"PAUSE: pauses the game" .. "<br>"
.. _"SPACE: toggles if building spaces are shown" .. "<br>"
.. _"M: toggles minimap" .. "<br>"
.. _"N: toggles messages (‘news’)" .. "<br>"
.. _"C: toggles census" .. "<br>"
.. _"S: toggles statistics" .. "<br>"
.. _"I: toggles stock inventory" .. "<br>"
.. _"O: toggles objectives" .. "<br>"
.. _"F: toggles fullscreen (if supported by the OS)" .. "<br>"
.. _"Home: centers main mapview on starting location" .. "<br>"
.. _"(CTRL+) 0-9: Remember and go to previously remembered locations" .. "<br>"
.. _",: goes to the previous location" .. "<br>"
.. _".: goes to the next location" .. "<br>"
.. _"F6: shows the debug console (only in debug-builds)" .. "<br>"
.. _"CTRL+F10: quits the game immediately" .. "<br>"
.. _"F11: takes a screenshot"
   ) .. p(_
[[In the message window, the following additional shortcuts are available:]]
   ) .. p(
_ "G: jumps to the location corresponding to the current message" .. "<br>"
.. _"DELETE: archives the current message"
   ) .. h2(_
[[Online Help]]
   ) .. p( _
[[If you need documentation or help for Widelands or the Widelands Editor, be
sure to visit our homepage. You can find an up to date online help in our Wiki
and a detailed description of the tribe economies in our online encyclopedia.]]
   ) .. h2(_
[[Reporting Bugs]]
   ) .. p(_
[[If you encounter a bug, please report it to our bugtracker:]]
   ) .. p(
[[https://bugs.launchpad.net/widelands]]
   ) .. p(_
[[Please provide enough background information. Tell us:]]
   ) .. p(_
[[- Detailed steps on how to trigger the bug, if possible.]]
   ) .. p(_
[[- What you expected to happen when performing the steps and what actually happened.]]
   ) .. p(_
[[- Which version of Widelands you are running (i.e. either the build number or
the revision number if you are running a development version or a daily build.)]]
   ) .. p(_
[[- Please also include which operating system you are running Widelands on as
some bugs can be platform specific.]]
   ) .. p(_
[[- Which map you were playing when the bug occurred.]]
   ) .. p(_
[[- If the bug concerns something not being displayed correctly or if it helps
demonstrate the issue, please include one or more screenshots.]]
   ) .. p(_
[[- If you have a save game or replay demonstrating the issue (for instance
crashes where it can be hard to tell exactly what is triggering it), please
include that too. For more information, see the section on Replays below.]]
   ) .. h2(_
[[Replays]]
   ) .. p(_
[[Widelands records every game you play in two files. One of these files has a
.wrpl.wgf extension and contains an initial savegame. The other file has a
.wrpl extension and contains all commands that were issued by all players, such
as building a road or attacking an enemy. The rest of the filenames will tell
you the time you started the game.]]
   ) .. p(_
[[Using the Watch Replay option in the main menu, a previously played game will
be precisely reconstructed for you to watch. Note, however, that even small
changes in the game data files or the Widelands executable can cause a replay
to become incompatible and to ‘go out of sync’. This is similar to how network
games will suffer from desynchronization problems if one of the players has a
different version of the game.]]
   ) .. p(_
[[Replays are a very useful tool to find bugs in the game logic, so they
can be useful as attachments to bug reports. The replay files are saved in the
~/.widelands/replays directory on Linux and Mac OS X. On Windows,
they are in the ‘replays’ subdirectory of your Widelands installation.]]
   )
)
}
