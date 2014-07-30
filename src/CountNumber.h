/**
 * @file CountNumber.h
 * @brief 读配置文件L G S数,作为动态申请内存的维数，节省内存
 * @author Maguangxu
 * @version ver 1.0
 * @date 2014-07-22
 */

#if !defined(_COUNT_NUMBER_H)
#define _COUNT_NUMBER_H

#include<string>
#include<iostream>
#include<tinyxml.h>

/**
  * @brief 传入函数四个int变量，即可得到配置文件中各部分数量
  */
int CountNumber(int& MAX_LST_NUM, int& MAX_GROUP_NUM, int& MAX_S_NUM_AGROUP, int& FILE_NUM);

#endif

