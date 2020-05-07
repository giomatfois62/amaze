#ifndef MATRIX_H
#define MATRIX_H

#include <vector>

template<class T, size_t N>
class Matrix {
public:
    Matrix() : m_data(nullptr) {}

    template < typename... Args >
    Matrix(size_t value, Args... args)
    {
        processDimensions(value, args...);
        allocate();
    }

    Matrix(const Matrix<T,N> &other)
    {
        m_dims = other.m_dims;
        allocate();
    }

    ~Matrix()
    {
        if(m_data)
            delete[] m_data;
    }

    Matrix& operator=(const Matrix<T,N> &other)
    {
        if(m_data)
            delete[] m_data;

        this->m_dims = other.m_dims;
        this->allocate();

        return *this;
    }

    template < typename... Args >
    T& operator()(size_t value, Args... args)
    {
        size_t index = computeIndex(0, value, args...);

        return m_data[index];
    }

    size_t rows() const
    {
        return (m_dims.size() > 0) ? m_dims[0] : 0;
        }

        size_t cols() const
        {
        if(!m_dims.size())
        return 0;

        if(m_dims.size() == 1)
            return 1;

        return m_dims[1];
    }

    size_t elementsCount() const
    {
        size_t totalSize = 1;

        for(size_t i = 0; i < m_dims.size(); ++i)
            totalSize *= m_dims[i];

        return m_dims.size() > 0 ? totalSize : 0;
    }

    T* data()
    {
        return m_data;
    }

    T* data() const
    {
        return m_data;
    }

    void fill(const T &val)
    {
        for(size_t i = 0; i < elementsCount(); i++)
            m_data[i] = val;
    }

    void swap(Matrix<T,N> &other)
    {
        std::swap(other.m_data, m_data);
        m_dims.swap(other.m_dims);
    }

    std::vector<size_t> dims()
    {
        return m_dims;
    }

private:
    T* m_data;
    std::vector<size_t> m_dims;

    void allocate()
    {
        size_t count = elementsCount();

        if(count > 0)
            m_data = new T[count];
    }

    template < typename... Args >
    void processDimensions(size_t value, Args... args)
    {
        m_dims.push_back(value);
        processDimensions(args...);
    }

    void processDimensions() {}

    template < typename... Args >
    size_t computeIndex(size_t current, size_t value, Args... args)
    {
        size_t factor = 1;

        for(size_t c = current + 1; c < m_dims.size(); ++c)
            factor *= m_dims[c];

        size_t index = value * factor;

        return index + computeIndex(current + 1, args...);
    }

    size_t computeIndex(size_t current)
    {
        return 0;
    }
};


#endif
