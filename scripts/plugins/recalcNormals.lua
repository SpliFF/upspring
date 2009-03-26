
local function RecalcNormals(selected)
	if not maxSmoothingAngle then
		maxSmoothingAngle = 45.0
	end
	
	local ans = input ("Enter max smoothing angle between faces", tostring(maxSmoothingAngle));
	
	if ans then 
		maxSmoothingAngle = ans
		
		local objs;
		
		if selected then
			objs = upsGetModel():GetSelectedObjects();
		else
			objs = upsGetModel():GetObjectList();
		end
		
		for i=0,#objs-1 do
			local o = objs[i];
			local pm = o:GetPolyMesh();
			
			if pm then
				pm:CalculateNormals2(maxSmoothingAngle);
			end
		end
	end	
end

function Recalc_Normals_SelObj()
	RecalcNormals(true)
end

function Recalc_Normals_AllObj()
	RecalcNormals(false)
end

upsAddMenuItem ("Object/Recalculate vertex normals, 3DO style/Selected objects", "Recalc_Normals_SelObj");
upsAddMenuItem ("Object/Recalculate vertex normals, 3DO style/All objects", "Recalc_Normals_AllObj");
