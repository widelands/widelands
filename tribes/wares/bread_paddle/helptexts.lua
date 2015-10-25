function ware_helptext(tribe)
   local helptext = {
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		default = pgettext("ware", "The bread paddle is the tool of the baker, each baker needs one."),
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		atlanteans = pgettext("atlanteans_ware", "Bread paddles are produced by the toolsmith."),
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		barbarians = pgettext("barbarians_ware", "Bread paddles are produced in the metal workshop like all other tools (but cease to be produced by the building if it is enhanced to an ax workshop and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Bread Paddle
		empire = pgettext("empire_ware", "Bread paddles are produced by the toolsmith.")
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
