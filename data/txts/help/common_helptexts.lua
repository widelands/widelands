-- This file contains helptexts that are used in multiple scripts,
-- e.g. for the readme and general in-game help
-- This cuts down on string maintenance.

include "scripting/formatting.lua"

set_textdomain("texts")

function help_online_help()
   return
      h2(_[[Online Help]]) ..
      p(_[[If you need more documentation or help for Widelands or the Widelands Editor, be sure to visit our homepage at %s. You can find online help in our Wiki there.]]):bformat(a("https://wl.widelands.org/"))
end

function help_introduction()
   return
      h2(_[[Introduction]]) ..
      p(_[[Widelands is a free, open source real-time strategy game with singleplayer campaigns and a multiplayer mode. The game was inspired by Settlers II but has significantly more variety and depth to it. Still, it is easy to get started through playable tutorials.]]) ..
      p(_[[In this game, you start out on a small piece of land with just a few resources. Using these, you’ll create an empire with many thousands of inhabitants. On your way towards this goal, you will have to establish an economic infrastructure, explore the land around you and face rivaling tribes.]])
end

function help_replays()
   return
      h2(_[[Replays]]) ..
      p(_[[Widelands records every game you play in two files. One of these files has a .wrpl.wgf extension and contains an initial savegame. The other file has a .wrpl extension and contains all commands that were issued by all players, such as building a road or attacking an enemy. The rest of the filenames will tell you the time you started the game.]]) ..
      p(_[[Using the Watch Replay option in the main menu, a previously played game will be precisely reconstructed for you to watch. Note, however, that even small changes in the game data files or the Widelands executable can cause a replay to become incompatible and to ‘go out of sync’. This is similar to how network games will suffer from desynchronization problems if one of the players has a different version of the game.]]) ..
      p((_[[Replays are a very useful tool to find bugs in the game logic, so they can be useful as attachments to bug reports. The replay files are saved in the %s directory on Linux and Mac OS X. On Windows, they are in the ‘replays’ subdirectory of your Widelands installation.]]):bformat(i("~/.widelands/replays"))
   )
end

function help_tribal_encyclopedia()
   return
      p(_"A detailed description of the tribes’ economies is available in the in-game Tribal Encyclopedia." .. " " .. "Use the %s button on the bottom right to access it."):bformat(b("?"))
end
