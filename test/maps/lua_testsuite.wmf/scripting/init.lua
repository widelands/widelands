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

-- =================================
-- Tests for the core functionality
-- =================================
include "map:scripting/egbase.lua"

include "map:scripting/math_random.lua"
include "map:scripting/string_bformat.lua"
include "map:scripting/map.lua"
include "map:scripting/cplayer.lua"
include "map:scripting/cfield.lua"
include "map:scripting/cplr_access.lua"

include "map:scripting/immovables.lua"

if not wl.editor then
   include "map:scripting/game.lua"

   include "map:scripting/gplayer.lua"
   include "map:scripting/gfield.lua"
   include "map:scripting/gplr_access.lua"

   include "map:scripting/objectives.lua"
   include "map:scripting/messages.lua"

   -- The functionality of UI scripting is identical but we only test it in game
   -- though as it is cumbersome to make those tests work also in the editor.
   include "map:scripting/ui.lua"
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
wl.ui.MapView():close()
