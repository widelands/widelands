push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_militarysite_type {
   name = "europeans_advanced_barrier",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("europeans_building", "Advanced Barrier"),
   icon = dirname .. "menu.png",
   size = "medium",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle_??.png"),
         hotspot = { 49, 77 }
      }
   },

   aihints = {
      fighting = true
   },

   max_soldiers = 8,
   heal_per_second = 200,
   conquers = 8,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Empire military site
      occupied = pgettext("europeans_building", "Your soldiers have occupied your barrier."),
      -- TRANSLATORS: Message sent by an Empire military site
      aggressor = pgettext("europeans_building", "Your barrier discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Empire military site
      attack = pgettext("europeans_building", "Your barrier is under attack."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_enemy = pgettext("europeans_building", "The enemy defeated your soldiers at the barrier."),
      -- TRANSLATORS: Message sent by an Empire military site
      defeated_you = pgettext("europeans_building", "Your soldiers defeated the enemy at the barrier.")
   },
}

pop_textdomain()
