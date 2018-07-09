
function show_npc_opinion(item, active)
  local p = player:pos()
  local setx, sety = game.choose_adjacent("対象を選択", p.x, p.y)
  local tpoint = tripoint(setx, sety, p.z)
  local npc = g:npc_at(tpoint)
  if npc then
    opinion = npc.op_of_u
    game.add_msg("信頼: " .. tostring(opinion.trust))
    game.add_msg("恐怖: " .. tostring(opinion.fear))
    game.add_msg("価値: " .. tostring(opinion.value))
    game.add_msg("怒り: " .. tostring(opinion.anger))
    game.add_msg("恩義: " .. tostring(opinion.owed))
  end
end

game.register_iuse("IUSE_SHOW_NPC_OPINION", show_npc_opinion)
