# live555_example
基于LIVE555实现的一个例子，这里采用回调函数的方式发送数据到LIVE555里面，可以实现低延时264/265的视频发送

1. 请先编译好LIVE555，然后安装到本目录下live555。
2. 修改Makefile以便支持其他嵌入式系统或者PC。
3. 例子是采用读取文件方式来实时发送视频流。
4. 使用时，请自行实现回调函数，然后用循环队列方式写入。

经实际测试，采用LIVE555的视频流延时可以做到120ms左右。
