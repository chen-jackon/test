#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define VIDEO_DEVICE "/dev/video0"
#define OUTPUT_FILE "output.yuv"
#define VIDEO_WIDTH 640
#define VIDEO_HEIGHT 480
#define VIDEO_FORMAT V4L2_PIX_FMT_YUYV

bool captureVideo() {
    int fd = open(VIDEO_DEVICE, O_RDWR);
    if (fd == -1) {
        std::cerr << "无法打开视频设备节点: " << VIDEO_DEVICE << std::endl;
        return false;
    }

    // 设置图像格式
    v4l2_format format{};
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = VIDEO_WIDTH;
    format.fmt.pix.height = VIDEO_HEIGHT;
    format.fmt.pix.pixelformat = VIDEO_FORMAT;
    format.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd, VIDIOC_S_FMT, &format) == -1) {
        std::cerr << "无法设置图像格式" << std::endl;
        close(fd);
        return false;
    }

    // 请求图像缓冲区
    v4l2_requestbuffers req{};
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    req.count = 1;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        std::cerr << "无法请求图像缓冲区" << std::endl;
        close(fd);
        return false;
    }

    // 映射图像缓冲区
    v4l2_buffer buffer{};
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buffer) == -1) {
        std::cerr << "无法查询图像缓冲区" << std::endl;
        close(fd);
        return false;
    }

    void* bufferStart = mmap(NULL, buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buffer.m.offset);
    if (bufferStart == MAP_FAILED) {
        std::cerr << "无法映射图像缓冲区" << std::endl;
        close(fd);
        return false;
    }

    // 入队图像缓冲区
    if (ioctl(fd, VIDIOC_QBUF, &buffer) == -1) {
        std::cerr << "无法入队图像缓冲区" << std::endl;
        munmap(bufferStart, buffer.length);
        close(fd);
        return false;
    }

    // 启动视频流
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        std::cerr << "无法启动视频流" << std::endl;
        munmap(bufferStart, buffer.length);
        close(fd);
        return false;
    }

    // 创建输出文件
    std::ofstream outputFile(OUTPUT_FILE, std::ios::binary);
    if (!outputFile) {
        std::cerr << "无法创建输出文件" << std::endl;
        munmap(bufferStart, buffer.length);
        close(fd);
        return false;
    }

    // 捕获图像数据
    std::cout << "开始录制视频..." << std::endl;

    time_t startTime = time(NULL);
    while (difftime(time(NULL), startTime) < 10.0) {
        // 读取图像数据
        if (ioctl(fd, VIDIOC_DQBUF, &buffer) == -1) {
            std::cerr << "无法读取图像数据" << std::endl;
            break;
        }

        // 将图像数据写入输出文件
        outputFile.write(reinterpret_cast<char*>(bufferStart), buffer.bytesused);

        // 重新入队图像缓冲区
        if (ioctl(fd, VIDIOC_QBUF, &buffer) == -1) {
            std::cerr << "无法重新入队图像缓冲区" << std::endl;
            break;
        }
    }

    // 停止视频流
    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        std::cerr << "无法停止视频流" << std::endl;
    }

    // 解除映射图像缓冲区
    munmap(bufferStart, buffer.length);

    // 关闭视频设备和输出文件
    close(fd);
    outputFile.close();

    std::cout << "视频录制完成" << std::endl;

    return true;
}

int main() {
    if (captureVideo()) {
        std::cout << "视频录制成功" << std::endl;
        return 0;
    } else {
        std::cerr << "视频录制失败" << std::endl;
        return 1;
    }
}

