#include "Packet.h"

Packet::Packet(uint ID, uint Head, bool if_divide, int index, uint packet_size, double delay)
	:m_ID(ID), m_Head(Head), m_if_divide(if_divide), m_index(index), m_packet_size(packet_size), m_delay(delay)
{ }

Packet::~Packet()
{ }