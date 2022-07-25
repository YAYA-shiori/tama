cd Release
upx --ultra-brute --no-align --best tama.exe
copy tama.exe tama_NUPX.exe
upx -d tama_NUPX.exe
cd ..
