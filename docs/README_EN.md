[![download num](https://img.shields.io/github/downloads/nikolat/tama/total)](https://github.com/nikolat/tama/releases/latest)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)


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
- A CI/AI-driven development tool `tamac.exe` that loads YAYA once, checks for errors, outputs the error content to stdout, and exits immediately
```bat
tamac.exe yaya.dll
# or
tamac.exe yaya.dll -l [fatal|error|warning|note]
# or (GitHub Actions CI mode)
tamac.exe yaya.dll --ci
```
With `--ci` (or automatically when the `GITHUB_ACTIONS` environment variable is set), tamac outputs errors as GitHub Actions annotations (`::error file=...`, `::warning ...`, etc.), allowing errors to be highlighted directly in the PR diff.
The CI mode output is compatible with [yaya-CI-check](https://github.com/YAYA-shiori/yaya-CI-check), and its implementation is based on that code.
- you can make your ghosts react to tama by using the `tamaOpen` and `tamaExit` events, the example code as follows  
```c
On_tamaOpen{
	SETTAMAHWND(reference[0])
	SHIORI3FW.Push_X_SSTP_PassThru('Title','tama test: Taromati2')//These settings are all optional
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
Some advanced tips:  
You can also keep tama's hwnd in a variable and re-call `SETTAMAHWND` after the shiori has been reloaded to keep ghost in touch with tama  
(This underlying functionality has been implemented in the [latest version of yaya's underlying dic](https://github.com/ponapalt/yaya-dic))  
```c
On_tamaOpen{
	TamaHwnd=reference[0]
	//...
}
On_tamaExit{
	ERASEVAR('TamaHwnd')
	//...
}
//...
OnInitialize{
	if reference[0]=='reload'
		SETTAMAHWND(TamaHwnd)
	//...
}

```
