
local function RescalePolyMeshNormals (pm)
	for i=0,#pm.verts-1 do
		pm.verts[i].normal:normalize();
	end	
end

function Rescale_Normals()
	local objects = upsGetModel():GetSelectedObjects();

	-- iterate through all objects
	for i=0,#objects-1 do
		if objects[i].isSelected then 
			local pm = objects[i]:GetPolyMesh();
			if pm then
				RescalePolyMeshNormals(pm) 
			end
		end
	end
	
	-- redraw
	upsUpdateViews();
end

-- upsAddMenuItem ("Object/Rescale normals of selected objects","Rescale_Normals")
