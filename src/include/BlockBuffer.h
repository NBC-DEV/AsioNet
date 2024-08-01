#pragma once

#include "MemPool.h"
#include <queue>

template<size_t V_BUFFER_SIZE>
struct BlockElem {
	// ����д�����Block��ֱ��bufferд��
	// ����ʵ�ʿ�����������
	size_t Write(const char* data, size_t trans)
	{
		size_t remain = V_BUFFER_SIZE >= wpos ? V_BUFFER_SIZE - wpos : 0;
		if (remain)
		{
			size_t copied = trans;
			if (remain < trans)	// ʣ��Ĳ�����
			{
				copied = remain;	// ֻ�ܿ�����ô��
			}
			memcpy_s(buffer + wpos, remain, data, copied);
			wpos += copied;
			return copied;
		}
		return 0;
	}

	bool IsFull()
	{
		return wpos >= V_BUFFER_SIZE;
	}

	char buffer[V_BUFFER_SIZE];
	size_t wpos;	// дƫ��
	BlockElem<V_BUFFER_SIZE>* next;
	bool done;
};

template<size_t V_BUFFER_SIZE/*ÿ��buffer�Ĵ�С*/,
			size_t V_EXTEND_NUM/*ÿ������Ĵ�С*/>
class BlockSendBuffer {
public:
	// ÿ�����伸��buffer
	BlockSendBuffer() :
		m_pool(V_EXTEND_NUM), head(nullptr), tail(nullptr), detachedHead(nullptr)
	{}
	~BlockSendBuffer()
	{}
	bool Empty()
	{
		return head == nullptr;
	}
	BlockElem<V_BUFFER_SIZE>* DetachHead()
	{
		if (!detachedHead && head) {
			detachedHead = head;
			if (tail && tail == detachedHead) {
				tail = tail->next;	// nil
			}
			head = head->next;	// could be nil
			return detachedHead;
		}
		return nullptr;
	}
	void FreeDeatched()
	{
		if (detachedHead)
		{
			m_pool.Del(detachedHead);
			detachedHead = nullptr;
		}
	}
	// ���ݿ��ܷ�ɢ�ڶ��block��
	bool Push(const char* data, size_t trans)
	{
		if (trans <= 0)
		{
			return false;
		}

		if (!tail)
		{
			tail = m_pool.New();	// this will memset the block
			if (!head)	// first push
			{
				head = tail;
			}
		}

		size_t copied = 0;
		do {
			copied += tail->Write(data + copied, trans - copied);

			if (tail->IsFull())	// �����˾��ٷ���һ����
			{
				tail->next = m_pool.New();
				tail = tail->next;
			}
		} while (copied < trans);
		return true;
	}

	void Clear()
	{
		m_pool.Clear();
		detachedHead = head = tail = nullptr;
	}
private:
	BlockElem<V_BUFFER_SIZE>* head, * tail, * detachedHead;
	MemPool_ThreadUnsafe<BlockElem<V_BUFFER_SIZE>> m_pool;
};


template<size_t V_BUFFER_SIZE>
struct BlockElem_1 {
	// ���Խ�����ȫ��д�����Block�����ʣ��buffer���㣬��д��
	size_t WriteAll(const char* data, size_t trans)
	{
		size_t remain = V_BUFFER_SIZE >= wpos ? V_BUFFER_SIZE - wpos : 0;
		if (remain >= trans)
		{
			memcpy_s(buffer + wpos, remain, data, trans);
			wpos += trans;
			return trans;
		}
		return 0;
	}
	bool IsFull()
	{
		return wpos >= V_BUFFER_SIZE;
	}
	char* Read(size_t r)
	{
		char* data = buffer + rpos;
		rpos += r;
		return data;
	}
	bool Empty()
	{
		return rpos >= wpos;
	}
	bool IsDone()
	{
		return done;
	}
	void Done()
	{
		done = true;
	}
	char buffer[V_BUFFER_SIZE];
	size_t wpos;	// дƫ��
	size_t rpos;	// ��ƫ��,max = V_BUFFER_SIZE
	BlockElem_1<V_BUFFER_SIZE>* next;
	bool done;
};

template<size_t V_BUFFER_SIZE/*ÿ��buffer�Ĵ�С*/,
			size_t V_EXTEND_NUM/*ÿ������Ĵ�С*/>
class BlockBuffer {
public:
	// ÿ�����伸��buffer
	BlockBuffer() :
		m_pool(V_EXTEND_NUM),head(nullptr), tail(nullptr)
	{}
	~BlockBuffer()
	{}
	bool Empty()
	{
		return head == nullptr;
	}

	// ��֤������һ��block��
	bool Push(const char* data, size_t trans)
	{
		// ��֤д��Ĵ�С��һ��block���ܷŵ���
		if (trans <= 0 || trans > V_BUFFER_SIZE)
		{
			return false;
		}
		if (!tail)
		{
			tail = m_pool.New();	// this will memset the block
			if (!head)	// first push
			{
				head = tail;
			}
		}

		if(tail->WriteAll(data,trans) != trans)
		{
			tail->Done();
			tail->next = m_pool.New();
			tail = tail->next;
			tail->WriteAll(data,trans);
		}
		que.push(trans);
		return true;
	}

	// ������д�봫����buffer��
	bool Pop(char** in,size_t* s)
	{
		if (!head || que.empty())
		{
			return false;
		}
		size_t len = que.front();
		char* buf = head->Read(len);
		memcpy_s(*in,len,buf,len);
		*s = len;

		if(head->IsDone() && head->Empty())
		{
			auto p = head;
			m_pool.Del(p);
			head = head->next;
		}

		que.pop();
		return true;
	}

	std::string PopToString()
	{
		if (!head || que.empty())
		{
			return "";
		}
		size_t len = que.front();
		char* buf = head->Read(len);
		if(head->IsDone() && head->Empty())
		{
			auto p = head;
			m_pool.Del(p);
			head = head->next;
		}
		que.pop();
		return std::string(buf,len);
	}
private:
	BlockElem_1<V_BUFFER_SIZE>* head, * tail;
	MemPool_ThreadUnsafe<BlockElem_1<V_BUFFER_SIZE>> m_pool;
	std::queue<size_t> que;
};
