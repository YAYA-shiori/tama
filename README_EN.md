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
- you can make your ghosts react to tama by using the `tamaOpen` and `tamaExit` events, the example code as follows  
```c
On_tamaOpen{
	SETTAMAHWND(reference[0])
	SHIORI3FW.Push_X_SSTP_PassThru('Tittle','tama test: Taromati2')//These settings are all optional
	SHIORI3FW.Push_X_SSTP_PassThru('Icon','IMG_PATH/ico/tama.ico')
	SHIORI3FW.Push_X_SSTP_PassThru('border.color','626262')
	SHIORI3FW.Push_X_SSTP_PassThru('background.color','1e1e1e')
	SHIORI3FW.Push_X_SSTP_PassThru('default.color','4ec9b0')//and any other settings you can find in tama.txt
	'tama opened'
}
On_tamaExit{
	SETTAMAHWND(0)
	'tama closed'
}
```
