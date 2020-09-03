push_textdomain("tribes")

dirname = path.dirname(__file__)

tribes:new_militarysite_type {
   name = "empire_castle",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("empire_building", "Castle"),
   icon = dirname .. "menu.png",
   size = "big",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 94, 106 }
      }
   },

   aihints = {},

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your castle.",
      aggressor = _"Your castle discovered an aggressor.",
      attack = _"Your castle is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the castle.",
      defeated_you = _"Your soldiers defeated the enemy at the castle."
   },
}

pop_textdomain()
