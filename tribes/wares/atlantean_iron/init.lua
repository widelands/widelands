dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Iron",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"iron",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 4,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Iron is made out of iron ore in the smelting works. It is used in the toolsmithy, armor smithy and weapon smithy.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 9 },
      },
   }
}
