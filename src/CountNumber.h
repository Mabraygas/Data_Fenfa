/**
 * @file CountNumber.h
 * @brief 读配置文件L G S数
 * @author Maguangxu
 * @version ver 1.0
 * @date 2014-07-22
 */

#if !defined(_COUNT_NUMBER_H)
#define _COUNT_NUMBER_H

#include<string>
#include<iostream>
#include<tinyxml.h>


int CountNumber(int& MAX_LST_NUM, int& MAX_GROUP_NUM, int& MAX_S_NUM_AGROUP);
/*
static int CountNumber(int& MAX_LST_NUM, int& MAX_GROUP_NUM, int& MAX_S_NUM_AGROUP) {
    const char* xFile = "soku.xml";
    MAX_LST_NUM = 50;
    MAX_GROUP_NUM = 3;
    MAX_S_NUM_AGROUP = 30;
    TiXmlDocument doc;
    if(!doc.LoadFile(xFile))
        return -1; //文件读取错误
    int i = 0, j = 0, k = 0;
    TiXmlElement* rootElement = doc.RootElement();
    TiXmlAttribute* attributeOfSoku = rootElement->FirstAttribute();

    if(strcmp("ListNumber", attributeOfSoku->Name()) == 0)
        MAX_LST_NUM = attributeOfSoku->IntValue();

    TiXmlElement* ListElement = rootElement->FirstChildElement();
    for(i = 0; ListElement != NULL ; ListElement = ListElement->NextSiblingElement(), i++)
    {
        TiXmlAttribute* attributeOfList = ListElement->FirstAttribute();
        for(; attributeOfList != NULL; attributeOfList = attributeOfList->Next())
        {
            if(strcmp(attributeOfList->Name(), "GroupNumber") == 0)
                MAX_GROUP_NUM = attributeOfList->IntValue();
        }
        TiXmlElement* GroupElement = ListElement->FirstChildElement();
        for(j = 0; GroupElement != NULL ; GroupElement = GroupElement->NextSiblingElement(), j++)
        {
            TiXmlAttribute* attributeOfGroup = GroupElement->FirstAttribute();
            for(; attributeOfGroup != NULL; attributeOfGroup = attributeOfGroup->Next())
            {
                if(strcmp(attributeOfGroup->Name(), "ServerNumber") == 0)
                    MAX_S_NUM_AGROUP = attributeOfGroup->IntValue();
            }
            TiXmlElement* ServerElement = GroupElement->FirstChildElement();
            for(k = 0; ServerElement != NULL; ServerElement = ServerElement->NextSiblingElement(), k++)
            {
                //NOTHING
            }
            MAX_S_NUM_AGROUP = k > MAX_S_NUM_AGROUP ? k : MAX_S_NUM_AGROUP;
        }
        MAX_GROUP_NUM = j > MAX_GROUP_NUM ? j : MAX_GROUP_NUM;
    }
    MAX_LST_NUM = i > MAX_LST_NUM ? i : MAX_LST_NUM;
    
    return 0; //Success
}
*/

#endif

