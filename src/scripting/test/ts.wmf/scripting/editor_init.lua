use("map", "common_init")

include "test_math_random"
include "test_map"
include "test_field"
include "test_immovables"
include "test_player"
include "test_table"

rv = lunit:run()
if rv == 0 then -- No errors in the testsuite. Exit the editor
   wl.debug.exit()
end
