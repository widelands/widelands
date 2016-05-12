include "scripting/formatting.lua"

won_game = {
  title = _"Congratulations!",
  body = rt(p(_"You have won this game!"))
}

lost_game = {
  title = _"You are defeated!",
  body = rt(p(_"You lost your last warehouse and are therefore defeated. You may continue as spectator if you want."))
}

won_game_over = {
  title = _"You won",
  body = rt(p(_"You are the winner!"))
}

lost_game_over = {
  title = _"You lost",
  body = rt(p(_"You’ve lost this game!"))
}

game_status = {
  title = _"Status",
  body = rt(p(_"Player overview:"))
}
