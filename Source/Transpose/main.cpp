#include <iostream>

using namespace std;

class Matrix
{
	friend ostream& operator<<(ostream &os, const Matrix &m);

private:
	int rows;
	int cols;
	float* mat;

public:
	Matrix(int r, int c) :
		rows(r), cols(c), mat(nullptr)
	{
		mat = new float[rows*cols]();
	}

	Matrix() : Matrix(2, 2) {}

	~Matrix()
	{
		delete[] mat;
	}

	Matrix(const Matrix &m) : 
		rows(m.rows), cols(m.cols), mat(nullptr)
	{
		mat = new float[m.rows*m.cols];
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < cols; ++j)
				mat[i + cols*j] = m.mat[j + m.cols*i];
	}

	Matrix& operator=(const Matrix &rhs)
	{
		float* newMat = new float[rhs.rows*rhs.cols];
		for (int i = 0; i < rhs.rows; ++i)
			for (int j = 0; j < rhs.cols; ++j)
				newMat[j + rhs.cols*i] = rhs.mat[j + rhs.cols*i];
		delete[] mat;
		mat = newMat;
		rows = rhs.rows;
		cols = rhs.cols;
		
		return *this;
	}

	float operator()(int r, int c) const
	{
		if (r >= 0 && r < rows && c >= 0 && c < cols)
			return mat[c + cols*r];
		else
			return 0.0f;
	}

	float& operator()(int r, int c)
	{
		return mat[c + cols*r];
	}

	int numRows() { return rows; }
	int numCols() { return cols; }


	Matrix transpose()
	{
		Matrix t(this->cols, this->rows);

		for (int i = 0; i < t.rows; ++i)
			for (int j = 0; j < t.cols; ++j)
				t(i,j) = (*this)(j,i);

		return t;
	}
};


ostream& operator<<(ostream &os, const Matrix &m)
{
	for (int i = 0; i < m.rows; ++i)
	{
		for (int j = 0; j < m.cols; ++j)
		{
			os << m(i,j) << "\t";
		}
		os << endl;
	}
	return os;
}




int main()
{
	Matrix m(4, 7);
	
	for (int i = 0; i < m.numRows(); ++i)
		for (int j = 0; j < m.numCols(); ++j)
			m(i, j) = j + m.numCols()*i;

	cout << m << endl << endl;
	cout << m.transpose() << endl;

	return 0;
}