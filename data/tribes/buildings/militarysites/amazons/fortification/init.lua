push_textdomain("tribes")

dirname = path.dirname (__file__)

tribes:new_militarysite_type {
   name = "amazons_fortification",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Fortification"),
   icon = dirname .. "menu.png",
   size = "big",
   vision_range = 16,

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {109, 91}},
      unoccupied = {hotspot = {109, 91}},
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true
   },

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      occupied = _"Your soldiers have occupied your fortification.",
      aggressor = _"Your fortification discovered an aggressor.",
      attack = _"Your fortification is under attack.",
      defeated_enemy = _"The enemy defeated your soldiers at the fortification.",
      defeated_you = _"Your soldiers defeated the enemy at the fortification."
   },
}

pop_textdomain()
