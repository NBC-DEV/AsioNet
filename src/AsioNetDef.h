#pragma once

namespace AsioNet {
	// make sure is x64
	//static_assert(sizeof(void*) == 8);
	//using int8 = short;
	//using uint8 = unsigned char;
	//using int16 = short;
	//using uint16 = unsigned short;
	//using int32 = int;
	//using uint32 = unsigned int;
	//using int64 = long long int;
	//using uint64 = unsigned long long int;

	/*
	����:���շ�����
	������:type���շ�����
	��Ա����:m_������
	ģ���������:V_��д
	ģ�����:��д
	��:��д_��д

	type˵����
	int32 ���Σ�����λ��
	uint32 �޷������Σ�����λ��
	k �ṹ��/��
	f float
	lf doubl
	*/
	using namespace boost::asio;
	using namespace boost::system;
	using namespace boost::asio::detail::socket_ops;
	
#define AN_INTERFACE
#define AN_DEFAULT_HANDLER
	struct AN_Msg {
		unsigned short len;	// max 65535 = 1024 * 64
		char data[0];
	};


}
