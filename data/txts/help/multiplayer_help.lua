include "scripting/richtext.lua"

return {
   func = function()
      push_textdomain("texts")
      set_fs_style(true)
      local r = rt(
         h1(_("Multiplayer Game Setup")) ..
         p(_("You are in the multiplayer launch game menu.")) ..

         h2(_("Client settings")) ..
         p(_("On the left side is a list of all clients including you. You can set your role with the button following your nickname. Available roles are:")) ..
         li_image("images/players/genstats_player.png", _("The player with the color of the flag. If more than one client selected the same color, these share control over the player (‘shared kingdom mode’).")) ..
         li_image("images/wui/fieldaction/menu_tab_watch.png", _("Spectator mode, meaning you can see everything, but cannot control any player")) ..

         h2(_("Player settings")) ..
         p(_("In the middle are the settings for the players. To start a game, each player must be one of the following:")) ..
         li_image("images/wui/stats/genstats_nrworkers.png", _("Connected to one or more clients (see ‘Client settings’).")) ..
         li_image("images/ai/ai_normal.png", _("Connected to a computer player (the face in the picture as well as the mouse hover texts indicate the strength of the currently selected computer player).")) ..
         li_image("images/ui_fsmenu/shared_in.png", _("Set as shared in starting position for another player.")) ..
         li_image("images/ui_basic/stop.png", _("Closed.")) ..
         p(_("The latter three can only be set by the hosting client by left-clicking the ‘type’ button of a player. Hosting players can also set the initialization of each player (the set of buildings, wares and workers the player starts with) and the tribe and team for computer players.")) ..
         p(_("Every client connected to a player (who isn’t a spectator) can set the tribe and the team for that player.")) ..

         h2(_("Map details")) ..
         p(_("You can see information about the selected map or savegame on the right-hand side. A button next to the map name allows the host to change to a different map. Furthermore, the host is able to set a specific win condition, and finally can start the game as soon as all players are set up."))
      )
      set_fs_style(false)
      pop_textdomain()
      return r
   end
}
