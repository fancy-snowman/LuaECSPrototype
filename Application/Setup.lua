-- e = scene.CreateEntity()
-- b = scene.SetComponent(e, "behaviour", "behaviour.lua")
-- 
-- print("[Lua] Printing behaviour:")
-- for k, v in pairs(b) do
-- 	print("\t", k, v)
-- end

local vector = require("vector")
local transform = require("transform")

--local monster = {
--	transform = {
--		position = vector(1, 2, 3),
--		rotation = vector(4, 5, 6),
--		scale = vector(7, 8, 9)
--	},
--	behaviour = "MonsterBehaviour.lua"
--}
--entity = scene.CreateEntity(monster)

for _ = 1, 100 do
	local entity = scene.CreateEntity()
	scene.SetComponent(entity, "health", 100)
	local tickDamage = math.random(1, 10)
	scene.SetComponent(entity, "poison", tickDamage)
end