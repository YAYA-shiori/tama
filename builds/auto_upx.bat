cd Release
upx --ultra-brute --no-align --best --compress-resources=1 tama.exe
copy tama.exe tama_NUPX.exe
upx -d tama_NUPX.exe
cd ..
