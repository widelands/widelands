function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Hammer
		default = pgettext("ware", "The hammer is an essential tool."),
		-- TRANSLATORS: Helptext for a ware: Hammer
		atlanteans = pgettext("atlanteans_ware", "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Hammer
		barbarians = pgettext("barbarians_ware", "Geologists, builders, blacksmiths and helmsmiths all need a hammer. Make sure you’ve always got some in reserve! They are one of the basic tools produced at the metal workshop (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Hammer
		empire = pgettext("empire_ware", "Geologists, builders, weaponsmiths and armorsmiths all need a hammer. Make sure you’ve always got some in reserve! They are produced by the toolsmith.")
   }
   local result = ""
   if tribe then
		result = helptext[tribe]
	else
		result = helptext["default"]
   end
   if (result == nil) then result = "" end
   return result
end
