/**
  * @file CountNumber.cpp
  * @author Maguangxu
  * @version ver 1.0
  * @dat 2014-07-23
  */

#include"CountNumber.h"

int CountNumber(int& MAX_LST_NUM, int& MAX_GROUP_NUM, int& MAX_S_NUM_AGROUP, int& FILE_NUM) {
    const char* xmlFile = "soku.xml";
    TiXmlDocument doc;
    if(!doc.LoadFile(xmlFile))
        return -1;
    MAX_GROUP_NUM = MAX_LST_NUM = MAX_S_NUM_AGROUP = FILE_NUM = 0;
    int i = 0, j = 0, k = 0, p = 0, q = 0;
    int file_find = 0;
    TiXmlElement* rootElement = doc.RootElement();
    TiXmlAttribute* attributeOfSoku = rootElement->FirstAttribute();

    if(strcmp("ListNumber", attributeOfSoku->Name()) == 0)
        MAX_LST_NUM = attributeOfSoku->IntValue();

    TiXmlElement* ListElement = rootElement->FirstChildElement();
    for(i = 0 ; ListElement != NULL; ListElement = ListElement->NextSiblingElement(), i++) {
        file_find = 0;
        TiXmlAttribute* attributeOfList = ListElement->FirstAttribute();
        for(; attributeOfList != NULL; attributeOfList = attributeOfList->Next()) {
            if(strcmp(attributeOfList->Name(), "GroupNumber") == 0)
                MAX_GROUP_NUM = MAX_GROUP_NUM > attributeOfList->IntValue() ? MAX_GROUP_NUM : attributeOfList->IntValue();
            if(strcmp(attributeOfList->Name(), "Source") == 0) {   //Source indicates that Element if "FILE" not "List"
                file_find = 1;
                break;
            }
        }
        if(file_find)
            break;
        TiXmlElement* GroupElement = ListElement->FirstChildElement();
        for(j = 0; GroupElement != NULL; GroupElement = GroupElement->NextSiblingElement(), j++) {
            TiXmlAttribute* attributeOfGroup = GroupElement->FirstAttribute();
            for(; attributeOfGroup != NULL; attributeOfGroup = attributeOfGroup->Next()) {
                if(strcmp(attributeOfGroup->Name(), "ServerNumber") == 0)
                    MAX_S_NUM_AGROUP = MAX_S_NUM_AGROUP > attributeOfGroup->IntValue() ? MAX_S_NUM_AGROUP : attributeOfGroup->IntValue();
            }
            TiXmlElement* ServerElement = GroupElement->FirstChildElement();
            for(k = 0; ServerElement != NULL; ServerElement = ServerElement->NextSiblingElement(), k++) {
                //Nothing
            }
            MAX_S_NUM_AGROUP = k > MAX_S_NUM_AGROUP ? k : MAX_S_NUM_AGROUP;
        }
        MAX_GROUP_NUM = j > MAX_GROUP_NUM ? j : MAX_GROUP_NUM;
    }
    MAX_LST_NUM = i > MAX_LST_NUM ? i : MAX_LST_NUM;
    
    for(p = 0; ListElement != NULL; ListElement = ListElement->NextSiblingElement(), p++) {
        TiXmlAttribute* attributeOfFile = ListElement->FirstAttribute();
        for(; attributeOfFile != NULL; attributeOfFile = attributeOfFile->Next()) {
            if(strcmp(attributeOfFile->Name(), "ServerNumber") == 0) {
                MAX_S_NUM_AGROUP = attributeOfFile->IntValue() > MAX_S_NUM_AGROUP ? attributeOfFile->IntValue() : MAX_S_NUM_AGROUP;
            }
        }
        TiXmlElement* ServerElement = ListElement->FirstChildElement();
        for(q = 0; ServerElement != NULL; ServerElement = ServerElement->NextSiblingElement(), q++) {
            //do nothing
        }
        MAX_S_NUM_AGROUP = q > MAX_S_NUM_AGROUP ? q : MAX_S_NUM_AGROUP;
    }

    TiXmlAttribute* attributeOfFile = rootElement->FirstAttribute()->Next();
    if(strcmp(attributeOfFile->Name(), "FileNumber") == 0)
        FILE_NUM = p > attributeOfFile->IntValue() ? p : attributeOfFile->IntValue();

    return 0;
}

