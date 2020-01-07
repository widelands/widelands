-- RST
-- .. _lua_world_terrains:
--
-- Terrains
-- --------
--
-- Terrains define the basic look of the map, and all terrains are defined in ``data/world/terrains/init.lua``.
--
-- .. code-block:: none
--
--         *-------*
--        / \     / \
--       /   \   /   \
--      /     \ /     \
--     *-------*-------*
--      \     / \     /
--       \   /   \   /
--        \ /     \ /
--         *-------*
--
-- Terrain tiles have a triangular shape, and 6 of them will be combined to form a hexagon. Each vertex between the terrains (* in the figure) will form a node that is influenced by the 6 terrains surrounding it, and where other map entities can be placed. You can find more information on the terrains' shape and on how to create textures on the `wiki <https://www.widelands.org/wiki/HelpTerrains/>`_.
--
-- Each terrain tile will also influence some properties for the map entities that are placed on its 3 vertices, like:
--
-- * Which resources can go on a map node.
-- * How well which type of tree will grow on a map node.
-- * Which map objects can be built on the nodes or move through them.

pics_dir = path.dirname(__file__) .. "pics/"

-- RST
-- .. function:: new_terrain_type{table}
--
--    This function adds the definition of a terrain to the engine.
--
--    :arg table: This table contains all the data that the game engine will add
--       to this terrain. It contains the following entries:
--
--    **name**
--        *Mandatory*. A string containing the internal name of this terrain, e.g.::
--
--            name = "summer_meadow1",
--
--    **descname**
--        *Mandatory*. The translatable display name, e.g.::
--
--            descname = _"Meadow 1",
--
--    **editor_category**
--        *Mandatory*. The category that is used in the editor tools for placing a
--        terrain of this type on the map, e.g.::
--
--            editor_category = "summer",
--
--    **is**
--        *Mandatory*. The type of this terrain, which determines if the nodes
--        surrounding the terrain will be walkable or navigable, if mines or buildings
--        can be built on them, if flags can be built on them, and so on.
--        The following properties are available:
--
--         * ``arable``: Allows building of normal buildings and roads.
--         * ``mineable``: Allows building of mines and roads.
--         * ``walkable``: Allows building of flags and roads only.
--         * ``water``: Nothing can be built here, but ships and aquatic animals can pass.
--         * ``unreachable``: Nothing can be built here, and nothing can walk on it,
--           and nothing will grow.
--         * ``unwalkable``: Nothing can be built here, and nothing can walk on it.
--
--        Example::
--
--           is = "arable",
--
--        *Note: There is currently some interdependency between ``is`` and
--        ``valid_resources``, so not all combinations are possible. See*
--        `Issue #2038 <https://github.com/widelands/widelands/issues/2038>`_
--        *for more information.*
--
--    **tooltips**
--        *Optional*. Additional custom tooltip entries, e.g.::
--
--            tooltips = {
--               _"likes trees",
--            },
--
--    **valid_resources**
--        *Mandatory*. The list of mineable resources that can be found on this terrain.
--        Leave this empty (``{}``) if you want no resources on this terrain. Example::
--
--            valid_resources = {"water"},
--
--        *Note: There is currently some interdependency between ``is`` and
--        ``valid_resources``, so not all combinations are possible. See*
--        `Issue #2038 <https://github.com/widelands/widelands/issues/2038>`_
--        *for more information.*
--
--    **default_resource**
--        *Mandatory*. A resource type that can always be found on this terrain when
--        a new game is started, unless the map maker places some resources there via
--        the editor. Use the empty string
--        (``""``) if you want no default resource. Example::
--
--            default_resource = "water",
--
--    **default_resource_amount**
--        *Mandatory*. The amount of the above default resource that will
--        automatically be placed on this terrain, e.g.::
--
--            default_resource_amount = 10,
--
--    **textures**
--        *Mandatory*. The images used for this terrain. Examples::
--
--            textures = { pics_dir .. "summer/meadow1_00.png" }, - A static terrain
--            textures = path.list_files(pics_dir .. "summer/lava/lava_??.png"), -- An animated terrain
--
--    **dither_layer**
--        *Mandatory*. Terrains will be blended slightly over each other in order
--        to hide the harsh edges of the triangles. This describes the
--        `z layer <https://en.wikipedia.org/wiki/Z-order>`_ of a terrain when
--        rendered next to another terrain. Terrains with a higher value will be
--        dithered on top of terrains with a lower value. Example::
--
--            dither_layer = 340,
--
--    **temperature**
--        *Mandatory*. A terrain affinity constant. These are used to model how well
--        trees will grow on this terrain. Temperature is in arbitrary units. Example::
--
--            temperature = 100,
--
--    **humidity**
--        *Mandatory*. A terrain affinity constant. These are used to model how well
--        trees will grow on this terrain. Values range from 1 - 1000 (1000 being very wet).
--        Example::
--
--            humidity = 600,
--
--    **fertility**
--        *Mandatory*. A terrain affinity constant. These are used to model how well
--        trees will grow on this terrain. Values range from 1 - 1000 (1000 being very
--        fertile). Example::
--
--            fertility = 700,
--
--    **enhancement**
--        *Optional*. The terrain this terrain can be turned into by buildings like
--        the amazon gardening center. Example::
--
--            enhancement = "summer_meadow3",
--

------------------------
--  Former greenland  --
------------------------

world:new_terrain_type{
   name = "summer_meadow1",
   descname = _ "Meadow 1",
   editor_category = "summer",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },

   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,

   textures = { pics_dir .. "summer/meadow1_00.png" },

   dither_layer = 340,

   temperature = 100,
   humidity = 600,
   fertility = 700,
}


world:new_terrain_type{
   name = "summer_meadow2",
   descname = _ "Meadow 2",
   editor_category = "summer",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "summer/meadow2_00.png" },
   dither_layer = 350,
   temperature = 100,
   humidity = 600,
   fertility = 650,
}


world:new_terrain_type{
   name = "summer_meadow3",
   descname = _ "Meadow 3",
   editor_category = "summer",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "summer/meadow3_00.png" },
   dither_layer = 350,
   temperature = 105,
   humidity = 550,
   fertility = 800,
}


world:new_terrain_type{
   name = "summer_meadow4",
   descname = _ "Meadow 4",
   editor_category = "summer",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "summer/meadow4_00.png" },
   dither_layer = 350,
   temperature = 110,
   humidity = 650,
   fertility = 750,
}


world:new_terrain_type{
   name = "summer_steppe",
   descname = _ "Steppe",
   editor_category = "summer",
   is = "arable",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "summer/steppe_00.png" },
   dither_layer = 330,
   temperature = 100,
   humidity = 400,
   fertility = 400,

   enhancement = "summer_mountain_meadow"
}


world:new_terrain_type{
   name = "summer_steppe_barren",
   descname = _ "Barren Steppe",
   editor_category = "summer",
   is = "arable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "summer/steppe_barren_00.png" },
   dither_layer = 320,
   temperature = 100,
   humidity = 150,
   fertility = 150,

   enhancement = "summer_steppe"
}


world:new_terrain_type{
   name = "summer_mountain_meadow",
   descname = _ "Mountain Meadow",
   editor_category = "summer",
   is = "arable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/mountain_meadow_00.png" },
   dither_layer = 160,
   temperature = 75,
   humidity = 800,
   fertility = 450,

   enhancement = "summer_meadow1"
}

world:new_terrain_type{
   name = "summer_forested_mountain1",
   descname = _ "Forested Mountain 1",
   editor_category = "summer",
   is = "mineable",
   -- You can add custom additional tooltip entries here.
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/forested_mountain1_00.png" },
   dither_layer = 71,
   temperature = 50,
   humidity = 750,
   fertility = 500,
}

world:new_terrain_type{
   name = "summer_forested_mountain2",
   descname = _ "Forested Mountain 2",
   editor_category = "summer",
   is = "mineable",
   -- You can add custom additional tooltip entries here.
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/forested_mountain2_00.png" },
   dither_layer = 71,
   temperature = 50,
   humidity = 750,
   fertility = 500,
}

world:new_terrain_type{
   name = "summer_mountain1",
   descname = _ "Mountain 1",
   editor_category = "summer",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/mountain1_00.png" },
   dither_layer = 70,
   temperature = 80,
   humidity = 100,
   fertility = 100,

   enhancement = "summer_forested_mountain1"
}


world:new_terrain_type{
   name = "summer_mountain2",
   descname = _ "Mountain 2",
   editor_category = "summer",
   is = "mineable",
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/mountain2_00.png" },
   dither_layer = 70,
   temperature = 80,
   humidity = 100,
   fertility = 100,

   enhancement = "summer_forested_mountain1"
}


world:new_terrain_type{
   name = "summer_mountain3",
   descname = _ "Mountain 3",
   editor_category = "summer",
   is = "mineable",
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/mountain3_00.png" },
   dither_layer = 70,
   temperature = 80,
   humidity = 100,
   fertility = 100,

   enhancement = "summer_forested_mountain2"
}


world:new_terrain_type{
   name = "summer_mountain4",
   descname = _ "Mountain 4",
   editor_category = "summer",
   is = "mineable",
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/mountain4_00.png" },
   dither_layer = 70,
   temperature = 80,
   humidity = 100,
   fertility = 100,

   enhancement = "summer_forested_mountain2"
}

world:new_terrain_type{
   name = "summer_beach",
   descname = _ "Beach",
   editor_category = "summer",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/beach_00.png" },
   dither_layer = 60,
   temperature = 120,
   humidity = 600,
   fertility = 200,
}

world:new_terrain_type{
   name = "summer_swamp",
   descname = _ "Swamp",
   editor_category = "summer",
   is = "unwalkable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 20,
   textures = path.list_files(pics_dir .. "summer/swamp/swamp_??.png"),
   dither_layer = 370,
   fps = 14,
   temperature = 105,
   humidity = 999,
   fertility = 100,
}
world:new_terrain_type{
   name = "summer_snow",
   descname = _ "Snow",
   editor_category = "summer",
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "summer/snow_00.png" },
   dither_layer = 220,
   temperature = 50,
   humidity = 999,
   fertility = 1,
}


world:new_terrain_type{
   name = "lava",
   descname = _ "Lava",
   editor_category = "summer",
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(pics_dir .. "summer/lava/lava_??.png"),
   dither_layer = 30,
   fps = 4,
   temperature = 1273,
   humidity = 1,
   fertility = 1,
}


world:new_terrain_type{
   name = "summer_water",
   descname = _ "Water",
   editor_category = "summer",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_files(pics_dir .. "summer/water/water_??.png"),
   dither_layer = 180,
   fps = 14,
   temperature = 100,
   humidity = 999,
   fertility = 1,
}

------------------------
--  Former blackland  --
------------------------


world:new_terrain_type{
   name = "ashes",
   descname = _ "Ashes 1",
   editor_category = "wasteland",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "wasteland/ashes_00.png" },
   dither_layer = 400,
   temperature = 120,
   humidity = 150,
   fertility = 900,

   enhancement = "hardground3"
}


world:new_terrain_type{
   name = "ashes2",
   descname = _ "Ashes 2",
   editor_category = "wasteland",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "wasteland/ashes2_00.png" },
   dither_layer = 410,
   temperature = 118,
   humidity = 130,
   fertility = 999,

   enhancement = "hardground1"
}


world:new_terrain_type{
   name = "hardground1",
   descname = _ "Hard Ground 1",
   editor_category = "wasteland",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground1_00.png" },
   dither_layer = 420,
   temperature = 100,
   humidity = 250,
   fertility = 800,
}


world:new_terrain_type{
   name = "hardground2",
   descname = _ "Hard Ground 2",
   editor_category = "wasteland",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground2_00.png" },
   dither_layer = 370,
   temperature = 95,
   humidity = 150,
   fertility = 850,
}


world:new_terrain_type{
   name = "hardground3",
   descname = _ "Hard Ground 3",
   editor_category = "wasteland",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground3_00.png" },
   dither_layer = 380,
   temperature = 105,
   humidity = 200,
   fertility = 900,
}


world:new_terrain_type{
   name = "hardground4",
   descname = _ "Hard Ground 4",
   editor_category = "wasteland",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "wasteland/hardground4_00.png" },
   dither_layer = 390,
   temperature = 90,
   humidity = 200,
   fertility = 800,
}


world:new_terrain_type{
   name = "hardlava",
   descname = _ "Igneous Rocks",
   editor_category = "wasteland",
   is = "arable",
   valid_resources = { "water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/hardlava_00.png" },
   dither_layer = 360,
   temperature = 120,
   humidity = 100,
   fertility = 200,

   enhancement = "drysoil"
}


world:new_terrain_type{
   name = "wasteland_forested_mountain1",
   descname = _ "Forested Mountain 1",
   editor_category = "wasteland",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/forested_mountain1_00.png" },
   dither_layer = 81,
   temperature = 110,
   humidity = 150,
   fertility = 950,
}

world:new_terrain_type{
   name = "wasteland_forested_mountain2",
   descname = _ "Forested Mountain 2",
   editor_category = "wasteland",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/forested_mountain2_00.png" },
   dither_layer = 81,
   temperature = 95,
   humidity = 200,
   fertility = 400,
}

world:new_terrain_type{
   name = "wasteland_mountain1",
   descname = _ "Mountain 1",
   editor_category = "wasteland",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain1_00.png" },
   dither_layer = 90,
   temperature = 80,
   humidity = 50,
   fertility = 200,

   enhancement = "wasteland_forested_mountain1"
}


world:new_terrain_type{
   name = "wasteland_mountain2",
   descname = _ "Mountain 2",
   editor_category = "wasteland",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain2_00.png" },
   dither_layer = 90,
   temperature = 80,
   humidity = 50,
   fertility = 200,

   enhancement = "wasteland_forested_mountain1"
}


world:new_terrain_type{
   name = "wasteland_mountain3",
   descname = _ "Mountain 3",
   editor_category = "wasteland",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain3_00.png" },
   dither_layer = 90,
   temperature = 80,
   humidity = 50,
   fertility = 200,

   enhancement = "wasteland_forested_mountain2"
}


world:new_terrain_type{
   name = "wasteland_mountain4",
   descname = _ "Mountain 4",
   editor_category = "wasteland",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/mountain4_00.png" },
   dither_layer = 80,
   temperature = 80,
   humidity = 50,
   fertility = 200,

   enhancement = "wasteland_forested_mountain2"
}


world:new_terrain_type{
   name = "wasteland_beach",
   descname = _ "Beach",
   editor_category = "wasteland",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "wasteland/beach_00.png" },
   dither_layer = 50,
   temperature = 60,
   humidity = 400,
   fertility = 200,
}


world:new_terrain_type{
   name = "lava-stone1",
   descname = _ "Lava Rocks 1",
   editor_category = "wasteland",
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(pics_dir .. "wasteland/lava_stone1/lava-stone1_??.png"),
   dither_layer = 20,
   fps = 7,
   temperature = 1273,
   humidity = 1,
   fertility = 1,
}


world:new_terrain_type{
   name = "lava-stone2",
   descname = _ "Lava Rocks 2",
   editor_category = "wasteland",
   is = "unreachable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(pics_dir .. "wasteland/lava_stone2/lava-stone2_??.png"),
   dither_layer = 10,
   fps = 7,
   temperature = 1273,
   humidity = 1,
   fertility = 1,
}


world:new_terrain_type{
   name = "wasteland_water",
   descname = _ "Water",
   editor_category = "wasteland",
   is = "water",
   valid_resources = { "fish" },
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_files(pics_dir .. "wasteland/water/water_??.png"),
   dither_layer = 170,
   fps = 14,
   temperature = 100,
   humidity = 999,
   fertility = 1,
}


-------------------------
--  Former Winterland  --
-------------------------


world:new_terrain_type{
   name = "tundra",
   descname = _ "Tundra 1",
   editor_category = "winter",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra_00.png" },
   dither_layer = 230,
   temperature = 50,
   humidity = 850,
   fertility = 450,
}


world:new_terrain_type{
   name = "tundra2",
   descname = _ "Tundra 2",
   editor_category = "winter",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra2_00.png" },
   dither_layer = 240,
   temperature = 55,
   humidity = 750,
   fertility = 450,
}


world:new_terrain_type{
   name = "tundra3",
   descname = _ "Tundra 3",
   editor_category = "winter",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra3_00.png" },
   dither_layer = 240,
   temperature = 50,
   humidity = 800,
   fertility = 400,
}


world:new_terrain_type{
   name = "tundra_taiga",
   descname = _ "Tundra Taiga",
   editor_category = "winter",
   is = "arable",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/tundra_taiga_00.png" },
   dither_layer = 230,
   temperature = 40,
   humidity = 750,
   fertility = 400,

   enhancement = "tundra2"
}


world:new_terrain_type{
   name = "taiga",
   descname = _ "Taiga",
   editor_category = "winter",
   is = "arable",
   valid_resources = { "water" },
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "winter/taiga_00.png" },
   dither_layer = 250,
   temperature = 35,
   humidity = 750,
   fertility = 300,

   enhancement = "tundra_taiga"
}


world:new_terrain_type{
   name = "snow",
   descname = _ "Snow",
   editor_category = "winter",
   is = "arable",
   valid_resources = { "water" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/snow_00.png" },
   dither_layer = 250,
   temperature = 25,
   humidity = 800,
   fertility = 100,

   enhancement = "taiga"
}


world:new_terrain_type{
   name = "winter_forested_mountain1",
   descname = _ "Forested Mountain 1",
   editor_category = "winter",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/forested_mountain1_00.png" },
   dither_layer = 101,
   temperature = 35,
   humidity = 700,
   fertility = 400,
}

world:new_terrain_type{
   name = "winter_forested_mountain2",
   descname = _ "Forested Mountain 2",
   editor_category = "winter",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/forested_mountain2_00.png" },
   dither_layer = 101,
   temperature = 35,
   humidity = 700,
   fertility = 400,
}

world:new_terrain_type{
   name = "winter_mountain1",
   descname = _ "Mountain 1",
   editor_category = "winter",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain1_00.png" },
   dither_layer = 110,
   temperature = 20,
   humidity = 300,
   fertility = 50,

   enhancement = "winter_forested_mountain1"
}


world:new_terrain_type{
   name = "winter_mountain2",
   descname = _ "Mountain 2",
   editor_category = "winter",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain2_00.png" },
   dither_layer = 110,
   temperature = 20,
   humidity = 300,
   fertility = 50,

   enhancement = "winter_forested_mountain1"
}


world:new_terrain_type{
   name = "winter_mountain3",
   descname = _ "Mountain 3",
   editor_category = "winter",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain3_00.png" },
   dither_layer = 100,
   temperature = 20,
   humidity = 300,
   fertility = 50,

   enhancement = "winter_forested_mountain2"
}


world:new_terrain_type{
   name = "winter_mountain4",
   descname = _ "Mountain 4",
   editor_category = "winter",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/mountain4_00.png" },
   dither_layer = 100,
   temperature = 20,
   humidity = 300,
   fertility = 50,

   enhancement = "winter_forested_mountain2"
}
world:new_terrain_type{
   name = "ice",
   descname = _ "Ice",
   editor_category = "winter",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/ice_00.png" },
   dither_layer = 260,
   temperature = 25,
   humidity = 500,
   fertility = 100,
}


world:new_terrain_type{
   name = "winter_beach",
   descname = _ "Beach",
   editor_category = "winter",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "winter/beach_00.png" },
   dither_layer = 40,
   temperature = 60,
   humidity = 500,
   fertility = 100,
}


world:new_terrain_type{
   name = "ice_floes",
   descname = _ "Ice Floes 1",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(pics_dir .. "winter/ice_floes1/ice_floes_??.png"),
   dither_layer = 210,
   fps = 5,
   temperature = 50,
   humidity = 999,
   fertility = 1,
}


world:new_terrain_type{
   name = "ice_floes2",
   descname = _ "Ice Floes 2",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "",
   default_resource_amount = 0,
   textures = path.list_files(pics_dir .. "winter/ice_floes2/ice_floes2_??.png"),
   dither_layer = 210,
   fps = 5,
   temperature = 50,
   humidity = 999,
   fertility = 1,
}


world:new_terrain_type{
   name = "winter_water",
   descname = _ "Water",
   editor_category = "winter",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_files(pics_dir .. "winter/water/water_??.png"),
   dither_layer = 190,
   fps = 8,
   temperature = 50,
   humidity = 999,
   fertility = 1,
}


---------------------
--  Former Desert  --
---------------------

world:new_terrain_type{
   name = "desert4",
   descname = _ "Desert 4",
   editor_category = "desert",
   is = "arable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 2,
   textures = { pics_dir .. "desert/desert4_00.png" },
   dither_layer = 270,
   temperature = 168,
   humidity = 1,
   fertility = 100,

   enhancement = "drysoil"
}

world:new_terrain_type{
   name = "drysoil",
   descname = _ "Dry Soil",
   editor_category = "desert",
   is = "arable",
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 4,
   textures = { pics_dir .. "desert/drysoil_00.png" },
   dither_layer = 300,
   temperature = 172,
   humidity = 200,
   fertility = 200,

   enhancement = "highmountainmeadow"
}
world:new_terrain_type{
   name = "desert_steppe",
   descname = _ "Steppe",
   editor_category = "desert",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 5,
   textures = { pics_dir .. "desert/steppe_00.png" },
   dither_layer = 360,
   temperature = 155,
   humidity = 500,
   fertility = 500,
}


world:new_terrain_type{
   name = "meadow",
   descname = _ "Meadow",
   editor_category = "desert",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "water",
   default_resource_amount = 10,
   textures = { pics_dir .. "desert/meadow_00.png" },
   dither_layer = 310,
   temperature = 160,
   humidity = 600,
   fertility = 600,
}


world:new_terrain_type{
   name = "mountainmeadow",
   descname = _ "Mountain Meadow",
   editor_category = "desert",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountainmeadow_00.png" },
   dither_layer = 150,
   temperature = 145,
   humidity = 500,
   fertility = 500,

   enhancement = "desert_steppe"
}


world:new_terrain_type{
   name = "highmountainmeadow",
   descname = _ "High Mountain Meadow",
   editor_category = "desert",
   is = "arable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"water"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/highmountainmeadow_00.png" },
   dither_layer = 150,
   temperature = 140,
   humidity = 400,
   fertility = 400,

   enhancement = "mountainmeadow"
}


world:new_terrain_type{
   name = "desert_forested_mountain1",
   descname = _ "Forested Mountain 1",
   editor_category = "desert",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/forested_mountain1_00.png" },
   dither_layer = 71,
   temperature = 141,
   humidity = 500,
   fertility = 500,
}

world:new_terrain_type{
   name = "desert_forested_mountain2",
   descname = _ "Forested Mountain 2",
   editor_category = "desert",
   is = "mineable",
   tooltips = {
      -- TRANSLATORS: This is an entry in a terrain tooltip. Try to use 1 word if possible.
      _"likes trees",
   },
   valid_resources = {"coal", "iron", "gold", "stones"},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/forested_mountain2_00.png" },
   dither_layer = 141,
   temperature = 120,
   humidity = 500,
   fertility = 500,
}


world:new_terrain_type{
   name = "mountain1",
   descname = _ "Mountain 1",
   editor_category = "desert",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain1_00.png" },
   dither_layer = 120,
   temperature = 130,
   humidity = 50,
   fertility = 50,

   enhancement = "desert_forested_mountain1"
}


world:new_terrain_type{
   name = "mountain2",
   descname = _ "Mountain 2",
   editor_category = "desert",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain2_00.png" },
   dither_layer = 120,
   temperature = 130,
   humidity = 50,
   fertility = 50,

   enhancement = "desert_forested_mountain1"
}


world:new_terrain_type{
   name = "mountain3",
   descname = _ "Mountain 3",
   editor_category = "desert",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain3_00.png" },
   dither_layer = 130,
   temperature = 130,
   humidity = 50,
   fertility = 50,

   enhancement = "desert_forested_mountain2"
}


world:new_terrain_type{
   name = "mountain4",
   descname = _ "Mountain 4",
   editor_category = "desert",
   is = "mineable",
   valid_resources = { "coal", "iron", "gold", "stones" },
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/mountain4_00.png" },
   dither_layer = 140,
   temperature = 130,
   humidity = 50,
   fertility = 50,

   enhancement = "desert_forested_mountain2"
}
world:new_terrain_type{
   name = "desert1",
   descname = _ "Desert 1",
   editor_category = "desert",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert1_00.png" },
   dither_layer = 290,
   temperature = 167,
   humidity = 1,
   fertility = 1,
}


world:new_terrain_type{
   name = "desert2",
   descname = _ "Desert 2",
   editor_category = "desert",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert2_00.png" },
   dither_layer = 280,
   temperature = 168,
   humidity = 1,
   fertility = 1,
}


world:new_terrain_type{
   name = "desert3",
   descname = _ "Desert 3",
   editor_category = "desert",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/desert3_00.png" },
   dither_layer = 280,
   temperature = 178,
   humidity = 1,
   fertility = 1,
}


world:new_terrain_type{
   name = "desert_beach",
   descname = _ "Beach",
   editor_category = "desert",
   is = "walkable",
   valid_resources = {},
   default_resource = "",
   default_resource_amount = 0,
   textures = { pics_dir .. "desert/beach_00.png" },
   dither_layer = 60,
   temperature = 179,
   humidity = 500,
   fertility = 100,
}


world:new_terrain_type{
   name = "desert_water",
   descname = _ "Water",
   editor_category = "desert",
   is = "water",
   valid_resources = {"fish"},
   default_resource = "fish",
   default_resource_amount = 4,
   textures = path.list_files(pics_dir .. "desert/water/water_??.png"),
   dither_layer = 200,
   fps = 5,
   temperature = 150,
   humidity = 999,
   fertility = 1,
}
