dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Meat",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"meat",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Meat contains a lot of energy. It is produced by hunters and used in the taverns, inns and big inns to prepare rations, snacks and meals for the miners. It is also consumed in the training sites (training camp and battle arena).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 6 },
      },
   }
}
