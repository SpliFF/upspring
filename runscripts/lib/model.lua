local model = {}

function model.Convert3DOToS3O(input)
    local _dirname = lib.utils.dirname(input)
    local _fileName = lib.utils.basename(input, lib.utils.get_suffix(input))
    local _fileDir = lib.utils.join_paths(_dirname, _fileName)

    print("Converting model '" .. input .. "':")

    local model = Model()
    local ok = model:Load3DO(input)
    if ok then
        print("-- Loaded the 3do")
    end

    local _texOut =  _fileDir .. "_tex.bmp"
    local ok = model:ConvertToS3O(_texOut, 512, 512);
    if ok then
        print("-- Converted to S3O")
    end

    local _s3oOut = _fileDir .. ".s3o"
    local ok = model:SaveS3O(_s3oOut)
    if ok then
        print("-- Stored S3O")
    end

    print("Done, output file is: '" .. _s3oOut .. "'")
end

return model