include "map:scripting/common_init.lua"

-- ==========
-- Constants
-- ==========
if not wl.editor then
   game = wl.Game()
   -- make sure we do not crash because of saving.
   game.allow_saving = false
   egbase = game
else
   editor = wl.Editor()
   egbase = editor
end
map = egbase.map

player1 = egbase.players[1]
player2 = egbase.players[2]
player3 = egbase.players[3]

-- TODO(GunChleoc): Editor does not conquer properly
player1:place_building("barbarians_headquarters", map.player_slots[1].starting_field, false, true)

-- For immovables_descriptions.lua
local field = map:get_field(63,79)
if (field.immovable == nil) then
   map:place_immovable("bush1", field)
end
field = map:get_field(62,79)
if (field.immovable == nil) then
   map:place_immovable("alder_summer_sapling", field)
end
field = map:get_field(61,79)
if (field.immovable == nil) then
   map:place_immovable("alder_summer_old", field)
end

-- =================================
-- Tests for the core functionality
-- =================================
include "map:scripting/egbase.lua"

include "map:scripting/gettext.lua"
include "map:scripting/math_random.lua"
include "map:scripting/string_bformat.lua"
include "map:scripting/path.lua"
include "map:scripting/map.lua"
include "map:scripting/cplayer.lua"
include "map:scripting/cfield.lua"
include "map:scripting/cplr_access.lua"

include "map:scripting/immovables.lua"
include "map:scripting/immovables_descriptions.lua"
include "map:scripting/terrains_resources_descriptions.lua"
include "map:scripting/tribes_descriptions.lua"

if not wl.editor then
   include "map:scripting/game.lua"

   include "map:scripting/geconomy.lua"
   include "map:scripting/gplayer.lua"
   include "map:scripting/gfield.lua"
   include "map:scripting/gplr_access.lua"

   include "map:scripting/objectives.lua"
   include "map:scripting/messages.lua"
else
   include "map:scripting/editor.lua"
   include "map:scripting/eplr_access.lua"

   include "map:scripting/efield.lua"
end

-- ===========================
-- Test for auxiliary scripts
-- ===========================
include "map:scripting/table.lua"
include "map:scripting/set.lua"

-- ============
-- Test Runner
-- ============
lunit:run()

include "scripting/coroutine.lua"
if wl.editor then
   wl.ui.MapView():close()
else
   run(function()
      sleep(1000)
      wl.ui.MapView():close()
   end)
end
