--[[
  Copyright (c) 2015 Dustin Reed Morado
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
--]]

--- General utilities module for tomoauto.
-- This is a small collection of useful utilities for tomoauto
-- @module utils
-- @author Dustin Reed Morado
-- @license MIT
-- @release 0.2.30

local lfs = require('lfs')
local io, math, os, string = io, math, os, string
local assert, type = assert, type
local pathsep = string.sub(_G.package.config, 1, 1)

_ENV = nil

local utils = {}

function utils.check_type (argument, Type)
  assert(type(Type) == 'string',
	 'ERROR: utils.check_type: Type must be string.')
  return type(argument) == Type
end

function utils.is_boolean (argument)
  return argument == not not argument
end

function utils.is_function (argument)
  return utils.check_type(argument, 'function')
end

function utils.is_nil (argument)
  return argument == nil
end

function utils.is_number (argument)
  return utils.check_type(argument, 'number')
end

function utils.is_string (argument)
  return utils.check_type(argument, 'string')
end

function utils.is_table (argument)
  return utils.check_type(argument, 'table')
end

function utils.is_thread (argument)
  return utils.check_type(argument, 'thread')
end

function utils.is_userdata (argument)
  return utils.check_type(argument, 'userdata')
end

function utils.is_path (path)
  assert(utils.is_string(path),
	 'ERROR: utils.is_path: path must be a string.')
  return lfs.attributes(path, 'mode') and path or nil
end

function utils.is_absolute_path (path)
  assert(utils.is_string(path),
	 'ERROR: utils.is_absolute_path: path must be a string.')
  return path:sub(1, 1) == pathsep and true or nil
end

function utils.is_directory (path)
  assert(utils.is_string(path),
	 'ERROR: utils.is_directory: path must be a string.')
  return lfs.attributes(path, 'mode') == 'directory' and true or nil
end

function utils.is_file (path)
  assert(utils.is_string(path),
	 'ERROR: utils.is_file: path must be a string.')
  return lfs.attributes(path, 'mode') == 'file' and true or nil
end

function utils.join_paths (path_1, path_2)
  assert(utils.is_string(path_1) and utils.is_string(path_2),
         'ERROR: utils.join_paths: paths must be strings.')

  if path_1:len() * path_2:len() == 0 then
    return path_1 .. path_2
  else
    return path_1 .. pathsep .. path_2
  end
end

function utils.absolute_path (path)
  if not utils.is_string(path) then return nil end
  local absolute_path = utils.is_absolute_path(path) and path or
			utils.join_paths(lfs.currentdir(), path)

  local done_flag, old_absolute_path
  repeat
    local found_flag, regex
    done_flag = 0
    old_absolute_path = absolute_path

    -- Resolve dot dot references
    -- UNIX: string.gsub(absolute_path, '[^/]+/%.%./?, '', 1)
    -- /alpha/beta/../gamma/delta --> /alpha/gamma/delta
    regex = '[^' .. pathsep .. ']+' .. pathsep .. '%.%.' .. pathsep .. '?'
    absolute_path, found_flag = absolute_path:gsub(regex, '', 1)
    done_flag = done_flag + found_flag

    -- Remove redundant dot references
    -- UNIX: string.gsub(absolute_path, '/./', '/')
    -- /alpha/beta/./gamma/delta --> /alpha/beta/gamma/delta
    regex = pathsep .. '%.' .. pathsep
    absolute_path, found_flag = absolute_path:gsub(regex, pathsep)
    done_flag = done_flag + found_flag

    -- Remove duplicate path separators
    -- UNIX: string.gsub(absolute_path, '//+', '/')
    -- /alpha/beta///gamma/delta --> /alpha/beta/gamma/delta
    regex = pathsep .. pathsep .. '+'
    absolute_path, found_flag = absolute_path:gsub(regex, pathsep)
    done_flag = done_flag + found_flag

    -- Remove redundant trailing path separators and optional dot
    -- UNIX: string.gsub(absolute_path, '/%.?$', '')
    -- /alpha/beta/gamma/delta/. --> /alpha/beta/gamma/delta
    regex = pathsep .. '%.?$'
    absolute_path, found_flag = absolute_path:gsub(regex, '')
    done_flag = done_flag + found_flag
  until done_flag == 0 or old_absolute_path == absolute_path

  return absolute_path == '' and pathsep or absolute_path
end

--- Splits argument into directory and file components.
-- This takes an argument and returns the absolute path of the directory and the
-- file part as two separate strings, if the argument is not an absolute path
-- the path is taken to be in relation to the current directory.
-- @tparam string path A file system path.
-- @return Returns the directory component of the path first and the file
-- component of the path second. If the path is a directory the file component
-- will be the empty string. Returns nil and an error message on failure.
function utils.split_path (path)
  if not utils.is_string(path) then return nil end
  local absolute_path = utils.absolute_path(path)
  local dirname, basename
  if utils.is_directory(absolute_path) then
    return absolute_path, ''
  else
    -- Strip of anything past the last slash
    -- UNIX: string.gsub(absolute_path, /[^/]+$, '')
    -- /alpha/beta/gamma/delta.txt --> /alpha/beta/gamma
    local dirname = absolute_path:gsub(pathsep .. '[^' .. pathsep .. ']+$', '')

    -- Capture everything past the last slash
    -- UNIX: string.match(absolute_path, /([^/]+)$')
    -- /alpha/beta/gamma/delta.txt --> delta.txt
    local basename = absolute_path:match(pathsep .. '([^' .. pathsep .. ']+)$')

    return dirname, basename
  end
end

--- Determines the absolute directory component of the argument.
-- This returns the first component of @{split_path}.
-- @tparam string path A file system path.
-- @return Returns the absolute path directory component of the path or nil and
-- an error message on failure.
function utils.dirname (path)
  if not utils.is_string(path) then return nil end
  return (utils.split_path(path))
end

--- Determines the final suffix of the argument.
-- This function just returns the last suffix of a given string including the
-- leading period.
-- @tparam string path A file system path or file name.
-- @return Returns the last suffix of the argument including the leading dot or
-- nil and an error message on failure
function utils.get_suffix (path)
  if not utils.is_string(path) then return nil end
  return (path:match('%.[%w]+$'))
end

--- Determines the file component of a path and optionally remove a suffix.
-- This function just returns the second component of @{split_path} and
-- if suffix is given removes the suffix from the file component. If the path
-- given is a directory the function returns an empty string.
-- @tparam string path A file system path.
-- @tparam string suffix An optional suffix to strip from the file component.
-- @return Returns the file component of the given path with the suffix
-- optionally stripped from the returned string or nil and an error message on
-- failure
function utils.basename (path, suffix)
  if not utils.is_string(path) then
    return nil
  elseif suffix and not utils.is_string(suffix) then
    return nil
  end

  local _, basename = utils.split_path(path)
  if suffix then
    return (basename:gsub(suffix:gsub('%.', '%%.'), ''))
  else
    return basename
  end
end

--- Determines the relative path of the argument
-- This function returns the relative path of the argument with respect to the
-- current directory.
-- @tparam string path A file system path.
-- @return Returns the relative path of the argument with respect to the
-- current directory or nil and an error message on failure.
function utils.relative_path (path)
  if not utils.is_string(path) then return nil end
  local source = lfs.currentdir()
  local target = utils.absolute_path(path)
  local relative_path = ''
  while source ~= '' do
    local s_word = source:match(pathsep .. '[^' .. pathsep .. ']+')
    local t_word = target:match(pathsep .. '[^' .. pathsep .. ']+')

    -- If source and target have the same path at this point remove it from both
    -- paths.
    if s_word == t_word then
      source = source:gsub(s_word, '', 1)
      target = target:gsub(t_word, '', 1)

    -- If source and target differ, remove it from the source path and add '..'
    -- to relative path.
    else
      source = source:gsub(s_word, '', 1)
      relative_path = utils.join_paths(relative_path, '..')

    end
  end

  -- Remove leading slash from target path if it exists.
  target = target:gsub('^' .. pathsep, '')
  return (utils.join_paths(relative_path, target))
end

--------------------------------------------------------------------------------
--                                 OS UTILITIES                               --
--------------------------------------------------------------------------------

--- Runs an OS command and exits Lua if the command fails.
-- This function runs a command in os.execute and if the command fails or is
-- terminated by a signal or exits successfully but with a non-zero status then
-- returns nil and an error message.
-- @tparam string command The command to run with os.execute.
-- @return Returns true if the command exited successfully with a zero exit
-- value and nil and an error message if the command exited unsuccessfully or
-- was terminated by a signal or exited successfully with a non-zero exit code.
function utils.run (command)
  if not utils.is_string(command) then return nil end
  local status, exit, signal = os.execute(command)
  if not status and exit == 'exit' then
    return nil, '\n\nERROR: tomoauto.utils.run:\n\t' ..
		'Command: ' .. command .. ' executed unsuccessfully with ' ..
		'signal: ' .. signal .. '.\n'
  elseif not status and exit == 'signal' then
    return nil, '\n\nERROR: tomoauto.utils.run:\n\t' ..
		'Command: ' .. command .. ' was terminated by signal ' ..
		signal .. '.\n'
  elseif signal ~= 0 then
    return nil, '\n\nERROR: tomoauto.utils.run:\n\t' ..
		'Command: ' .. command .. ' exited successfully but with a ' ..
		'non-zero signal: ' .. signal .. '.\n'
  else
    return true
  end
end

--- Backs up a file if it exists.
-- Backs up a file replacing previous backup if it exists.
-- @tparam string path A filesystem path.
-- @tparam string suffix Optional suffix to add to backup [Default: '.bak'].
-- @treturn boolean Returns true if no file or successfully backs the file up
-- and returns nil and an error message otherwise.
function utils.backup (path, suffix)
  if not utils.is_string(path) then
    return nil
  elseif suffix and not utils.is_string(suffix) then
    return nil
  end

  local absolute_path = utils.absolute_path(path)
  if utils.is_file(absolute_path) then
    local backup = suffix and absolute_path .. suffix or absolute_path .. '.bak'
    local status, err = os.rename(absolute_path, backup)
    if not status then
      return nil, '\n\nERROR: tomoauto.utils.backup:\n\t' ..
		  err .. '.\n'
    else
      return true
    end
  else
    return true
  end
end

--------------------------------------------------------------------------------
--                              OTHER UTILITIES                               --
--------------------------------------------------------------------------------

--- Reads Bio3d style floats in the extended header.
-- This function is included in the MRC header documentation for IMOD. I have
-- not seen it used anywhere but it could be used in the future and it was
-- easily implemented.
-- @tparam number short_1 A short number.
-- @tparam number short_2 A short number.
-- @return Returns the float encoded by the two shorts or nil and an error
-- message on failure.
function utils.IMOD_short_to_float (short_1, short_2)
  if type(short_1) ~= 'number' then
    return nil, '\n\nERROR: tomoauto.utils.IMOD_short_to_float:\n\t' ..
		'Short_1 must be given and be a number type.\n'
  elseif type(short_2) ~= 'number' then
    return nil, '\n\nERROR: tomoauto.utils.IMOD_short_to_float:\n\t' ..
		'Short_2 must be given and be a number type.\n'
  end

  local sgn_1, sgn_2 = short_1 < 0 and -1 or 1, short_2 < 0 and -1 or 1
  local abs_1, abs_2 = math.abs(short_1), math.abs(short_2)
  return sgn_1 * ((256 * abs_1) + (abs_2 % 256)) * 2 ^ (sgn_2 * (abs_2 / 256))
end

return utils
-- vim: set ft=lua tw=80 ts=8 sts=2 sw=2 noet :