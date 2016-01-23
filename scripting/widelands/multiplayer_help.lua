include "scripting/formatting.lua"

function picture_li(imagepath, text)
   return "<rt image=" .. imagepath .. " image-align=left>" .. p(text) .. "</rt>"
end

return {
   func = function()
      set_textdomain("widelands")
      local result = rt(h1(_"Multiplayer Game Setup"))
      result = result .. rt(p(_"You are in the multiplayer launch game menu."))


      result = result .. rt(h2(_"Client settings"))
      result = result .. rt(p(_"On the left side is a list of all clients including you. You can set your role with the button following your nickname. Available roles are:"))
      result = result .. picture_li("pics/genstats_enable_plr_08.png", _"The player with the color of the flag. If more than one client selected the same color, these share control over the player (‘shared kingdom mode’).")
      result = result .. picture_li("pics/menu_tab_watch.png", _"Spectator mode, meaning you can see everything, but cannot control any player")

      result = result .. rt(h2(_"Player settings"))
      result = result .. rt(p(_"In the middle are the settings for the players. To start a game, each player must be one of the following:"))
      result = result .. picture_li("pics/genstats_nrworkers.png", _"Connected to one or more clients (see ‘Client settings’).")
      result = result .. picture_li("pics/ai_normal.png", _"Connected to a computer player (the face in the picture as well as the mouse hover texts indicate the strength of the currently selected computer player).")
      result = result .. picture_li("pics/shared_in.png", _"Set as shared in starting position for another player.")
      result = result .. picture_li("pics/stop.png", _"Closed.")
      result = result .. rt(p(_"The latter three can only be set by the hosting client by left-clicking the ‘type’ button of a player. Hosting players can also set the initialization of each player (the set of buildings, wares and workers the player starts with) and the tribe and team for computer players"))
      result = result .. rt(p(_"Every client connected to a player (the set ‘role’ player) can set the tribe and the team for that player"))

      result = result .. rt(h2(_"Map details"))
      result = result .. rt(p(_"You can see information about the selected map or savegame on the right-hand side. A button next to the map name allows the host to change to a different map. Furthermore, the host is able to set a specific win condition, and finally can start the game as soon as all players are set up."))
      return result
   end
}
