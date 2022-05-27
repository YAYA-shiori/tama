[![download num](https://img.shields.io/github/downloads/nikolat/tama/total)](https://github.com/nikolat/tama/releases/latest)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](http://makeapullrequest.com)


Tama是[AYA]( http://umeici.onjn.jp/ )/[YAYA]( https://github.com/ponapalt/yaya-shiori )的基本操作检查工具，也可以作为AYA/YAYA执行日志的实时查看器。  
原作者：[umeici](http://umeici.onjn.jp/)  
新功能：  
- ghost选择ui：当有多个ghost同时运行时  
- 多语言支持。中文、日文和英文  
- 指定ghost的命令行参数（如[ghost_terminal](https://github.com/Taromati2/ghost_terminal)一般）  
```bat
tama.exe -g ghost_name
# 或
tama.exe -gh ghost_hwnd
```
- 你可以通过使用`tamaOpen`和`tamaExit`事件使你的ghost对tama做出反应，示例代码如下  
```c
On_tamaOpen{
	SETTAMAHWND(reference[0])
	SHIORI3FW.Push_X_SSTP_PassThru('Title','tama test: Taromati2')//这些设置都是可选的
	SHIORI3FW.Push_X_SSTP_PassThru('Icon','IMG_PATH/ico/tama.ico')
	SHIORI3FW.Push_X_SSTP_PassThru('border.color','626262')
	SHIORI3FW.Push_X_SSTP_PassThru('background.color','1e1e1e')
	SHIORI3FW.Push_X_SSTP_PassThru('default.color','4ec9b0')//以及你在tama.txt中可以找到的任何其他设置。
	'tama打开了'
}
On_tamaExit{
	SETTAMAHWND(0)
	'tama关闭了'
}
```
一些进阶技巧：  
你还可以将tama的hwnd保存在变量中并在shiori重载过后重新调用`SETTAMAHWND`来使得ghost与tama保持联系  
这需要一些更为复杂的改动，尽管我实现了这个功能但我用到了一些taromati2的dic特有的东西（`OnShioriReloaded`），所以我就不在此贴出参考代码了  
对于那些有兴趣的人，这里是改动链接以供参考：  
https://github.com/Taromati2/ghost/compare/c6ac682b4a7039e6801d36c83dccdefdaf946cee...ad652766efa09b0b582b9e9a5f9e42ad9156137e  
