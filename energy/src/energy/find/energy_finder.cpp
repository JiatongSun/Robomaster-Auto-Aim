﻿//
// Created by xixiliadorabarry on 1/24/19.
//
#include "energy/energy.h"

using namespace cv;
using std::cout;
using std::endl;
using std::vector;

int Energy::findFan(const cv::Mat &src, vector<EnergyPart> &fans, int &last_fans_cnt) {
    if (src.empty())return 0;
    static Mat src_bin;
    src_bin = src.clone();
//    threshold(src, src_bin, energy_part_param_.FAN_GRAY_THRESH, 255, THRESH_BINARY);
    if(src.type() == CV_8UC3){
        cvtColor(src_bin, src_bin, CV_BGR2GRAY);
    }
	std::vector<vector<Point> > fan_contours;

	StructingElementClose(src_bin);
//	imshow("fan struct",src_bin);

	findContours(src_bin, fan_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for (auto &fan_contour : fan_contours) {
		if (!isValidFanContour(fan_contour)) {
			continue;
		}

//        double cur_contour_area = contourArea(fan_contour);
//        RotatedRect cur_rect = minAreaRect(fan_contour);
//        Size2f cur_size = cur_rect.size;
//
//		  cout<<"cur_contour_area: "<<cur_contour_area<<'\t'<<"rect_area: "<<cur_size.area()<<'\t'<<"ratio: "<<cur_contour_area/cur_size.area()<<endl;

//        float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
//        float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;

//		if(length>5&&width>5){
//			cout<<cur_rect.center;
//			fans.emplace_back(fan_contour);
//			cout<<"fan area: "<<length<<'\t'<<width<<endl;
//		}

        fans.emplace_back(fan_contour);
//        cout<<"fan area: "<<length<<'\t'<<width<<endl;

	}
	if(fans.size() < last_fans_cnt){
		last_fans_cnt = static_cast<int>(fans.size());
		return -1;
	}
	last_fans_cnt = static_cast<int>(fans.size());
	return static_cast<int>(fans.size());
}



int Energy::findArmor(const cv::Mat &src, vector<EnergyPart> &armors, int &last_armors_cnt) {
	if (src.empty())return 0;
    static Mat src_bin;
    src_bin = src.clone();
//    threshold(src, src_bin, energy_part_param_.ARMOR_GRAY_THRESH, 255, THRESH_BINARY);
    if(src.type() == CV_8UC3){
        cvtColor(src_bin, src_bin, CV_BGR2GRAY);
    }
	std::vector<vector<Point> > armor_contours;
    std::vector<vector<Point> > armor_contours_external;//用总轮廓减去外轮廓，只保留内轮廓，除去流动条的影响。

    StructingElementClose(src_bin);
//    imshow("armor struct",src_bin);

	findContours(src_bin, armor_contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
    findContours(src_bin, armor_contours_external, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    for (int i = 0; i < armor_contours_external.size(); i++)//去除外轮廓
    {
        unsigned long external_contour_size = armor_contours_external[i].size();
        for (int j = 0; j < armor_contours.size(); j++)
        {
            unsigned long all_size = armor_contours[j].size();
            if (external_contour_size == all_size)
            {
                swap(armor_contours[j], armor_contours[armor_contours.size() - 1]);
                armor_contours.pop_back();
                break;
            }
        }
    }

    for (auto &armor_contour : armor_contours) {
		if (!isValidArmorContour(armor_contour))
		{
			continue;
		}

        RotatedRect cur_rect = minAreaRect(armor_contour);
        Size2f cur_size = cur_rect.size;
        float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
        float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;

//        if(length>10&&width>10){
//            armors.emplace_back(armor_contour);
//            cout<<"armor area: "<<length<<'\t'<<width<<endl;
//        }
        armors.emplace_back(armor_contour);
        cout<<"armor area: "<<length<<'\t'<<width<<endl;

	}
	if(armors.size() < last_armors_cnt){
		last_armors_cnt = static_cast<int>(armors.size());
		return -1;
	}
	last_armors_cnt = static_cast<int>(armors.size());
	return static_cast<int>(armors.size());
}

int Energy::findGimbleZeroPoint(const cv::Mat &src, vector<EnergyPart> &gimble_zero_points) {
    if (src.empty())return 0;
    static Mat src_bin;
    src_bin = src.clone();
//    threshold(src, src_bin, energy_part_param_.FAN_GRAY_THRESH, 255, THRESH_BINARY);
    if(src.type() == CV_8UC3){
        cvtColor(src_bin, src_bin, CV_BGR2GRAY);
    }
    std::vector<vector<Point> > zero_point_contours;

    findContours(src_bin, zero_point_contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    for (auto &zero_point_contour : zero_point_contours) {

        double cur_contour_area = contourArea(zero_point_contour);
        RotatedRect cur_rect = minAreaRect(zero_point_contour);
        Size2f cur_size = cur_rect.size;

//        cout<<"cur_contour_area: "<<cur_contour_area<<'\t'<<"rect_area: "<<cur_size.area()<<'\t'<<"ratio: "<<cur_contour_area/cur_size.area()<<endl;

        float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
        float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;

		if(length<10&&width<10&&length>1&&width>1){
			cout<<"zero point center: "<<cur_rect.center<<endl;
			cout<<"zero point area: "<<length<<'\t'<<width<<endl;
			gimble_zero_points.emplace_back(zero_point_contour);
		}

    }

    return static_cast<int>(fans.size());
}

bool Energy::isValidFanContour(const vector<cv::Point> &fan_contour) {
	double cur_contour_area = contourArea(fan_contour);
	if (cur_contour_area > energy_part_param_.FAN_CONTOUR_AREA_MAX ||
		cur_contour_area < energy_part_param_.FAN_CONTOUR_AREA_MIN)
	{
		//cout<<cur_contour_area<<" "<<energy_fan_param_.CONTOUR_AREA_MIN<<" "<<energy_fan_param_.CONTOUR_AREA_MAX<<endl;
		//cout<<"area fail."<<endl;
		return false;
	}

	RotatedRect cur_rect = minAreaRect(fan_contour);
	Size2f cur_size = cur_rect.size;
	float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
	float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;
	if (length < energy_part_param_.FAN_CONTOUR_LENGTH_MIN || width < energy_part_param_.FAN_CONTOUR_WIDTH_MIN)
	{
		//cout<<"length width min fail."<<endl;
		return false;
	}
//	float length_width_ratio = length / width;
//	if (length_width_ratio > energy_part_param_.FAN_CONTOUR_HW_RATIO_MAX ||
//		length_width_ratio < energy_part_param_.FAN_CONTOUR_HW_RATIO_MIN)
//	{
//		//cout<<"length width ratio fail."<<endl;
//		return false;
//	}
	if (cur_contour_area / cur_size.area() < 0.6) return false;

	return true;
}



bool Energy::isValidArmorContour(const vector<cv::Point> &armor_contour) {
	double cur_contour_area = contourArea(armor_contour);
//	if (cur_contour_area > energy_part_param_.ARMOR_CONTOUR_AREA_MAX ||
//		cur_contour_area < energy_part_param_.ARMOR_CONTOUR_AREA_MIN)
//	{
//		//cout<<cur_contour_area<<" "<<energy_fan_param_.CONTOUR_AREA_MIN<<" "<<energy_fan_param_.CONTOUR_AREA_MAX<<endl;
//		//cout<<"area fail."<<endl;
//		return false;
//	}
	RotatedRect cur_rect = minAreaRect(armor_contour);
	Size2f cur_size = cur_rect.size;
	float length = cur_size.height > cur_size.width ? cur_size.height : cur_size.width;
	float width = cur_size.height < cur_size.width ? cur_size.height : cur_size.width;
	if (length < energy_part_param_.ARMOR_CONTOUR_LENGTH_MIN || width < energy_part_param_.ARMOR_CONTOUR_WIDTH_MIN)
	{
		//cout<<"length width min fail."<<endl;
		return false;
	}

	if (length > energy_part_param_.ARMOR_CONTOUR_LENGTH_MAX||width>energy_part_param_.ARMOR_CONTOUR_WIDTH_MAX)
	{
		//cout<<"length width max fail."<<endl;
		return false;
	}

	float length_width_ratio = length / width;
	if (length_width_ratio > energy_part_param_.ARMOR_CONTOUR_HW_RATIO_MAX ||
		length_width_ratio < energy_part_param_.ARMOR_CONTOUR_HW_RATIO_MIN)
	{
		//cout<<"length width ratio fail."<<endl;
		return false;
	}
	if (cur_contour_area / cur_size.area() < 0.7) return false;

	return true;
}

