dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"iron",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Weapons, armor and tools are made of iron. It is extracted from iron ore in the smelting works.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 8 },
      },
   }
}
