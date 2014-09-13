dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_coal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Coal",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"coal",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The fires of the Imperial smelting works, armor smithies and weapon smithies are usually fed with coal. It is mined in coal mines or produced by a charcoal kiln out of logs.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 6 },
      },
   }
}
