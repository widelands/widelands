dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Flour",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"flour",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Flour is produced by the mill out of wheat and is needed in the bakery to produce the tasty Empire bread.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 15 },
      },
   }
}
