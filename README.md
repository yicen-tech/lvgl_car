一、移植说明

音视频文件在app目录下的music和video，你也可以将文件放在自己的文件目录下，然后修改代码的打开路径。

音乐：![image](https://github.com/user-attachments/assets/7f85360f-8299-486d-824e-69e3ea486fb3)

视频：![image-1](https://github.com/user-attachments/assets/3d81dd9e-14ad-4a27-a1a4-c398464949d4)

将整个工程粘贴到你的Linux开发板上，直接在lvgl_demo目录下make编译就行（默认配置好交叉编译环境），makefile文件已经写好了。你也可以在电脑上Ubuntu虚拟机编译好可执行文件再传输到开发板上运行。
在编译中如果遇到报错，是因为一些库你并没有安装，比如curl、sdl，软件解码avcodec avformat avutil swscale swresample等，按照报错信息，安装对应的库即可。
