
local MOD = {}

mods["Invisible_Helm"] = MOD

-- コールバック関数worn_overlay_id_referencedを受け取る
-- 引数   item : 装備しているアイテム
-- 返り値 装備のタイルIDのオーバーライド(string型)
function MOD.worn_overlay_id_referenced(worn_item)
    local cover_head = false
    local cover_other = false

    for i = 1, 12 do
        game.add_msg(tostring(i))
        if worn_item:covers(enums["body_part"][i]) then
            if i == 2 then
                cover_head = true
            else
                cover_other = true
            end
        end
    end
    if cover_head and (not cover_other) then
        return "__invisible__" -- 存在しないタイルIDを返す
    else
        return nil -- nilを返せばそのままのタイルIDを使う
    end
end

--[[
MEMO
    "bp_torso",
    "bp_head",
    "bp_eyes",
    "bp_mouth",
    "bp_arm_l",
    "bp_arm_r",
    "bp_hand_l",
    "bp_hand_r",
    "bp_leg_l",
    "bp_leg_r",
    "bp_foot_l",
    "bp_foot_r",
    "num_bp",
]]
