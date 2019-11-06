CROSS_COMPILE?=aarch64-himix100-linux-

all:
	$(CROSS_COMPILE)g++ -c *.c -I./ -I./live555/include -I./live555/include/liveMedia -I./live555/include/BasicUsageEnvironment -I./live555/include/groupsock -I./live555/include/UsageEnvironment
	$(CROSS_COMPILE)g++ -c *.cpp -I./ -I./live555/include -I./live555/include/liveMedia -I./live555/include/BasicUsageEnvironment -I./live555/include/groupsock -I./live555/include/UsageEnvironment
	$(CROSS_COMPILE)g++ *.o ./live555/lib/libliveMedia.a ./live555/lib/libgroupsock.a ./live555/lib/libBasicUsageEnvironment.a ./live555/lib/libUsageEnvironment.a -o h264rtsp -lm -lpthread

clean:
	rm -rf *.o

