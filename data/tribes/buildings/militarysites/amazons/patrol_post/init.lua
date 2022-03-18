push_textdomain("tribes")

dirname = path.dirname (__file__)

wl.Descriptions():new_militarysite_type {
   name = "amazons_patrol_post",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext ("amazons_building", "Patrol Post"),
   icon = dirname .. "menu.png",
   size = "small",

   buildcost = {
      granite = 1,
      log = 1,
      rope = 1
   },
   return_on_dismantle = {
      granite = 1,
   },

   animation_directory = dirname,
   animations = {
      idle = {hotspot = {39, 46}},
      unoccupied = {hotspot = {39, 46}}
   },

   aihints = {
      expansion = true,
      fighting = true,
   },

   max_soldiers = 2,
   heal_per_second = 100,
   conquers = 6,
   prefer_heroes = false,

   messages = {
      -- TRANSLATORS: Message sent by an Amazon military site
      occupied = pgettext("amazons_building", "Your soldiers have occupied your patrol post."),
      -- TRANSLATORS: Message sent by an Amazon military site
      aggressor = pgettext("amazons_building", "Your patrol post discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Amazon military site
      attack = pgettext("amazons_building", "Your patrol post is under attack."),
      -- TRANSLATORS: Message sent by an Amazon military site
      defeated_enemy = pgettext("amazons_building", "The enemy defeated your soldiers at the patrol post."),
      -- TRANSLATORS: Message sent by an Amazon military site
      defeated_you = pgettext("amazons_building", "Your soldiers defeated the enemy at the patrol post.")
   },
}
pop_textdomain()
