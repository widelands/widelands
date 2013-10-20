use("map", "common_init")

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
include "test_egbase"

include "test_math_random"
include "test_string_bformat"
include "test_map"
include "test_cplayer"
include "test_cfield"
include "test_cplr_access"

include "test_immovables"

if not wl.editor then
   include "test_game"

   include "test_gplayer"
   include "test_gfield"
   include "test_gplr_access"

   include "test_objectives"
   include "test_messages"

   -- The functionality of UI scripting is identical but we only test it in game
   -- though as it is cumbersome to make those tests work also in the editor.
   include "test_ui"
else
   include "test_editor"
   include "test_eplr_access"

   include "test_efield"
end

-- ===========================
-- Test for auxiliary scripts
-- ===========================
include "test_table"
include "test_set"

-- ============
-- Test Runner
-- ============
lunit:run()
wl.ui.MapView():close()
