#include "Mac_Pdu.h"


Mac_Pdu::Mac_Pdu(uint Head) :m_Head(Head)
{
	Packet a;
	m_Sdu = a;
}
Mac_Pdu::~Mac_Pdu()
{ }