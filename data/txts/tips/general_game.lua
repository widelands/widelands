include "scripting/richtext.lua"
include "txts/help/common_helptexts.lua"

push_textdomain("texts")
tips = {
   {
      text = _("The building process will only start after a road is built to the construction site."),
      seconds = 4
   },
   {
      text = _("Be sure to send a geologist to survey sites before building a well or a mine."),
      seconds = 4
   },
   {
      text = _("Your territory can be expanded by building various military buildings – Sentry, Fortress, Blockhouse, etc."),
      seconds = 5
   },
   {
      text = _("Miners will work only if supplied with food."),
      seconds = 3
   },
   {
      text = _("Split your roads into shorter sections by planting some flags. This will speed up transportation of your wares."),
      seconds = 6
   },
   {
      -- TRANSLATORS: %s = URL to the Widelands website
      text = (_("Any further questions? Our FAQ, the wiki and of course our community forums can be found online at %s.")):bformat(a("widelands.org", "url", "https://www.widelands.org")),
      seconds = 6
   },
   {
      text = _("Use the priority sliders in building windows to adjust delivery of a specific ware. Holding down Shift while clicking will adjust all wares at once."),
      seconds = 7
   },
   {
      text = _("Use the ‘configure economy’ menu to adjust the target quantity of wares. It can be accessed via the flag menu."),
      seconds = 7
   },
   {
      text = _("Manage your messages more efficiently: press ‘%1%’ to access the messages window, then use ‘%2%’ for Go To, and the ‘%3%’ key to archive messages."):bformat(
         wl.ui.get_shortcut("game_messages"), wl.ui.get_shortcut("game_msg_goto"), wl.ui.get_shortcut("delete")),
      seconds = 6
   },
   {
      text = _("Press %1% to store important locations, then press %2% to recall them."):bformat(help_set_landmarks_hotkeys(), help_goto_landmarks_hotkeys()),
      seconds = 4
   },
   {
      text = _("Removing a road while holding Ctrl lets you remove all flags up to the first junction."),
      seconds = 4
   },
   {
      text = _("New soldiers are recruited in a barracks. Their abilities can then be improved in training sites."),
      seconds = 6
   },
   {
      text = _("If you like to live dangerously, pressing Ctrl while clicking the enhance, dismantle or destroy building buttons will skip the confirmation."),
      seconds = 6
   },
   {
      text = _("Press ‘%s’ to quickly toggle the display of your stock inventory."):bformat(wl.ui.get_shortcut("game_stats_stock")),
      seconds = 4
   },
   {
      text = _("An economy consists of all buildings and flags connected by roads, ferries, or ships. The supplies and demands of buildings are handled in their economy. A player can have more than one economy."),
      seconds = 8
   },
   {
      text = _("You can have more than one economy if their flags are not connected by any means of transportation. This is why the Configure Economy menu can only be accessed from flags and warehouses, not the player’s main menu."),
      seconds = 8
   },
   {
      text = _("Some buildings, such as the farm, have a very small work area. If the building is surrounded with trees or roads, production will stop."),
      seconds = 7
   },
   {
      text = _("Zoom in/out with the mouse wheel or %1%/%2%. Press %3% to reset the zoom."):bformat(
         wl.ui.get_shortcut("zoom_in"),
         wl.ui.get_shortcut("zoom_out"),
         wl.ui.get_shortcut("zoom_reset")),
      seconds = 3
   },
   {
      text = _("Use the ‘%1$s’ and ‘%2$s’ keys to quickly jump between recently visited locations."):bformat(wl.ui.get_shortcut("quicknav_prev"), wl.ui.get_shortcut("quicknav_next")),
      seconds = 3
   },
   {
      text = _("Speed up or slow down the game with %1%/%2%. Use %3%/%4% for rapid change; %5%/%6% will adjust speed smoothly."):bformat(
         wl.ui.get_shortcut("game_speed_up"),
         wl.ui.get_shortcut("game_speed_down"),
         wl.ui.get_shortcut("game_speed_up_fast"),
         wl.ui.get_shortcut("game_speed_down_fast"),
         wl.ui.get_shortcut("game_speed_up_slow"),
         wl.ui.get_shortcut("game_speed_down_slow")),
      seconds = 7
   },
   {
      text = _("Move map view using %1%. %2% will move the view faster, %3% moves slowly."):bformat(pgettext("hotkey", "Arrow keys"), pgettext("hotkey", "Ctrl + Arrow keys"), pgettext("hotkey", "Shift + Arrow keys")),
      seconds = 4
   },
   {
      text = _("If you play on a laptop with a touchpad, you can assign functionality to touchpad scrolling in the ‘Options’ → ‘Edit keyboard and mouse actions’ menu. For example, moving the map view can be much more comfortable by scrolling instead of dragging with the right mouse button."),
      seconds = 8
   },
   {
      text = _("When a building is destroyed, its workers will start looking for a nearby flag from which they can move by road to a connected warehouse. If there are no such flags in the vicinity, they will keep wandering around and eventually die."),
      seconds = 7
   },

}
pop_textdomain()
return tips
