dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_bronze_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Bronze Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"bronze axes",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The bronze ax is considered a weapon that is hard to handle. Only skilled warriors can use it. It is produced at the war mill and used in the training camp (it trains – together with food – from attack level 2 to level 3).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
