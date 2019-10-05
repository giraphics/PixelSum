#pragma once

#include "MemoryAllocator.h"

// An Image helper class that hold the pixel buffer.
class Image
{
public:
    Image(int p_Width, int p_Height)
        : m_Width(p_Width)
        , m_Height(p_Height)
    {
        m_Buffer = m_MemoryAllocator->Allocate(m_Width * m_Height);
    }

    ~Image()
    {
        m_MemoryAllocator->Free(m_Buffer);
    }

    unsigned char* GetPixelBufferPtr() { return reinterpret_cast<unsigned char*>(m_Buffer); }
    void PrintData();

private:
    VM::MemoryAllocator* m_MemoryAllocator = &VM::MemoryAllocator::GetInstance();
    void* m_Buffer = nullptr;

    int m_Width  = 0;
    int m_Height = 0;
};

void Image::PrintData()
{
    std::cout << "Image Data \n---------------- ";
    const unsigned char* imageData = reinterpret_cast<unsigned char*>(m_Buffer);
    for (int row = 0; row < m_Height; row++)
    {
        std::cout << std::endl;
        for (int col = 0; col < m_Width; col++)
        {
            std::cout << "[" << (int)imageData[row * m_Width + col] << "]" << ", ";
        }
    }
}
