-- Test script that will show up in Scripts menu

function ExecTestScript()
	upsMsgBox "Empty script..."
end

upsAddMenuItem ("Scripts/Scripts/Empty test script", "ExecTestScript")



-- Test script that creates a small tree of objects

function CreateTestTree(goal)
	local mdl = upsGetModel();
	
	if mdl.root ~= nil then 
		mdl:DeleteObject(mdl.root);
	end
	
	local o = MdlObject();
	upsDisableGCCollect(o)
	mdl:SetRoot(o)
	
	local ch = MdlObject();
	o:AddChild(ch)
	upsDisableGCCollect(ch)
	ch.name = "ChildObj";
	o.name = "LuaCreatedObj";
end

-- Register the scripts to upspring
upsAddMenuItem ("Scripts/Scripts/Object tree script", "CreateTestTree")

