dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Meat",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"meat",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Meat contains a lot of energy. It is used in the inns and taverns to prepare lunch for the miners and is consumed at the training sites (arena, colosseum, training camp). There are two sources of meat: pork from piggeries and the meat of wild game hunted by the hunters. ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 6 },
      },
   }
}
