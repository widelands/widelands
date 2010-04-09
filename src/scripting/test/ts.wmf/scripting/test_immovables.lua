include "test_baseimmovables"

include "test_flag"
include "test_road"
   
include "test_warehouse"

-- TODO: those should also work in the editor
if not wl.editor then
   include "test_productionsite"
   include "test_militarysite"
   include "test_trainingsite"
end
