include "scripting/richtext.lua"

won_game = {
  title = _"Congratulations!",
  body = p(_"You have won this game!")
}

lost_game = {
  title = _"You are defeated!",
  body = p(_"You lost your last warehouse and are therefore defeated. You may continue as spectator if you want.")
}

won_game_over = {
  title = _"You won",
  body = h2(_"You are the winner!")
}

lost_game_over = {
  title = _"You lost",
  body = h2(_"You’ve lost this game!")
}

game_status = {
  title = _"Status",
  -- TRANSLATORS: This is an overview for all players.
  body = h2(_"Player overview:")
}
