push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_militarysite_type {
   name = "atlanteans_tower_high",
   -- TRANSLATORS: This is a building name used in lists of buildings
   descname = pgettext("atlanteans_building", "High Tower"),
   icon = dirname .. "menu.png",
   size = "medium",
   vision_range = 21,

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 53, 85 },
      }
   },

   aihints = {},

   max_soldiers = 5,
   heal_per_second = 170,
   conquers = 9,
   prefer_heroes = true,

   messages = {
      -- TRANSLATORS: Message sent by an Atlantean military site
      occupied = pgettext("atlanteans_building", "Your soldiers have occupied your high tower."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      aggressor = pgettext("atlanteans_building", "Your high tower discovered an aggressor."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      attack = pgettext("atlanteans_building", "Your high tower is under attack."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      defeated_enemy = pgettext("atlanteans_building", "The enemy defeated your soldiers at the high tower."),
      -- TRANSLATORS: Message sent by an Atlantean military site
      defeated_you = pgettext("atlanteans_building", "Your soldiers defeated the enemy at the high tower.")
   },
}

pop_textdomain()
