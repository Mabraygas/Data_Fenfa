/**
 * @file SymbolMonitor.h
 * @brief 流程控制程序
 * @author Yuantu
 * @version ver 1.0
 * @date 2012-11-12
 */

#ifndef _SYMBOL_MONITOR_H_
#define _SYMBOL_MONITOR_H

#include <iostream>

#include "work_client.h"

#define ALL_UPDATE_SYMBOLE	    "AllUpdateSymbole.dat"
#define DONOT_MERGE_PLUS1M 	    "UpdateServiceSymbole.dat" 
#define EXPORT_END_SYMBOLE 	    "ExportCutEnd.dat"
#define TRANSMIT_END_SYMBOLE 	"TransmitEnd.dat"          //tell broker data transmit end
#define BROKER_START_SYMBOLE 	"BrokerStart.dat"          //Update data symbol every time, broker create in distribution endpoint

#define MAX_BASES_NUM           5
#define MAX_IS_NUM              5
#define MAX_BROKERS_NUM         5
#define MAX_GROUPS_NUM          5



using namespace std;

typedef struct EndNode
{
	string       m_strIP;
	uint16_t     m_iPort;
	TCPClient*   m_tcp;
}_End_Node_;

typedef struct GroupInfo
{
	int32_t          m_iGroupID;
	int32_t          m_iCntIS;
	int32_t          m_iCntBroker;
	
	string           m_strListName;
	string           m_strISRoot;
	string           m_strBrokerRoot;
	WorkClient*      m_apIS;
	WorkClient*      m_apBroker;


}*pGroup;

class SymbolMonitor
{
	public:
		SymbolMonitor();
		SymbolMonitor(EndNode* pEndNode);
		~SymbolMonitor();

	public:

		int             setRemote(const string& strIP, const uint32_t iPort);
		int             loadConfig(const char* configFile);
		int             Init(const char* root, const char* configFile);

		//Thread function
		static void*    Thread_CallBack(void* lpvd);
		void            start();
		int             run();
		void            join();

	private:

		pthread_t       m_tid;
		//WorkClient*     m_apBrokers;
		WorkClient*     m_apBases;
		GroupInfo*      m_apGroup;

		//int32_t         m_iCntBrokers;
		int32_t         m_iCntBases;
		int32_t         m_iCntGroup;

		string          m_strRoot;
		string          m_strBaseRoot;
		//string          m_strBrokerRoot;

		//symbol name
		string          m_strAllUpdate; 
		string          m_strUpdateService; 
		//string          m_strTransmitEnd;
		string          m_strBrokerStart;

};
#endif  //end of SymbolMonitor
