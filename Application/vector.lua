local vector = {}

vector.__index = vector

function new(x, y, z)
	local t = {
		x = x or 0,
		y = y or 0,
		z = z or 0
	}
	return setmetatable(t, vector)
end

local function isvector(t)
	return getmetatable(t) == vector
end

------------------------------------------------------------------- Meta events

function vector:__newindex(k, v)
	print("vector - not possible to assign new fields")
end

function vector:__tostring()
	return "(" .. self.x .. ", " .. self.y .. ", " .. self.z .. ")"
end

----------------------------------------------------- Meta mathematic operators

function vector.__unm()
	return new(-self.x, -self.y, -self.z)
end

function vector.__add(a, b)
	assert(isvector(a) and isvector(b), "vector add - expected args: vector, vector")
	return new(a.x+b.x, a.y+b.y, a.z+b.z)
end

function vector.__sub(a, b)
	assert(isvector(a) and isvector(b), "vector sub - expected args: vector, vector")
	return new(a.x-b.x, a.y-b.y, a.z-b.z)
end

function vector.__mul(a, b)
	-- Check for scalar multiplication
	if type(a) == "number" then
		return new(a*b.x, a*b.y, a*b.z)
	elseif type(b) == "number" then
		return new(a.x*b, a.y*b, a.z*b)
	else
		assert(isvector(a) and isvector(b), "vector mul - expected args: vector or number")
		return new(a.x*b.x, a.y*b.y, a.z*b.z)
	end
end

function vector.__div(a, b)
	assert(isvector(a), "vector div - expected args: vector, vector or number")
	if type(b) == "number" then
		return new(a.x/b, a.y/b, a.z/b)
	else
		assert(isvector(b), "vector div - expected args: vector, vector or number")
		return new(a.x/b.x, a.y/b.y, a.z/b.z)
	end
end

---------------------------------------------------------- Meta logic operators

function vector.__eq(a, b)
	assert(isvector(a) and isvector(b), "vector eq - expected args: vector, vector")
	return a.x==b.x, a.y==b.y, a.z==b.z
end

----------------------------------------------------------- Wrap up and send :)

vector.new = new
vector.isvector = isvector

return setmetatable(vector, {__call = function(_,...) return new(...) end})
