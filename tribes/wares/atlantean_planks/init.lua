dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_planks",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Planks",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"planks",
   tribe = "atlanteans",
   default_target_quantity = 40,
   preciousness = 7,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Planks are an important building material of the Atlanteans. They are produced out of logs by the sawmill. The weapon smithy and the shipyard also use planks to produce the different tridents and mighty ships.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 12, 17 },
      },
   }
}
