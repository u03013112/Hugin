# 思路
1、尝试简化nona，不用兼容那么多参数，直接可以完成我需要的
./nona3 -o out -m TIFF_m /src/test/t.pto /src/test/image0.jpg /src/test/image2.jpg
已完成，简化为./nona3 不需要任何参数

2、尝试将简化过的部分编程so
已完成，但是目前只导出so

3、尝试在py中调用这部分so
4、尝试对接api，不再使用文件系统，而是直接在py中将图片传给c++ so，然后c++ so将输出传给py
5、对这部分进行效率测试，如果效率实在低下，再找原因
6、将整个编译环境移植到macOS本地
7、将这部分封装好，以便后面使用