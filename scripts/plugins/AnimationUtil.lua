-- Animation Utility code


function AU_ClearAllAnimation()
	local objList = upsGetModel():GetObjectList()
	
	for i = 0, #objList-1 do
		objList[i].animInfo:ClearAnimData()
	end
end

-- Register the script to upspring
upsAddMenuItem ("Animation/Clear all animation", "AU_ClearAllAnimation")

