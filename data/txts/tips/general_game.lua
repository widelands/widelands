include "scripting/richtext.lua"

push_textdomain("texts")
tips = {
   {
      text = _"The building process will only start after a road is built to the construction site.",
      seconds = 4
   },
   {
      text = _"Be sure to send a geologist to survey sites before building a well or a mine.",
      seconds = 4
   },
   {
      text = _"Your territory can be expanded by building various military buildings – Sentry, Fortress, Blockhouse, etc.",
      seconds = 5
   },
   {
      text = _"Miners will work only if supplied with food.",
      seconds = 3
   },
   {
      text = _"Split your roads into shorter sections by planting some flags. This will speed up transportation of your wares.",
      seconds = 6
   },
   {
      -- TRANSLATORS: %s = URL to the Widelands website
      text = (_"Any further questions? Our FAQ, the wiki and of course our community forums can be found online at %s."):bformat(u("widelands.org")),
      seconds = 6
   },
   {
      text = _"Use the priority buttons in building windows to adjust delivery of a specific ware. Pressing Ctrl while clicking will adjust all wares at once.",
      seconds = 7
   },
   {
      text = _"Use the ‘configure economy’ menu to adjust the target quantity of wares. It can be accessed via the flag menu.",
      seconds = 7
   },
   {
      text = _"Manage your messages more efficiently: press ‘N’ for News to access the messages window, then use ‘G’ for Go To, and the Delete key to archive messages.",
      seconds = 6
   },
   {
      text = _"Press %1% to store important locations, then press %2% to recall them.":bformat(pgettext("hotkey", "Ctrl + (1-9)"), pgettext("hotkey", "1-9")),
      seconds = 4
   },
   {
      text = _"Removing a road while holding Ctrl lets you remove all flags up to the first junction.",
      seconds = 4
   },
   {
      text = _"New soldiers are recruited in a barracks. Their abilities can then be improved in training sites.",
      seconds = 6
   },
   {
      text = _"If you like to live dangerously, pressing Ctrl while clicking the enhance, dismantle or destroy building buttons will skip the confirmation.",
      seconds = 6
   },
   {
      text = _"Press I to quickly toggle the display of your stock inventory.",
      seconds = 4
   },
   {
      text = _"An economy consists of all buildings and flags connected by roads. The supplies and demands of buildings are handled in its economy.",
      seconds = 8
   },
   {
      text = _"Some buildings, such as the farm, have a very small work area. If the building is surrounded with trees or roads, production will stop.",
      seconds = 7
   },
   {
      text = _"Zoom in/out with the mouse wheel or %1%. Press %2% to reset the zoom.":bformat(pgettext("hotkey", "Ctrl + (+/-)"), pgettext("hotkey", "Ctrl + 0")),
      seconds = 3
   },
   {
      text = _"Use the ‘,’ (comma) and ‘.’ (period) keys to quickly jump between recently visited locations.",
      seconds = 3
   },
   {
      text = _"Speed up or slow down the game with %1%. Use %2% for rapid change, %3% will adjust speed smoothly.":bformat(pgettext("hotkey", "Page Up/Page Down"), pgettext("hotkey", "Ctrl + (Page Up/Page Down)"), pgettext("hotkey", "Shift + (Page Up/Page Down)")),
      seconds = 7
   },
   {
      text = _"Move map view using %1%. %2% will move the view faster, %3% moves slowly.":bformat(pgettext("hotkey", "Arrow keys"), pgettext("hotkey", "Ctrl + Arrow keys"), pgettext("hotkey", "Shift + Arrow keys")),
      seconds = 4
   },
   {
      text = _"When a building is destroyed, its workers will start looking for a nearby flag from which they can move by road to a connected warehouse. If there are no such flags in the vicinity, they will keep wandering around and eventually die.",
      seconds = 7
   },

}
pop_textdomain()
return tips
