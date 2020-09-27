#pragma once

#include  <thread>
#include <queue>
#include <mutex>


using namespace std;



class myClass
{
public:
	myClass(const int & cameraNo, std::queue <cv::Mat>  & deq_Mat);
	virtual ~myClass();


	void start();
	void run();


private:
	std::thread threadNo;
	int  cameraNo;
	std::queue <cv::Mat > m_deq;
	std::mutex  m_mtx;
};