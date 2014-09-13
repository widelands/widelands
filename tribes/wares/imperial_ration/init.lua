dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_ration",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Ration",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"rations",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A small bite to keep miners strong and working. Rations are also consumed by the scout on his scouting trips. They are produced in a tavern out of fish or meat or bread.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 5 },
      },
   }
}
