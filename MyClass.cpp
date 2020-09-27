
#include "stdafx.h"
#include <opencv\cv.h>
#include <opencv2\opencv.hpp>
#include "MyClass.h"




myClass::myClass(const int & cameraNo, std::queue <cv::Mat>  & deq_Mat) : cameraNo(cameraNo), m_deq(deq_Mat)
{

}


myClass :: ~myClass()
{

}

void myClass::start()
{

}

void myClass::run()
{

}

