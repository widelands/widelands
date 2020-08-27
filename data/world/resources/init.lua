-- RST
-- .. _lua_world_resources:
--
-- Resources
-- ---------
--
-- Resources are mineable map resources.
-- All resources are defined in ``data/world/resources/init.lua``.
--
-- * **Fish** can be placed in water terrain only and be fished by a fisher.
-- * **Water** can be placed on any non-mountain terrain and be pulled up by a well.
-- * **Stones**, **Coal**, **Iron** and **Gold** are placed on mountain terrain
--   and can be dug up by mines.
--
-- Which resource can be placed where on the map is defined in each terrain's
-- ``valid_resources`` table.

push_textdomain("world")

pics_dir = path.dirname(__file__) .. "pics/"

-- RST
-- .. function:: new_resource_type{table}
--
--    This function adds the definition of a resource to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--       to this resource. It contains the following entries:
--
--    **name**
--        *Mandatory*. A string containing the internal name of this resource, e.g.::
--
--            name = "coal",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--            descname = _"Coal",
--
--    **max_amount**
--        *Mandatory*. The maximum possible amount of this map resource that can
--        be placed on a map tile, e.g.::
--
--            max_amount = 20,
--
--    **detectable**
--        *Mandatory*. Set this to ``true`` if a geologist can find it (water or
--        mountain resources), and to ``false`` otherwise (fish), e.g.::
--
--            detectable = true,
--
--    **timeout_ms**
--        *Mandatory*. Defines the time for which geologists messages for this
--        resource will be muted within this area after a find, e.g.::
--
--            timeout_ms = 300000,
--
--    **timeout_radius**
--        *Mandatory*. Defines the radius within which geologists messages for this
--        resource will be muted after a find, e.g.::
--
--            timeout_radius = 8,
--
--    **representative_image**
--        *Mandatory*. Path to an image file that will represent the resource in menus
--        etc., e.g.::
--
--            representative_image = pics_dir .. "coal4.png",
--
--    **editor_pictures**
--        *Mandatory*. A table of pictures that will indicate the resource amount
--        on the map, for use in the editor, e.g.::
--
--            editor_pictures = {
--               [5] = pics_dir .. "coal1.png",    -- Use this image for amount 0-5;
--               [10] = pics_dir .. "coal2.png",   -- Use this image for amount 6-10;
--               [15] = pics_dir .. "coal3.png",   -- Use this image for amount 11-15;
--               [1000] = pics_dir .. "coal4.png", -- Use this image for amount > 15;
--            }
--
wl.World():new_resource_type{
   name = "resource_coal",
   descname = _ "Coal",
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "coal4.png",
   editor_pictures = {
      [5] = pics_dir .. "coal1.png",
      [10] = pics_dir .. "coal2.png",
      [15] = pics_dir .. "coal3.png",
      [1000] = pics_dir .. "coal4.png",
   }
}

wl.World():new_resource_type{
   name = "resource_gold",
   descname = _ "Gold",
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "gold4.png",
   editor_pictures = {
      [5] = pics_dir .. "gold1.png",
      [10] = pics_dir .. "gold2.png",
      [15] = pics_dir .. "gold3.png",
      [1000] = pics_dir .. "gold4.png",
   }
}

wl.World():new_resource_type{
   name = "resource_iron",
   descname = _ "Iron",
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "iron4.png",
   editor_pictures = {
      [5] = pics_dir .. "iron1.png",
      [10] = pics_dir .. "iron2.png",
      [15] = pics_dir .. "iron3.png",
      [1000] = pics_dir .. "iron4.png",
   }
}

wl.World():new_resource_type{
   name = "resource_stones",
   descname = _ "Stones",
   max_amount = 20,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "stones4.png",
   editor_pictures = {
      [5] = pics_dir .. "stones1.png",
      [10] = pics_dir .. "stones2.png",
      [15] = pics_dir .. "stones3.png",
      [1000] = pics_dir .. "stones4.png",
   }
}

wl.World():new_resource_type{
   name = "resource_water",
   descname = _ "Water",
   max_amount = 50,
   detectable = true,
   timeout_ms = 300000,
   timeout_radius = 8,
   representative_image = pics_dir .. "water4.png",
   editor_pictures = {
      [10] = pics_dir .."water1.png",
      [20] = pics_dir .."water2.png",
      [30] = pics_dir .."water3.png",
      [1000] = pics_dir .. "water4.png",
   }
}

wl.World():new_resource_type{
   name = "resource_fish",
   descname = _ "Fish",
   max_amount = 20,
   detectable = false,
   timeout_ms = 0,
   timeout_radius = 0,
   representative_image = pics_dir .. "fish.png",
   editor_pictures = {
      [5] = pics_dir .. "fish1.png",
      [10] = pics_dir .. "fish2.png",
      [15] = pics_dir .. "fish3.png",
      [1000] = pics_dir .. "fish4.png",
   }
}

pop_textdomain()
