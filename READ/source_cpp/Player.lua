


local function write(filename, contents)
  local fh = assert(io.open(filename, "wb"))
  fh:write(contents)
  fh:flush()
  fh:close()
end


local function getData(filename)
  local file = io.open(filename, "rb")
  str = file:read("*a")
  file:close()
  
  return str
end



local function decodeInt(buffer, density, ind_cur)

  local cont_shift = 0
  local ind_buf = 44
  local data_size = 0
  local data_shift = 0
  for i=0, 31 do
    local temp = bit32.band(buffer:byte(ind_cur+1), bit32.lshift(1, cont_shift))
    if temp ~= 0 then
      data_size = bit32.bor(data_size, bit32.lshift(1, data_shift))
    end    
    data_shift = data_shift + 1
    cont_shift = (cont_shift + 1) % density
    if cont_shift == 0 then
      ind_cur = ind_cur + 1
    end
  end
  return data_size
end


local function decodeBytes(buffer, density, ind_cur, ind_end)
  local count = 0
  local str = ""
  while ind_cur < ind_end do
    local cont_shift = 0  
    local char = 0
    local data_shift = 0    
    for i=0, 7 do
      local temp = bit32.band(buffer:byte(ind_cur+1), bit32.lshift(1, cont_shift))
      if temp ~= 0 then
        char = bit32.bor(char, bit32.lshift(1, data_shift))
      end      
      data_shift = data_shift + 1
      cont_shift = (cont_shift + 1) % density
      if cont_shift == 0 then
        ind_cur = ind_cur + 1
      end
    end
    count = count + 1
    str = str .. string.char(char)
  end
  return str
end


DecodeToFile = function(path, density)
  DATA_START = 44
  INT_SIZE = 32
  CHAR_SIZE = 8
  NAME_SIZE = 20
  buffer = getData("D:\\my_documents\\Workplace\\lua_code\\result.wav")
  text_size = decodeInt(buffer, density, DATA_START) 
  NAME_START = DATA_START + (INT_SIZE / density)
  TEXT_START = NAME_START + ((NAME_SIZE * CHAR_SIZE) / density)
  TEXT_END = TEXT_START + (((tonumber(text_size) - 24) * CHAR_SIZE) / density)
  name = decodeBytes(buffer, density, NAME_START, TEXT_START)  
  text = decodeBytes(buffer, density, TEXT_START, TEXT_END)

  write("D:\\program_files_2\\foobar\\components\\music.txt", text)

  return "OK"
end


