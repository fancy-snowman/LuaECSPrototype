local monster = {}

monster.lifespan = 3
monster.print = false
monster.sleepTime = 1

function monster:OnCreate()
	print("Created!")
end

function monster:OnUpdate(delta)
	-- print("Lifespan", self.lifespan)
	-- self.lifespan = self.lifespan - 1
	-- if self.lifespan == 0 then
	-- 	self:RemoveComponent("behaviour")
	-- end

	--if self.print then
	--	print("Monster updated!")
	--end

	print("\n\nMonster has restarted counting!")
	for i = 1, 5 do
		print("\nMonster counting ", i)
		print("Monster will now sleeping for", self.sleepTime)
		coroutine.yield(self.sleepTime)
		print("Monster awoken!");
	end
end

return monster