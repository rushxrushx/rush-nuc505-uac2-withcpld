# rush-nuc505-uac2-withcpld


#2019.02.23

1,解决缓存bug

2,增加usb1.0模式(32bit 44khz,windows needs xmos driver)

![](https://github.com/rushxrushx/rush-nuc505-uac2-withcpld/blob/master/images/bufissue.png?raw=true)

![](https://github.com/rushxrushx/rush-nuc505-uac2-withcpld/blob/master/images/bufissue1.png?raw=true)

![](https://github.com/rushxrushx/rush-nuc505-uac2-withcpld/blob/master/images/bufissue2.png?raw=true)

![](https://github.com/rushxrushx/rush-nuc505-uac2-withcpld/blob/master/images/bufissue3.png?raw=true)


我是硬生生按照这张图片做

破音就是缓存bug了

我这么做有一个很大的问题，

如果HOST先IN 后OUT，程序就错乱了

但是我目前发现HOST都是和图上一样先IN后OUT

而且即使我EPA refresh时间是4ms

HOST每个SOF也会来read

哎呀哎呀

不爆音的感觉真好

解决了bug以后，感觉小姐姐的声音靠的更近了，糊我一脸的感觉
