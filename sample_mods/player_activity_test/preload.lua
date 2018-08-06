
function radio_exercises(item, active)
  local moves = 60000
  player:assign_activity(activity_id("ACT_RADIO_EXERCISES"), moves, 0, 0, "")
end

function radio_exercises_do_turn(act, p)
  local turn = game.get_calendar_turn():get_turn()
  if turn % 100 == 0 then
    game.add_msg("体操をしています・・・")
  end
end

function radio_exercises_finish(act, p)
  game.add_msg("体操を終えました")
  act:set_to_null()
end

game.register_iuse("RADIO_EXERCISES", radio_exercises)
game.register_activity_do_turn("ACT_RADIO_EXERCISES", radio_exercises_do_turn)
game.register_activity_finish("ACT_RADIO_EXERCISES", radio_exercises_finish)
