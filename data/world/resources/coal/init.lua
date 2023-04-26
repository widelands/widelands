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
--
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
--            descname = _("Coal"),
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
-- For making the UI texts translateable, we also need to push/pop the correct textdomain.
--
-- Example:
--
-- .. code-block:: lua
--
--    push_textdomain("world")
--
--    pics_dir = path.dirname(__file__) .. "pics/"
--
--    wl.Descriptions():new_resource_type{
--       name = "resource_coal",
--       descname = _("Coal"),
--       max_amount = 20,
--       detectable = true,
--       timeout_ms = 300000,
--       timeout_radius = 8,
--       representative_image = pics_dir .. "coal4.png",
--       editor_pictures = {
--          [5] = pics_dir .. "coal1.png",
--          [10] = pics_dir .. "coal2.png",
--          [15] = pics_dir .. "coal3.png",
--          [1000] = pics_dir .. "coal4.png",
--       }
--    }
--
--    pop_textdomain()

push_textdomain("world")

local pics_dir = path.dirname(__file__) .. "../pics/"

wl.Descriptions():new_resource_type{
   name = "resource_coal",
   descname = _("Coal"),
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

pop_textdomain()
