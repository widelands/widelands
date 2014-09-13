dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_gold",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Gold",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"gold",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Gold is one of the most valuable materials. It is produced out of gold ore in the smelting works. Gold is used by the armor smithy, the weapon smithy and the gold spinning mill.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 10 },
      },
   }
}
