#ifndef PACKET_H
#define PACKET_H

typedef unsigned int uint;

#define Packet_size  264//正常包大小，算上头部

class Packet
{
public:
	Packet(int ID = -1,
		uint Head = 8,//RLC层head为8，MAC层head为23
		bool if_divide = false,
		int index = -1,
		uint packet_size = 256,//单位bit
		double delay = 0 //单位ms
	);
	~Packet();

	int GetID()
	{
		return m_ID;
	}
	void SetID(int ID)
	{
		m_ID = ID;
	}


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
	void Setindex(int index)
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
	void Setdelay(double delay)
	{
		m_delay = delay;
	}
	void Adddelay(double delay)
	{
		m_delay += delay;
	}

	uint Gettotalsize()
	{
		return m_packet_size + m_Head;//8为RLC头大小，RLC只增加了一个时间戳
	}

private:
	int m_ID;
	uint m_Head;//头长度
	int m_if_divide;
	int m_index;
	uint m_packet_size;
	double m_delay;

};

#endif
