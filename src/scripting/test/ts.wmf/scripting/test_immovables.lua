include "test_baseimmovables"

include "test_flag"
include "test_road"
   
include "test_warehouse"
include "test_productionsite"
include "test_militarysite"

-- TODO: those should also work in the editor
if not wl.editor then
   include "test_trainingsite"
end
