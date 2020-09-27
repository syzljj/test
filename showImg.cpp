#include "stdafx.h"
#include "showImg.h"



showClass::showClass(std::queue <cv::Mat>  & deq_Mat) : m_deq(deq_Mat)
{

}



showClass :: ~showClass()
{

}



void showClass::run()
{

}



void showClass::start()
{
	threadNo = std::thread(&showClass::run, this);
	threadNo.join();
}