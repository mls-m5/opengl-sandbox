/*
 * vec.h
 *
 *      Author: Mattias Larsson Sköld
 */

#ifndef __Vec__
#define __Vec__

#include <math.h>
//#include <LinearMath/btVector3.h>
#include "common.h" //for PI

class Vec
{
	public:
		double x, y, z;

		Vec (): x(0), y(0), z(0) {};
		Vec (double nx, double ny, double nz):x(nx), y(ny), z(nz) {}
		Vec (double nx, double ny): x(nx), y(ny), z(0) {}
		Vec (const Vec& v): x(v.x), y(v.y), z(v.z) {}
		template <class pointerType>
		Vec (const pointerType *p) {
			x = p[0]; y = p[1]; z = p[2];
		}

		template <class pointerType>
		Vec& operator= (const pointerType *p) {
			x = p[0]; y = p[1]; z = p[2];
			return *this;
		}


		Vec &operator+=(Vec v){
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		Vec &operator-=(Vec v){
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}


		Vec &operator*=(double t){
			x *= t;
			y *= t;
			z *= t;
			return *this;
		}

		Vec &operator/=(double t){
			x /= t;
			y /= t;
			z /= t;
			return *this;
		}

		Vec &scale(double x, double y, double z) {
			this->x /= x;
			this->y /= y;
			this->z /= z;
			return *this;
		}

		Vec operator*(double t) const {
			return Vec(x *t, y*t, z*t);
		}
		Vec operator/(double t) const {
			return Vec(x/t, y/t, z/t);
		}

		double operator *(Vec v2) const {
			return x * v2.x +
				y * v2.y +
				z * v2.z;
		}

		Vec operator -(Vec v) const {
			return Vec(
				x - v.x,
				y - v.y,
				z - v.z);
		}

		Vec operator +(Vec v) const {
			return Vec(
				x + v.x,
				y + v.y,
				z + v.z);
		}

		inline double &operator[] (int index) {
			return (&x)[index];
		}

		inline double operator[] (int index) const {
			return (&x)[index];
		}

		inline void operator() (double x, double y, double z = 0) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		bool operator== (Vec v) const {
			return x == v.x && y == v.y && z == v.z;
		}

		double abs() const {
			return sqrt(x*x + y*y + z*z);
		}

		double abs2() const {
			return x*x + y*y + z*z;
		}

		Vec &normalize(){
			* this /= abs();
			return *this;
		}

		Vec cross(Vec v){
			return Vec(
				y * v.z - z * v.y,
				z * v.x - x * v.z,
				x * v.y - y * v.x
			);
		}

		double angle(double a) const {
			double angle = atan2(x,y) + a;

			redo1:
			if (angle < pi){
				angle += pi2;
				goto redo1;
			}

			redo2:
			if (angle > pi){
				angle -= pi2;
				goto redo2;
			}
			return angle;
		}

		double angle() const {
			return atan2(x,y);
		}
};

template <class T>
T &operator <<(T &out,const Vec &v) {
	out << v.x << ", " << v.y << ", " << v.z;
	return out;
}


template <class T>
inline Vec operator *(T f, const Vec &v) {
	return v * f;
}


class MapVec{
public:
	MapVec() {}
	MapVec(int nx, int ny, double z = 0): x(nx), y(ny), z(z){}
	MapVec(const Vec &v, int size = 1.){
		conv(v, size);
	}
	int x = 0, y = 0;
	double z = 0;

	void conv(Vec v, int size = 1.){
		x = v.x - (double) size / 2.;
		y = v.y - (double) size / 2.;
		z = v.z;
	}

	inline Vec toVec(int size = 1.) const {
		return Vec( (double) x + (double) size / 2.,
					(double) y + (double) size / 2., z);
	}

	inline MapVec &operator () (int x, int y, double z = 0) {
		this->x = x;
		this->y = y;
		this->z = z;
		return *this;
	}

	//Return the manhatan/taxicab-distance
	int l1Norm() {
		return std::abs(x) + std::abs(y);
	}

	int diagonalDistance() {
		auto absx = std::abs(x);
		auto absy = std::abs(y);
		bool xIsMore = absx > absy;
		return xIsMore * absx + (!xIsMore) * absy;
	}

	float abs() {
		return sqrt((double)abs2());
	}

	int abs2() {
		return x * x + y * y;
	}

//		inline operator Vec() const {
//			return toVec(1);
//		}

	MapVec &operator+=(MapVec v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	MapVec &operator-=(MapVec v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	MapVec operator+(MapVec v) const {
		return MapVec(
			x + v.x,
			y + v.y);
	}

	MapVec operator-(MapVec v) const {
		return MapVec(
			x - v.x,
			y - v.y);
	}
	bool operator ==(MapVec v) const {
		if (v.x == x && v.y == y){
			return true;
		}
		return false;
	}
	bool operator !=(MapVec v) const {
		if (v.x == x && v.y == y){
			return false;
		}
		return true;
	}

	operator bool() const {
		return x != 0 || y != 0;
	}
};

inline double abs(const Vec& v) {
	return v.abs();
}

template <class Ar>
void serialize (Ar ar, Vec& v){
	ar & v.x;
	ar & v.y;
	ar & v.z;
}

#endif
