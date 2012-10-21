--[[
 	BVH animation import/export

  Adapt TransformPosition/TransformRotation to change the scaling/mirroring of the values
]]

-- modify pos.x, pos.y or pos.z if needed
local function TransformPosition(pos)
	pos.x = -pos.x
end

-- modify rot.x, rot.y, rot.z if needed, these are angles in radians
local function TransformRotation(rot)
	
end




-- ParseElem parses a single text element such as a word or a number or the { } tokens
local function ParseElem(pd)
	local t
	local ret = ""

	while true do
		t=pd.file:read(1)
		if not t then return nil end
		
		if string.find(t, "%s") then
			if string.len(ret) > 0 then 
		--		upsPrint(ret, "\n")
				return ret 
			end
			
			-- increase line
			if t == "\n" then 
				pd.line = pd.line + 1 
			end
		else
			ret = ret .. t
		end
	end
end

-- helper function for parsing
local function pcheck(val, errmsg)
	if not val then
		error(errmsg, 0)
	end
end

local function ParseJoint(pd, j)
	j.name = ParseElem(pd)           -- read name
	pcheck(ParseElem(pd) == "{", "Expecting {")
	
	table.insert(pd.joints, j) -- add to joints list
	
	while true do
		local elem = ParseElem(pd)
		if not elem then return end
		
		if elem == "OFFSET" then
			j.Xposition = tonumber(ParseElem(pd))
			j.Yposition = tonumber(ParseElem(pd))
			j.Zposition = tonumber(ParseElem(pd))
			pcheck(j.Xposition and j.Yposition and j.Zposition, "Expecting joint offset (X Y Z)")
		end
		
		if elem == "CHANNELS" then
			local nChan = tonumber(ParseElem(pd))
			pcheck(nChan, "Expecting number of channels")
			
			j.nChan = nChan
			j.channels = {}
			for c = 0, nChan-1 do
				j.channels[ParseElem(pd)] = c
			end
		end
		
		-- sub-joint
		if elem == "JOINT" then
			local chj = { childs={} }
			table.insert(j.childs, chj)
			
			ParseJoint(pd, chj)
		end
		
		if elem == "End" then
			pcheck(ParseElem(pd) == "Site", "Expecting 'Site' after 'End'")
			
			-- ignore End Site
			pcheck(ParseElem(pd) == "{", "Expecting '{' after 'End Site'")
			while ParseElem(pd) ~= "}" do end
		end
		
		if elem == "}" then
			break
		end
	end
	
	
end

-- Parse a BVH file and store all the data in the "Parse Data" table (pd)
local function Parse(pd)
	pcheck(ParseElem(pd) == "HIERARCHY", "Expecting HIERARCHY")
	pcheck(ParseElem(pd) == "ROOT", "Expecting ROOT")
	
	-- initialize "parse data" structure (pd)
	pd.joints = {}                        -- non-recursive joint list
	
	-- Parse 
	pd.root = { childs={} }
	ParseJoint(pd, pd.root)
	
	-- Parse motion data
	
	pcheck(ParseElem(pd) == "MOTION" , "Expecting MOTION section")
	pcheck(ParseElem(pd) == "Frames:", "Expecting 'Frames:'")
	pd.nFrames = tonumber(ParseElem(pd))
	
	pcheck(ParseElem(pd) == "Frame" and ParseElem(pd) == "Time:", "Expecting 'Frame Time:'")
	pd.frameTime = tonumber(ParseElem(pd))

	local numKeys = 0
	for a,j in pairs(pd.joints) do
		j.firstIndex = numKeys
		numKeys = numKeys + j.nChan
	end
	
	upsPrint("NumKeys: ", numKeys, "\n")
	
	pd.frameData = {}
	for frame = 0, pd.nFrames-1 do
		local frameData = {}
		pd.frameData[frame] = frameData
		
		for a = 0, numKeys-1 do
			frameData[a] = tonumber(ParseElem(pd))
		end
	end
end

local function FindAnimProp(obj, name)
	local animProps = obj.animInfo.properties
	
	-- animProps is a C++ vector
	for i=0, #animProps-1 do
		if animProps[i].name == name then
			return animProps[i]
		end
	end
end

local function CopyAnimationToObject(pd, j, obj)

	local function GetPropertyValue(map, value, frame)
		local keys
		
		for k,v in pairs(map) do
		
			if j.channels[v] then
				local keyIndex = j.channels[v] + j.firstIndex
				local keyVal = pd.frameData[frame][keyIndex] 
				if keyVal then
					--upsPrint("k=",k, ", v=", v, ". j.channels[v]=", j.channels[v], "  KeyIndex=", keyIndex, " val=", keyVal, "\n");
					value[k] = keyVal
				end
					
				keys = true
			else
				value[k] = j[v]
			end
		end

		return keys, value
	end

	local posProp = FindAnimProp(obj, "position")
	local rotProp = FindAnimProp(obj, "rotation")
	
	for f=0, pd.nFrames-1 do
		local keys, pos = GetPropertyValue( {x="Xposition", y="Yposition", z="Zposition"}, Vector3(), f)
		if keys and posProp then
			TransformPosition(pos)
			upsAnimInsertVectorKey(posProp, pd.frameTime * f, pos)
		end
		
		local rot
		keys, rot = GetPropertyValue( {x="Xrotation", y="Yrotation", z="Zrotation"}, Vector3(), f)
		
		if keys and rotProp then
			rot = rot * M_PI / 180
			TransformRotation(rot)
			upsAnimInsertVectorKey(rotProp, pd.frameTime * f, rot)
		end
	end
end

local function ConvertTree(pd, j)
	local obj = MdlObject()
	upsDisableGCCollect(obj)
	
	obj.name = j.name
	obj.position.x = j.Xposition
	obj.position.y = j.Yposition
	obj.position.z = j.Zposition
	
--	upsPrint("Numchilds:", #j.childs)

	for k,v in pairs(j.childs) do
--		upsPrint("Processing " , j.name, " child", k, "\n")
		local ch = ConvertTree(pd, v)
		if ch then
			obj:AddChild(ch)
		end
	end
	
	return obj
end

local function LoadAnims(pd, j, objMap)
	-- find the MdlObject with the same name in the objMap
	if objMap[j.name] then
		CopyAnimationToObject(pd, j, objMap[j.name])
	end

	-- recurse
	for k,ch in pairs(j.childs) do
		LoadAnims(pd, ch, objMap)
	end
end

local function GetObjectMap()
	local objList = upsGetModel():GetObjectList()
	local objMap = {}
	
	for i = 0, #objList-1 do
		objMap [objList[i].name] = objList[i]
	end
	
	return objMap
end

function BVH_Import(animOnly)
	local result = upsFileOpenDlg("Select BVH file to import:", "BVH animation data\0*.bvh\0")
	if result == nil then return end
	
	-- open the file, and show the error when it failed
	local f,err = io.open(result)
	
	if f == nil then
		upsMsgBox(err)
		return
	end
	
	-- parse
	local pd = { file = f, line=1 }
	local r, errMsg = pcall(Parse, pd)

	if r then
		if not animOnly then
			-- create a skeleton with the animation in it	
			local obj = ConvertTree(pd, pd.root)
			
			if obj then
				local mdl = upsGetModel()
				
				if upsGetRootObj() then
					mdl:DeleteObject(upsGetRootObj())
				end
				
				mdl:SetRoot(obj)
			end
		end

		-- clear old animation data if any
		local objMap = GetObjectMap()
		for k,v in pairs(objMap) do
			v.animInfo:ClearAnimData()
		end

		-- copy the animation from the BVH to the current model
		LoadAnims(pd, pd.root, objMap)
			
	else
		upsMsgBox( result .. ": Parse error in line ", pd.line, ": ", errMsg)
	end
	
	f:close()
end


function BVHImportAnimOnly()
	BVH_Import(true)
end

-- Register the script to upspring
upsAddMenuItem ("Animation/Import skeleton+anim from BVH", "BVH_Import")
upsAddMenuItem ("Animation/Import animation from BVH", "BVHImportAnimOnly")
