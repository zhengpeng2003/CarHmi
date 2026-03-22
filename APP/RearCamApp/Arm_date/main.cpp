#include <gst/gst.h>
#include <gst/app/gstappsink.h>

#include <opencv2/opencv.hpp>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cerrno>

static const char* SOCKET_PATH = "/tmp/rearcam_ipc.sock";

static bool sendAll(int fd, const unsigned char* data, size_t len)
{
    size_t total = 0;
    while (total < len) {
        ssize_t n = ::send(fd, data + total, len - total, 0);
        if (n <= 0) {
            return false;
        }
        total += static_cast<size_t>(n);
    }
    return true;
}

static bool sendFrame(int fd, const std::vector<uchar>& jpegBuf)
{
    uint32_t len = static_cast<uint32_t>(jpegBuf.size());
    uint32_t beLen = htonl(len);

    if (!sendAll(fd, reinterpret_cast<unsigned char*>(&beLen), 4)) {
        return false;
    }

    if (!jpegBuf.empty()) {
        if (!sendAll(fd, jpegBuf.data(), jpegBuf.size())) {
            return false;
        }
    }
    return true;
}

static int connectServer()
{
    int fd = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "socket create failed\n";
        return -1;
    }

    sockaddr_un addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    std::snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        std::cerr << "connect failed: " << std::strerror(errno) << "\n";
        ::close(fd);
        return -1;
    }

    std::cout << "connected to " << SOCKET_PATH << "\n";
    return fd;
}

static void processFrame(cv::Mat& frame)
{
    if (frame.empty()) {
        return;
    }

    // 1) 镜像：倒车影像一般需要
    cv::flip(frame, frame, 1);

    // 2) 画简单倒车辅助线
    int w = frame.cols;
    int h = frame.rows;

    cv::Point bottomCenter(w / 2, h - 20);

    cv::line(frame, cv::Point(w / 2, h - 20), cv::Point(w / 2, h / 2),
             cv::Scalar(255, 255, 255), 2);

    cv::line(frame, cv::Point(w / 2 - 180, h - 20), cv::Point(w / 2 - 60, h / 2),
             cv::Scalar(0, 255, 0), 2);
    cv::line(frame, cv::Point(w / 2 + 180, h - 20), cv::Point(w / 2 + 60, h / 2),
             cv::Scalar(0, 255, 0), 2);

    cv::line(frame, cv::Point(w / 2 - 145, h - 70), cv::Point(w / 2 + 145, h - 70),
             cv::Scalar(0, 255, 0), 2);

    cv::line(frame, cv::Point(w / 2 - 120, h - 130), cv::Point(w / 2 + 120, h - 130),
             cv::Scalar(0, 255, 255), 2);

    cv::line(frame, cv::Point(w / 2 - 90, h - 190), cv::Point(w / 2 + 90, h - 190),
             cv::Scalar(0, 0, 255), 2);

    // 3) 左上角状态字
    cv::putText(frame, "Rear Camera", cv::Point(20, 35),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
}

int main(int argc, char* argv[])
{
    gst_init(&argc, &argv);

    // 你的摄像头命令行能跑的是：
    // v4l2src device=/dev/video1 ! video/x-raw, format=YUY2, width=640, height=480, framerate=15/1 ! videoconvert ! fbdevsink
    //
    // 这里改成 appsink 给程序拿帧
    const char* pipelineStr =
        "v4l2src device=/dev/video1 ! "
        "video/x-raw, format=(string)YUY2, width=(int)640, height=(int)480, framerate=(fraction)15/1 ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! "
        "appsink name=m_sink emit-signals=false sync=false max-buffers=1 drop=true";

    GError* error = nullptr;
    GstElement* pipeline = gst_parse_launch(pipelineStr, &error);
    if (!pipeline) {
        std::cerr << "gst_parse_launch failed\n";
        if (error) {
            std::cerr << "error: " << error->message << "\n";
            g_error_free(error);
        }
        return -1;
    }

    GstElement* sinkElem = gst_bin_get_by_name(GST_BIN(pipeline), "m_sink");
    if (!sinkElem) {
        std::cerr << "get appsink failed\n";
        gst_object_unref(pipeline);
        return -1;
    }

    GstAppSink* appSink = GST_APP_SINK(sinkElem);

    gst_app_sink_set_drop(appSink, true);
    gst_app_sink_set_max_buffers(appSink, 1);

    GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        std::cerr << "set pipeline PLAYING failed\n";
        gst_object_unref(sinkElem);
        gst_object_unref(pipeline);
        return -1;
    }

    int sockfd = -1;
    while (sockfd < 0) {
        sockfd = connectServer();
        if (sockfd < 0) {
            std::cout << "viewer not ready, retry after 1s...\n";
            ::sleep(1);
        }
    }

    std::vector<int> jpegParams;
    jpegParams.push_back(cv::IMWRITE_JPEG_QUALITY);
    jpegParams.push_back(80);

    int frameCount = 0;

    while (true) {
        GstSample* sample = gst_app_sink_pull_sample(appSink);
        if (!sample) {
            std::cerr << "pull sample failed\n";
            break;
        }

        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstCaps* caps = gst_sample_get_caps(sample);

        if (!buffer || !caps) {
            std::cerr << "invalid sample\n";
            gst_sample_unref(sample);
            continue;
        }

        GstStructure* s = gst_caps_get_structure(caps, 0);
        int width = 0;
        int height = 0;
        gst_structure_get_int(s, "width", &width);
        gst_structure_get_int(s, "height", &height);

        GstMapInfo map;
        if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
            std::cerr << "gst_buffer_map failed\n";
            gst_sample_unref(sample);
            continue;
        }

        // 这里 caps 已经要求 videoconvert 输出 BGR
        cv::Mat frame(height, width, CV_8UC3, (void*)map.data);

        // clone 一份，避免后面 unmap 后内存失效
        cv::Mat work = frame.clone();

        gst_buffer_unmap(buffer, &map);
        gst_sample_unref(sample);

        if (work.empty()) {
            std::cerr << "empty frame\n";
            continue;
        }

        processFrame(work);

        std::vector<uchar> jpegBuf;
        if (!cv::imencode(".jpg", work, jpegBuf, jpegParams)) {
            std::cerr << "jpeg encode failed\n";
            continue;
        }

        if (!sendFrame(sockfd, jpegBuf)) {
            std::cerr << "send frame failed, reconnect...\n";
            ::close(sockfd);
            sockfd = -1;

            while (sockfd < 0) {
                sockfd = connectServer();
                if (sockfd < 0) {
                    ::sleep(1);
                }
            }
            continue;
        }

        ++frameCount;
        if (frameCount % 30 == 0) {
            std::cout << "sent frames = " << frameCount
                      << ", jpeg bytes = " << jpegBuf.size()
                      << ", frame = " << work.cols << "x" << work.rows
                      << "\n";
        }
    }

    if (sockfd >= 0) {
        ::close(sockfd);
    }

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(sinkElem);
    gst_object_unref(pipeline);

    return 0;
}
