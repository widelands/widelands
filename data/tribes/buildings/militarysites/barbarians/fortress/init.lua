push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "barbarians_fortress",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("barbarians_building", "Fortress"),
   icon = dirname .. "menu.png",
   size = "big",

   enhancement = {
      name = "barbarians_citadel",
      enhancement_cost = {
         blackwood = 9,
         log = 5,
         granite = 4,
         grout = 2
      },
      enhancement_return_on_dismantle = {
         blackwood = 4,
         log = 2,
         granite = 3,
         grout = 1
      }
   },

   buildcost = {
      blackwood = 9,
      log = 5,
      granite = 4,
      grout = 2
   },
   return_on_dismantle = {
      blackwood = 4,
      log = 2,
      granite = 2,
      grout = 1
   },

   animation_directory = dirname,
   animations = {
      unoccupied = {
         hotspot = { 103, 80 }
      }
   },
   spritesheets = {
      idle = {
         fps = 10,
         frames = 20,
         rows = 5,
         columns = 4,
         hotspot = { 101, 80 }
      },
      build = {
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 101, 80 }
      },
   },

   aihints = {
      expansion = true,
      fighting = true,
      mountain_conqueror = true,
      prohibited_till=1400
   },

   max_soldiers = 8,
   heal_per_second = 170,
   conquers = 11,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by a Barbarian military site
      occupied = pgettext("barbarians_building", "Your soldiers have occupied your fortress."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      aggressor = pgettext("barbarians_building", "Your fortress discovered an aggressor."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      attack = pgettext("barbarians_building", "Your fortress is under attack."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      defeated_enemy = pgettext("barbarians_building", "The enemy defeated your soldiers at the fortress."),
      -- TRANSLATORS: Message sent by a Barbarian military site
      defeated_you = pgettext("barbarians_building", "Your soldiers defeated the enemy at the fortress.")
   },
}

pop_textdomain()
