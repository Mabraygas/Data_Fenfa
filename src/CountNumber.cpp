/**
  * @file CountNumber.cpp
  * @author Maguangxu
  * @version ver 1.0
  * @dat 2014-07-23
  */

#include"CountNumber.h"

int CountNumber(int& MAX_LST_NUM, int& MAX_GROUP_NUM, int& MAX_S_NUM_AGROUP) {
    const char* xmlFile = "soku.xml";
    TiXmlDocument doc;
    if(!doc.LoadFile(xmlFile))
        return -1;
    int i = 0, j = 0, k = 0;
    TiXmlElement* rootElement = doc.RootElement();
    TiXmlAttribute* attributeOfSoku = rootElement->FirstAttribute();

    if(strcmp("ListNumber", attributeOfSoku->Name()) == 0)
        MAX_LST_NUM = attributeOfSoku->IntValue();

    TiXmlElement* ListElement = rootElement->FirstChildElement();
    for(i = 0 ; ListElement != NULL; ListElement = ListElement->NextSiblingElement(), i++) {
        TiXmlAttribute* attributeOfList = ListElement->FirstAttribute();
        for(; attributeOfList != NULL; attributeOfList = attributeOfList->Next()) {
            if(strcmp(attributeOfList->Name(), "GroupNumber") == 0)
                MAX_GROUP_NUM = attributeOfList->IntValue();
        }
        TiXmlElement* GroupElement = ListElement->FirstChildElement();
        for(j = 0; GroupElement != NULL; GroupElement = GroupElement->NextSiblingElement(), j++) {
            TiXmlAttribute* attributeOfGroup = GroupElement->FirstAttribute();
            for(; attributeOfGroup != NULL; attributeOfGroup = attributeOfGroup->Next()) {
                if(strcmp(attributeOfGroup->Name(), "ServerNumber") == 0)
                    MAX_S_NUM_AGROUP = attributeOfGroup->IntValue();
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

    return 0;
}

