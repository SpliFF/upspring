
-- This file is used internally by upspring, don't modify!

-- You can add custom scripts in the plugins directory, and they will be automatically executed when upspring starts




function GlobalizeModule(lib)
	if type(_G[lib])=="table" then
		for key,value in pairs(_G[lib]) do 
			_G[key]=value 
		end
	end
end


-- insanely huge lua preprocessor ;)
function prep(file)
  local chunk = {n=0}
  for line in file:lines() do
     if string.find(line, "^#") then
      table.insert(chunk, string.sub(line, 2) .. "\n")
     else
      local last = 1
      for text, expr, index in string.gfind(line, "(.-)$(%b())()") do 
        last = index
        if text ~= "" then
          table.insert(chunk, string.format('io.write %q ', text))
        end
        table.insert(chunk, string.format('io.write%s ', expr))
      end
      table.insert(chunk, string.format('io.write %q\n',
                                         string.sub(line, last).."\n"))
    end
  end
  return loadstring(table.concat(chunk))()
end


-- class.lua
function class(base,ctor)
  local c = {}     -- a new class instance
  if not ctor and type(base) == 'function' then
      ctor = base
      base = nil
  elseif type(base) == 'table' then
   -- our new class is a shallow copy of the base class!
      for i,v in pairs(base) do
          c[i] = v
      end
      c._base = base
  end
  -- the class will be the metatable for all its objects,
  -- and they will look up their methods in it.
  c.__index = c

  -- expose a ctor which can be called by <classname>(<args>)
  local mt = {}
  mt.__call = function(class_tbl,...)
    local obj = {}
    setmetatable(obj,c)
    if ctor then
       ctor(obj,unpack(arg))
    else 
    -- make sure that any stuff from the base class is initialized!
       if base and base.init then
         base.init(obj,unpack(arg))
       end
    end
    return obj
  end
  c.init = ctor
  c.is_a = function(self,klass)
      local m = getmetatable(self)
      while m do 
         if m == klass then return true end
         m = m._base
      end
      return false
    end
  setmetatable(c,mt)
  return c
end

function vectoriterate(v)
	local itfun = function(v, key)
		if key >= v:size() then return nil end
		local val = v[key]
		return key+1, val
	end
	return itfun, v, 0
end

function listiterate(l)
	-- state = end iterator, key = iterator
	local itfun = function(e, key)
		if key == e then return nil end
		local v = key:value()
		key:next()
		return key, v
	end
	return itfun, l:end_it(), l:begin_it()
end

function listiteratereverse(l)
	-- state = end iterator, key = iterator
	local itfun = function(e, key)
		if key == e then return nil end
		local v = key:value()
		key:next()
		return key, v
	end
	return itfun, l:rend_it(), l:rbegin_it()
end

function upsPrint(...)
	local arg={...};
	for k,v in ipairs(arg) do
		d_trace (v)
	end
end

function upsMsgBox(...)
	local arg={...};
	local s="";
	for k,v in ipairs(arg) do
		s = s..tostring(v)
	end
	message (s)
end

function WeakTbl() 
	local t={}
	setmetatable(t, {__mode='v'});
	return t
end

function upsGetObjects()
	local vec = upsGetModel():GetObjectList()
	local r = {}
	for i=0, vec:size()-1 do
		r[i] = vec[i]
	end
	return r
end

function upsLoadScript(fn)
	local f,err = loadfile(fn);
	if f == nil then 
		upsMsgBox("Error loading ", fn, ": ", err); 
		return
	else
		local r = { pcall(f) }
		if r[1] == false then
			upsMsgBox("Error running ", fn, ": ", r[2]);
			return
		end
	end
	upsPrint (fn , " loaded\n")
end

function upsFileOpenDlg(msg, pattern)
	local fn = cppstring()
	local r = _upsFileOpenDlg(msg, pattern, fn)
	if r==false then return nil end
	return fn:c_str()
end

function upsFileSaveDlg(msg, pattern)
	local fn = cppstring()
	local r = _upsFileSaveDlg(msg, pattern, fn)
	if r==false then return nil end
	return fn:c_str()
end

function upsDisableGCCollect(o)
	PreventCollectTbl[PCTblSize] = o
	PCTblSize = PCTblSize+1
end

-- initialize globals

PCTblSize = 0
PreventCollectTbl = {}
GlobalizeModule("upspring")


