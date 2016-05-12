-- This is used by the mapinfo standalone executable to get buildings' helptexts.
-- Note that this can't handle localization properly.

return {
   func = function(buildingname)
      local building_description = wl.Game():get_building_description(buildingname)
      include(building_description.helptext_script)
      return building_helptext_purpose()
   end
}
