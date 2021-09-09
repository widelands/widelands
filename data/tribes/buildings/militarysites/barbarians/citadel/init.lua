push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "barbarians_citadel",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Citadel"),
   icon = dirname .. "menu.png",
   size = "big",

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 102, 102 }
      }
   },
   spritesheets ={
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 102, 102 }
      },
      build = {
         frames = 6,
         rows = 3,
         columns = 2,
         hotspot = { 102, 102 }
      },
   },

   aihints = {},

   max_soldiers = 12,
   heal_per_second = 220,
   conquers = 12,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by a Barbarian military site
      occupied = pgettext("barbarians_building", "Your soldiers have occupied your citadel."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      aggressor = pgettext("barbarians_building", "Your citadel discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      attack = pgettext("barbarians_building", "Your citadel is under attack."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      defeated_enemy = pgettext("barbarians_building", "The enemy defeated your soldiers at the citadel."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      defeated_you = pgettext("barbarians_building", "Your soldiers defeated the enemy at the citadel.")
   },
}

pop_textdomain()
