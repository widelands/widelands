-- RST
-- .. _lua_tribes_tribes_military_capacity:
--
-- Military Capacity Strings
-- -------------------------
--
-- The ``tribes/initialization/<tribename>/military_capacity.lua`` file returns translated census strings for military sites.
-- Copy/paste the script from another tribe and replace the `npgettext` message context using the tribe's name.
--

return {
   func = function(present, stationed, capacity)
      push_textdomain("tribes")
      local result = ""
      if present == stationed then
         if capacity > stationed then
            -- TRANSLATORS: %1% is the number of Atlantean soldiers the plural refers to. %2% is the maximum number of soldier slots in the building.
            result = npgettext("atlanteans_building", "%1% soldier (+%2%)", "%1% soldiers (+%2%)", stationed):bformat(stationed, (capacity - stationed))
         else
            -- TRANSLATORS: Number of Atlantean soldiers stationed at a militarysite.
            result = npgettext("atlanteans_building", "%1% soldier", "%1% soldiers", stationed):bformat(stationed)
         end
      else
         if capacity > stationed then
            -- TRANSLATORS: %1% is the number of Atlantean soldiers the plural refers to. %2% are currently open soldier slots in the building. %3% is the maximum number of soldier slots in the building
            result = npgettext("atlanteans_building", "%1%(+%2%) soldier (+%3%)", "%1%(+%2%) soldiers (+%3%)", stationed):bformat(present, (stationed - present), (capacity - stationed))
         else
            -- TRANSLATORS: %1% is the number of Atlantean soldiers the plural refers to. %2% are currently open soldier slots in the building.
            result = npgettext("atlanteans_building", "%1%(+%2%) soldier", "%1%(+%2%) soldiers", stationed):bformat(present, (stationed - present))
         end
      end
      pop_textdomain()
      return result
   end
}
