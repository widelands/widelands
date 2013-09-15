use("aux", "formatting")
use("aux", "format_help")

set_textdomain("texts")


return {
   title = _ "README",
   text =
   rt("image=pics/wl-logo-64.png image-align=center text-align=center") ..
   rt("text-align=center",
    p("font-size=38 font-face=Widelands/Widelands font-color=#2F9131", "Widelands")

.. p("font-size=14 font-style=italic text-align=center", _"an open source strategy game")
) .. rt(
 h2(_"Introduction")
.. p(_
[[Widelands is a strategy game aiming for gameplay similar to Settlers II by BlueByte.<br>In this game, you start out on a small piece of land with just a few resources. Using these, you'll create an empire with many thousands of inhabitants. On your way towards this goal, you will have to establish an economic infrastructure, explore the lands around you and face enemies who are trying to rule the world just like you.]])
.. h3(_"Check out the Widelands project homepage:")
.. p("http://wl.widelands.org")
.. p(_"Widelands is licensed under the GNU General Public License (GPL). For more information, see the file 'COPYING' or select the License button from the main menu.")
.. h3(_"Status")
.. p(_
[[Widelands is now on the step of being playable, but to become more playable volunteers are needed. Your contributions (yes, I'm talking to you) are required, especially in the area of graphics, to make Widelands a better game.]])
.. h2(_"General game control")
.. p(_"To close an open window, right click it.")
.. p(_"To minimize/maximize a window, click on the titlebar of that window either with middle mousebutton or with left mousebutton while holding down CTRL.")
.. h2(_"Keyboard shortcuts (in-game)")
.. p(_"SPACE toggles build-help" .. "<br>"
.. _"M        toggles minimap" .. "<br>"
.. _"N toggles messages ('news')" .. "<br>"
.. _"C toggles census" .. "<br>"
.. _"S toggles statistics" .. "<br>"
.. _"I toggles stock inventory" .. "<br>"
.. _"O toggles objectives" .. "<br>"
.. _"F toggles fullscreen" .. "<br>"
.. _"Home centers main mapview on starting location" .. "<br>"
.. _"(CTRL+) 0-9  Remember and go to previously remembered locations" .. "<br>"
.. _", goes to previous location" .. "<br>"
.. _". goes to next location" .. "<br>"
.. _"PAGEUP         increases gamespeed" .. "<br>"
.. _"PAGEDOWN    decreases gamespeed" .. "<br>"
.. _"PAUSE         pauses the game (only for local games)" .. "<br>"
.. _"F6           shows the debug console (only in debug-builds)" .. "<br>"
.. _"Ctrl+F10    quits the game immediately" .. "<br>"
.. _"F11         takes a screenshot")
.. p(_"When finishing a road, flags will be placed along it if Ctrl is held down. Default is to place flags from end to start. If Shift is also held down, they are placed from start to end instead.")
.. p(_"When removing a road, if Ctrl is pressed all flags up to the first junction are removed.")
.. p(_"In the message window, the following additional shortcuts are available:")
.. p(_"G           goes to location corresponding to current message" .. "<br>"
.. _"DELETE      archives the current message")
.. h2(_"Online Help")
.. p(_"If you need documentation or help for Widelands or the Widelands Editor, be sure to visit our homepage. You can find an up to date online help at:")
.. p("http://wl.widelands.org/wiki/")
.. h2(_"Reporting Bugs")

-- TODO: should we still mention the mailing list at the end as a backup for people who don't wish to register at Launchpad?
-- Sending an email somewhere might be considered a smaller "investment" than registering at a site.
.. p(_"If you encounter a bug, please report it to our bugtracker:")
.. p("https://bugs.launchpad.net/widelands")
.. p(_"Please provide enough background information. Tell us:")
.. p(_"- Detailed steps on how to trigger the bug, if possible.")
.. p(_"- What you expected to happen when performing the steps and what actually happened.")
.. p(_"- Which version of Widelands you are running (i.e. either the build number or the revision number if you are running the development release)")
.. p(_"- Please also include which operating system you are running Widelands on, as some bugs can be platform specific.")
.. p(_"- Which map you were playing when the bug occured.")
.. p(_"- If the bug concerns something not being displayed correctly or if it helps demonstrate the issue, please include one or more screenshots.")
.. p(_"- If you have a save game or replay demonstrating the issue (for instance crashes where it can be hard to tell exactly what is triggering it), please include that too. For more information, see the section on Replays below.")
.. p(_"- send us a compressed (zip, gzip, bzip2) session record if possible, unless the<br>bug is trivial to reproduce. In fact, it may be a good idea to always record<br>your sessions in case a bug surfaces. For more explanation, see below.")
.. h2(_"Replays")
.. p(_"Widelands records every game you play in two files. One of these files has a .wrpl.wgf extension and contains an initial savegame. The other file has just a .wrpl extension and contains all commands that were issued by all players, such as building a road or attacking an enemy. The rest of the filenames will tell you the time you started the game.")
.. p(_"Using the Watch Replay option in the main menu, a previously played game will be precisely reconstructed for you to watch. Note, however, that even small changes in the game data files or the Widelands executable can cause a replay to become incompatible and to 'go out of sync'. This is similar to how network games will suffer from desynchronization problems if one of the players has a different version of the game.")
.. p(_"Replays are a very useful tool to find bugs in the game logic code, so we ask you to attach replay files to all bug reports that you submit. The replay files are saved in the ~/.widelands/replays directory in your home on Linux and in the replays subdirectory of your Widelands installation on Windows.")
.. h2(_"Creating a Session Record")
.. p(_"Widelands can record all your keypresses and mouse movements to a file. It can then replay an entire session from starting the game to exiting it. This is really helpful for debugging problems.")
.. p(_"To create a session record, start the game with the --record parameter. On Linux, simply start Widelands like so: $ ./widelands --record=filename.rec")
.. p(_"On Windows, create a shortcut to the file widelands.exe (right-click on widelands.exe and choose Create Shortcut). Then edit the shortcut (right-click on the newly created shortcut and choose Properties). The Target field should read something like c:\games\widelands\widelands.exe. Change this to c:\games\widelands\widelands.exe --record=filename.rec.")
.. p(_"A binary file called filename.rec will be created. To play it back, perform the same steps as outlined above, but replace --record with --playback.")
.. p(_"Note that while recorded files are platform independent, they generally won't work across different versions of Widelands, because the user interface and the order in which low-level functions can be called may change between versions. Session records are not intended to serve as demo files; they are mostly a debugging tool which helps us to track down bugs.")
   ),
}
