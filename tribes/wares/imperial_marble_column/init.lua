dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_marble_column",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Marble Column",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"marble columns",
   tribe = "empire",
   default_target_quantity = 10,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"TMarble columns represent the high culture of the Empire, so they are needed for nearly every larger building. They are produced out of marble at a stonemason’s house.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 9 },
      },
   }
}
