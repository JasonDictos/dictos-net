#pragma once

namespace dictos::net::buffer {

template<class BufferType>
class SharedBuffer
{
public:
	SharedBuffer(BufferType buff) :
		m_buff(std::make_shared<BufferType>(std::move(buff)))
	{
	}

	SharedBuffer() = default;

	SharedBuffer(SharedBuffer<BufferType> &&buff)
	{
		operator = (std::move(buff));
	}

	SharedBuffer(const SharedBuffer<BufferType> &buff)
	{
		operator = (buff);
	}

	SharedBuffer<BufferType> & operator = (const SharedBuffer<BufferType> &buff)
	{
		if (this == &buff)
			return *this;

		m_buff = buff.m_buff;
		return *this;
	}

	SharedBuffer<BufferType> & operator = (SharedBuffer<BufferType> &&buff)
	{
		m_buff = std::move(buff.m_buff);
		buff.m_buff = 0;
		return *this;
	}

	Size size() const noexcept { return m_buff->size(); }
	template<class T>
	auto cast() { return m_buff->cast<T>(); }

	template<class T>
	auto cast() const { return m_buff->cast<T>(); }

	const std::byte *begin() const { return m_buff->begin(); }
	const std::byte *end() const { return m_buff->end(); }

	std::byte *begin() { return m_buff->begin(); }
	std::byte *end() { return m_buff->end(); }

protected:
	std::shared_ptr<BufferType> m_buff;
};

}
