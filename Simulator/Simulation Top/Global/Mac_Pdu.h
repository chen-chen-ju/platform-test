#pragma once
#include "packet.h"
class Mac_Pdu
{
public:
	Mac_Pdu(uint Head=0
	);
	~Mac_Pdu();
	uint GetHead()
	{
		return m_Head;
	}
	void SetHead(uint Head)
	{
		m_Head = Head;
	}

	Packet GetSdu()
	{
		return m_Sdu;
	}
	void SetSdu(Packet Sdu)
	{
		m_Sdu = Sdu;
	}


private:
	uint m_Head;
	Packet m_Sdu;
};

Mac_Pdu::Mac_Pdu(uint Head):m_Head(Head)
{ 
	Packet a;
	m_Sdu = a;
}
Mac_Pdu::~Mac_Pdu()
{ }