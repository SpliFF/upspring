--[[ 
 	Example exporter, outputs some animation/position data of objects to a file 
]]


local function vec3str(v)
	return string.format ("%3.3f, %3.3f, %3.3f", v.x, v.y, v.z)
end

local function OutputAnimationInfo(file, obj)
	for propindex,prop in vectoriterate(obj.animInfo.properties) do
		file:write(" AnimProperty ", prop.name, " with ", upsAnimGetNumKeys(prop), " keys\n")
		
		local type = upsAnimGetType(prop, i)
		-- iterate through all the keys of the property
		for i = 0, upsAnimGetNumKeys(prop)-1 do
			local time = upsAnimGetKeyTime(prop, i)
			file:write (string.format("  Key time: %3.2f,  ", time))
			if type == ANIMTYPE_FLOAT then
			
				local value = upsAnimGetFloatKey(prop, i)
				file:write ("Float: ", value, "\n")
				
			elseif type == ANIMTYPE_VECTOR3 then
			
				local vector = upsAnimGetVector3Key(prop,i)
				file:write ("Vector3: ", vec3str(vector), "\n")
				
			elseif type == ANIMTYPE_ROTATION then
			
				local rot = upsAnimGetRotationKey(prop,i) -- return euler angles in a vector3
				file:write ("Rotation: ", vec3str(rot), "\n")
				
			end
		end
	end
end

local function OutputObject(file, obj)
	file:write("Object ", obj.name, "\n")
	file:write(" Current Pos:", vec3str(obj.position), "\n");
	file:write(" Current Rotation:", vec3str(obj.rotation:GetEuler()), "\n");
	file:write(" Current Scale:", vec3str(obj.scale), "\n");
	
	OutputAnimationInfo(file, obj)

	for x=0,obj.childs:size()-1 do
		OutputObject(file, obj.childs[x])
	end 
end

local function Exec()
	local ok,result
	result = upsFileSaveDlg("Enter txt file to save to:", "Text file(TXT)\0txt\0")
	if result ~= nil then
		local f = assert(io.open(result, "w"))
		
		local o = upsGetRootObj()
		if o ~= nil then
			OutputObject(f, o)
		end
		
		f:close()
		
		upsMsgBox("File exported: ", result);
	end
end

function ExampleExportHandler(goal)
	Exec()
end


-- Register the script to upspring
upsAddMenuItem ("Scripts/Scripts/Example exporter script", "ExampleExportHandler");

