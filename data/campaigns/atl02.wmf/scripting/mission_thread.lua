-- =======================================================================
--                      Atlanteans Tutorial Mission 02
-- =======================================================================

-- ==============
-- Logic Threads
-- ==============
function intro()
   sleep(500)

   p1.see_all = true -- TODO: remove this
   include "map:scripting/starting_conditions.lua"

   local o1 = add_campaign_objective(obj_basic_infrastructure)
   local o2 = add_campaign_objective(obj_tools)
   local o3 = add_campaign_objective(obj_explore)
   msg_boxes(initial_messages)


end

run(intro)

