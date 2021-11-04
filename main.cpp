#include <iostream>
#include <thread>
#include <chrono>

#include <opencv2/opencv.hpp>

#include <Argus/Argus.h>
#include <EGLStream/EGLStream.h>

using namespace std;
using namespace cv;

int main()
{
    using namespace Argus;

    Argus::Status status;

    auto CP = CameraProvider::create();
    ICameraProvider* icp = static_cast<ICameraProvider*>(CP->getInterface(IID_CAMERA_PROVIDER));
    if(!icp){
        return 1;
    }

    vector<CameraDevice*> devices;
    icp->getCameraDevices(&devices);

    auto sess = icp->createCaptureSession(devices[0]);

    if(!sess){
        return 1;
    }

    auto isess = static_cast<ICaptureSession*>(sess->getInterface(IID_CAPTURE_SESSION));

    auto iprop = interface_cast<ICameraProperties>(devices[0]);

    vector<SensorMode*> smodes;
    status = iprop->getAllSensorModes(&smodes);

    for(auto m: smodes){
        auto sm = interface_cast<ISensorMode>(m);
        auto sz = sm->getResolution();
        auto bd = sm->getInputBitDepth();
        cout << sz.width() << " " << sz.height() << "  " << bd << endl;
    }

    auto smode = interface_cast<ISensorMode>(smodes[0]);
    auto sz = smode->getResolution();
    auto bdo = smode->getOutputBitDepth();
    auto bdi = smode->getInputBitDepth();

    auto sett = isess->createOutputStreamSettings(STREAM_TYPE_EGL);
    auto isett = interface_cast<IEGLOutputStreamSettings>(sett);

    status = isett->setResolution(sz);
    status = isett->setPixelFormat(PIXEL_FMT_RAW16);
    status = isett->setMode(EGL_STREAM_MODE_FIFO);

    OutputStream* stream = isess->createOutputStream(sett);
    auto istream = interface_cast<IEGLOutputStream>(stream);

    auto req = isess->createRequest();
    auto ireq = static_cast<IRequest*>(req->getInterface(IID_REQUEST));

    auto consumer = EGLStream::FrameConsumer::create(stream);
    auto iconsumer = interface_cast<EGLStream::IFrameConsumer>(consumer);

    status = ireq->enableOutputStream(stream);
    //isett->setPixelFormat();

    status = isess->repeat(req);

    status = istream->waitUntilConnected();

    int tmp;

    while(1){
        UniqueObj<EGLStream::Frame> frame(iconsumer->acquireFrame());
        if(frame){
            auto iframe = interface_cast<EGLStream::IFrame>(frame);

            auto image = iframe->getImage();
            auto iimage = interface_cast<EGLStream::IImage>(image);
            auto image2 = interface_cast<EGLStream::IImage2D>(image);

            auto sz = image2->getSize();
            auto t = image2->getStride();

            tmp = iimage->getBufferCount();
            tmp = iimage->getBufferSize();

            auto mapdata = iimage->mapBuffer(&status);

            cv::Mat mat = cv::Mat(sz.height(), sz.width(), CV_16UC1, (void*)mapdata, t);

            imshow("O", mat);
            imwrite("O.bmp", mat/2);

            cout << t << endl;            
        }

        this_thread::sleep_for(chrono::milliseconds(5));
    }

    return 0;
}
