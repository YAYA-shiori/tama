cd Release
"E:\tools\upx\upx.exe" --ultra-brute --no-align --best tama.exe
copy tama.exe tama_NUPX.exe
"E:\tools\upx\upx.exe" -d tama_NUPX.exe
cd ..
