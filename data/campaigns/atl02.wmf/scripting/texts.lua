-- =======================================================================
--                 Texts for the Atlantean Mission 2
-- =======================================================================

include "scripting/richtext_scenarios.lua"

function jundlina(title, text)
   return speech("map:princess.png", "2F9131", title, text)
end
function loftomor(text)
   return speech("map:loftomor.png", "FDD53D", "Loftomor", text)
end
function sidolus(text)
   return speech("map:sidolus.png", "FF1A30", "Sidolus", text)
end
function colionder(text)
   return speech("map:colionder.png", "33A9FD", "Colionder", text)
end
function opol(text)
   return speech("map:opol.png", "FF3FFB", "Opol", text)
end
function ostur(text)
   return speech("map:ostur.png", "375FFC", "Ostur", text)
end

-- Append an objective text to a dialog box in a nice fashion.
function new_objectives(...)
   local s = rt(
   "<p font=FreeSerif font-size=18 font-weight=bold font-color=D1D1D1>"
   .. _"New Objectives" .. "</p>")

   for idx,obj in ipairs{...} do
      s = s .. obj.body
   end
   return s
end

-- =======================================================================
--                                Objectives
-- =======================================================================
-- obj_ensure_build_wares_production = {
   -- name = "obj_ensure_build_wares_production",
   -- title = _ "Ensure the supply of build wares",
   -- body = objective_text(_"The supply of build wares", _
-- [[Build a quarry, two woodcutter's houses, two forester's houses and a
-- sawmill.]]
   -- ),
-- }
obj_basic_infrastructure = {
   name = "obj_basic_infrastructure",
   title = _ "Ensure the supply of build wares",
   body = objective_text(_"Establish a basic production", _
[[You will need at least a quarry, a few lumberjacks and a sawmill.
A forester might or might not be needed depending on how fast the trees
are growing around you. Find the balance.]]
   ),
}
obj_tools = {
   name = "obj_tools",
   title = _ "Build tools quickly",
   body = objective_text(_"Make new tools", _
[[Most of your tools have been lost in the ship tragedy. Your people will
not be able to fulfill their duties without proper tools. Luckily you still
have some iron ore and plenty of coal. Build a production of tools, it is
a prerequisite for success on island.]]
   ),
}

obj_explore = {
   name = "obj_explore",
   title = _"Explore the surroundings",
   body = objective_text(_"Explore your surroundings", _
[[We are pretty sure that we are not alone on the island. We do not know
if the others on this island are friends or foes so we need to treat carefully.
Expand and explore the island by building at least one habitat for a scout
and military buildings. Favor Towers over other military buildings for their
bigger view range.]]
   ),
}

-- =======================================================================
--                                  Texts
-- =======================================================================

-- This is an array or message descriptions
initial_messages = {
{
   title = _ "The princess' memoir",
   body = jundlina( _"Jundlina writes her diary", _
[[Our escape from the cursed island was a close one: we managed to load three
ships and make our escape, but the currents of the sinking island tested our
new ships and their design. And one failed the test: the last days of loading was 
done in chaos and without much planing and so one of ships ended up carrying
most of the heavy wares in our possession. This plus the current was too much for
the ship and it was pulled below the sea level by Lutas and his currents and we lost
most men and all wares it carried.]]
   .. "<br><br>" .. _
[[The fear was great in us all. We expected the other ships to be drowned as well, but
they persisted. We escaped the island and began our smooth sailing to other costs.
Many weeks have passed and we did only see small islands not suitable for habitation and
without any sign of being blessed by Satul, the fire god. Last night however, we made
out a new island. We saw smoke hovering over it. The black comes from the fiery mountain
in the north of the island, the grey however comes from settlers.]]
   )
},
{
   title = _ "Planing for the future",
   body = jundlina(_"Jundlina continues", _
[[At least the east side is inhabited, so much is clear. We do not know if the people
there are friends or foes. We do not even know what tribe they might be from. The only
thing that is sure that they can't be Atlantians for no ship of us has sailed as far
as us in the last hundreds years. I can only hope they are friendly. We have not much water
left, our food is running low, we lost all our tools and most of our building materials
with the third ship. We can't afford waging war until we rooted ourselves here.]]
)
},
{
   title = _ "Jundlina gives orders",
   body = jundlina(_ "Jundlina decides", _
[[We still have plenty of coal and iron ore, so a metal workshop and a smelter will
provide us with good tools in no time. Also, we need to solve our building material
problem. The island is full of trees, we might not even need foresters. But we need
stone, that is for sure]]
   .. "<br><br>" .. _
[[Let's also explore this island. But we need to be careful. We should prefer towers over
other military buildings so that we can see potential enemies before they see us. We will
be careful with the stones though. When we no longer need a building, we will dismantle it instead
of burning it down. This will take more time, but we can reuse some of its materials. Also, 
I want a scout out and exploring at all times.
]]) .. new_objectives(obj_tools,obj_basic_infrastructure,obj_explore)
},
} -- end of initial messages.



