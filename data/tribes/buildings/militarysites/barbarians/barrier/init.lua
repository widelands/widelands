push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_militarysite_type {
   name = "barbarians_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Barrier"),
   icon = dirname .. "menu.png",
   size = "medium",

   buildcost = {
      blackwood = 5,
      grout = 2
   },
   return_on_dismantle = {
      blackwood = 2,
      grout = 1
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 44, 62 },
      }
   },
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 44, 62 }
      },
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 44, 62 }
      },
   },

   aihints = {},

   max_soldiers = 5,
   heal_per_second = 130,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by a Barbarian military site
      occupied = pgettext("barbarians_building", "Your soldiers have occupied your barrier."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      aggressor = pgettext("barbarians_building", "Your barrier discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      attack = pgettext("barbarians_building", "Your barrier is under attack."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      defeated_enemy = pgettext("barbarians_building", "The enemy defeated your soldiers at the barrier."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      defeated_you = pgettext("barbarians_building", "Your soldiers defeated the enemy at the barrier.")
   },
}

pop_textdomain()
