#include <iostream>

using namespace std;

class Matrix
{
	friend ostream& operator<<(ostream &os, const Matrix &m);

private:
	int rows;
	int cols;
	float* mat;

	Matrix(int r, int c) :
		rows(r), cols(c), mat(nullptr)
	{
		mat = new float[rows*cols]();
	}

public:
	Matrix() : Matrix(4, 4) {}

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
				mat[j + cols*i] = m.mat[j + m.cols*i];
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

	Matrix minor(int r, int c)
	{
		Matrix m(rows - 1, cols - 1);

		for (int i = 0, mi = 0; i < rows; ++i)
		{
			if (i == r)
				continue;

			for (int j = 0, mj = 0; j < cols; ++j)
			{
				if (j != c)
				{
					float value = (*this)(i, j);
					m(mi, mj) = value;
					++mj;
				}
			}

			++mi;
		}

		return m;
	}

	float determinant()
	{
		if (rows == 1 && cols == 1)
			return mat[0];
		else
		{
			float det = 0;
			for (int j = 0; j < cols; ++j)
			{
				float factor = (j % 2 == 0) ? 1.0f : -1.0f;

				det += (*this)(0, j) * factor * minor(0, j).determinant();
			}
			return det;
		}
	}

	Matrix inverse()
	{
		Matrix cofactor;
		float det = (*this).determinant();

		if (det == 0)
			return cofactor;

		for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				float factor = ((i + j) % 2 == 0) ? 1.0f : -1.0f;

				cofactor(i, j) = factor * this->minor(i, j).determinant();
			}
		}
		
		Matrix inverse = cofactor.transpose();

		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < cols; ++j)
				inverse(i, j) = cofactor(j, i) / det;

		return inverse;
	}

	Matrix mul(const Matrix& m)
	{
		Matrix result(this->rows, m.cols);

		if (this->cols != m.rows)
			return result;

		for (int i = 0; i < this->rows; ++i)
		{
			for (int j = 0; j < m.cols; ++j)
			{
				for (int k = 0; k < this->cols; ++k)
				{
					result(i, j) += (*this)(i, k)*m(k, j);
				}
			}
		}

		return result;
	}
};


ostream& operator<<(ostream &os, const Matrix &m)
{
	for (int i = 0; i < m.rows; ++i)
	{
		for (int j = 0; j < m.cols; ++j)
		{
			os << m.mat[j + m.cols*i] << "\t";
		}
		os << endl;
	}
	return os;
}




int main()
{
	Matrix m;

	m(0, 0) = 1.0f;
	m(1, 1) = 2.0f;
	m(2, 2) = 4.0f;
	m(3, 0) = 1.0f;
	m(3, 1) = 2.0f;
	m(3, 2) = 3.0f;
	m(3, 3) = 1.0f;

	cout << "M = " << endl << m << endl;
	cout << "M.transpose() = " << endl << m.transpose() << endl;
	cout << "M.minor(2,3) = " << endl << m.minor(2, 3) << endl;
	cout << "M.minor(0,1) = " << endl << m.minor(0, 1) << endl;
	cout << "M.determinant() = " << endl << m.determinant() << endl << endl;
	cout << "M.inverse() = " << endl << m.inverse() << endl;
	cout << "M*M^-1 = " << endl << m.mul(m.inverse()) << endl;
	
	int n;
	cin >> n;

	return 0;
}