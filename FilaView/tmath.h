#ifndef __TMATH_INC__
#define __TMATH_INC__

#include "tvec.h"
#include <algorithm>


namespace tg {

template <typename T, int n>
static inline T length(const vecN<T, n> &v)
{
  double result = 0;
  for (int i = 0; i < n; ++i) {
    const T &t = v[i];
    result += t * t;
  }
  return (T)sqrt(result);
}

template <typename T, int n>
static inline T square(const vecN<T, n> &v)
{
  T result(0);
  for (int i = 0; i < n; ++i) {
    const T &t = v[i];
    result += t * t;
  }
  return result;
}

template <typename T, int n>
static inline vecN<T, n> normalize(const vecN<T, n> &v)
{
  return v / length(v);
}

template <typename T, int n>
static inline T distance(const vecN<T, n> &a, const vecN<T, n> &b)
{
  return length(b - a);
}

template <typename T, int n>
static inline T angle(const vecN<T, n> &a, const vecN<T, n> &b)
{
  return arccos(dot(a, b));
}

template <typename T, int n>
static inline vecN<T, n> abs(const vecN<T, n> &a)
{
  vecN<T, n> result;
  for (int i = 0; i < n; i++) {
    result[i] = fabs(a[i]);
  }
  return result;
}

template <typename T, int n>
inline bool operator==(const vecN<T, n> &v1, const vecN<T, n> &v2)
{
  for (int i = 0; i < n; i++) {
    if (fabs(v1[i] - v2[i]) > teps<T>::eps)
      return false;
  }
  return true;
}

template <typename T>
static inline Tquat<T> normalize(const Tquat<T> &q)
{
  return q / length(vecN<T, 4>(q));
}

template <typename T>
inline Tquat<T> quaternion(const T &rad, const Tvec3<T> &axis)
{
  return Tquat<T>(Tvec4<T>(axis * sin(rad / 2.0), cos(rad / 2.0)));
}

template <typename T>
inline Tquat<T> quaternion(const T &rad, const T &x, const T &y, const T &z)
{
  return rotate(rad, Tvec3<T>(x, y, z));
}

template <typename T>
Tquat<T> quaternion(const Tmat3<T> &mat)
{
  Tvec4<T> tq, q;
  tq[0] = 1 + mat[0][0] + mat[1][1] + mat[2][2];
  tq[1] = 1 + mat[0][0] - mat[1][1] - mat[2][2];
  tq[2] = 1 - mat[0][0] + mat[1][1] - mat[2][2];
  tq[3] = 1 - mat[0][0] - mat[1][1] + mat[2][2];
  int i = 0, j = 0;
  for (i = 1; i < 4; i++)
    j = (tq[i] > tq[j]) ? i : j;
  if (j == 0) {
    q.w() = tq[0];
    q.x() = mat[1][2] - mat[2][1];
    q.y() = mat[2][0] - mat[0][2];
    q.z() = mat[0][1] - mat[1][0];
  } else if (j == 1) {
    q.w() = mat[1][2] - mat[2][1];
    q.x() = tq[1];
    q.y() = mat[0][1] + mat[1][0];
    q.z() = mat[2][0] + mat[0][2];
  } else if (j == 2) {
    q.w() = mat[2][0] - mat[0][2];
    q.x() = mat[0][1] + mat[1][0];
    q.y() = tq[2];
    q.z() = mat[1][2] + mat[2][1];
  } else /* if (j==3) */ {
    q.w() = mat[0][1] - mat[1][0];
    q.x() = mat[2][0] + mat[0][2];
    q.y() = mat[1][2] + mat[2][1];
    q.z() = tq[3];
  }

  T s = sqrt(0.25 / tq[j]);
  q *= s;
  return Tquat<T>(q);
}

template <typename T, int m, int n, int u>
inline matNM<T, m, u> operator*(const matNM<T, m, n> &lhs, const matNM<T, n, u> &rhs)
{
  matNM<T, m, u> result(0);
  for (uint32_t i = 0; i < m; i++) {
    for (uint32_t j = 0; j < u; j++) {
      T sum = 0;
      const vecN<T, m> &v = rhs[j];
      for (uint32_t k = 0; k < n; k++)
        sum += lhs[k][i] * v[k];
      result[j][i] = sum;
    }
  }
  return result;
}

template <typename T, const int m, const int n>
inline vecN<T, n> operator*(const vecN<T, m> &vec, const matNM<T, m, n> &mat)
{
  vecN<T, n> result(T(0));
  for (int i = 0; i < n; i++) {
    T sum = 0;
    for (int j = 0; j < m; j++) {
      sum += vec[j] * mat[i][j];
    }
    result[i] = sum;
  }
  return result;
}

template <typename T, typename U, const int m, const int n>
inline vecN<T, m> operator*(const matNM<T, m, n> &mat, const vecN<U, n> &vec)
{
  vecN<T, m> result(T(0));
  for (int i = 0; i < m; i++) {
    T sum = 0;
    for (int j = 0; j < n; j++) {
      sum += vec[j] * mat[j][i];
    }
    result[i] = sum;
  }
  return result;
}

template <typename T, typename U>
inline Tvec3<T> operator*(const matNM<T, 4, 4> &mat, const Tvec3<U> &vec)
{
  Tvec4<T> tmp(vec, T(1));
  vecN<T, 4> ret = operator* <T, U, 4, 4>(mat, tmp);
  return Tvec3<T>(ret[0] / ret[3], ret[1] / ret[3], ret[2] / ret[3]);
}

template <typename T, const int n>
inline vecN<T, n> operator/(const T s, const vecN<T, n> &v)
{
  vecN<T, n> result;

  for (int i = 0; i < n; i++) {
    result[i] = s / v[i];
  }

  return result;
}

template <typename T, int n>
bool inverse(matNM<T, n, n> &des, const matNM<T, n, n> &ori)
{
  constexpr int width = 2 * n;
  T mat[n][width];
  memset(&mat, 0, sizeof(T) * n * width);
  T *cidx[n];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      mat[i][j] = ori[j][i];
    }
    mat[i][n + i] = 1;
    cidx[i] = (T *)&mat[i];
  }
  for (int i = 0; i < n; i++) {
    if (fabs(cidx[i][i]) < teps<T>::eps) {
      int k = i + 1;
      while (k < n) {
        if (fabs(cidx[k][i]) > teps<T>::eps)
          break;
        k++;
      }
      if (k == n)
        return false;
      else {
        T *tmp = cidx[i];
        cidx[i] = cidx[k];
        cidx[k] = tmp;
      }
    }
    T tmp = cidx[i][i];
    for (int j = 0; j < n; j++) {
      if (j == i)
        continue;
      T temp = cidx[j][i];
      for (int k = 0; k < width; k++) {
        cidx[j][k] = cidx[j][k] * tmp - temp * cidx[i][k];
      }
    }
  }
  for (int i = 0; i < n; i++) {
    for (int j = n; j < width; j++) {
      cidx[i][j] /= cidx[i][i];
    }
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      des[j][i] = cidx[i][j + n];
    }
  }
  return des;
}

//----------------------------------------------------------------------------------------------

inline mat4d frustum(double l, double r, double b, double t, double n, double f)
{
  double A = (2.0 * n) / (r - l);
  double B = (2.0 * n) / (t - b);
  double C = (r + l) / (r - l);
  double D = (t + b) / (t - b);

#if defined(DEPTH_REVERSE) && defined(ZERO_NEAR)
  double E = 0.0f;
  double F = n;
#elif defined(DEPTH_REVERSE) && !defined(ZERO_NEAR)
  double E = n / (f - n);
  double F = (f * n) / (f - n);
#elif !defined(DEPTH_REVERSE) && defined(ZERO_NEAR)
  double E = -1.0f;
  double F = -2.0f * n;
#else
  double E = -(f + n) / (f - n);
  double F = -(2.0f * f * n) / (f - n);
#endif

  mat4d m;
  m[0] = vec4d(A, 0, 0, 0);
  m[1] = vec4d(0, B, 0, 0);
  m[2] = vec4d(C, D, E, -1);
  m[3] = vec4d(0, 0, F, 0);
  return m;
}

inline bool get_frustum(const tg::mat4d &m, double &l, double &r, double &b, double &t, double &n, double &f)
{
  if (m[0][3] != 0.0 || m[1][3] != 0.0 || m[2][3] != -1.0 || m[3][3] != 0.0)
    return false;

#if DEPTH_REVERSE && ZERO_NEAR
  n = 0.0;
  f = m[3][2] / m[2][2];
#elif DEPTH_REVERSE && !ZERO_NEAR
  n = m[3][2] / m[2][2];
  f = m[3][2] / (m[2][2] + 1.0f);
#elif !DEPTH_REVERSE && ZERO_NEAR
  n = 0.0;
  f = m[3][2] / (m[2][2] + 1.0);
#else
  n = m[3][2] / (m[2][2] - 1.0);
  f = m[3][2] / (1.0 + m[2][2]);
#endif

#if defined(DEPTH_REVERSE) && defined(ZERO_NEAR)
  double ref_depth = f;
#else
  double ref_depth = n;
#endif

  l = ref_depth * (m[2][0] - 1.0f) / m[0][0];
  r = ref_depth * (1.0f + m[2][0]) / m[0][0];
  t = ref_depth * (1.0f + m[2][1]) / m[1][1];
  b = ref_depth * (m[2][1] - 1.0f) / m[1][1];
  return true;
}

// aspect = width/height
inline mat4d perspective(double fovy, double aspect, double n, double f)
{
  double q = 1.0 / tan(radians(0.5 * fovy));
  double A = q / aspect;
#if DEPTH_REVERSE && ZERO_NEAR
  double B = 0;
  double C = n;
#elif DEPTH_REVERSE && !ZERO_NEAR
  double B = n / (f - n);
  double C = (n * f) / (f - n);
#elif !DEPTH_REVERSE && ZERO_NEAR
  double B = -1.0;
  double C = -2.0 * n;
#else
  double B = (f + n) / (n - f);
  double C = (2.0 * f * n) / (n - f);
#endif

  mat4d m;
  m[0] = vec4d(A, 0.0f, 0.0f, 0.0f);
  m[1] = vec4d(0.0f, q, 0.0f, 0.0f);
  m[2] = vec4d(0.0f, 0.0f, B, -1.0f);
  m[3] = vec4d(0.0f, 0.0f, C, 0.0f);
  return m;
}

inline bool get_perspective(const tg::mat4d &m, double &fovy, double &aspect, double &n, double &f)
{
  double l, r, b, t;
  if (!get_frustum(m, l, r, b, t, n, f))
    return false;

  fovy = 2.0 * atan((t - b) / (2.0 * n)) * (180.0 / M_PI);
  aspect = (r - l) / (t - b);
  return true;
}

inline mat4d ortho(double l, double r, double b, double t, double n, double f)
{
  mat4d m;
  m[0] = vec4d(2.0 / (r - l), 0.0, 0.0, 0.0);
  m[1] = vec4d(0.0, 2.0 / (t - b), 0.0, 0.0);
#ifdef ZERO_NEAR
  m[2] = vec4d(0.0, 0.0, 1.0 / (n - f), 0.0);
  m[3] = vec4d((l + r) / (l - r), (b + t) / (b - t), n / (n - f), 1.0);
#else
  m[2] = vec4d(0.0, 0.0, 2.0 / (n - f), 0.0);
  m[3] = vec4d((l + r) / (l - r), (b + t) / (b - t), (n + f) / (n - f), 1.0);
#endif
  return m;
}

// reverse///////////////////////////////////////////////////////////////////////
inline void view_planes(const mat4d &transmat, vec4d &l, vec4d &r, vec4d &b, vec4d &t, vec4d &n, vec4d &f)
{
  // mi represent ith row of transmat;
  //  left = m4 + m1
  l = vec4d(transmat[0][0] + transmat[0][3], transmat[1][0] + transmat[1][3], transmat[2][0] + transmat[2][3], transmat[3][0] + transmat[3][3]);
  // right = m4 - m1
  r = vec4d(transmat[0][3] - transmat[0][0], transmat[1][3] - transmat[1][0], transmat[2][3] - transmat[2][0], transmat[3][3] - transmat[3][0]);
  // bottom = m2 + m4
  b = vec4d(transmat[0][1] + transmat[0][3], transmat[1][1] + transmat[1][3], transmat[2][1] + transmat[2][3], transmat[3][1] + transmat[3][3]);
  // top = m4 - m2
  t = vec4d(transmat[0][3] - transmat[0][1], transmat[1][3] - transmat[1][1], transmat[2][3] - transmat[2][1], transmat[3][3] - transmat[3][1]);
  // near = m3 + m4
  n = vec4d(transmat[0][2] + transmat[0][3], transmat[1][2] + transmat[1][3], transmat[2][2] + transmat[2][3], transmat[3][2] + transmat[3][3]);
  // far = m4 - m3
  f = vec4d(transmat[0][3] - transmat[0][2], transmat[1][3] - transmat[1][2], transmat[2][3] - transmat[2][2], transmat[3][3] - transmat[3][2]);
}

// frustum space clip
inline void near_clip(mat4 &prjmat, const vec4 &clip_plane)
{
  auto sgn = [](float x) -> float {
    if (x > 0)
      return 1.f;
    else if (x < 0)
      return -1.f;
    return 0.f;
  };

  vec4 q;
  q[0] = (sgn(clip_plane[0]) + prjmat[2][0]) / prjmat[0][0];
  q[1] = (sgn(clip_plane[1]) + prjmat[2][1]) / prjmat[1][1];
  q[2] = -1;
  q[3] = (1 + prjmat[2][2]) / prjmat[3][2];

  q = clip_plane * (2.f / dot<float>(clip_plane, q));

  prjmat[0][2] = q[0];
  prjmat[1][2] = q[1];
  prjmat[2][2] = q[2] + 1;
  prjmat[3][2] = q[3];
}

template <typename T>
inline Tmat3<T> translate(T x, T y)
{
  return Tmat3<T>(Tvec3(1.f, 0.f, 0.f), Tvec3(0.f, 1.f, 0.f), Tvec3(x, y, 1.f));
}

template <typename T>
inline Tmat3<T> translate(const Tvec2<T> &v)
{
  return translate(v[0], v[1]);
}

template <typename T>
inline Tmat4<T> translate(T x, T y, T z)
{
  return Tmat4<T>(Tvec4<T>(T(1), T(0), T(0), T(0)), Tvec4<T>(T(0), T(1), T(0), T(0)), Tvec4<T>(T(0), T(0), T(1), T(0)), Tvec4<T>(x, y, z, T(1)));
}

template <typename T>
inline Tmat4<T> translate(const Tvec3<T> &v)
{
  return translate(v[0], v[1], v[2]);
}

template <typename T>
inline Tmat4<T> lookat(const Tvec3<T> &eye, const Tvec3<T> &center = Tvec3<T>(0), const Tvec3<T> &up = Tvec3<T>(0, 0, 1))
{
  const Tvec3<T> f = normalize(center - eye);
  const Tvec3<T> s = normalize(cross(f, up));
  const Tvec3<T> u = normalize(cross(s, f));
  const Tmat4<T> M =
    Tmat4<T>(Tvec4<T>(s[0], u[0], -f[0], T(0)), Tvec4<T>(s[1], u[1], -f[1], T(0)), Tvec4<T>(s[2], u[2], -f[2], T(0)), Tvec4<T>(T(0), T(0), T(0), T(1)));

  return M * translate<T>(-eye);
}

template <typename T>
inline Tmat4<T> scale(T x, T y, T z)
{
  return Tmat4<T>(Tvec4<T>(x, 0.0f, 0.0f, 0.0f), Tvec4<T>(0.0f, y, 0.0f, 0.0f), Tvec4<T>(0.0f, 0.0f, z, 0.0f), Tvec4<T>(0.0f, 0.0f, 0.0f, 1.0f));
}

template <typename T>
inline Tmat4<T> scale(const Tvec3<T> &v)
{
  return scale(v[0], v[1], v[2]);
}

template <typename T>
inline Tmat4<T> scale(T x)
{
  return Tmat4<T>(Tvec4<T>(x, 0.0f, 0.0f, 0.0f), Tvec4<T>(0.0f, x, 0.0f, 0.0f), Tvec4<T>(0.0f, 0.0f, x, 0.0f), Tvec4<T>(0.0f, 0.0f, 0.0f, 1.0f));
}

template <typename T>
inline Tmat4<T> rotate(T rads, T x, T y, T z)
{
  Tmat4<T> result;

  const T x2 = x * x;
  const T y2 = y * y;
  const T z2 = z * z;
  const double c = cos(rads);
  const double s = sin(rads);
  const double omc = 1.0f - c;

  result[0] = Tvec4<T>(T(x2 * omc + c), T(y * x * omc + z * s), T(x * z * omc - y * s), T(0));
  result[1] = Tvec4<T>(T(x * y * omc - z * s), T(y2 * omc + c), T(y * z * omc + x * s), T(0));
  result[2] = Tvec4<T>(T(x * z * omc + y * s), T(y * z * omc - x * s), T(z2 * omc + c), T(0));
  result[3] = Tvec4<T>(T(0), T(0), T(0), T(1));

  return result;
}

template <typename T>
inline Tmat4<T> rotate(T angle, const vecN<T, 3> &v)
{
  return rotate<T>(angle, v[0], v[1], v[2]);
}

template <typename T>
inline Tmat4<T> rotate(T angle_x, T angle_y, T angle_z)
{
  return rotate(angle_z, 0.0f, 0.0f, 1.0f) * rotate(angle_y, 0.0f, 1.0f, 0.0f) * rotate(angle_x, 1.0f, 0.0f, 0.0f);
}

template <typename T, int n>
inline vecN<T, n> min(const vecN<T, n> &x, const vecN<T, n> &y)
{
  vecN<T, n> t;
  for (int i = 0; i < n; i++) {
    t[i] = std::min(x[i], y[i]);
  }
  return t;
}

template <typename T>
inline T clamp(T t, T min = 0, T max = 1)
{
  return t > max ? max : t < min ? min : t;
}

template <typename T, const int n>
inline vecN<T, n> max(const vecN<T, n> &x, const vecN<T, n> &y)
{
  vecN<T, n> t;
  for (int i = 0; i < n; i++) {
    t[i] = std::max<T>(x[i], y[i]);
  }
  return t;
}

template <typename T, const int n>
inline vecN<T, n> clamp(const vecN<T, n> &x, const vecN<T, n> &minv, const vecN<T, n> &maxv)
{
  return std::min<T>(std::max<T>(x, minv), maxv);
}

template <typename T, const int n>
inline vecN<T, n> smoothstep(const vecN<T, n> &edge0, const vecN<T, n> &edge1, const vecN<T, n> &x)
{
  vecN<T, n> t;
  t = clamp((x - edge0) / (edge1 - edge0), vecN<T, n>(T(0)), vecN<T, n>(T(1)));
  return t * t * (vecN<T, n>(T(3)) - vecN<T, n>(T(2)) * t);
}

template <typename T, const int n>
inline vecN<T, n> reflect(const vecN<T, n> &vi, const vecN<T, n> &vn)
{
  return vi - 2 * dot(vn, vi) * vn;
}

template <typename T, const int n>
inline vecN<T, n> refract(const vecN<T, n> &vi, const vecN<T, n> &vn, T eta)
{
  T d = dot(vn, vi);
  T k = T(1) - eta * eta * (T(1) - d * d);
  if (k < 0.0) {
    return vecN<T, n>(0);
  } else {
    return eta * vi - (eta * d + sqrt(k)) * vn;
  }
}

template <typename T>
inline T mix(const T &a, const T &b, typename T::ele_type c)
{
  return b + c * (b - a);
}

template <typename T>
inline T mix(const T &a, const T &b, const T &t)
{
  return b + t * (b - a);
}

inline void frisvad_tangent(const vec3 &n, vec3 &t, vec3 &b)
{
  if (n.y() < -1.f + teps<float>::eps) {
    t = vec3(0, -1, 0);
    b = vec3(-1, 0, 0);
    return;
  }

  const float a = 1.f / (1.f + n.z());
  const float c = -n.x() * n.y() * a;
  t = vec3(c, -n.z(), 1.f - n.y() * n.y() * a);
  b = vec3(1.f - n.x() * n.x() * a, -n.x(), c);
}

template <typename T>
class Tboundingbox
{
public:
  Tboundingbox() : _min(std::numeric_limits<T>::max()), _max(-_min) {}

  Tboundingbox(const Tvec3<T> &min, const Tvec3<T> &max) : _min(min), _max(max) {}

  Tvec3<T> &min() { return _min; }
  const Tvec3<T> &min() const { return _min; }

  Tvec3<T> &max() { return _max; }
  const Tvec3<T> &max() const { return _max; }

  Tvec3<T> center() const { return _min / 2.0 + _max / 2.0; }

  template <typename U>
  inline void expand(const Tvec3<U> &v)
  {
    if (v.x() < _min.x())
      _min.x() = v.x();
    if (v.x() > _max.x())
      _max.x() = v.x();
    if (v.y() < _min.y())
      _min.y() = v.y();
    if (v.y() > _max.y())
      _max.y() = v.y();
    if (v.z() < _min.z())
      _min.z() = v.z();
    if (v.z() > _max.z())
      _max.z() = v.z();
  }

  inline const Tvec3<T> corner(uint32_t pos) const
  {
    return Tvec3<T>(pos & 1 ? _max.x() : _min.x(), pos & 2 ? _max.y() : _min.y(), pos & 4 ? _max.z() : _min.z());
  }

  inline T radius() const { return static_cast<T>(length(_max - _min) * 0.5); }

  inline bool valid() const { return _max.x() > _min.x() && _max.y() > _min.y() && _max.z() > _min.z(); }

private:
  Tvec3<T> _min, _max;
};

using boundingbox = Tboundingbox<float>;

}; // namespace tg

#endif /* __TMATH_H__ */
