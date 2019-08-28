

--write data to file
local function write(filename, contents)
  local fh = assert(io.open(filename, "wb"))
  fh:write(contents)
  fh:flush()
  fh:close()
end

--read data from file
local function getData(filename)
  local file = io.open(filename, "rb")
  str = file:read("*a")
  file:close()  
  return str
end

--get INT value from track's samples with specified density
local function decodeInt(buffer, density, ind_cur, shift)
  local cont_shift = 0
  local numb = 0
  local data_shift = 0
  local extra_shift = 0
  local extra_ind = 0  
  for i=0, 31 do
    local temp = bit32.band(buffer:byte(ind_cur + extra_ind), bit32.lshift(1, cont_shift))
    if temp ~= 0 then
      numb = numb + bit32.lshift(1, data_shift)
    end        
    data_shift = data_shift + 1
    extra_shift = (extra_shift + 1) % density
    extra_ind = math.floor(extra_shift / 8)
    cont_shift = extra_shift % 8      
    if extra_shift == 0 then
      ind_cur = ind_cur + shift
    end
  end
  return numb, ind_cur
end

--get N bytes from track's samples with specified density
local function decodeBytes(buffer, density, ind_cur, N, shift)
  local count = 0
  local str = ""
  local cont_shift = 0  
  local extra_shift = 0
  local data_shift = 0  
  local extra_ind = 0
  
  for i = 0, N-1 do        
    local char = 0
    for i = 0, 7 do
      local temp = bit32.band(buffer:byte(ind_cur + extra_ind), bit32.lshift(1, cont_shift))
      if temp ~= 0 then
        char = char + bit32.lshift(1, data_shift)
      end      
      data_shift = (data_shift + 1) % 8
      extra_shift = (extra_shift + 1) % density
      extra_ind = math.floor(extra_shift / 8)
      cont_shift = extra_shift % 8
      if extra_shift == 0 then
        ind_cur = ind_cur + shift
      end      
    end
    str = str .. string.char(char)
  end
  return str, ind_cur
end

--get value of density
--number 44 is taken from the WAV-file's stucture (data block start)
local function decodeDensity(buffer, shift)
  density = 0 
  for i = 0, 4 do
    if bit32.band(buffer:byte(44 + 1 + shift * i), 1) == 1 then 
      density = density + (bit32.lshift(1, i))
    end
  end
  return density + 1  
end

--get track's bits per second value
--number 34 is taken from the WAV-file's stucture
local function getBPS(buffer)
  bps = buffer:byte(34 + 1)
  bps = bps + bit32.lshift(buffer:byte(36), 8)  
  return bps
end


DecodeToFile = function(pathToFile, pathToDecode)
  --44 + 
  DATA_START = 45
  NAME_SIZE = 20  
  INT_SIZE = 32
  BYTE_SIZE = 8
  DEN_SIZE = 5
  
  if pathToDecode == nil then
    pathToDecode = "D:\\new\\"
  end
  
  buffer = getData(pathToFile)  
  BPS = getBPS(buffer)    
  BPS_BYTES = BPS / BYTE_SIZE
  density = decodeDensity(buffer, BPS_BYTES)
  mark, ind = decodeInt(buffer, density, DATA_START + BPS_BYTES * DEN_SIZE, BPS_BYTES)
  
  if mark == 111111 then
    text_size, ind = decodeInt(buffer, density, ind, BPS_BYTES)
    name, ind = decodeBytes(buffer, density, ind, NAME_SIZE, BPS_BYTES)
    text = decodeBytes(buffer, density, ind, (tonumber(text_size) - 32) , BPS_BYTES)
    write(pathToDecode .. name, text)    
    return pathToDecode .. name
  else
    return "ERROR"
  end
end

