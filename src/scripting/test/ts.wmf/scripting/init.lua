use("map", "common_init")

include "test_math_random"
include "test_map"
include "test_field"
include "test_immovables"
include "test_player"

-- TODO: those tests should also be run in the editor
if not wl.editor then
   include "test_ui"
end

include "test_table"


rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit.
   wl.debug.exit()
elseif not wl.editor then
   wl.game.Player(1).see_all = true -- Reveal everything, easier for debugging
end
