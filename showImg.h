#pragma once

#include  <thread>
#include <queue>

#include <opencv2/opencv.hpp>


using namespace std;



class showClass
{
public:
	showClass(std::queue <cv::Mat>  & deq_Mat);

	virtual ~showClass();


	void start();
	void run();


private:
	std::thread threadNo;
	std::queue <cv::Mat > m_deq;
};