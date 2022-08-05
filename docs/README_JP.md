[![download num](https://img.shields.io/github/downloads/nikolat/tama/total)](https://github.com/nikolat/tama/releases/latest)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)


[AYA](http://umeici.onjn.jp/) / [YAYA](https://github.com/ponapalt/yaya-shiori) の基本動作確認ツール、およびAYA / YAYA実行ログのリアルタイムビューアとして使用できます。  
原作者：[umeici](http://umeici.onjn.jp/)  
新機能を追加しました：  
- ゴースト選択ui: 複数のゴーストが同時に動作している場合  
- 多言語対応。中国語、日本語、英語  
- ゴーストを指定するコマンドライン引数（[ghost_terminal](https://github.com/Taromati2/ghost_terminal)のようなもの）。  
```bat
tama.exe -g ghost_name
# または
tama.exe -gh ghost_hwnd
```
- `tamaOpen` と `tamaExit` イベントを使用すると、ゴーストが tama に反応するようにすることができます。 
```c
On_tamaOpen{
	SETTAMAHWND(reference[0])
	SHIORI3FW.Push_X_SSTP_PassThru('Title','tama test: Taromati2')//これらの設定はすべて任意です。
	SHIORI3FW.Push_X_SSTP_PassThru('Icon','IMG_PATH/ico/tama.ico')
	SHIORI3FW.Push_X_SSTP_PassThru('border.color','626262')
	SHIORI3FW.Push_X_SSTP_PassThru('background.color','1e1e1e')
	SHIORI3FW.Push_X_SSTP_PassThru('default.color','4ec9b0')//およびその他の設定は、tama.txtにあります。
	'tamaが開いた'
}
On_tamaExit{
	SETTAMAHWND(0)
	'tamaが閉じた'
}
```
上級者向けのヒントも：  
また、tamaのhwndを変数に保持し、shioriがリロードされた後に`SETTAMAHWND`を再コールすれば、ghostがtamaと連絡を取り続けることができるようになります  
(この基礎となる機能は、[yayaの基礎となるdicの最新版](https://github.com/ponapalt/yaya-dic)で実装されています。)  
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
