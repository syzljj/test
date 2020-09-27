// threadCamera.cpp : 定义控制台应用程序的入口点。
//

#include <dlib/opencv.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/gui_widgets.h>
#include <dlib/dnn.h>
#include <dlib/gui_widgets.h>
#include <dlib/clustering.h>
#include <dlib/string.h>
#include <dlib/image_io.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include<opencv2/core/core.hpp>
#include<cmath>
#include<algorithm>
#include <iostream>
#include <vector>
#include <cmath>
#include<time.h>
#include <iostream>
#include <Windows.h>
#include <thread>
#include <deque>
#include <mutex>
#include<ctime>
#include "stdafx.h"

using namespace std;
using namespace cv;
using namespace dlib;


#include "MyClass.h"
#include "showImg.h"

#define RATIO 4;


std::queue <cv::Mat>  deque_m;

bool flag = true;

std::mutex data_mutex; 

std::condition_variable data_var;// 全局条件变量

//double t=0;
//double fps;


void run_capture()
{
	cv::VideoCapture  capture(0);

	cv::Mat frame;

	while (1)
	{
		capture.read(frame);
		//std::this_thread::sleep_for(std::chrono::seconds(1));
		std::unique_lock<std::mutex> lck(data_mutex);
		data_var.wait(lck, [] {return flag; });
		deque_m.push(frame);
		flag = false;
		data_var.notify_one();  //唤醒线程
		
		//cout << deque_m.size() << std::endl;
	}

	capture.release();
}

void run_queue()
{

	//Initial frame count and total blink
	int eCounter = 0;
	int eTotal = 0;
	//Set judgment parameters
	float EYE_AR_THRESH = 0.18;
	float EYE_AR_CONSEC_FRAME = 3;

	//Initial frame count and total yawns
	int mCounter = 0;
	int mTotal = 0;
	//Yawn aspect ratio, scintillation threshold
	float MOUTH_MAR_CONSEC_FRAMES = 4;
	float MAR_THRESH = 0.7;

	//Store the number of blinks
	unsigned int count_blink = 0;			//blinks

	/*//Each blink of an EAR goes from greater than 0.2- less than 0.2- greater than 0.2
	float blink_EAR_before = 0.0;		//Before the blink of an eye
	float blink_EAR_now = 0.2;			//In the blink of an eye
	float blink_EAR_after = 0.0;			//After the blink of an eye
	*/
	frontal_face_detector detector = get_frontal_face_detector();

	shape_predictor pos_modle;

	////加载模型shape_predictor_68_face_landmarks/shape_predictor_68_face_landmarks.dat  
	deserialize("shape_predictor_68_face_landmarks.dat") >> pos_modle;

	//double ratio = 0;
		
	try
	{
		
		while (1)
		{
			std::unique_lock<std::mutex> lck(data_mutex);
			data_var.wait(lck, [] {return !flag; });

			if (deque_m.empty())
			{
				break;
			}

			else
			{
				cv::Mat  frame = deque_m.front();

				if (frame.empty())
				{
					std::cout << "Could not get the Frame!\t" << endl;
					
					return;
				}
				else
				{
					//将bgr图像转换为dlib图像
					cv_image<bgr_pixel> cimg(frame);


					std::vector<dlib::rectangle> faces = detector(cimg);
					
					std::vector<full_object_detection> shapes;

					unsigned int faceNumber = faces.size();   //Gets the number of vectors in the container the number of faces
					//std::vector<full_object_detection> shapes;

					for (unsigned int i = 0; i < faceNumber; i++)
					{
						shapes.push_back(pos_modle(cimg, faces[i]));

						cv::rectangle(frame, cvRect(faces[i].left(),
							faces[i].top(), faces[i].width(), faces[i].height()), Scalar(0, 0, 255), 1); //人脸画框
					}

					if (!shapes.empty())
					{
						int faceNumber = shapes.size();
						for (int j = 0; j < faceNumber; j++)
						{
							for (int i = 0; i < 68; i++)
							{
								//The point where you draw the eigenvalues
								circle(frame, cvPoint(shapes[j].part(i).x(), shapes[j].part(i).y()), 1, Scalar(0, 0, 255), -1);
								
								//Parameters describe the width of the center line of the image and the type of color line
								//According to digital
								//putText(temp, to_string(i), cvPoint(shapes[0].part(i).x(), shapes[0].part(i).y()), cv::FONT_HERSHEY_PLAIN, 1, Scalar(0, 0, 255));
															

							}
							
						}
						//int shapes_
						


						//left eyes
						int ratio1 = (shapes[0].part(41).y()-shapes[0].part(37).y());
						int ratio2 = (shapes[0].part(40).y() - shapes[0].part(38).y());
						float height_left_eye = (ratio1 + ratio2) / 2;

						int length_left_eye = (shapes[0].part(39).x() - shapes[0].part(36).x());

						//cout << "左眼长度\t" << length_left_eye << endl;
						if (height_left_eye == 0)    //When the eye is closed, the distance may be detected as 0 and the aspect ratio may be wrong
							height_left_eye = 1;

						float EAR_left_eye;			//Eye width to height ratio
						EAR_left_eye = height_left_eye / length_left_eye;

						//cout << "ratio" << EAR_left_eye << endl;

						//right eyes

						int ratio3 = (shapes[0].part(47).y() - shapes[0].part(43).y());
						int ratio4 = (shapes[0].part(46).y() - shapes[0].part(44).y());
						float height_right_eye = (ratio3 + ratio4) / 2;  //Eye up and down distance

						int length_right_eye = (shapes[0].part(45).x() - shapes[0].part(42).x());

						if (height_right_eye == 0)  //When the eye is closed, the distance may be detected as 0 and the aspect ratio may be wrong
							height_right_eye = 1;
						float EAR_right_eye;
						EAR_right_eye= height_right_eye / length_right_eye;

						//Select the average aspect ratio of the two eyes as the aspect ratio of the eyes
						float EAR_eyes = (EAR_left_eye + EAR_right_eye) / 2;

						cout << "The average ratio of width to height of eyes is:" << EAR_eyes << endl;
						double firstTime;

						if (EAR_eyes <= EYE_AR_THRESH)
						{
							//if(firstTime){
							//	begin = std::clock();
							//	starttime2 = (std::clock() - begin) / (double)CLOCKS_PER_SEC;
								//firstTime = false;
								
							//}
							eCounter = eCounter + 1;
						}
						if (eCounter >= EYE_AR_CONSEC_FRAME)
						{
							eTotal = eTotal + 1;
							cout << "The number of blinks:" << eTotal << endl;
							eCounter = 0;

						}

						//mouth
						int ratio5 = (shapes[0].part(58).y() - shapes[0].part(50).y());
						int ratio6 = (shapes[0].part(56).y() - shapes[0].part(52).y());

						float height_mouth = (ratio5 + ratio6) / 2;  //mouth up and down distance

						if (height_mouth == 0)  //When the mouth is closed, the distance may be detected as 0 and the aspect ratio may be wrong
							height_mouth = 1;
						int length_mouth = (shapes[0].part(54).x() - shapes[0].part(48).x());

						float MAR_mouth;			//Mouth aspect ratio
						MAR_mouth = height_mouth / length_mouth;

						cout << "The length-width ratio of the mouth is:" << MAR_mouth << endl;

						//Count yawns
						if (MAR_mouth > MAR_THRESH)			//The yawning threshold was 0.6
						{
							mCounter = mCounter + 1;
						}
						//If the threshold is less than 4 times in a row, one yawn is recorded
						if (mCounter >= MOUTH_MAR_CONSEC_FRAMES)
						{
							mTotal = mTotal + 1; //Threshold value is 3
							cout << "The number of yawns is:" << mTotal << endl;
							mCounter = 0;
						}

						//显示height_left_eye、length_left_eye以及ERA_left_eye

						//Convert the hight left eye from float to string
						char count_blink_text[30];

						_gcvt_s(count_blink_text, eTotal, 10);   //Convert the hight left eye from float to string

						putText(frame, count_blink_text, Point(10, 100), FONT_HERSHEY_COMPLEX, 1.0, Scalar(0, 0, 255), 1, LINE_AA);

						char mTotal_text[30];

						_gcvt_s(mTotal_text, mTotal, 10);
						putText(frame, mTotal_text, Point(10, 150), FONT_HERSHEY_COMPLEX, 1.0, Scalar(125, 0, 0), 1, LINE_AA);

					}
					else
					{
						putText(frame, "Change the sitting position!!!", Point(20, 20), 5, 1, Scalar(255, 255, 100), 1, 8);
						
					}

					if (eTotal >= 1 || mTotal >= 1) 
					{
						//Time one minute (60 seconds)
						clock_t start = clock();
						//clock_t finish = clock();
						double Time = ((double)(clock_t() - start)) / (double)CLOCKS_PER_SEC;//Note the position converted to double

						if (eTotal >= 20)
						{
							if (Time  < 60)
							{
								cout << "You are tired, Please have a rest!\t" << endl;
								eTotal = 0;
								mTotal = 0;
								//cout << "时长1：" << consumeTime << endl;
								Time = 0;
								imshow("You are tired", frame);
								//Beep(5000, 1000);//Make sound prompts
								continue;
							}

						}
						else
						{
							if (Time > 60)
							{
								//cout << "时长2：" << consumeTime << endl;
								Time = 0;
								eTotal = 0;
								mTotal = 0;
								continue;

							}
						}


						if (mTotal >= 8)
						{
							if (Time  < 60)
							{
								cout << "You are tired, Please have a rest!\t" << endl;
								//cout << "时长3：" << consumeTime << endl;
								Time = 0;
								mTotal = 0;
								eTotal = 0;
								imshow("You are tired", frame);
								//Beep(5000, 1000);    //Make sound prompts
								cout << "时间3：" << Time << endl;

								continue;
							}

						}
						else
						{
							if (Time > 60)
							{
								//cout << "时长4：" << consumeTime << endl;
								Time = 0;
								mTotal = 0;
								eTotal = 0;
								continue;
								cout << "时间：" << Time << endl;
							}


						}
					}
					else {
						double Time = 0;
					}
					
					
				}

				deque_m.pop();

				cv::imshow("frame", frame);
				cv::waitKey(1);
				//cv::imshow("Blink waveform figure", Eye_Waveform);

			}
			flag = true;
			data_var.notify_one();
		}


	}

	catch (serialization_error& e) {
		std::cout << "You need dlib‘s default face landmarking file to run this example." << endl;
		std::cout << endl << e.what() << endl;
	}
	
	catch (const std::exception& e)
	{
		std::cout << e.what() << endl;
	}
		
}



int main()
{

	std::thread th1 = std::thread(run_capture);
	std::thread th2 = std::thread(run_queue);
	th1.join();
	th2.join();

	
	//m.start();
	//s.start();

		
	return 0;
}

