Tama can be used as a basic operation check tool for [AYA](http://umeici.onjn.jp/) / [YAYA](https://github.com/ponapalt/yaya-shiori) and as a real-time viewer of AYA / YAYA execution logs.  
Originally written by [umeici](http://umeici.onjn.jp/)  
New features:  
- Ghost selection ui: when there are multiple ghosts running at the same time  
- Multilingual support: Chinese, Japanese and English
- Command line argument to specify ghost (like [ghost_terminal](https://github.com/Taromati2/ghost_terminal)):  
```bat
tama.exe -g ghost_name
# or
tama.exe -gh ghost_hwnd
```
- you can make your ghosts react to yatama by using the `tamaOpen` and `tamaExit` events, the example code as follows  
```c
On_tamaOpen{
    SETTAMAHWND(reference0)
    SHIORI_FW.Make_X_SSTP_PassThru('Tittle','tama test: Taromati2')//set tittle
    //SHIORI_FW.Make_X_SSTP_PassThru('Icon',GetSelfIconFullPath)//set icon if y want
    'tama打开了'
}
On_tamaExit{
    'tama结束了'
    SETTAMAHWND(0)
}
```
