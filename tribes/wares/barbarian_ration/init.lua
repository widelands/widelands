dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_ration",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Ration",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"rations",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A small bite to keep miners strong and working. Also the scout consumes rations on his scouting trips. Rations are produced in a tavern, an inn or a big inn out of fish or meat or pitta bread. ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 5 },
      },
   }
}
