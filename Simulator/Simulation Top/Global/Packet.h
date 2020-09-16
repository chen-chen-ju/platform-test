#pragma once
typedef unsigned int uint;

class Packet
{
public:
	Packet( uint Head = 0,
		bool if_divide = false,
		int index = 0,
		uint packet_size = 256,//单位bit
		double delay = 0 //单位ms
	);
	~Packet();

	uint GetHead()
	{
		return m_Head;
	}
	void SetHead(uint Head)
	{
		m_Head = Head;
	}

	bool Getdivide()
	{
		return m_if_divide;
	}
	void Setdivide(bool if_divide)
	{
		m_if_divide = if_divide;
	}

	int Getindex()
	{
		return m_index;
	}
	void SetHeadsize(int index)
	{
		m_index = index;
	}

	uint GetSize()
	{
		return m_packet_size;
	}
	void SetSize(uint packet_size)
	{
		m_packet_size = packet_size;
	}

	double Getdelay()
	{
		return m_delay;
	}
	void SetSize(double delay)
	{
		m_delay = delay;
	}

	uint Gettotalsize()
	{
		return m_packet_size + 8;//8为RLC头大小，RLC只增加了一个时间戳
	}

private:
	uint m_Head;
	int m_if_divide;
	int m_index;
	uint m_packet_size;
	double m_delay;

};

Packet::Packet(uint Head, bool if_divide, int index, uint packet_size, double delay)
	:m_Head(Head), m_if_divide(if_divide), m_index(index), m_packet_size(packet_size),m_delay(delay)
{ }

Packet::~Packet()
{ }

