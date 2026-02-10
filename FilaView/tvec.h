#ifndef __TVEC_INC__
#define __TVEC_INC__

#ifndef NOMINMAX
  #define NOMINMAX 1
#endif

namespace tg {
// template <typename T, const int w, const int h> class matNM;
// template <typename T, const int n> class vecN;
// template <typename T> class Tquat;

#ifndef _USE_MATH_DEFINES
  #define _USE_MATH_DEFINES
  #define M_E 2.71828182845904523536       // e
  #define M_LOG2E 1.44269504088896340736   // log2(e)
  #define M_LOG10E 0.434294481903251827651 // log10(e)
  #define M_LN2 0.693147180559945309417    // ln(2)
  #define M_LN10 2.30258509299404568402    // ln(10)
  #ifndef M_PI
    #define M_PI 3.14159265358979323846 // pi
  #endif
  #define M_PI_2 1.57079632679489661923     // pi/2
  #define M_PI_4 0.785398163397448309616    // pi/4
  #define M_1_PI 0.318309886183790671538    // 1/pi
  #define M_2_PI 0.636619772367581343076    // 2/pi
  #define M_2_SQRTPI 1.12837916709551257390 // 2/sqrt(pi)
  #define M_SQRT2 1.41421356237309504880    // sqrt(2)
  #define M_SQRT1_2 0.707106781186547524401 // 1/sqrt(2)
#endif

template <typename T>
struct teps
{
  static constexpr T eps = 0;
};

template <>
struct teps<float>
{
  static constexpr float eps = float(1e-6);
};

template <>
struct teps<double>
{
  static constexpr double eps = 1e-15;
};

template <typename T>
inline T degrees(T angleInRadians)
{
  return angleInRadians * static_cast<T>(180.0 / M_PI);
}

template <typename T>
inline T radians(T angleInDegrees)
{
  return angleInDegrees * static_cast<T>(M_PI / 180.0);
}

template <typename T, int n>
class vecN
{
public:
  using type = vecN<T, n>;

  inline vecN() {}

  explicit inline vecN(const type &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] = that[i];
  }

  explicit inline vecN(T s) { set(s); }

  template <typename U, int m>
  vecN(const vecN<U, m> &that)
  {
    constexpr int s = n < m ? n : m;
    for (int i = 0; i < s; i++)
      data_[i] = that[i];
  }

  template <typename U>
  inline void set(const U *u)
  {
    for (int i = 0; i < n; i++)
      data_[i] = static_cast<T>(u[i]);
  }

  inline void set(T t)
  {
    for (int i = 0; i < n; i++)
      data_[i] = t;
  }

  inline vecN<T, n> &operator=(const vecN &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] = that.data_[i];
    return *this;
  }

  inline vecN<T, n> &operator=(const T &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] = that;
    return *this;
  }

  template <typename U, const int m>
  inline vecN<T, n> &operator=(const vecN<U, m> &that)
  {
    constexpr int sz = n < m ? n : m;
    for (int i = 0; i < sz; i++)
      data_[i] = that[i];
    return *this;
  }

  inline vecN operator+(const vecN &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] + that.data_[i];
    return result;
  }

  inline vecN &operator+=(const vecN &that) { return (*this = *this + that); }

  inline vecN operator-() const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = -data_[i];
    return result;
  }

  inline vecN operator-(const vecN &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] - that.data_[i];
    return result;
  }

  inline vecN &operator-=(const vecN &that) { return (*this = *this - that); }

  inline vecN operator*(const vecN &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] * that.data_[i];
    return result;
  }

  inline vecN &operator*=(const vecN &that) { return (*this = *this * that); }

  inline vecN operator*(const T &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] * that;
    return result;
  }

  inline vecN &operator*=(const T &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] *= that;
    return *this;
  }

  inline vecN operator/(const vecN &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] / that.data_[i];
    return result;
  }

  inline vecN operator/(const T &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] / that;
    return result;
  }

  inline vecN &operator/=(const T &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] /= that;
    return *this;
  }

  inline vecN &operator/=(const vecN &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] /= that.data_[i];
    return *this;
  }

  inline T &operator[](int i) { return data_[i]; }
  inline const T &operator[](int i) const { return data_[i]; }

  inline T *data() { return static_cast<T *>(data_); }
  inline const T *data() const { return static_cast<const T *>(data_); }

  inline static int size(void) { return n; }

protected:
  T data_[n] = {};
};

template <typename T>
class Tvec2 : public vecN<T, 2>
{
public:
  typedef vecN<T, 2> base;
  typedef Tvec2<T> type;

  inline Tvec2() {}

  explicit inline Tvec2(const type &v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
  }

  explicit inline Tvec2(const base &v) : base(v) {}

  inline Tvec2(T x, T y)
  {
    base::data_[0] = x;
    base::data_[1] = y;
  }

  template <typename U>
  inline Tvec2(const vecN<U, 2> &that) : base(that)
  {
  }

  template <typename U, int n>
  inline type operator=(const vecN<U, n> &that)
  {
    base::operator=(that);
    return *this;
  }

  inline void operator=(const T &t)
  {
    base::data_[0] = t;
    base::data_[1] = t;
  }

  inline T &x() { return base::data_[0]; }
  inline T &y() { return base::data_[1]; }

  inline const T &x() const { return base::data_[0]; }
  inline const T &y() const { return base::data_[1]; }
};

template <typename T>
class Tvec3 : public vecN<T, 3>
{
public:
  using base = vecN<T, 3>;
  using type = Tvec3<T>;

  inline Tvec3() : base(0) {}

  explicit inline Tvec3(T t) : base(t) {}

  explicit inline Tvec3(const type &v) : base(v) {}

  inline Tvec3(const base &v) : base(v) {}

  inline Tvec3(T x, T y, T z) : base()
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
  }

  inline Tvec3(const Tvec2<T> &v, T z) : base()
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = z;
  }

  inline Tvec3(T x, const Tvec2<T> &v) : base()
  {
    base::data_[0] = x;
    base::data_[1] = v[0];
    base::data_[2] = v[1];
  }

  inline Tvec3(const vecN<T, 4> &v) : base()
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
  }

  inline type operator=(const T &t)
  {
    base::data_[0] = t;
    base::data_[1] = t;
    base::data_[2] = t;
    return *this;
  }

  template <typename U>
  inline Tvec3(const U *ptr)
  {
    base::set(ptr);
  }

  template <typename U>
  inline Tvec3(const vecN<U, 3> &that) : base(that)
  {
  }

  template <typename U, int n>
  inline type operator=(vecN<U, n> vec)
  {
    base::operator=(vec);
    return *this;
  }

  inline T &x() { return base::data_[0]; }
  inline T &y() { return base::data_[1]; }
  inline T &z() { return base::data_[2]; }

  inline const T &x() const { return base::data_[0]; }
  inline const T &y() const { return base::data_[1]; }
  inline const T &z() const { return base::data_[2]; }

  inline void set(const T &x, const T &y, const T &z)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
  }
};

template <typename T>
class Tvec4 : public vecN<T, 4>
{
public:
  typedef vecN<T, 4> base;
  typedef Tvec4<T> type;

  inline Tvec4() {}

  explicit inline Tvec4(const type &v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = v[3];
  }

  template <typename U>
  inline Tvec4(const Tvec4<U> &that) : base(that)
  {
  }

  template <typename U>
  inline Tvec4(const U *ptr)
  {
    set(ptr);
  }

  inline Tvec4(T x, T y, T z, T w)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
    base::data_[3] = w;
  }

  inline Tvec4(const Tvec2<T> &v, T z, T w)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = z;
    base::data_[3] = w;
  }

  inline Tvec4(T x, const Tvec2<T> &v, T w)
  {
    base::data_[0] = x;
    base::data_[1] = v[0];
    base::data_[2] = v[1];
    base::data_[3] = w;
  }

  inline Tvec4(T x, T y, const Tvec2<T> &v)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = v[0];
    base::data_[3] = v[1];
  }

  inline Tvec4(const Tvec2<T> &u, const Tvec2<T> &v)
  {
    base::data_[0] = u[0];
    base::data_[1] = u[1];
    base::data_[2] = v[0];
    base::data_[3] = v[1];
  }

  inline Tvec4(const Tvec3<T> &v, T w)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = w;
  }

  inline Tvec4(T x, const Tvec3<T> &v)
  {
    base::data_[0] = x;
    base::data_[1] = v[0];
    base::data_[2] = v[1];
    base::data_[3] = v[2];
  }

  explicit inline Tvec4(const Tvec3<T> &v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = 0;
  }

  template <typename U>
  inline Tvec4(const vecN<U, 4> &that) : base(that)
  {
  }

  template <typename U, int n>
  inline type operator=(vecN<U, n> vec)
  {
    base::operator=(vec);
    return *this;
  }

  inline type &operator=(const type &v)
  {
    base::data_[0] = v[0];
    base::data_[1] = v[1];
    base::data_[2] = v[2];
    base::data_[3] = v[3];
    return *this;
  }

  inline void operator=(const T &t)
  {
    base::data_[0] = t;
    base::data_[1] = t;
    base::data_[2] = t;
    base::data_[3] = t;
  }

  inline operator Tvec3<T>() { return Tvec3<T>(base::data_[0], base::data_[1], base::data_[2]); }

  inline T &x() { return base::data_[0]; }
  inline T &y() { return base::data_[1]; }
  inline T &z() { return base::data_[2]; }
  inline T &w() { return base::data_[3]; }

  inline const T &x() const { return base::data_[0]; }
  inline const T &y() const { return base::data_[1]; }
  inline const T &z() const { return base::data_[2]; }
  inline const T &w() const { return base::data_[3]; }

  inline void set(const T &x, const T &y, const T &z, const T &w)
  {
    base::data_[0] = x;
    base::data_[1] = y;
    base::data_[2] = z;
    base::data_[3] = w;
  }
};

typedef vecN<float, 1> vec1;
typedef vecN<int, 1> vec1i;
typedef vecN<unsigned int, 1> vec1u;
typedef vecN<double, 1> vec1d;

typedef Tvec2<float> vec2;
typedef Tvec2<double> vec2d;
typedef Tvec2<int> vec2i;
typedef Tvec2<unsigned int> vec2u;

typedef Tvec3<float> vec3;
typedef Tvec3<double> vec3d;
typedef Tvec3<char> vec3b;
typedef Tvec3<unsigned char> vec3ub;
typedef Tvec3<int> vec3i;
typedef Tvec3<unsigned int> vec3u;

typedef Tvec4<float> vec4;
typedef Tvec4<double> vec4d;
typedef Tvec4<char> vec4b;
typedef Tvec4<unsigned char> vec4ub;
typedef Tvec4<int> vec4i;
typedef Tvec4<unsigned int> vec4u;

template <typename T, int n>
static inline const vecN<T, n> operator*(T x, const vecN<T, n> &v)
{
  return v * x;
}

template <typename T>
static inline const Tvec2<T> operator/(T x, const Tvec2<T> &v)
{
  return Tvec2<T>(x / v[0], x / v[1]);
}

template <typename T>
static inline const Tvec3<T> operator/(T x, const Tvec3<T> &v)
{
  return Tvec3<T>(x / v[0], x / v[1], x / v[2]);
}

template <typename T>
static inline const Tvec4<T> operator/(T x, const Tvec4<T> &v)
{
  return Tvec4<T>(x / v[0], x / v[1], x / v[2], x / v[3]);
}

template <typename T, int n>
static inline T dot(const vecN<T, n> &a, const vecN<T, n> &b)
{
  T total(0);
  for (int i = 0; i < n; i++) {
    total += a[i] * b[i];
  }
  return total;
}

template <typename T>
static inline vecN<T, 3> cross(const vecN<T, 3> &a, const vecN<T, 3> &b)
{
  return Tvec3<T>(a[1] * b[2] - b[1] * a[2], a[2] * b[0] - b[2] * a[0], a[0] * b[1] - b[0] * a[1]);
}

// Quaternion///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class Tquat
{
  template <typename T>
  friend class Tmat3;

public:
  inline Tquat() : s_(T(1)), v_(T(0)) {}

  inline Tquat(const Tquat &q) : s_(q.s_), v_(q.v_) {}

  inline Tquat(const Tvec4<T> &v) : s_(v[3]), v_(v[0], v[1], v[2]) {}

  inline Tquat(T x, T y, T z, T w) : s_(w), v_(x, y, z) {}

  inline Tquat &operator=(const Tquat &q)
  {
    s_ = q.s_;
    v_ = q.v_;
    return *this;
  }

  inline T &operator[](int n) { return data_[n]; }

  inline const T &operator[](int n) const { return data_[n]; }

  inline Tquat operator+(const Tquat &q) const { return Tquat(v_ + q.v_, s_ + q.s_); }

  inline Tquat &operator+=(const Tquat &q)
  {
    s_ += q.s_;
    v_ += q.v_;
    return *this;
  }

  inline Tquat operator-(const Tquat &q) const { return Tquat(v_ - q.v_, s_ - q.s_); }

  inline Tquat &operator-=(const Tquat &q)
  {
    s_ -= q.s_;
    v_ -= q.v_;

    return *this;
  }

  inline Tquat operator-() const { return Tquat(-v_, -s_); }

  inline Tquat operator*(const T s) const { return Tquat(data_[0] * s, data_[1] * s, data_[2] * s, data_[3] * s); }

  inline Tquat &operator*=(const T s)
  {
    s_ *= s;
    v_ *= s;
    return *this;
  }

  inline Tquat operator*(const Tquat &q) const
  {
    const T &x1 = data_[0];
    const T &y1 = data_[1];
    const T &z1 = data_[2];
    const T &w1 = data_[3];
    const T &x2 = q.data_[0];
    const T &y2 = q.data_[1];
    const T &z2 = q.data_[2];
    const T &w2 = q.data_[3];

    return Tquat(w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2, w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2, w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2,
                 w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2);
  }

  inline Tvec3<T> operator*(const Tvec3<T> &v) const
  {
    Tvec3<T> uv = cross(v_, v);
    Tvec3<T> uuv = cross(v_, uv);
    uv *= (static_cast<T>(2) * s_);
    uuv *= static_cast<T>(2);
    return v + uv + uuv;
  }

  inline Tquat operator/(const T s) const { return Tquat(data_[0] / s, data_[1] / s, data_[2] / s, data_[3] / s); }

  inline Tquat &operator/=(const T t)
  {
    s_ /= t;
    v_ /= t;
    return *this;
  }

  inline operator Tvec4<T> &() { return *(Tvec4<T> *)data_; }

  inline operator const Tvec4<T> &() const { return *(const Tvec4<T> *)data_; }

  inline operator Tvec3<T> &() { return v_; }

  inline operator const Tvec3<T> &() const { return v_; }

  inline bool operator==(const Tquat &q) const { return (s_ == q.s_) && (v_ == q.v_); }

  inline bool operator!=(const Tquat &q) const { return (s_ != q.s_) || (v_ != q.v_); }

  inline Tquat<T> conjugate() const { return Tquat<T>(Tvec4<T>(-v_, s_)); }

private:
  Tquat(const Tvec3<T> &v, T s) : v_(v), s_(s) {}

  union {
    struct
    {
      Tvec3<T> v_;
      T s_;
    };
    struct
    {
      T x_;
      T y_;
      T z_;
      T w_;
    };
    T data_[4];
  };
};

typedef Tquat<float> quat;
typedef Tquat<int> quati;
typedef Tquat<double> quatd;

template <typename T>
static inline Tquat<T> operator*(T a, const Tquat<T> &b)
{
  return b * a;
}

template <typename T>
static inline Tquat<T> operator/(T a, const Tquat<T> &b)
{
  return Tquat<T>(a / b[0], a / b[1], a / b[2], a / b[3]);
}

template <typename T, int m, int n>
class matNM
{
public:
  using type = matNM<T, n, m>;

  inline matNM() {}

  explicit inline matNM(T f)
  {
    for (int i = 0; i < n; i++) {
      data_[i] = f;
    }
  }

  template <typename U>
  matNM(const matNM<U, m, n> &that)
  {
    for (int i = 0; i < n; i++) {
      data_[i] = that[i];
    }
  }

  template <const int s, const int t>
  matNM(const matNM<T, s, t> &that)
  {
    constexpr int row = m < s ? m : s;
    constexpr int col = n < t ? n : t;
    for (int i = 0; i < n; i++)
      data_[i] = 0;
    for (int i = 0; i < col; i++)
      for (int j = 0; j < row; j++)
        data_[i][j] = that[i][j];
  }

  explicit inline matNM(const vecN<T, m> &v)
  {
    for (int i = 0; i < n; i++)
      data_[i] = v;
  }

  inline matNM &operator=(const type &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] = that[i];
    return *this;
  }

  template <typename U>
  matNM &operator=(const matNM<U, n, m> &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] = that[i];
    return *this;
  }

  inline matNM operator+(const type &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] + that.data_[i];
    return result;
  }

  inline type &operator+=(const type &that) { return (*this = *this + that); }

  inline type operator-(const type &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] - that.data_[i];
    return result;
  }

  inline type &operator-=(const type &that) { return (*this = *this - that); }

  inline type operator*(const T &that) const
  {
    type result;
    for (int i = 0; i < n; i++)
      result.data_[i] = data_[i] * that;
    return result;
  }

  inline type &operator*=(const T &that)
  {
    for (int i = 0; i < n; i++)
      data_[i] = data_[i] * that;
    return *this;
  }

  inline vecN<T, n> &operator[](int i) { return data_[i]; }
  inline const vecN<T, n> &operator[](int i) const { return data_[i]; }
  inline operator T *() { return &data_[0][0]; }
  inline operator const T *() const { return &data_[0][0]; }

  inline matNM<T, n, m> transpose() const
  {
    matNM<T, n, m> result;
    for (int i = 0; i < n; i++)
      for (int j = 0; j < m; j++)
        result.data_[j][i] = data_[i][j];
    return result;
  }

  inline void identity()
  {
    for (int i = 0; i < n; i++) {
      data_[i] = 0;
      data_[i][i] = 1;
    }
  }

  static inline int row(void) { return m; }
  static inline int col(void) { return n; }

  template <typename U>
  inline void set(const U *ele)
  {
    for (int i = 0; i < n; i++) {
      data_[i].set(ele + m * i);
    }
  }

  inline void set(T v)
  {
    for (int i = 0; i < n; i++)
      data_[i].set(v);
  }

protected:
  vecN<T, m> data_[n] = {};
};

template <typename T>
class Tmat2 : public matNM<T, 2, 2>
{
public:
  typedef matNM<T, 2, 2> base;
  typedef Tmat2<T> type;

  inline Tmat2() {}
  inline Tmat2(const type &that) : base(that) {}
  inline Tmat2(const base &that) : base(that) {}
  inline Tmat2(const vecN<T, 2> &v) : base(v) {}
  inline Tmat2(const vecN<T, 2> &v0, const vecN<T, 2> &v1)
  {
    base::data_[0] = v0;
    base::data_[1] = v1;
  }
  template <typename U>
  Tmat2(const matNM<U, 2, 2> &that) : base(that)
  {
  }
  template <typename U>
  type &operator=(const matNM<U, 2, 2> &that)
  {
    base::operator=(that);
    return *this;
  }
};
typedef Tmat2<float> mat2;

template <typename T>
class Tmat3 : public matNM<T, 3, 3>
{
public:
  typedef matNM<T, 3, 3> base;
  typedef Tmat3<T> type;

  inline Tmat3() {}
  inline Tmat3(const type &that) : base(that) {}
  inline Tmat3(const vecN<T, 3> &v) : base(v) {}
  inline Tmat3(const vecN<T, 3> &v0, const vecN<T, 3> &v1, const vecN<T, 3> &v2)
  {
    base::data_[0] = v0;
    base::data_[1] = v1;
    base::data_[2] = v2;
  }
  Tmat3(const Tquat<T> &quat)
  {
    Tvec4<T> v(quat);
    // const T ww = v.w() * v.w();
    const T xx = v.x() * v.x();
    const T yy = v.y() * v.y();
    const T zz = v.z() * v.z();
    const T xy = v.x() * v.y();
    const T xz = v.x() * v.z();
    const T xw = v.x() * v.w();
    const T yz = v.y() * v.z();
    const T yw = v.y() * v.w();
    const T zw = v.z() * v.w();

    auto &m = base::data_;

    m[0][0] = T(1) - T(2) * (yy + zz);
    m[0][1] = T(2) * (xy + zw);
    m[0][2] = T(2) * (xz - yw);

    m[1][0] = T(2) * (xy - zw);
    m[1][1] = T(1) - T(2) * (xx + zz);
    m[1][2] = T(2) * (yz + xw);

    m[2][0] = T(2) * (xz + yw);
    m[2][1] = T(2) * (yz - xw);
    m[2][2] = T(1) - T(2) * (xx + yy);
  }
  template <int m, int n>
  Tmat3(const matNM<T, m, n> &that) : base(that)
  {
  }
  template <typename U>
  type &operator=(const matNM<U, 3, 3> &that)
  {
    base::operator=(that);
    return *this;
  }
};
typedef Tmat3<float> mat3;
typedef Tmat3<int> imat3;
typedef Tmat3<double> dmat3;

template <typename T>
class Tmat4 : public matNM<T, 4, 4>
{
public:
  typedef matNM<T, 4, 4> base;
  typedef Tmat4<T> type;

  inline Tmat4() {}
  inline Tmat4(const type &that) : base(that) {}
  explicit inline Tmat4(const vecN<T, 4> &v) : base(v) {}
  inline Tmat4(const vecN<T, 4> &v0, const vecN<T, 4> &v1, const vecN<T, 4> &v2, const vecN<T, 4> &v3)
  {
    base::data_[0] = v0;
    base::data_[1] = v1;
    base::data_[2] = v2;
    base::data_[3] = v3;
  }
  inline Tmat4(const Tmat3<T> &that) : base(that)
  {
    base::data_[0][3] = T(0);
    base::data_[1][3] = T(0);
    base::data_[2][3] = T(0);

    base::data_[3][0] = T(0);
    base::data_[3][1] = T(0);
    base::data_[3][2] = T(0);
    base::data_[3][3] = T(1);
  }
  template <typename U>
  Tmat4(const matNM<U, 4, 4> &that) : base(that)
  {
  }
  template <typename U>
  type &operator=(const matNM<U, 4, 4> &that)
  {
    base::operator=(that);
    return *this;
  }
};
typedef Tmat4<float> mat4;
typedef Tmat4<int> mat4i;
typedef Tmat4<unsigned int> mat4u;
typedef Tmat4<double> mat4d;

}; // namespace tg

#endif /* __TVEC_INC__ */
